/*
 * patient.c
 * ------------------------------------------------------------
 * 患者管理模块。
 * 负责患者档案的新增、修改、删除、查询和住院患者展示。
 */

#include "system.h"

#if defined(HIS_BUILD_FROM_MAIN) || !HIS_MAIN_ONE_CLICK_BUILD

/* 按患者编号查找有效患者*/
Patient *find_patient_by_id(HospitalSystem *system, int patient_id) {
    Patient *current = system->patients;
    while (current != NULL) {
        if (current->patient_id == patient_id && current->is_deleted == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

#define PATIENT_COL_ID 8
#define PATIENT_COL_NAME 12
#define PATIENT_COL_GENDER 6
#define PATIENT_COL_AGE 6
#define PATIENT_COL_PHONE 14
#define PATIENT_COL_IDCARD 20
#define PATIENT_COL_BLOOD 8
#define PATIENT_COL_STATUS 8
#define INPATIENT_COL_BED 8

/* 普通患者信息表格总宽度*/
static int patient_table_width(void) {
    return PATIENT_COL_ID + PATIENT_COL_NAME + PATIENT_COL_GENDER + PATIENT_COL_AGE +
           PATIENT_COL_PHONE + PATIENT_COL_IDCARD + PATIENT_COL_BLOOD + PATIENT_COL_STATUS +
           3 * 8 + 1;
}

/* 住院患者信息表格总宽度*/
static int inpatient_table_width(void) {
    return PATIENT_COL_ID + PATIENT_COL_NAME + PATIENT_COL_GENDER + PATIENT_COL_AGE +
           PATIENT_COL_PHONE + INPATIENT_COL_BED + 3 * 6 + 1;
}

/* 打印患者总表表头*/
static void print_patient_table_header(void) {
    char id[32], name[32], gender[32], age[32], phone[32], id_card[40], blood[32], status[32];
    print_table_border(patient_table_width(), '=');
    format_display_cell("编号", id, (int)sizeof(id), PATIENT_COL_ID);
    format_display_cell("姓名", name, (int)sizeof(name), PATIENT_COL_NAME);
    format_display_cell("性别", gender, (int)sizeof(gender), PATIENT_COL_GENDER);
    format_display_cell("年龄", age, (int)sizeof(age), PATIENT_COL_AGE);
    format_display_cell("电话", phone, (int)sizeof(phone), PATIENT_COL_PHONE);
    format_display_cell("证件号", id_card, (int)sizeof(id_card), PATIENT_COL_IDCARD);
    format_display_cell("血型", blood, (int)sizeof(blood), PATIENT_COL_BLOOD);
    format_display_cell("住院", status, (int)sizeof(status), PATIENT_COL_STATUS);
    printf("| %s | %s | %s | %s | %s | %s | %s | %s |\n",
           id, name, gender, age, phone, id_card, blood, status);
    print_table_border(patient_table_width(), '-');
}

/* 打印单行患者信息*/
static void print_patient_row(const Patient *patient) {
    char raw_id[32], id[32], name[NAME_LEN * 2], gender[32], raw_age[32], age[32], phone[40], id_card[ID_LEN * 2];
    char blood[32], status[32];
    if (patient == NULL) {
        return;
    }
    snprintf(raw_id, sizeof(raw_id), "%d", patient->patient_id);
    snprintf(raw_age, sizeof(raw_age), "%d", patient->age);
    format_display_cell(raw_id, id, (int)sizeof(id), PATIENT_COL_ID);
    format_display_cell(patient->name, name, (int)sizeof(name), PATIENT_COL_NAME);
    format_display_cell(patient->gender, gender, (int)sizeof(gender), PATIENT_COL_GENDER);
    format_display_cell(raw_age, age, (int)sizeof(age), PATIENT_COL_AGE);
    format_display_cell(patient->phone, phone, (int)sizeof(phone), PATIENT_COL_PHONE);
    format_display_cell(patient->id_card, id_card, (int)sizeof(id_card), PATIENT_COL_IDCARD);
    format_display_cell(patient->blood_type, blood, (int)sizeof(blood), PATIENT_COL_BLOOD);
    format_display_cell(patient->is_inpatient ? "是" : "否", status, (int)sizeof(status), PATIENT_COL_STATUS);
    printf("| %s | %s | %s | %s | %s | %s | %s | %s |\n",
           id, name, gender, age, phone, id_card, blood, status);
}

/* 打印住院患者表头*/
static void print_inpatient_table_header(void) {
    char id[32], name[32], gender[32], age[32], phone[32], bed[32];
    print_table_border(inpatient_table_width(), '=');
    format_display_cell("编号", id, (int)sizeof(id), PATIENT_COL_ID);
    format_display_cell("姓名", name, (int)sizeof(name), PATIENT_COL_NAME);
    format_display_cell("性别", gender, (int)sizeof(gender), PATIENT_COL_GENDER);
    format_display_cell("年龄", age, (int)sizeof(age), PATIENT_COL_AGE);
    format_display_cell("电话", phone, (int)sizeof(phone), PATIENT_COL_PHONE);
    format_display_cell("床位", bed, (int)sizeof(bed), INPATIENT_COL_BED);
    printf("| %s | %s | %s | %s | %s | %s |\n",
           id, name, gender, age, phone, bed);
    print_table_border(inpatient_table_width(), '-');
}

/* 打印单行住院患者信息*/
static void print_inpatient_row(const Patient *patient) {
    char raw_id[32], id[32], name[NAME_LEN * 2], gender[32], raw_age[32], age[32], phone[40], raw_bed[32], bed[32];
    if (patient == NULL) {
        return;
    }
    snprintf(raw_id, sizeof(raw_id), "%d", patient->patient_id);
    snprintf(raw_age, sizeof(raw_age), "%d", patient->age);
    snprintf(raw_bed, sizeof(raw_bed), "%d", patient->current_bed_id);
    format_display_cell(raw_id, id, (int)sizeof(id), PATIENT_COL_ID);
    format_display_cell(patient->name, name, (int)sizeof(name), PATIENT_COL_NAME);
    format_display_cell(patient->gender, gender, (int)sizeof(gender), PATIENT_COL_GENDER);
    format_display_cell(raw_age, age, (int)sizeof(age), PATIENT_COL_AGE);
    format_display_cell(patient->phone, phone, (int)sizeof(phone), PATIENT_COL_PHONE);
    format_display_cell(raw_bed, bed, (int)sizeof(bed), INPATIENT_COL_BED);
    printf("| %s | %s | %s | %s | %s | %s |\n",
           id, name, gender, age, phone, bed);
}

/* 按身份证号查找患者。
 * 主要用于防止重复建档。
 */
static Patient *find_patient_by_id_card(HospitalSystem *system, const char *id_card) {
    Patient *current = system->patients;
    while (current != NULL) {
        if (current->is_deleted == 0 && strcmp(current->id_card, id_card) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

/* 向患者链表尾部追加一个患者*/
void add_patient(HospitalSystem *system, Patient patient) {
    Patient *node = (Patient *)malloc(sizeof(Patient));
    Patient *tail = NULL;
    if (node == NULL) {
        return;
    }
    *node = patient;
    node->next = NULL;
    if (system->patients == NULL) {
        system->patients = node;
    } else {
        tail = system->patients;
        while (tail->next != NULL) {
            tail = tail->next;
        }
        tail->next = node;
    }
}

/* 管理员端交互式新增患者*/
void add_patient_interactive(HospitalSystem *system) {
    Patient patient;
    memset(&patient, 0, sizeof(patient));
    patient.patient_id = system->next_patient_id++;
    /* 表单取消时回收预分配编号，避免生成空患者资料*/
    if (!fill_patient_form(&patient)) {
        system->next_patient_id--;
        printf("新增患者已取消。\n");
        return;
    }
    if (find_patient_by_id_card(system, patient.id_card) != NULL) {
        printf("系统中已存在相同证件号的患者，请先查询原有信息。\n");
        system->next_patient_id--;
        return;
    }
    patient.is_inpatient = 0;
    patient.current_bed_id = 0;
    generate_initial_password("P", patient.patient_id, patient.password, PASSWORD_LEN);
    patient.must_change_password = 1;
    patient.is_deleted = 0;
    add_patient(system, patient);
    append_log(system, admin_log_role(system), "新增患者", patient.name);
    printf("新增患者成功，患者编号为 %d。\n", patient.patient_id);
    printf("系统生成初始密码：%s，请提醒患者首次登录后修改。\n", patient.password);
}

/* 患者自助建档。
 * 成功时返回新患者编号，失败时返回 0，便于登录流程直接接续使用。
 */
int register_patient_self_interactive(HospitalSystem *system) {
    Patient patient;
    memset(&patient, 0, sizeof(patient));
    patient.patient_id = system->next_patient_id++;
    if (!fill_patient_form(&patient)) {
        system->next_patient_id--;
        return 0;
    }
    if (find_patient_by_id_card(system, patient.id_card) != NULL) {
        printf("系统中已存在相同证件号的患者，请直接使用已有患者编号登录。\n");
        system->next_patient_id--;
        return 0;
    }
    patient.is_inpatient = 0;
    patient.current_bed_id = 0;
    generate_initial_password("P", patient.patient_id, patient.password, PASSWORD_LEN);
    patient.must_change_password = 1;
    patient.is_deleted = 0;
    add_patient(system, patient);
    append_log(system, "患者", "自助建档", patient.name);
    printf("自助登记成功，您的患者编号为 %d，请妥善保存。\n", patient.patient_id);
    printf("系统生成初始密码：%s，请首次正式登录后修改。\n", patient.password);
    return patient.patient_id;
}

/* 交互式修改患者信息*/
void modify_patient_interactive(HospitalSystem *system) {
    int patient_id = 0;
    list_patients(system);
    if (read_int_step("请输入要修改的患者编号（输入 b 返回上一级）: ", &patient_id, 1, 999999, 1) == -1) {
        printf("患者信息修改已取消。\n");
        return;
    }
    Patient *patient = find_patient_by_id(system, patient_id);
    Patient old_patient;
    Patient *duplicate = NULL;
    if (patient == NULL) {
        printf("未找到对应患者。\n");
        return;
    }
    print_title("当前患者信息");
    print_patient_table_header();
    print_patient_row(patient);
    print_table_border(patient_table_width(), '=');
    printf("修改时直接按回车可保留原值，输入 b 可取消修改。\n");
    old_patient = *patient;
    /* 修改中断时回滚原资料，再继续做证件号重复校验*/
    if (!fill_patient_form(patient)) {
        *patient = old_patient;
        printf("患者信息修改已取消。\n");
        return;
    }
    duplicate = find_patient_by_id_card(system, patient->id_card);
    if (duplicate != NULL && duplicate->patient_id != patient->patient_id) {
        *patient = old_patient;
        printf("系统中已存在相同证件号的其他患者，本次修改已取消。\n");
        return;
    }
    append_log(system, admin_log_role(system), "修改患者", patient->name);
    printf("患者信息修改成功。\n");
}

/* 交互式删除患者。
 * 如果患者仍在住院，则不允许删除，防止住院记录失去主体对象。
 */
void delete_patient_interactive(HospitalSystem *system) {
    int patient_id = read_int("请输入要删除的患者编号: ");
    Patient *patient = find_patient_by_id(system, patient_id);
    if (patient == NULL) {
        printf("未找到对应患者。\n");
        return;
    }
    if (patient->is_inpatient == 1 || find_active_admission_by_patient(system, patient_id) != NULL) {
        printf("该患者当前正在住院，不能删除。\n");
        return;
    }
    patient->is_deleted = 1;
    append_log(system, admin_log_role(system), "删除患者", patient->name);
    printf("患者已逻辑删除。\n");
}

/* 患者查询入口：支持编号、姓名、电话、身份证等多方式模糊查询。 */
void query_patient_interactive(HospitalSystem *system) {
    char keyword[NAME_LEN];
    int found = 0;
    Patient *current = system->patients;
    print_title("患者查询");
    if (system->current_role == ROLE_PATIENT) {
        current = find_patient_by_id(system, system->current_patient_id);
        if (current == NULL) {
            printf("当前患者身份无效。\n");
            return;
        }
        print_patient_table_header();
        print_patient_row(current);
        print_table_border(patient_table_width(), '=');
        return;
    }
    read_string("请输入患者编号/姓名/电话/身份证关键字: ", keyword, NAME_LEN);
    print_patient_table_header();
    while (current != NULL) {
        char id_text[32];
        char age_text[32];
        snprintf(id_text, sizeof(id_text), "%d", current->patient_id);
        snprintf(age_text, sizeof(age_text), "%d", current->age);
        if (current->is_deleted == 0 &&
            (str_contains_ignore_case(id_text, keyword) ||
             str_contains_ignore_case(current->name, keyword) ||
             str_contains_ignore_case(current->gender, keyword) ||
             str_contains_ignore_case(age_text, keyword) ||
             str_contains_ignore_case(current->phone, keyword) ||
             str_contains_ignore_case(current->id_card, keyword) ||
             str_contains_ignore_case(current->blood_type, keyword)) &&
            (system->current_role != ROLE_DOCTOR || doctor_can_access_patient(system, current->patient_id)) &&
            (system->current_role != ROLE_NURSE || nurse_can_access_patient(system, current->patient_id))) {
            print_patient_row(current);
            found = 1;
        }
        current = current->next;
    }
    print_table_border(patient_table_width(), '=');
    if (!found) {
        printf("未找到匹配的患者。\n");
    }
}

/* 列出全部有效患者*/
void list_patients(HospitalSystem *system) {
    Patient *current = system->patients;
    int found = 0;
    print_title("患者列表");
    print_patient_table_header();
    while (current != NULL) {
        if (current->is_deleted == 0) {
            print_patient_row(current);
            found = 1;
        }
        current = current->next;
    }
    print_table_border(patient_table_width(), '=');
    if (!found) {
        printf("当前暂无患者数据。\n");
    }
}

/* 列出全部当前住院中的患者*/
void list_inpatients(HospitalSystem *system) {
    Patient *current = system->patients;
    int found = 0;
    print_title("当前住院患者");
    print_inpatient_table_header();
    while (current != NULL) {
        if (current->is_deleted == 0 && current->is_inpatient == 1) {
            print_inpatient_row(current);
            found = 1;
        }
        current = current->next;
    }
    print_table_border(inpatient_table_width(), '=');
    if (!found) {
        printf("当前没有住院患者。\n");
    }
}

#endif
