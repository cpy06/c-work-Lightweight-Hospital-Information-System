/* 预测分析模块
 * 基于住院记录和发药记录，给出住院人数、床位需求和药品需求的简单可解释预测结果
 */

#include "system.h"

#if defined(HIS_BUILD_FROM_MAIN) || !HIS_MAIN_ONE_CLICK_BUILD


/* 找到住院数据中的最新日期序列值
 * 如果当前没有历史住院数据，就使用当天日期
 */
static int get_latest_serial_from_admissions(HospitalSystem *system) {
    int latest = 0;
    AdmissionHistory *current = system->admissions;
    while (current != NULL) {
        int admit_serial = date_to_serial(current->admit_date);
        latest = admit_serial > latest ? admit_serial : latest;
        if (strcmp(current->discharge_date, "-") != 0) {
            int discharge_serial = date_to_serial(current->discharge_date);
            latest = discharge_serial > latest ? discharge_serial : latest;
        }
        current = current->next;
    }
    if (latest == 0) {
        char today[DATE_LEN];
        get_current_date(today);
        latest = date_to_serial(today);
    }
    return latest;
}

#define FORECAST_COL_ID 8//ID列宽度
#define FORECAST_COL_NAME 24//名称列宽度
#define DRUG_FORECAST_COL_NAME DRUG_NAME_COL_WIDTH//药品名称列宽度
#define FORECAST_COL_VALUE 14//值列宽度
#define FORECAST_COL_DATE 12//日期列宽度

/* 预测结果表格总宽度 */
static int forecast_table_width(void) {
    return FORECAST_COL_ID + FORECAST_COL_NAME + FORECAST_COL_VALUE + FORECAST_COL_VALUE + 3 * 4 + 1;
}

// 药品预测表格总宽度
static int drug_forecast_table_width(void) {
    return FORECAST_COL_ID + DRUG_FORECAST_COL_NAME + FORECAST_COL_DATE + FORECAST_COL_DATE +
           FORECAST_COL_VALUE + FORECAST_COL_VALUE + 3 * 6 + 1;
}

/* 打印科室预测表头 */
static void print_department_forecast_header(void) {
    char id[32], name[32], admission[32], bed[32];
    print_table_border(forecast_table_width(), '=');
    format_display_cell("科室ID", id, (int)sizeof(id), FORECAST_COL_ID);
    format_display_cell("科室名称", name, (int)sizeof(name), FORECAST_COL_NAME);
    format_display_cell("预计住院人数", admission, (int)sizeof(admission), FORECAST_COL_VALUE);
    format_display_cell("预计床位需求", bed, (int)sizeof(bed), FORECAST_COL_VALUE);
    printf("| %s | %s | %s | %s |\n", id, name, admission, bed);
    print_table_border(forecast_table_width(), '-');
}

/* 打印药品预测表头 */
static void print_drug_forecast_header(void) {
    char id[32], name[DRUG_DISPLAY_NAME_LEN], production[32], expiry[32], stock[32], demand[32];
    print_table_border(drug_forecast_table_width(), '=');
    format_display_cell("药品ID", id, (int)sizeof(id), FORECAST_COL_ID);
    format_display_cell("通用名（俗称/品牌）", name, (int)sizeof(name), DRUG_FORECAST_COL_NAME);
    format_display_cell("生产日期", production, (int)sizeof(production), FORECAST_COL_DATE);
    format_display_cell("有效期至", expiry, (int)sizeof(expiry), FORECAST_COL_DATE);
    format_display_cell("当前库存", stock, (int)sizeof(stock), FORECAST_COL_VALUE);
    format_display_cell("预计需求量", demand, (int)sizeof(demand), FORECAST_COL_VALUE);
    printf("| %s | %s | %s | %s | %s | %s |\n", id, name, production, expiry, stock, demand);
    print_table_border(drug_forecast_table_width(), '-');
}

/* 找到发药历史中的最新日期序列值
 * 如果当前没有历史发药数据，就使用当天日期
 */
static int get_latest_serial_from_usage(HospitalSystem *system) {
    int latest = 0;
    DrugUsageHistory *current = system->drug_usages;
    while (current != NULL) {
        int serial = date_to_serial(current->date);
        latest = serial > latest ? serial : latest;
        current = current->next;
    }
    if (latest == 0) {
        char today[DATE_LEN];
        get_current_date(today);
        latest = date_to_serial(today);
    }
    return latest;
}

/* 统计某个药品在指定窗口内的累计发药数量 */
static int drug_quantity_in_range(HospitalSystem *system, int drug_id, int start_serial, int end_serial) {
    int total = 0;
    DrugUsageHistory *current = system->drug_usages;
    while (current != NULL) {
        int serial = date_to_serial(current->date);
        if (current->drug_id == drug_id && serial >= start_serial && serial <= end_serial) {
            total += current->quantity;
        }
        current = current->next;
    }
    return total;
}

/* 预测某个科室未来窗口期内的住院人数
 * 采用 0.5 / 0.3 / 0.2 的加权移动平均法
 * 让最近一期数据权重最大，兼顾历史趋势
 */
double forecast_department_admission(HospitalSystem *system, int department_id, int window_days) {
    int latest = get_latest_serial_from_admissions(system);
    int c1 = count_department_admissions_in_range(system, department_id, latest - window_days + 1, latest);
    int c2 = count_department_admissions_in_range(system, department_id, latest - 2 * window_days + 1, latest - window_days);
    int c3 = count_department_admissions_in_range(system, department_id, latest - 3 * window_days + 1, latest - 2 * window_days);
    return c1 * 0.5 + c2 * 0.3 + c3 * 0.2;
}

/* 预测某个科室未来床位需求
 * 新增床位需求：预测入院人数乘以平均住院天数，除以天数得到每日平均需要的床位天数，再转换为床位数
 * 保留床位：当前占用床位的 40% 用于维持现有患者的连续治疗
 * 同时考虑未来预计住院量、平均住院天数和当前已占用床位。
 */
double forecast_department_bed_need(HospitalSystem *system, int department_id, int window_days) {
    double predicted = forecast_department_admission(system, department_id, window_days);
    double avg_stay = average_department_stay_days(system, department_id);
    double occupied = (double)count_department_occupied_beds(system, department_id);
    return predicted * avg_stay / window_days + occupied * 0.4;
}

/* 预测某个药品未来窗口期内的需求量
 * 采用 0.5 / 0.3 / 0.2 的加权移动平均法
 * 让最近一期数据权重最大，兼顾历史趋势
 */
double forecast_drug_demand(HospitalSystem *system, int drug_id, int window_days) {
    int latest = get_latest_serial_from_usage(system);
    int q1 = drug_quantity_in_range(system, drug_id, latest - window_days + 1, latest);
    int q2 = drug_quantity_in_range(system, drug_id, latest - 2 * window_days + 1, latest - window_days);
    int q3 = drug_quantity_in_range(system, drug_id, latest - 3 * window_days + 1, latest - 2 * window_days);
    return q1 * 0.5 + q2 * 0.3 + q3 * 0.2;
}

/* 显示全部科室的住院人数和床位需求预测结果 */
void show_department_forecast(HospitalSystem *system) {
    Department *current = system->departments;
    int window_days = read_int_in_range("请输入预测窗口天数(建议7): ", 3, 30);
    int found = 0;
    print_title("科室住院需求预测");
    print_department_forecast_header();
    while (current != NULL) {
        if (current->is_deleted == 0) {
            char raw_id[32], id[32], name[NAME_LEN * 2], admission[32], bed[32];
            snprintf(raw_id, sizeof(raw_id), "%d", current->department_id);
            snprintf(admission, sizeof(admission), "%.2f", forecast_department_admission(system, current->department_id, window_days));
            snprintf(bed, sizeof(bed), "%.2f", forecast_department_bed_need(system, current->department_id, window_days));
            format_display_cell(raw_id, id, (int)sizeof(id), FORECAST_COL_ID);
            format_display_cell(current->name, name, (int)sizeof(name), FORECAST_COL_NAME);
            format_display_cell(admission, admission, (int)sizeof(admission), FORECAST_COL_VALUE);
            format_display_cell(bed, bed, (int)sizeof(bed), FORECAST_COL_VALUE);
            printf("| %s | %s | %s | %s |\n", id, name, admission, bed);
            found = 1;
        }
        current = current->next;
    }
    print_table_border(forecast_table_width(), '=');
    if (!found) {
        printf("当前暂无科室预测数据\n");
    }
}

/* 显示全部药品的短期需求预测结果 */
void show_drug_forecast(HospitalSystem *system) {
    Drug *current = system->drugs;
    int window_days = read_int_in_range("请输入药品预测窗口天数(建议7): ", 3, 30);
    int found = 0;
    print_title("药品短期需求预测");
    print_drug_forecast_header();
    while (current != NULL) {
        if (current->is_deleted == 0) {
            char raw_id[32], id[32], raw_name[DRUG_DISPLAY_NAME_LEN], name[DRUG_DISPLAY_NAME_LEN];
            char production[32], expiry[32], stock[32], demand[32];
            snprintf(raw_id, sizeof(raw_id), "%d", current->drug_id);
            build_drug_display_name(current, raw_name, (int)sizeof(raw_name));
            snprintf(stock, sizeof(stock), "%d", current->stock);
            snprintf(demand, sizeof(demand), "%.2f", forecast_drug_demand(system, current->drug_id, window_days));
            format_display_cell(raw_id, id, (int)sizeof(id), FORECAST_COL_ID);
            format_display_cell(raw_name, name, (int)sizeof(name), DRUG_FORECAST_COL_NAME);
            format_display_cell(current->production_date, production, (int)sizeof(production), FORECAST_COL_DATE);
            format_display_cell(current->expiry_date, expiry, (int)sizeof(expiry), FORECAST_COL_DATE);
            format_display_cell(stock, stock, (int)sizeof(stock), FORECAST_COL_VALUE);
            format_display_cell(demand, demand, (int)sizeof(demand), FORECAST_COL_VALUE);
            printf("| %s | %s | %s | %s | %s | %s |\n", id, name, production, expiry, stock, demand);
            found = 1;
        }
        current = current->next;
    }
    print_table_border(drug_forecast_table_width(), '=');
    if (!found) {
        printf("当前暂无药品预测数据\n");
    }
}



#endif
