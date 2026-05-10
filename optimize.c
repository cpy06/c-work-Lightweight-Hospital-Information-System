/* 优化建议模块。
 * 在统计与预测结果基础上，进一步输出病房利用率分析，药品趋势分析和资源优化建议。
 */

#include "system.h"

#if defined(HIS_BUILD_FROM_MAIN) || !HIS_MAIN_ONE_CLICK_BUILD

/* 用 ASCII 字符绘制一个简易条形图
 * 这样在纯控制台环境中也能直观看到利用率高低
 */
static void print_ascii_bar(const char *label, double ratio) {
    int i = 0;
    int blocks = (int)(ratio * 20.0 + 0.5);// 计算条形块数量（比例 × 20，0.5 用于四舍五入）
    if (blocks < 0) {
        blocks = 0;
    }
    if (blocks > 20) {
        blocks = 20;
    }// 确保条形图宽度在 0-20 范围内
    printf("%-10s %6.1f%% ", label, ratio * 100.0);//打印标签和利用率百分比（左对齐10字符，百分比保留1位小数）
    for (i = 0; i < blocks; i++) {
        putchar('#');
    }
    putchar('\n');
}

#define DRUG_TREND_COL_ID 8
#define DRUG_TREND_COL_NAME DRUG_NAME_COL_WIDTH
#define DRUG_TREND_COL_DATE 12
#define DRUG_TREND_COL_STOCK 10
#define DRUG_TREND_COL_USED 10
#define DRUG_TREND_COL_DEMAND 12
#define DRUG_TREND_COL_SUGGESTION 24

static int drug_trend_table_width(void) {
    return DRUG_TREND_COL_ID + DRUG_TREND_COL_NAME + DRUG_TREND_COL_DATE + DRUG_TREND_COL_DATE + DRUG_TREND_COL_STOCK +
           DRUG_TREND_COL_USED + DRUG_TREND_COL_DEMAND + DRUG_TREND_COL_SUGGESTION +
           3 * 8 + 1;
}

static void print_drug_trend_header(void) {
    char id[32], name[DRUG_DISPLAY_NAME_LEN], production[32], expiry[32], stock[32], used[32], demand[32], suggestion[64];
    print_table_border(drug_trend_table_width(), '=');
    format_display_cell("药品ID", id, (int)sizeof(id), DRUG_TREND_COL_ID);
    format_display_cell("通用名（俗称/品牌）", name, (int)sizeof(name), DRUG_TREND_COL_NAME);
    format_display_cell("生产日期", production, (int)sizeof(production), DRUG_TREND_COL_DATE);
    format_display_cell("有效期至", expiry, (int)sizeof(expiry), DRUG_TREND_COL_DATE);
    format_display_cell("当前库存", stock, (int)sizeof(stock), DRUG_TREND_COL_STOCK);
    format_display_cell("已使用量", used, (int)sizeof(used), DRUG_TREND_COL_USED);
    format_display_cell("预测需求", demand, (int)sizeof(demand), DRUG_TREND_COL_DEMAND);
    format_display_cell("建议", suggestion, (int)sizeof(suggestion), DRUG_TREND_COL_SUGGESTION);
    printf("| %s | %s | %s | %s | %s | %s | %s | %s |\n",
           id, name, production, expiry, stock, used, demand, suggestion);
    print_table_border(drug_trend_table_width(), '-');
}

/* 展示科室历史住院分析
 * 会输出近 N 天入院/出院人数、平均住院天数、床位利用率，
 * 同时识别热门科室和冷门科室
 */
void show_department_history_analysis(HospitalSystem *system, int window_days) {
    Department *current = system->departments;
    Department *hot = NULL;
    Department *cold = NULL;
    int hot_value = -1;
    int cold_value = 1000000;
    print_title("科室历史住院分析");
    printf("%-8s %-12s %-10s %-10s %-10s %-10s\n",
           "科室ID", "科室名称", "近N天入院", "近N天出院", "平均住院天数", "床位利用率");
    while (current != NULL) {
        if (current->is_deleted == 0) {
            int in_count = count_department_admissions_in_recent_days(system, current->department_id, window_days);
            int out_count = count_department_discharges_in_recent_days(system, current->department_id, window_days);
            int total_beds = count_department_beds(system, current->department_id);
            int occupied_beds = count_department_occupied_beds(system, current->department_id);
            double rate = total_beds > 0 ? (double)occupied_beds / total_beds : 0.0;
            printf("%-8d %-12s %-10d %-10d %-10.2f %-10.2f\n",
                   current->department_id,
                   current->name,
                   in_count,
                   out_count,
                   average_department_stay_days(system, current->department_id),
                   rate);
            print_ascii_bar(current->name, rate);
            if (in_count > hot_value) {
                hot_value = in_count;
                hot = current;
            }
            if (in_count < cold_value) {
                cold_value = in_count;
                cold = current;
            }
        }
        current = current->next;
    }
    if (hot != NULL) {
        printf("热门科室：%s，最近 %d 天入院人数最多\n", hot->name, window_days);
    }
    if (cold != NULL) {
        printf("冷门科室：%s，最近 %d 天入院人数较少\n", cold->name, window_days);
    }
}

/* 给出病房资源优化建议
 * 预测需求超过90%时，建议增加床位
 * 利用率低于40%且总床位数大于2时，建议减少床位
 * 否则，建议维持现状
 */
void show_ward_optimization_suggestions(HospitalSystem *system, int window_days) {
    Department *current = system->departments;
    print_title("病房优化建议");
    printf("%-12s %-8s %-8s %-10s %-10s %-24s\n",
           "科室", "总床位", "已占用", "利用率", "预测需求", "建议");
    while (current != NULL) {
        if (current->is_deleted == 0) {
            int total_beds = count_department_beds(system, current->department_id);
            int occupied = count_department_occupied_beds(system, current->department_id);
            double rate = total_beds > 0 ? (double)occupied / total_beds : 0.0;
            double predict_need = forecast_department_bed_need(system, current->department_id, window_days);
            char suggestion[TEXT_LEN];
            if (predict_need > total_beds * 0.9) {
                sprintf(suggestion, "建议增加 %d 张床位", (int)(predict_need - total_beds * 0.9 + 1));
            } else if (rate < 0.4 && total_beds > 2) {
                sprintf(suggestion, "建议释放 1 张空闲床位供调剂");
            } else {
                safe_copy(suggestion, "建议维持现状", TEXT_LEN);
            }
            printf("%-12s %-8d %-8d %-10.2f %-10.2f %-24s\n",
                   current->name, total_beds, occupied, rate, predict_need, suggestion);
        }
        current = current->next;
    }
}

/* 给出药品趋势分析与补货建议
 * 预测需求超过当前库存时，建议补货
 * 低频药品（已使用量小于5）建议缓采
 * 否则，建议库存可维持
 */
void show_drug_trend_analysis(HospitalSystem *system, int window_days) {
    Drug *current = system->drugs;
    Drug *top_drug = NULL;
    int top_usage = -1;
    int found = 0;
    print_title("药品趋势分析与补货建议");
    print_drug_trend_header();
    while (current != NULL) {
        if (current->is_deleted == 0) {
            double predicted = forecast_drug_demand(system, current->drug_id, window_days);
            char suggestion[TEXT_LEN];
            char raw_id[32], id[32], raw_name[DRUG_DISPLAY_NAME_LEN], drug_name[DRUG_DISPLAY_NAME_LEN];
            char production[32], expiry[32], raw_stock[32], stock[32], raw_used[32], used[32], raw_demand[32], demand[32], suggestion_cell[TEXT_LEN * 2];
            build_drug_display_name(current, raw_name, (int)sizeof(raw_name));
            if (current->stock < predicted) {
                sprintf(suggestion, "建议补货 %d", (int)(predicted - current->stock + 1));
            } else if (current->total_used < 5) {
                safe_copy(suggestion, "低频药品，可缓采", TEXT_LEN);
            } else {
                safe_copy(suggestion, "库存可维持", TEXT_LEN);
            }
            snprintf(raw_id, sizeof(raw_id), "%d", current->drug_id);
            snprintf(raw_stock, sizeof(raw_stock), "%d", current->stock);
            snprintf(raw_used, sizeof(raw_used), "%d", current->total_used);
            snprintf(raw_demand, sizeof(raw_demand), "%.2f", predicted);
            format_display_cell(raw_id, id, (int)sizeof(id), DRUG_TREND_COL_ID);
            format_display_cell(raw_name, drug_name, (int)sizeof(drug_name), DRUG_TREND_COL_NAME);
            format_display_cell(current->production_date, production, (int)sizeof(production), DRUG_TREND_COL_DATE);
            format_display_cell(current->expiry_date, expiry, (int)sizeof(expiry), DRUG_TREND_COL_DATE);
            format_display_cell(raw_stock, stock, (int)sizeof(stock), DRUG_TREND_COL_STOCK);
            format_display_cell(raw_used, used, (int)sizeof(used), DRUG_TREND_COL_USED);
            format_display_cell(raw_demand, demand, (int)sizeof(demand), DRUG_TREND_COL_DEMAND);
            format_display_cell(suggestion, suggestion_cell, (int)sizeof(suggestion_cell), DRUG_TREND_COL_SUGGESTION);
            printf("| %s | %s | %s | %s | %s | %s | %s | %s |\n",
                   id, drug_name, production, expiry, stock, used, demand, suggestion_cell);
            found = 1;
            if (current->total_used > top_usage) {
                top_usage = current->total_used;
                top_drug = current;
            }
        }
        current = current->next;
    }
    print_table_border(drug_trend_table_width(), '=');
    if (!found) {
        printf("当前暂无药品趋势数据\n");
    }
    if (top_drug != NULL) {
        char drug_name[DRUG_DISPLAY_NAME_LEN];
        build_drug_display_name(top_drug, drug_name, (int)sizeof(drug_name));
        printf("高频药品代表：%s，总使用量为 %d\n",
               drug_name, top_drug->total_used);
    }
}




#endif
