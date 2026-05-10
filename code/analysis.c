/*
 * analysis.c
 * 统计分析模块
 * 负责把患者、医生、科室、病房、药品和住院记录等数据
 * 汇总成查询报表与统计结果，供管理层和业务人员查看
 */

#include "system.h"

#if defined(HIS_BUILD_FROM_MAIN) || !HIS_MAIN_ONE_CLICK_BUILD

/* 统计某位医生的接诊记录数量 */
static int count_records_by_doctor(HospitalSystem *system, int doctor_id) {
    int count = 0;
    MedicalRecord *current = system->records;
    while (current != NULL) {
        if (current->doctor_id == doctor_id) {
            count++;
        }
        current = current->next;
    }
    return count;
}

#define ANALYSIS_DRUG_COL_ID 8
#define ANALYSIS_DRUG_COL_NAME DRUG_NAME_COL_WIDTH
#define ANALYSIS_DRUG_COL_VALUE 10
#define ANALYSIS_DRUG_COL_DATE 12

/* 药品分析表格总宽度 */
static int analysis_drug_table_width(void) {
    return ANALYSIS_DRUG_COL_ID + ANALYSIS_DRUG_COL_NAME +
           ANALYSIS_DRUG_COL_VALUE + ANALYSIS_DRUG_COL_VALUE + ANALYSIS_DRUG_COL_VALUE +
           ANALYSIS_DRUG_COL_VALUE + ANALYSIS_DRUG_COL_DATE + ANALYSIS_DRUG_COL_DATE + 3 * 8 + 1;
}

/* 打印药品分析表头 */
static void print_analysis_drug_header(void) {
    char id[32], drug_name[DRUG_DISPLAY_NAME_LEN], stock[32], purchase[32], sale[32], used[32];
    char production[32], expiry[32];
    print_table_border(analysis_drug_table_width(), '=');
    format_display_cell("编号", id, (int)sizeof(id), ANALYSIS_DRUG_COL_ID);
    format_display_cell("通用名（俗称/品牌）", drug_name, (int)sizeof(drug_name), ANALYSIS_DRUG_COL_NAME);
    format_display_cell("库存", stock, (int)sizeof(stock), ANALYSIS_DRUG_COL_VALUE);
    format_display_cell("进价", purchase, (int)sizeof(purchase), ANALYSIS_DRUG_COL_VALUE);
    format_display_cell("售价", sale, (int)sizeof(sale), ANALYSIS_DRUG_COL_VALUE);
    format_display_cell("已使用", used, (int)sizeof(used), ANALYSIS_DRUG_COL_VALUE);
    format_display_cell("生产日期", production, (int)sizeof(production), ANALYSIS_DRUG_COL_DATE);
    format_display_cell("有效期至", expiry, (int)sizeof(expiry), ANALYSIS_DRUG_COL_DATE);
    printf("| %s | %s | %s | %s | %s | %s | %s | %s |\n",
           id, drug_name, stock, purchase, sale, used, production, expiry);
    print_table_border(analysis_drug_table_width(), '-');
}

/* 打印单行药品分析数据 */
static void print_analysis_drug_row(const Drug *drug) {
    char raw_id[32], id[32], raw_name[DRUG_DISPLAY_NAME_LEN], drug_name[DRUG_DISPLAY_NAME_LEN];
    char raw_stock[32], stock[32], raw_purchase[32], purchase[32], raw_sale[32], sale[32], raw_used[32], used[32];
    char production[32], expiry[32];
    if (drug == NULL) {
        return;
    }
    snprintf(raw_id, sizeof(raw_id), "%d", drug->drug_id);
    build_drug_display_name(drug, raw_name, (int)sizeof(raw_name));
    snprintf(raw_stock, sizeof(raw_stock), "%d", drug->stock);
    snprintf(raw_purchase, sizeof(raw_purchase), "%.2f", drug->purchase_price);
    snprintf(raw_sale, sizeof(raw_sale), "%.2f", drug->sale_price);
    snprintf(raw_used, sizeof(raw_used), "%d", drug->total_used);
    format_display_cell(raw_id, id, (int)sizeof(id), ANALYSIS_DRUG_COL_ID);
    format_display_cell(raw_name, drug_name, (int)sizeof(drug_name), ANALYSIS_DRUG_COL_NAME);
    format_display_cell(raw_stock, stock, (int)sizeof(stock), ANALYSIS_DRUG_COL_VALUE);
    format_display_cell(raw_purchase, purchase, (int)sizeof(purchase), ANALYSIS_DRUG_COL_VALUE);
    format_display_cell(raw_sale, sale, (int)sizeof(sale), ANALYSIS_DRUG_COL_VALUE);
    format_display_cell(raw_used, used, (int)sizeof(used), ANALYSIS_DRUG_COL_VALUE);
    format_display_cell(drug->production_date, production, (int)sizeof(production), ANALYSIS_DRUG_COL_DATE);
    format_display_cell(drug->expiry_date, expiry, (int)sizeof(expiry), ANALYSIS_DRUG_COL_DATE);
    printf("| %s | %s | %s | %s | %s | %s | %s | %s |\n",
           id, drug_name, stock, purchase, sale, used, production, expiry);
}

/* 统计某个科室的接诊记录数 */
static int count_records_by_department(HospitalSystem *system, int department_id) {
    int count = 0;
    MedicalRecord *current = system->records;
    while (current != NULL) {
        if (current->department_id == department_id) {
            count++;
        }
        current = current->next;
    }
    return count;
}

/* 统计当前有效患者总数 */
static int count_active_patients(HospitalSystem *system) {
    int count = 0;
    Patient *current = system->patients;
    while (current != NULL) {
        if (current->is_deleted == 0) {
            count++;
        }
        current = current->next;
    }
    return count;
}

/* 统计当前有效医生总数 */
static int count_active_doctors(HospitalSystem *system) {
    int count = 0;
    Doctor *current = system->doctors;
    while (current != NULL) {
        if (current->is_deleted == 0) {
            count++;
        }
        current = current->next;
    }
    return count;
}

/* 统计某个科室当前正在住院的患者人数 */
static int count_inpatients_by_department(HospitalSystem *system, int department_id) {
    int count = 0;
    AdmissionHistory *current = system->admissions;
    while (current != NULL) {
        if (current->department_id == department_id && current->status == ADMISSION_ACTIVE) {
            count++;
        }
        current = current->next;
    }
    return count;
}

/* 统计某位患者的累计费用
 * 费用由病历中的 cost 和发药记录中的 amount 两部分组成
 */
static double sum_cost_by_patient(HospitalSystem *system, int patient_id) {
    double sum = 0.0;
    MedicalRecord *record = system->records;
    DrugUsageHistory *usage = system->drug_usages;
    while (record != NULL) {
        if (record->patient_id == patient_id) {
            sum += record->cost;
        }
        record = record->next;
    }
    while (usage != NULL) {
        if (usage->patient_id == patient_id) {
            sum += usage->amount;
        }
        usage = usage->next;
    }
    return sum;
}

/* 统计床位总数以及各状态数量 */
static void count_beds(HospitalSystem *system, int *total, int *free_count, int *occupied, int *repair) {
    Bed *current = system->beds;
    *total = 0;
    *free_count = 0;
    *occupied = 0;
    *repair = 0;
    while (current != NULL) {
        (*total)++;
        if (current->status == BED_STATUS_FREE) {
            (*free_count)++;
        } else if (current->status == BED_STATUS_OCCUPIED) {
            (*occupied)++;
        } else if (current->status == BED_STATUS_REPAIR) {
            (*repair)++;
        }
        current = current->next;
    }
}

/* 汇总系统中的累计费用/收入 */
static double total_income(HospitalSystem *system) {
    double sum = 0.0;
    MedicalRecord *record = system->records;
    DrugUsageHistory *usage = system->drug_usages;
    while (record != NULL) {
        sum += record->cost;
        record = record->next;
    }
    while (usage != NULL) {
        sum += usage->amount;
        usage = usage->next;
    }
    return sum;
}

static double total_record_income(HospitalSystem *system) {
    double sum = 0.0;
    MedicalRecord *record = system->records;
    while (record != NULL) {
        sum += record->cost;
        record = record->next;
    }
    return sum;
}

static double total_drug_sales(HospitalSystem *system) {
    double sum = 0.0;
    DrugUsageHistory *usage = system->drug_usages;
    while (usage != NULL) {
        sum += usage->amount;
        usage = usage->next;
    }
    return sum;
}

static double total_drug_cost(HospitalSystem *system) {
    double sum = 0.0;
    DrugUsageHistory *usage = system->drug_usages;
    while (usage != NULL) {
        sum += usage->purchase_price * usage->quantity;
        usage = usage->next;
    }
    return sum;
}

static double total_drug_profit(HospitalSystem *system) {
    return total_drug_sales(system) - total_drug_cost(system);
}

static double total_hospital_profit(HospitalSystem *system) {
    return total_record_income(system) + total_drug_profit(system);
}

/* 显示当前登录医生所在科室的统计信息 */
static void show_current_doctor_statistics(HospitalSystem *system) {
    Doctor *doctor = get_current_doctor(system);
    Department *department = NULL;
    if (doctor == NULL) {
        printf("当前医生身份无效\n");
        return;
    }
    department = find_department_by_id(system, doctor->department_id);
    print_title("本科室统计");
    printf("医生姓名：%s\n", doctor->name);
    printf("所属科室：%s\n", department != NULL ? department->name : "未知");
    printf("个人接诊记录数：%d\n", count_records_by_doctor(system, doctor->doctor_id));
    printf("本科室接诊记录数：%d\n", count_records_by_department(system, doctor->department_id));
    printf("本科室当前住院人数：%d\n", count_inpatients_by_department(system, doctor->department_id));
    printf("本科室可用床位：\n");
    list_available_beds_by_department(system, doctor->department_id);
}

/* 输出患者视角查询报表 */
void show_patient_query_report(HospitalSystem *system) {
    char keyword[NAME_LEN];
    Patient *patient = system->patients;
    int found = 0;
    if (system->current_role == ROLE_PATIENT) {
        patient = find_patient_by_id(system, system->current_patient_id);
        if (patient == NULL) {
            printf("患者不存在\n");
            return;
        }
        print_title("患者视角查询");
        printf("编号：%d  姓名：%s  性别：%s  年龄：%d  住院状态：%s\n",
               patient->patient_id, patient->name, patient->gender, patient->age,
               patient->is_inpatient ? "住院中" : "非住院");
        printf("累计费用：%.2f\n", sum_cost_by_patient(system, patient->patient_id));
        list_records_by_patient(system, patient->patient_id);
        return;
    }
    read_string("请输入患者编号/姓名/电话/身份证关键字: ", keyword, NAME_LEN);
    while (patient != NULL) {
        char id_text[32];
        snprintf(id_text, sizeof(id_text), "%d", patient->patient_id);
        if (patient->is_deleted == 0 &&
            (str_contains_ignore_case(id_text, keyword) ||
             str_contains_ignore_case(patient->name, keyword) ||
             str_contains_ignore_case(patient->phone, keyword) ||
             str_contains_ignore_case(patient->id_card, keyword)) &&
            (system->current_role != ROLE_DOCTOR || doctor_can_access_patient(system, patient->patient_id)) &&
            (system->current_role != ROLE_NURSE || nurse_can_access_patient(system, patient->patient_id))) {
            print_title("患者视角查询");
            printf("编号：%d  姓名：%s  性别：%s  年龄：%d  住院状态：%s\n",
                   patient->patient_id, patient->name, patient->gender, patient->age,
                   patient->is_inpatient ? "住院中" : "非住院");
            printf("累计费用：%.2f\n", sum_cost_by_patient(system, patient->patient_id));
            list_records_by_patient(system, patient->patient_id);
            found = 1;
        }
        patient = patient->next;
    }
    if (!found) {
        printf("未找到匹配的患者\n");
    }
}

/* 输出医生视角查询报表 */
void show_doctor_query_report(HospitalSystem *system) {
    char keyword[NAME_LEN];
    Doctor *doctor = system->doctors;
    int found = 0;
    if (system->current_role == ROLE_DOCTOR) {
        doctor = get_current_doctor(system);
        if (doctor == NULL) {
            printf("医生不存在\n");
            return;
        }
        print_title("医生视角统计");
        printf("医生姓名：%s\n", doctor->name);
        printf("接诊记录数：%d\n", count_records_by_doctor(system, doctor->doctor_id));
        return;
    }
    read_string("请输入医生编号/姓名/职称/电话/身份证关键字: ", keyword, NAME_LEN);
    while (doctor != NULL) {
        char id_text[32];
        Department *department = find_department_by_id(system, doctor->department_id);
        snprintf(id_text, sizeof(id_text), "%d", doctor->doctor_id);
        if (doctor->is_deleted == 0 &&
            (str_contains_ignore_case(id_text, keyword) ||
             str_contains_ignore_case(doctor->name, keyword) ||
             str_contains_ignore_case(doctor->title, keyword) ||
             str_contains_ignore_case(doctor->phone, keyword) ||
             str_contains_ignore_case(doctor->id_card, keyword))) {
            print_title("医生视角统计");
            printf("医生姓名：%s\n", doctor->name);
            printf("所属科室：%s\n", department != NULL ? department->name : "未知");
            printf("接诊记录数：%d\n", count_records_by_doctor(system, doctor->doctor_id));
            found = 1;
        }
        doctor = doctor->next;
    }
    if (!found) {
        printf("未找到匹配的医生\n");
    }
}

/* 输出科室视角查询报表 */
void show_department_query_report(HospitalSystem *system) {
    char keyword[NAME_LEN];
    Department *department = system->departments;
    int found = 0;
    if (system->current_role == ROLE_DOCTOR) {
        Doctor *doctor = get_current_doctor(system);
        department = doctor != NULL ? find_department_by_id(system, doctor->department_id) : NULL;
        if (department == NULL) {
            printf("科室不存在\n");
            return;
        }
        print_title("科室视角统计");
        printf("科室名称：%s\n", department->name);
        printf("医生数量：%d\n", count_doctors_in_department(system, department->department_id));
        printf("接诊记录数：%d\n", count_records_by_department(system, department->department_id));
        printf("当前住院人数：%d\n", count_inpatients_by_department(system, department->department_id));
        printf("可用床位：\n");
        list_available_beds_by_department(system, department->department_id);
        return;
    }
    read_string("请输入科室编号/名称/负责人关键字: ", keyword, NAME_LEN);
    while (department != NULL) {
        char id_text[32];
        snprintf(id_text, sizeof(id_text), "%d", department->department_id);
        if (department->is_deleted == 0 &&
            (str_contains_ignore_case(id_text, keyword) ||
             str_contains_ignore_case(department->name, keyword) ||
             str_contains_ignore_case(department->manager, keyword))) {
            print_title("科室视角统计");
            printf("科室名称：%s\n", department->name);
            printf("医生数量：%d\n", count_doctors_in_department(system, department->department_id));
            printf("接诊记录数：%d\n", count_records_by_department(system, department->department_id));
            printf("当前住院人数：%d\n", count_inpatients_by_department(system, department->department_id));
            printf("可用床位：\n");
            list_available_beds_by_department(system, department->department_id);
            found = 1;
        }
        department = department->next;
    }
    if (!found) {
        printf("未找到匹配的科室\n");
    }
}

/* 输出药品查询报表 */
void show_drug_query_report(HospitalSystem *system) {
    char keyword[DRUG_NAME_LEN];
    Drug *current = system->drugs;
    int found = 0;
    read_string("请输入药品编号/关键字(通用名/俗称/品牌): ", keyword, DRUG_NAME_LEN);
    print_title("药品查询");
    print_analysis_drug_header();
    while (current != NULL) {
        char id_text[32];
        snprintf(id_text, sizeof(id_text), "%d", current->drug_id);
        if (current->is_deleted == 0 &&
            (str_contains_ignore_case(id_text, keyword) ||
             str_contains_ignore_case(current->generic_name, keyword) ||
             str_contains_ignore_case(current->common_name, keyword) ||
             str_contains_ignore_case(current->brand_name, keyword))) {
            print_analysis_drug_row(current);
            found = 1;
        }
        current = current->next;
    }
    print_table_border(analysis_drug_table_width(), '=');
    if (!found) {
        printf("未找到匹配的药品\n");
    }
}

/* 输出管理员统计总览
 * 会把患者、医生、床位、药品和收入等关键指标集中展示出来
 */
void show_management_statistics(HospitalSystem *system) {
    int total_beds = 0;
    int free_count = 0;
    int occupied = 0;
    int repair = 0;
    Department *department = NULL;
    Doctor *doctor = NULL;
    print_title("管理员统计总览");
    count_beds(system, &total_beds, &free_count, &occupied, &repair);
    printf("患者总数：%d\n", count_active_patients(system));
    printf("医生总数：%d\n", count_active_doctors(system));
    printf("当前住院人数：%d\n", current_inpatient_count(system));
    printf("床位总数：%d，空闲：%d，占用：%d，维修：%d，使用率：%.2f%%\n",
           total_beds, free_count, occupied, repair, total_beds > 0 ? occupied * 100.0 / total_beds : 0.0);
    printf("累计收入/费用：%.2f\n", total_income(system));
    printf("其中挂号/病历等收入：%.2f\n", total_record_income(system));
    printf("药品销售额：%.2f，药品成本：%.2f，药品总利润：%.2f\n",
           total_drug_sales(system), total_drug_cost(system), total_drug_profit(system));
    printf("医院总体利润：%.2f\n", total_hospital_profit(system));
    printf("各科室接诊人数：\n");
    department = system->departments;
    while (department != NULL) {
        if (department->is_deleted == 0) {
            printf("%-12s %d\n", department->name, count_records_by_department(system, department->department_id));
        }
        department = department->next;
    }
    printf("各医生接诊人数：\n");
    doctor = system->doctors;
    while (doctor != NULL) {
        if (doctor->is_deleted == 0) {
            printf("%-12s %d\n", doctor->name, count_records_by_doctor(system, doctor->doctor_id));
        }
        doctor = doctor->next;
    }
}

/* 输出医护视角统计
 * 医生登录时展示本科室数据，管理员等身份则展示更偏全局的医疗统计
 */
void show_medical_statistics(HospitalSystem *system) {
    if (system->current_role == ROLE_DOCTOR) {
        show_current_doctor_statistics(system);
        return;
    }
    print_title("医护视角统计");
    show_department_history_analysis(system, 7);
    show_department_forecast(system);
}

/* 患者统计入口
 * 当前实现直接复用患者查询报表逻辑
 */
void show_patient_statistics(HospitalSystem *system) {
    show_patient_query_report(system);
}

/* 导出综合分析报告到文本文件
 * 这是一个面向答辩和结果留存的函数，会把统计、预测和优化建议汇总输出
 */
void export_analysis_report(HospitalSystem *system, const char *file_name) {
    FILE *fp = fopen(file_name, "w");
    Department *department = NULL;
    Drug *drug = NULL;
    int total_beds = 0;
    int free_count = 0;
    int occupied = 0;
    int repair = 0;
    if (fp == NULL) {
        printf("无法导出分析报告\n");
        return;
    }
    fputs("\xEF\xBB\xBF", fp);
    count_beds(system, &total_beds, &free_count, &occupied, &repair);
    fprintf(fp, "小型医院轻量级 HIS 分析报告\n");
    fprintf(fp, "========================================\n");
    fprintf(fp, "当前住院人数：%d\n", current_inpatient_count(system));
    fprintf(fp, "床位总数：%d  空闲：%d  占用：%d  维修：%d\n", total_beds, free_count, occupied, repair);
    fprintf(fp, "累计费用：%.2f\n", total_income(system));
    fprintf(fp, "挂号/病历等收入：%.2f\n", total_record_income(system));
    fprintf(fp, "药品销售额：%.2f  药品成本：%.2f  药品总利润：%.2f\n",
            total_drug_sales(system), total_drug_cost(system), total_drug_profit(system));
    fprintf(fp, "医院总体利润：%.2f\n", total_hospital_profit(system));
    fprintf(fp, "\n各科室接诊统计：\n");
    department = system->departments;
    while (department != NULL) {
        if (department->is_deleted == 0) {
            fprintf(fp, "%s：接诊 %d 人次，预测下周期住院 %.2f 人，预测床位需求 %.2f\n",
                    department->name,
                    count_records_by_department(system, department->department_id),
                    forecast_department_admission(system, department->department_id, 7),
                    forecast_department_bed_need(system, department->department_id, 7));
        }
        department = department->next;
    }
    fprintf(fp, "\n药品分析：\n");
    drug = system->drugs;
    while (drug != NULL) {
        if (drug->is_deleted == 0) {
            char drug_name[DRUG_DISPLAY_NAME_LEN];
            build_drug_display_name(drug, drug_name, (int)sizeof(drug_name));
            fprintf(fp, "%s：生产日期 %s，有效期至 %s，库存 %d，已用 %d，预测需求 %.2f\n",
                    drug_name, drug->production_date, drug->expiry_date, drug->stock, drug->total_used,
                    forecast_drug_demand(system, drug->drug_id, 7));
        }
        drug = drug->next;
    }
    fprintf(fp, "\n优化建议：\n");
    department = system->departments;
    while (department != NULL) {
        if (department->is_deleted == 0) {
            int total = 0;
            int occ = 0;
            double predict = 0.0;
            total = 0;
            occ = 0;
            {
                Bed *bed = system->beds;
                while (bed != NULL) {
                    if (bed->department_id == department->department_id) {
                        total++;
                        if (bed->status == BED_STATUS_OCCUPIED) {
                            occ++;
                        }
                    }
                    bed = bed->next;
                }
            }
            predict = forecast_department_bed_need(system, department->department_id, 7);
            if (predict > total * 0.9) {
                fprintf(fp, "%s：建议增加床位\n", department->name);
            } else if (total > 0 && occ * 1.0 / total < 0.4) {
                fprintf(fp, "%s：建议释放 1 张空闲床位供调剂参考\n", department->name);
            } else {
                fprintf(fp, "%s：建议维持现状\n", department->name);
            }
        }
        department = department->next;
    }
    fclose(fp);
    printf("分析报告已导出到 %s\n", file_name);
}

static void export_drug_analysis_report(HospitalSystem *system, const char *file_name) {
    FILE *fp = fopen(file_name, "w");
    Drug *drug = NULL;
    if (fp == NULL) {
        printf("无法导出药品分析报告\n");
        return;
    }
    fputs("\xEF\xBB\xBF", fp);
    fprintf(fp, "药品分析报告\n");
    fprintf(fp, "========================================\n");
    fprintf(fp, "药品销售额：%.2f\n", total_drug_sales(system));
    fprintf(fp, "药品成本：%.2f\n", total_drug_cost(system));
    fprintf(fp, "药品总利润：%.2f\n", total_drug_profit(system));
    fprintf(fp, "\n药品明细：\n");
    for (drug = system->drugs; drug != NULL; drug = drug->next) {
        if (drug->is_deleted == 0) {
            char drug_name[DRUG_DISPLAY_NAME_LEN];
            build_drug_display_name(drug, drug_name, (int)sizeof(drug_name));
            fprintf(fp, "%s：库存 %d，已用 %d，预测需求 %.2f\n",
                    drug_name, drug->stock, drug->total_used,
                    forecast_drug_demand(system, drug->drug_id, 7));
        }
    }
    fclose(fp);
    printf("药品分析报告已导出到 %s\n", file_name);
}

static void export_department_analysis_report(HospitalSystem *system, const char *file_name) {
    FILE *fp = fopen(file_name, "w");
    Department *department = NULL;
    if (fp == NULL) {
        printf("无法导出科室分析报告\n");
        return;
    }
    fputs("\xEF\xBB\xBF", fp);
    fprintf(fp, "科室分析报告\n");
    fprintf(fp, "========================================\n");
    for (department = system->departments; department != NULL; department = department->next) {
        if (department->is_deleted == 0) {
            fprintf(fp, "%s：接诊 %d 人次，预测住院 %.2f 人，预测床位需求 %.2f\n",
                    department->name,
                    count_records_by_department(system, department->department_id),
                    forecast_department_admission(system, department->department_id, 7),
                    forecast_department_bed_need(system, department->department_id, 7));
        }
    }
    fclose(fp);
    printf("科室分析报告已导出到 %s\n", file_name);
}

static void export_profit_report(HospitalSystem *system, const char *file_name) {
    FILE *fp = fopen(file_name, "w");
    if (fp == NULL) {
        printf("无法导出利润报告\n");
        return;
    }
    fputs("\xEF\xBB\xBF", fp);
    fprintf(fp, "医院利润报告\n");
    fprintf(fp, "========================================\n");
    fprintf(fp, "挂号/病历等收入：%.2f\n", total_record_income(system));
    fprintf(fp, "药品销售额：%.2f\n", total_drug_sales(system));
    fprintf(fp, "药品成本：%.2f\n", total_drug_cost(system));
    fprintf(fp, "药品总利润：%.2f\n", total_drug_profit(system));
    fprintf(fp, "医院总体利润：%.2f\n", total_hospital_profit(system));
    fclose(fp);
    printf("利润报告已导出到 %s\n", file_name);
}

void export_analysis_report_interactive(HospitalSystem *system) {
    int choice = 0;
    print_title("导出分析报告");
    printf("1. 整体分析报告\n");
    printf("2. 科室分析报告\n");
    printf("3. 药品分析报告\n");
    printf("4. 利润报告\n");
    choice = read_int_in_range("请选择导出范围: ", 1, 4);
    if (choice == 1) {
        export_analysis_report(system, "analysis_report.txt");
    } else if (choice == 2) {
        export_department_analysis_report(system, "department_analysis_report.txt");
    } else if (choice == 3) {
        export_drug_analysis_report(system, "drug_analysis_report.txt");
    } else {
        export_profit_report(system, "profit_report.txt");
    }
}

#endif

