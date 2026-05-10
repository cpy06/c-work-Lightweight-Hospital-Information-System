/*
 * drug.c
 * 药品与药房模块
 * 负责药品维护、入库、出库、发药和库存展示
 */

#include "system.h"

#if defined(HIS_BUILD_FROM_MAIN) || !HIS_MAIN_ONE_CLICK_BUILD

/* 按药品编号查找有效药品 */
Drug *find_drug_by_id(HospitalSystem *system, int drug_id) {
    Drug *current = system->drugs;
    while (current != NULL) {
        if (current->drug_id == drug_id && current->is_deleted == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

static const char *drug_access_mode_to_text(int access_mode) {
    return access_mode == DRUG_ACCESS_DENY_ALL ? "默认不可用" : "默认可用";
}

static const char *drug_operator_role(HospitalSystem *system) {
    if (system != NULL && system->current_role == ROLE_PHARMACY) {
        return "药房工作人员";
    }
    if (system != NULL && system->current_role == ROLE_ADMIN) {
        return admin_log_role(system);
    }
    return "药房工作人员";
}

static DrugDepartmentRule *find_drug_department_rule(HospitalSystem *system, int drug_id, int department_id) {
    DrugDepartmentRule *current = system->drug_department_rules;
    while (current != NULL) {
        if (current->drug_id == drug_id && current->department_id == department_id) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

static void remove_drug_department_rule(HospitalSystem *system, int drug_id, int department_id) {
    DrugDepartmentRule *current = system->drug_department_rules;
    DrugDepartmentRule *prev = NULL;
    while (current != NULL) {
        if (current->drug_id == drug_id && current->department_id == department_id) {
            if (prev == NULL) {
                system->drug_department_rules = current->next;
            } else {
                prev->next = current->next;
            }
            free(current);
            return;
        }
        prev = current;
        current = current->next;
    }
}

static void clear_drug_department_rules(HospitalSystem *system, int drug_id) {
    DrugDepartmentRule *current = system->drug_department_rules;
    DrugDepartmentRule *prev = NULL;
    while (current != NULL) {
        DrugDepartmentRule *next = current->next;
        if (current->drug_id == drug_id) {
            if (prev == NULL) {
                system->drug_department_rules = next;
            } else {
                prev->next = next;
            }
            free(current);
        } else {
            prev = current;
        }
        current = next;
    }
}

static int upsert_drug_department_rule(HospitalSystem *system, int drug_id, int department_id, int rule_type) {
    DrugDepartmentRule *rule = find_drug_department_rule(system, drug_id, department_id);
    DrugDepartmentRule *tail = NULL;
    if (rule != NULL) {
        rule->rule_type = rule_type;
        return 1;
    }
    rule = (DrugDepartmentRule *)malloc(sizeof(DrugDepartmentRule));
    if (rule == NULL) {
        return 0;
    }
    rule->drug_id = drug_id;
    rule->department_id = department_id;
    rule->rule_type = rule_type;
    rule->next = NULL;
    if (system->drug_department_rules == NULL) {
        system->drug_department_rules = rule;
        return 1;
    }
    tail = system->drug_department_rules;
    while (tail->next != NULL) {
        tail = tail->next;
    }
    tail->next = rule;
    return 1;
}

static int drug_is_available_in_department(HospitalSystem *system, const Drug *drug, int department_id) {
    DrugDepartmentRule *rule = NULL;
    if (system == NULL || drug == NULL || find_department_by_id(system, department_id) == NULL) {
        return 0;
    }
    rule = find_drug_department_rule(system, drug->drug_id, department_id);
    if (drug->access_mode == DRUG_ACCESS_DENY_ALL) {
        return rule != NULL && rule->rule_type == DRUG_RULE_ALLOW;
    }
    return !(rule != NULL && rule->rule_type == DRUG_RULE_DENY);
}

static int drug_is_expired(const Drug *drug) {
    char today[DATE_LEN];
    if (drug == NULL || !is_valid_date_text(drug->expiry_date)) {
        return 1;
    }
    get_current_date(today);
    return date_to_serial(today) > date_to_serial(drug->expiry_date);
}

static int prompt_department_id(HospitalSystem *system) {
    int department_id = 0;
    list_departments(system);
    if (read_int_step("请输入科室编号（输入 b 返回）: ", &department_id, 1, 999999, 1) == -1) {
        return 0;
    }
    if (find_department_by_id(system, department_id) == NULL) {
        printf("科室不存在\n");
        return 0;
    }
    return department_id;
}
/*按科室找药品*/
static void show_drug_department_access(HospitalSystem *system, Drug *drug) {
    Department *department = NULL;
    int available_found = 0;
    int unavailable_found = 0;
    char display_name[DRUG_DISPLAY_NAME_LEN];
    if (drug == NULL) {
        return;
    }
    build_drug_display_name(drug, display_name, (int)sizeof(display_name));
    print_title("药品科室可用性");
    printf("药品：%s（%d）\n", display_name, drug->drug_id);
    printf("默认策略：%s\n", drug_access_mode_to_text(drug->access_mode));
    printf("可用科室：\n");
    for (department = system->departments; department != NULL; department = department->next) {
        if (department->is_deleted == 0 && drug_is_available_in_department(system, drug, department->department_id)) {
            printf("  [%d] %s\n", department->department_id, department->name);
            available_found = 1;
        }
    }
    if (!available_found) {
        printf("  无\n");
    }
    printf("不可用科室：\n");
    for (department = system->departments; department != NULL; department = department->next) {
        if (department->is_deleted == 0 && !drug_is_available_in_department(system, drug, department->department_id)) {
            printf("  [%d] %s\n", department->department_id, department->name);
            unavailable_found = 1;
        }
    }
    if (!unavailable_found) {
        printf("  无\n");
    }
}

#define DRUG_COL_ID 8
#define DRUG_COL_NAME DRUG_NAME_COL_WIDTH
#define DRUG_COL_PRICE 10
#define DRUG_COL_STOCK 8
#define DRUG_COL_DATE 12

/* 药品表格总宽度 */
static int drug_table_total_width(void) {
    return DRUG_COL_ID + DRUG_COL_NAME + DRUG_COL_PRICE + DRUG_COL_PRICE + DRUG_COL_STOCK +
           DRUG_COL_DATE + DRUG_COL_DATE + 3 * 7 + 1;
}

/* 打印药品表头 */
static void print_drug_table_header(void) {
    char id[32], drug_name[DRUG_DISPLAY_NAME_LEN];
    char purchase[32], sale[32], stock[32], production[32], expiry[32];
    print_table_border(drug_table_total_width(), '=');
    format_display_cell("编号", id, (int)sizeof(id), DRUG_COL_ID);
    format_display_cell("通用名（俗称/品牌）", drug_name, (int)sizeof(drug_name), DRUG_COL_NAME);
    format_display_cell("进价", purchase, (int)sizeof(purchase), DRUG_COL_PRICE);
    format_display_cell("售价", sale, (int)sizeof(sale), DRUG_COL_PRICE);
    format_display_cell("库存", stock, (int)sizeof(stock), DRUG_COL_STOCK);
    format_display_cell("生产日期", production, (int)sizeof(production), DRUG_COL_DATE);
    format_display_cell("有效期至", expiry, (int)sizeof(expiry), DRUG_COL_DATE);
    printf("| %s | %s | %s | %s | %s | %s | %s |\n",
           id, drug_name, purchase, sale, stock, production, expiry);
    print_table_border(drug_table_total_width(), '-');
}

/* 打印单行药品数据 */
static void print_drug_table_row(const Drug *drug) {
    char raw_id[32], id[32], raw_name[DRUG_DISPLAY_NAME_LEN], drug_name[DRUG_DISPLAY_NAME_LEN];
    char raw_purchase[32], purchase[32];
    char raw_sale[32], sale[32], raw_stock[32], stock[32], production[32], expiry[32];
    if (drug == NULL) {
        return;
    }
    snprintf(raw_id, sizeof(raw_id), "%d", drug->drug_id);
    snprintf(raw_purchase, sizeof(raw_purchase), "%.2f", drug->purchase_price);
    snprintf(raw_sale, sizeof(raw_sale), "%.2f", drug->sale_price);
    snprintf(raw_stock, sizeof(raw_stock), "%d", drug->stock);
    build_drug_display_name(drug, raw_name, (int)sizeof(raw_name));
    format_display_cell(raw_id, id, (int)sizeof(id), DRUG_COL_ID);
    format_display_cell(raw_name, drug_name, (int)sizeof(drug_name), DRUG_COL_NAME);
    format_display_cell(raw_purchase, purchase, (int)sizeof(purchase), DRUG_COL_PRICE);
    format_display_cell(raw_sale, sale, (int)sizeof(sale), DRUG_COL_PRICE);
    format_display_cell(raw_stock, stock, (int)sizeof(stock), DRUG_COL_STOCK);
    format_display_cell(drug->production_date, production, (int)sizeof(production), DRUG_COL_DATE);
    format_display_cell(drug->expiry_date, expiry, (int)sizeof(expiry), DRUG_COL_DATE);
    printf("| %s | %s | %s | %s | %s | %s | %s |\n",
           id, drug_name, purchase, sale, stock, production, expiry);
}

/* 向药品链表尾部追加新药品 */
void add_drug(HospitalSystem *system, Drug drug) {
    Drug *node = (Drug *)malloc(sizeof(Drug));
    Drug *tail = NULL;
    if (node == NULL) {
        return;
    }
    *node = drug;
    node->next = NULL;
    if (system->drugs == NULL) {
        system->drugs = node;
    } else {
        tail = system->drugs;
        while (tail->next != NULL) {
            tail = tail->next;
        }
        tail->next = node;
    }
}

/* 检查通用名和俗称是否一一对应 */
static int validate_drug_common_name(HospitalSystem *system, const Drug *drug, int ignore_drug_id) {
    Drug *current = NULL;
    if (system == NULL || drug == NULL) {
        return 0;
    }
    if (is_blank_string(drug->generic_name) || is_blank_string(drug->common_name)) {
        printf("通用名和俗称都不能为空\n");
        return 0;
    }
    current = system->drugs;
    while (current != NULL) {
        if (current->is_deleted == 0 && current->drug_id != ignore_drug_id) {
            if (strcmp(current->generic_name, drug->generic_name) == 0 &&
                strcmp(current->common_name, drug->common_name) != 0) {
                printf("通用名“%s”已经对应俗称“%s”，不能再对应“%s”\n",
                       drug->generic_name, current->common_name, drug->common_name);
                return 0;
            }
            if (strcmp(current->common_name, drug->common_name) == 0 &&
                strcmp(current->generic_name, drug->generic_name) != 0) {
                printf("俗称“%s”已经对应通用名“%s”，不能再对应“%s”\n",
                       drug->common_name, current->generic_name, drug->generic_name);
                return 0;
            }
        }
        current = current->next;
    }
    return 1;
}

/* 交互式新增药品 */
void add_drug_interactive(HospitalSystem *system) {
    Drug drug;
    memset(&drug, 0, sizeof(drug));
    drug.drug_id = system->next_drug_id++;
    /* 药品表单取消时回收编号，避免留下没有名称的药品。 */
    if (!fill_drug_form(&drug)) {
        system->next_drug_id--;
        printf("药品新增已取消\n");
        return;
    }
    if (!validate_drug_common_name(system, &drug, drug.drug_id)) {
        system->next_drug_id--;
        printf("药品新增已取消\n");
        return;
    }
    drug.total_used = 0;
    drug.access_mode = DRUG_ACCESS_ALLOW_ALL;
    drug.is_deleted = 0;
    add_drug(system, drug);
    append_log(system, drug_operator_role(system), "新增药品", drug.generic_name);
    printf("药品新增成功，编号为 %d\n", drug.drug_id);
}

/* 交互式修改药品信息 */
void modify_drug_interactive(HospitalSystem *system) {
    int drug_id = 0;
    list_drugs(system);
    if (read_int_step("请输入要修改的药品编号（输入 b 返回上一级）: ", &drug_id, 1, 999999, 1) == -1) {
        printf("药品信息修改已取消\n");
        return;
    }
    Drug *drug = find_drug_by_id(system, drug_id);
    Drug old_drug;
    if (drug == NULL) {
        printf("药品不存在\n");
        return;
    }
    print_title("当前药品信息");
    print_drug_table_header();
    print_drug_table_row(drug);
    print_table_border(drug_table_total_width(), '=');
    printf("修改时直接按回车可保留原值，输入 b 可取消修改。\n");
    old_drug = *drug;
    /* 修改中断时恢复原药品，避免价格或库存只改了一部分。 */
    if (!fill_drug_form(drug)) {
        *drug = old_drug;
        printf("药品信息修改已取消\n");
        return;
    }
    if (!validate_drug_common_name(system, drug, drug->drug_id)) {
        *drug = old_drug;
        printf("药品信息未修改\n");
        return;
    }
    append_log(system, drug_operator_role(system), "修改药品", drug->generic_name);
    printf("药品信息修改成功\n");
}

/* 交互式删除药品
 * 采用逻辑删除，便于保留历史发药记录中的药品引用
 */
void delete_drug_interactive(HospitalSystem *system) {
    int drug_id = read_int("请输入要删除的药品编号: ");
    Drug *drug = find_drug_by_id(system, drug_id);
    if (drug == NULL) {
        printf("药品不存在\n");
        return;
    }
    drug->is_deleted = 1;
    append_log(system, drug_operator_role(system), "删除药品", drug->generic_name);
    printf("药品已逻辑删除\n");
}

/* 按药品关键字查询，支持编号、通用名、俗称和品牌匹配 */
void query_drug_interactive(HospitalSystem *system) {
    char keyword[DRUG_NAME_LEN];
    Drug *current = system->drugs;
    int found = 0;
    read_string("请输入药品编号/名称关键字(通用名/俗称/品牌): ", keyword, DRUG_NAME_LEN);
    print_drug_table_header();
    while (current != NULL) {
        char id_text[32];
        snprintf(id_text, sizeof(id_text), "%d", current->drug_id);
        if (current->is_deleted == 0 &&
            (str_contains_ignore_case(id_text, keyword) ||
             str_contains_ignore_case(current->generic_name, keyword) ||
             str_contains_ignore_case(current->common_name, keyword) ||
             str_contains_ignore_case(current->brand_name, keyword))) {
            print_drug_table_row(current);
            found = 1;
        }
        current = current->next;
    }
    print_table_border(drug_table_total_width(), '=');
    if (!found) {
        printf("未找到匹配的药品\n");
    }
}

/* 列出全部有效药品 */
void list_drugs(HospitalSystem *system) {
    Drug *current = system->drugs;
    int found = 0;
    print_title("药品列表");
    print_drug_table_header();
    while (current != NULL) {
        if (current->is_deleted == 0) {
            print_drug_table_row(current);
            found = 1;
        }
        current = current->next;
    }
    print_table_border(drug_table_total_width(), '=');
    if (!found) {
        printf("当前暂无药品数据\n");
    }
}

/* 药品入库 */
void stock_in_interactive(HospitalSystem *system) {
    int drug_id = 0;
    int quantity = 0;
    char detail[TEXT_LEN];
    Drug *drug = NULL;
    if (read_int_step("请输入药品编号（输入 b 取消）: ", &drug_id, 1, 999999, 1) == -1) {
        printf("药品入库已取消\n");
        return;
    }
    drug = find_drug_by_id(system, drug_id);
    if (drug == NULL) {
        printf("药品不存在\n");
        return;
    }
    print_title("当前药品信息");
    print_drug_table_header();
    print_drug_table_row(drug);
    print_table_border(drug_table_total_width(), '=');
    if (read_int_step("请输入入库数量（输入 b 取消）: ", &quantity, 1, 100000, 1) == -1) {
        printf("药品入库已取消\n");
        return;
    }
    drug->stock += quantity;
    snprintf(detail, sizeof(detail), "%s 入库%d", drug->generic_name, quantity);
    append_log(system, drug_operator_role(system), "药品入库", detail);
    printf("入库成功，当前库存为 %d\n", drug->stock);
}

/* 药品出库
 * 出库前会先检查库存是否足够，如果不够输出不足
 */
void stock_out_interactive(HospitalSystem *system) {
    int drug_id = read_int("请输入药品编号: ");
    int quantity = 0;
    Drug *drug = find_drug_by_id(system, drug_id);
    if (drug == NULL) {
        printf("药品不存在\n");
        return;
    }
    quantity = read_int_in_range("请输入出库数量: ", 1, 100000);
    if (drug->stock < quantity) {
        printf("库存不足，无法出库\n");
        return;
    }
    drug->stock -= quantity;
    append_log(system, drug_operator_role(system), "药品出库", drug->generic_name);
    printf("出库成功，当前库存为 %d\n", drug->stock);
}

/* 执行发药逻辑
 * 该函数不仅会扣减库存和累计用量，还会生成一条发药历史记录
 * 以便后续统计分析和需求预测模块复用这些历史数据
 */
int dispense_drug(HospitalSystem *system, int drug_id, int patient_id, int doctor_id, int quantity, const char *date_text, const char *note) {
    Drug *drug = find_drug_by_id(system, drug_id);
    Doctor *doctor = find_doctor_by_id(system, doctor_id);
    DrugUsageHistory *node = NULL;
    DrugUsageHistory *tail = NULL;
    if (drug == NULL || doctor == NULL || quantity <= 0) {
        return 0;
    }
    if (!drug_is_available_in_department(system, drug, doctor->department_id)) {
        return 0;
    }
    if (drug_is_expired(drug)) {
        return 0;
    }
    if (drug->stock < quantity) {
        return 0;
    }
    node = (DrugUsageHistory *)malloc(sizeof(DrugUsageHistory));
    if (node == NULL) {
        return 0;
    }
    drug->stock -= quantity;
    drug->total_used += quantity;
    node->usage_id = system->next_usage_id++;
    node->drug_id = drug_id;
    node->patient_id = patient_id;
    node->doctor_id = doctor_id;
    node->department_id = doctor->department_id;
    safe_copy(node->date, date_text, DATE_LEN);
    node->quantity = quantity;
    node->unit_price = drug->sale_price;
    node->purchase_price = drug->purchase_price;
    node->amount = quantity * node->unit_price;
    safe_copy(node->note, note, TEXT_LEN);
    node->next = NULL;

    if (system->drug_usages == NULL) {
        system->drug_usages = node;
    } else {
        tail = system->drug_usages;
        while (tail->next != NULL) {
            tail = tail->next;
        }
        tail->next = node;
    }
    append_log(system, drug_operator_role(system), "发药", drug->generic_name);
    return 1;
}

/* 交互式发药入口 */
void dispense_drug_interactive(HospitalSystem *system) {
    int drug_id = read_int("请输入药品编号: ");
    int patient_id = read_int("请输入患者编号: ");
    int doctor_id = read_int("请输入医生编号: ");
    int quantity = read_int_in_range("请输入发药数量: ", 1, 1000);
    char date_text[DATE_LEN];
    char note[TEXT_LEN];
    Drug *drug = find_drug_by_id(system, drug_id);
    Patient *patient = find_patient_by_id(system, patient_id);
    Doctor *doctor = find_doctor_by_id(system, doctor_id);
    Department *department = NULL;
    if (drug == NULL) {
        printf("药品不存在\n");
        return;
    }
    if (patient == NULL) {
        printf("患者不存在\n");
        return;
    }
    if (doctor == NULL) {
        printf("医生不存在\n");
        return;
    }
    department = find_department_by_id(system, doctor->department_id);
    if (department == NULL) {
        printf("医生所属科室不存在，请检查医生信息\n");
        return;
    }
    if (!drug_is_available_in_department(system, drug, department->department_id)) {
        printf("药品“%s”当前对科室“%s”不可用，无法发药\n", drug->generic_name, department->name);
        return;
    }
    if (drug_is_expired(drug)) {
        printf("药品“%s”已过有效期（有效期至：%s），无法发药\n", drug->generic_name, drug->expiry_date);
        return;
    }
    get_current_date(date_text);
    printf("本次发药将记入科室：%s（%d）\n", department->name, department->department_id);
    read_string("请输入发药备注: ", note, TEXT_LEN);
    if (dispense_drug(system, drug_id, patient_id, doctor_id, quantity, date_text, note)) {
        printf("发药成功\n");
    } else {
        printf("发药失败，可能是库存不足、药品不存在，或当前科室不可用\n");
    }
}

static Prescription *find_prescription_by_id(HospitalSystem *system, int prescription_id) {
    Prescription *current = system->prescriptions;
    while (current != NULL) {
        if (current->prescription_id == prescription_id) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

static void append_prescription(HospitalSystem *system, Prescription *prescription) {
    Prescription *node = (Prescription *)malloc(sizeof(Prescription));
    Prescription *tail = NULL;
    if (node == NULL) {
        return;
    }
    *node = *prescription;
    node->next = NULL;
    if (system->prescriptions == NULL) {
        system->prescriptions = node;
    } else {
        tail = system->prescriptions;
        while (tail->next != NULL) {
            tail = tail->next;
        }
        tail->next = node;
    }
}

static int append_prescription_item(HospitalSystem *system, PrescriptionItem *item) {
    PrescriptionItem *node = (PrescriptionItem *)malloc(sizeof(PrescriptionItem));
    PrescriptionItem *tail = NULL;
    if (node == NULL) {
        return 0;
    }
    *node = *item;
    node->next = NULL;
    if (system->prescription_items == NULL) {
        system->prescription_items = node;
    } else {
        tail = system->prescription_items;
        while (tail->next != NULL) {
            tail = tail->next;
        }
        tail->next = node;
    }
    return 1;
}

static const char *prescription_status_to_text(int status) {
    if (status == PRESCRIPTION_PENDING) {
        return "待发药";
    }
    if (status == PRESCRIPTION_DISPENSED) {
        return "已发药";
    }
    if (status == PRESCRIPTION_CANCELED) {
        return "已取消";
    }
    return "未知";
}

static int current_role_can_view_prescription(HospitalSystem *system, const Prescription *prescription) {
    Doctor *doctor = NULL;
    Nurse *nurse = NULL;
    Patient *patient = NULL;
    if (system == NULL || prescription == NULL) {
        return 0;
    }
    if (system->current_role == ROLE_ADMIN || system->current_role == ROLE_PHARMACY) {
        return 1;
    }
    if (system->current_role == ROLE_DOCTOR) {
        doctor = get_current_doctor(system);
        return doctor != NULL &&
               (prescription->doctor_id == doctor->doctor_id ||
                prescription->department_id == doctor->department_id);
    }
    if (system->current_role == ROLE_NURSE) {
        nurse = get_current_nurse(system);
        return nurse != NULL && prescription->department_id == nurse->department_id;
    }
    if (system->current_role == ROLE_PATIENT) {
        patient = get_current_patient(system);
        return patient != NULL && prescription->patient_id == patient->patient_id;
    }
    return 0;
}

void list_prescriptions(HospitalSystem *system) {
    Prescription *current = NULL;
    int found = 0;
    print_title("处方列表");
    printf("%-8s %-8s %-8s %-8s %-20s %-8s %s\n", "处方ID", "患者", "医生", "科室", "开具时间", "状态", "备注");
    for (current = system->prescriptions; current != NULL; current = current->next) {
        if (current_role_can_view_prescription(system, current)) {
            printf("%-8d %-8d %-8d %-8d %-20s %-8s %s\n",
                   current->prescription_id, current->patient_id, current->doctor_id,
                   current->department_id, current->created_time,
                   prescription_status_to_text(current->status), current->note);
            found = 1;
        }
    }
    if (!found) {
        printf("当前暂无可查看的处方。\n");
    }
}

static void list_prescription_items(HospitalSystem *system, int prescription_id) {
    PrescriptionItem *item = NULL;
    int found = 0;
    printf("处方明细：\n");
    printf("%-8s %-30s %-8s %s\n", "药品ID", "药品", "数量", "说明");
    for (item = system->prescription_items; item != NULL; item = item->next) {
        if (item->prescription_id == prescription_id) {
            Drug *drug = find_drug_by_id(system, item->drug_id);
            char drug_name[DRUG_DISPLAY_NAME_LEN];
            if (drug != NULL) {
                build_drug_display_name(drug, drug_name, (int)sizeof(drug_name));
            } else {
                snprintf(drug_name, sizeof(drug_name), "未知药品");
            }
            printf("%-8d %-30s %-8d %s\n", item->drug_id, drug_name, item->quantity, item->note);
            found = 1;
        }
    }
    if (!found) {
        printf("该处方没有明细。\n");
    }
}

void prescribe_drug_interactive(HospitalSystem *system) {
    Doctor *doctor = get_current_doctor(system);
    Prescription prescription;
    PrescriptionItem item;
    int patient_id = 0;
    int item_count = 0;
    int i = 0;
    int valid_count = 0;
    char note[TEXT_LEN];
    if (doctor == NULL) {
        printf("当前医生身份无效，请重新登录。\n");
        return;
    }
    patient_id = read_int("请输入患者编号: ");
    if (find_patient_by_id(system, patient_id) == NULL) {
        printf("患者不存在。\n");
        return;
    }
    if (!doctor_can_access_patient(system, patient_id)) {
        printf("无权限为非本科室且非本人患者开具处方。\n");
        return;
    }
    item_count = read_int_in_range("请输入本处方药品种类数: ", 1, 10);
    memset(&prescription, 0, sizeof(prescription));
    prescription.prescription_id = system->next_prescription_id++;
    prescription.patient_id = patient_id;
    prescription.doctor_id = doctor->doctor_id;
    prescription.department_id = doctor->department_id;
    get_current_datetime(prescription.created_time);
    prescription.status = PRESCRIPTION_PENDING;
    read_string("请输入处方备注: ", note, TEXT_LEN);
    safe_copy(prescription.note, note, TEXT_LEN);
    append_prescription(system, &prescription);
    for (i = 0; i < item_count; i++) {
        Drug *drug = NULL;
        Department *department = find_department_by_id(system, doctor->department_id);
        memset(&item, 0, sizeof(item));
        item.prescription_id = prescription.prescription_id;
        printf("第 %d 种药品：\n", i + 1);
        item.drug_id = read_int("请输入药品编号: ");
        drug = find_drug_by_id(system, item.drug_id);
        if (drug == NULL) {
            printf("药品不存在，本条明细跳过。\n");
            continue;
        }
        if (!drug_is_available_in_department(system, drug, doctor->department_id)) {
            printf("药品“%s”当前对科室“%s”不可用，本条明细跳过。\n",
                   drug->generic_name, department != NULL ? department->name : "未知");
            continue;
        }
        if (drug_is_expired(drug)) {
            printf("药品“%s”已过有效期，本条明细跳过。\n", drug->generic_name);
            continue;
        }
        item.quantity = read_int_in_range("请输入数量: ", 1, 1000);
        read_string("请输入用药说明: ", item.note, TEXT_LEN);
        if (!append_prescription_item(system, &item)) {
            printf("处方明细保存失败，内存不足。\n");
            continue;
        }
        valid_count++;
    }
    if (valid_count == 0) {
        Prescription *saved = find_prescription_by_id(system, prescription.prescription_id);
        if (saved != NULL) {
            saved->status = PRESCRIPTION_CANCELED;
        }
        printf("没有成功保存任何处方明细，处方已取消。\n");
        return;
    }
    append_log(system, "医生", "开具处方", note);
    printf("处方开具完成，处方编号为 %d。药房可按该处方一键发药。\n", prescription.prescription_id);
}

void dispense_prescription_interactive(HospitalSystem *system) {
    int prescription_id = 0;
    Prescription *prescription = NULL;
    PrescriptionItem *item = NULL;
    int has_item = 0;
    char date_text[DATE_LEN];
    char note[TEXT_LEN];
    list_prescriptions(system);
    prescription_id = read_int("请输入要发药的处方编号: ");
    prescription = find_prescription_by_id(system, prescription_id);
    if (prescription == NULL || !current_role_can_view_prescription(system, prescription)) {
        printf("处方不存在或无权限查看。\n");
        return;
    }
    if (prescription->status != PRESCRIPTION_PENDING) {
        printf("该处方不是待发药状态，不能重复发药。\n");
        return;
    }
    for (item = system->prescription_items; item != NULL; item = item->next) {
        if (item->prescription_id == prescription_id) {
            Drug *drug = find_drug_by_id(system, item->drug_id);
            has_item = 1;
            if (drug == NULL || drug->is_deleted) {
                printf("处方中存在无效药品 %d，发药已取消。\n", item->drug_id);
                return;
            }
            if (!drug_is_available_in_department(system, drug, prescription->department_id)) {
                printf("药品“%s”当前对处方科室不可用，发药已取消。\n", drug->generic_name);
                return;
            }
            if (drug_is_expired(drug)) {
                printf("药品“%s”已过有效期，发药已取消。\n", drug->generic_name);
                return;
            }
            if (drug->stock < item->quantity) {
                printf("药品“%s”库存不足，发药已取消。\n", drug->generic_name);
                return;
            }
        }
    }
    if (!has_item) {
        printf("该处方没有明细，不能发药。\n");
        return;
    }
    list_prescription_items(system, prescription_id);
    if (!read_yes_no("确认按处方一键发药？(y/n): ")) {
        printf("发药已取消。\n");
        return;
    }
    get_current_date(date_text);
    for (item = system->prescription_items; item != NULL; item = item->next) {
        if (item->prescription_id == prescription_id) {
            snprintf(note, sizeof(note), "处方ID:%d；%s", prescription_id, item->note);
            if (!dispense_drug(system, item->drug_id, prescription->patient_id,
                               prescription->doctor_id, item->quantity, date_text, note)) {
                printf("发药中断，请检查库存和药品规则。\n");
                return;
            }
        }
    }
    prescription->status = PRESCRIPTION_DISPENSED;
    append_log(system, "药房工作人员", "按处方发药", note);
    printf("处方发药完成。\n");
}

#define USAGE_COL_ID 8
#define USAGE_COL_DRUG 30
#define USAGE_COL_PERSON 8
#define USAGE_COL_DATE 12
#define USAGE_COL_QTY 6
#define USAGE_COL_AMOUNT 10
#define USAGE_COL_NOTE 20

static int usage_table_width(void) {
    return USAGE_COL_ID + USAGE_COL_DRUG + USAGE_COL_PERSON + USAGE_COL_PERSON + USAGE_COL_PERSON +
           USAGE_COL_DATE + USAGE_COL_QTY + USAGE_COL_AMOUNT + USAGE_COL_AMOUNT + USAGE_COL_NOTE +
           3 * 10 + 1;
}

static int current_role_can_view_usage(HospitalSystem *system, const DrugUsageHistory *usage) {
    Doctor *doctor = NULL;
    Nurse *nurse = NULL;
    Patient *patient = NULL;
    if (system == NULL || usage == NULL) {
        return 0;
    }
    if (system->current_role == ROLE_PHARMACY || system->current_role == ROLE_ADMIN) {
        return 1;
    }
    if (system->current_role == ROLE_DOCTOR) {
        doctor = get_current_doctor(system);
        return doctor != NULL && usage->department_id == doctor->department_id;
    }
    if (system->current_role == ROLE_NURSE) {
        nurse = get_current_nurse(system);
        return nurse != NULL && usage->department_id == nurse->department_id;
    }
    if (system->current_role == ROLE_PATIENT) {
        patient = get_current_patient(system);
        return patient != NULL && usage->patient_id == patient->patient_id;
    }
    return 0;
}

static void print_usage_header(void) {
    char id[32], drug[64], patient[32], doctor[32], dept[32], date[32], qty[32], unit[32], amount[32], note[48];
    print_table_border(usage_table_width(), '=');
    format_display_cell("发药ID", id, (int)sizeof(id), USAGE_COL_ID);
    format_display_cell("药品", drug, (int)sizeof(drug), USAGE_COL_DRUG);
    format_display_cell("患者", patient, (int)sizeof(patient), USAGE_COL_PERSON);
    format_display_cell("医生", doctor, (int)sizeof(doctor), USAGE_COL_PERSON);
    format_display_cell("科室", dept, (int)sizeof(dept), USAGE_COL_PERSON);
    format_display_cell("日期", date, (int)sizeof(date), USAGE_COL_DATE);
    format_display_cell("数量", qty, (int)sizeof(qty), USAGE_COL_QTY);
    format_display_cell("单价", unit, (int)sizeof(unit), USAGE_COL_AMOUNT);
    format_display_cell("金额", amount, (int)sizeof(amount), USAGE_COL_AMOUNT);
    format_display_cell("备注", note, (int)sizeof(note), USAGE_COL_NOTE);
    printf("| %s | %s | %s | %s | %s | %s | %s | %s | %s | %s |\n",
           id, drug, patient, doctor, dept, date, qty, unit, amount, note);
    print_table_border(usage_table_width(), '-');
}

static void print_usage_row(HospitalSystem *system, const DrugUsageHistory *usage) {
    Drug *drug = NULL;
    char raw_id[32], id[32], raw_drug[DRUG_DISPLAY_NAME_LEN], drug_name[64];
    char raw_patient[32], patient[32], raw_doctor[32], doctor[32], raw_dept[32], dept[32];
    char date[32], raw_qty[32], qty[32], raw_unit[32], unit[32], raw_amount[32], amount[32], note[TEXT_LEN * 2];
    if (usage == NULL) {
        return;
    }
    drug = find_drug_by_id(system, usage->drug_id);
    if (drug != NULL) {
        build_drug_display_name(drug, raw_drug, (int)sizeof(raw_drug));
    } else {
        snprintf(raw_drug, sizeof(raw_drug), "%d", usage->drug_id);
    }
    snprintf(raw_id, sizeof(raw_id), "%d", usage->usage_id);
    snprintf(raw_patient, sizeof(raw_patient), "%d", usage->patient_id);
    snprintf(raw_doctor, sizeof(raw_doctor), "%d", usage->doctor_id);
    snprintf(raw_dept, sizeof(raw_dept), "%d", usage->department_id);
    snprintf(raw_qty, sizeof(raw_qty), "%d", usage->quantity);
    snprintf(raw_unit, sizeof(raw_unit), "%.2f", usage->unit_price);
    snprintf(raw_amount, sizeof(raw_amount), "%.2f", usage->amount);
    format_display_cell(raw_id, id, (int)sizeof(id), USAGE_COL_ID);
    format_display_cell(raw_drug, drug_name, (int)sizeof(drug_name), USAGE_COL_DRUG);
    format_display_cell(raw_patient, patient, (int)sizeof(patient), USAGE_COL_PERSON);
    format_display_cell(raw_doctor, doctor, (int)sizeof(doctor), USAGE_COL_PERSON);
    format_display_cell(raw_dept, dept, (int)sizeof(dept), USAGE_COL_PERSON);
    format_display_cell(usage->date, date, (int)sizeof(date), USAGE_COL_DATE);
    format_display_cell(raw_qty, qty, (int)sizeof(qty), USAGE_COL_QTY);
    format_display_cell(raw_unit, unit, (int)sizeof(unit), USAGE_COL_AMOUNT);
    format_display_cell(raw_amount, amount, (int)sizeof(amount), USAGE_COL_AMOUNT);
    format_display_cell(usage->note, note, (int)sizeof(note), USAGE_COL_NOTE);
    printf("| %s | %s | %s | %s | %s | %s | %s | %s | %s | %s |\n",
           id, drug_name, patient, doctor, dept, date, qty, unit, amount, note);
}

void list_drug_usage_records(HospitalSystem *system) {
    DrugUsageHistory *current = NULL;
    int found = 0;
    if (system == NULL) {
        return;
    }
    print_title("发药记录");
    print_usage_header();
    for (current = system->drug_usages; current != NULL; current = current->next) {
        if (current_role_can_view_usage(system, current)) {
            print_usage_row(system, current);
            found = 1;
        }
    }
    print_table_border(usage_table_width(), '=');
    if (!found) {
        printf("当前暂无可查看的发药记录\n");
    }
}

/* 交互式维护药品对科室的可用性规则 */
void manage_drug_department_access_interactive(HospitalSystem *system) {
    int drug_id = 0;
    int choice = 0;
    Drug *drug = NULL;
    char detail[TEXT_LEN];
    if (system == NULL) {
        return;
    }
    list_drugs(system);
    if (read_int_step("请输入要设置的药品编号（输入 b 返回上一级）: ", &drug_id, 1, 999999, 1) == -1) {
        printf("药品科室可用性设置已取消\n");
        return;
    }
    drug = find_drug_by_id(system, drug_id);
    if (drug == NULL) {
        printf("药品不存在\n");
        return;
    }
    while (1) {
        int department_id = 0;
        DrugDepartmentRule *rule = NULL;
        show_drug_department_access(system, drug);
        printf("1. 设为默认全科室可用\n");
        printf("2. 设为默认全科室不可用\n");
        printf("3. 设置某科室可用\n");
        printf("4. 设置某科室不可用\n");
        printf("5. 删除某科室单独规则\n");
        printf("0. 返回\n");
        choice = read_int_in_range("请选择: ", 0, 5);
        if (choice == 0) {
            return;
        }
        if (choice == 1) {
            drug->access_mode = DRUG_ACCESS_ALLOW_ALL;
            clear_drug_department_rules(system, drug->drug_id);
            snprintf(detail, sizeof(detail), "%s -> 默认全科室可用", drug->generic_name);
            append_log(system, drug_operator_role(system), "设置药品科室可用性", detail);
            printf("已设为默认全科室可用，并清除该药品的单独规则\n");
            return;
        }
        if (choice == 2) {
            drug->access_mode = DRUG_ACCESS_DENY_ALL;
            clear_drug_department_rules(system, drug->drug_id);
            snprintf(detail, sizeof(detail), "%s -> 默认全科室不可用", drug->generic_name);
            append_log(system, drug_operator_role(system), "设置药品科室可用性", detail);
            printf("已设为默认全科室不可用，并清除该药品的单独规则\n");
            return;
        }
        department_id = prompt_department_id(system);
        if (department_id == 0) {
            continue;
        }
        rule = find_drug_department_rule(system, drug->drug_id, department_id);
        if (choice == 3) {
            if (drug->access_mode == DRUG_ACCESS_DENY_ALL) {
                if (!upsert_drug_department_rule(system, drug->drug_id, department_id, DRUG_RULE_ALLOW)) {
                    printf("设置失败，内存不足\n");
                    continue;
                }
            } else if (rule != NULL && rule->rule_type == DRUG_RULE_DENY) {
                remove_drug_department_rule(system, drug->drug_id, department_id);
            } else {
                printf("该科室当前已经可用，无需额外设置\n");
                continue;
            }
            snprintf(detail, sizeof(detail), "%s -> 科室 %d 设为可用", drug->generic_name, department_id);
            append_log(system, drug_operator_role(system), "设置药品科室可用性", detail);
            printf("设置成功\n");
            return;
        }
        if (choice == 4) {
            if (drug->access_mode == DRUG_ACCESS_ALLOW_ALL) {
                if (!upsert_drug_department_rule(system, drug->drug_id, department_id, DRUG_RULE_DENY)) {
                    printf("设置失败，内存不足\n");
                    continue;
                }
            } else if (rule != NULL && rule->rule_type == DRUG_RULE_ALLOW) {
                remove_drug_department_rule(system, drug->drug_id, department_id);
            } else {
                printf("该科室当前已经不可用，无需额外设置\n");
                continue;
            }
            snprintf(detail, sizeof(detail), "%s -> 科室 %d 设为不可用", drug->generic_name, department_id);
            append_log(system, drug_operator_role(system), "设置药品科室可用性", detail);
            printf("设置成功\n");
            return;
        }
        if (rule == NULL) {
            printf("该科室当前没有单独规则\n");
            continue;
        }
        remove_drug_department_rule(system, drug->drug_id, department_id);
        snprintf(detail, sizeof(detail), "%s -> 删除科室 %d 单独规则", drug->generic_name, department_id);
        append_log(system, drug_operator_role(system), "设置药品科室可用性", detail);
        printf("已删除单独规则，科室可用性将回到默认策略\n");
        return;
    }
}

#endif
