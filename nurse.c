/*
 * nurse.c
 * ------------------------------------------------------------
 * 护士管理模块
 * 负责护士主数据维护，以及护士角色下的住院患者与病房查看能力
 */

#include "system.h"

#if defined(HIS_BUILD_FROM_MAIN) || !HIS_MAIN_ONE_CLICK_BUILD

/* 按护士编号查找有效护士 */
Nurse *find_nurse_by_id(HospitalSystem *system, int nurse_id) {
    Nurse *current = system->nurses;
    while (current != NULL) {
        if (current->nurse_id == nurse_id && current->is_deleted == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

#define NURSE_COL_ID 8
#define NURSE_COL_NAME 12
#define NURSE_COL_GENDER 6
#define NURSE_COL_TITLE 12
#define NURSE_COL_DEPT 12
#define NURSE_COL_PHONE 14

/* 根据是否显示科室列，动态计算护士表格宽度 */
static int nurse_table_width(int include_department) {
    return NURSE_COL_ID + NURSE_COL_NAME + NURSE_COL_GENDER + NURSE_COL_TITLE +
           (include_department ? NURSE_COL_DEPT : 0) + NURSE_COL_PHONE +
           (include_department ? 3 * 6 + 1 : 3 * 5 + 1);
}

/* 打印护士表头 */
static void print_nurse_header(int include_department) {
    char id[32], name[32], gender[32], title[32], dept[32], phone[32];
    print_table_border(nurse_table_width(include_department), '=');
    format_display_cell("编号", id, (int)sizeof(id), NURSE_COL_ID);
    format_display_cell("姓名", name, (int)sizeof(name), NURSE_COL_NAME);
    format_display_cell("性别", gender, (int)sizeof(gender), NURSE_COL_GENDER);
    format_display_cell("职称", title, (int)sizeof(title), NURSE_COL_TITLE);
    format_display_cell("电话", phone, (int)sizeof(phone), NURSE_COL_PHONE);
    if (include_department) {
        format_display_cell("科室", dept, (int)sizeof(dept), NURSE_COL_DEPT);
        printf("| %s | %s | %s | %s | %s | %s |\n",
               id, name, gender, title, dept, phone);
    } else {
        printf("| %s | %s | %s | %s | %s |\n",
               id, name, gender, title, phone);
    }
    print_table_border(nurse_table_width(include_department), '-');
}

/* 打印单行护士数据 */
static void print_nurse_row(const Nurse *nurse, const char *department_name, int include_department) {
    char raw_id[32], id[32], name[NAME_LEN * 2], gender[32], title[NAME_LEN * 2];
    char dept[NAME_LEN * 2], phone[40];
    if (nurse == NULL) {
        return;
    }
    snprintf(raw_id, sizeof(raw_id), "%d", nurse->nurse_id);
    format_display_cell(raw_id, id, (int)sizeof(id), NURSE_COL_ID);
    format_display_cell(nurse->name, name, (int)sizeof(name), NURSE_COL_NAME);
    format_display_cell(nurse->gender, gender, (int)sizeof(gender), NURSE_COL_GENDER);
    format_display_cell(nurse->title, title, (int)sizeof(title), NURSE_COL_TITLE);
    format_display_cell(nurse->phone, phone, (int)sizeof(phone), NURSE_COL_PHONE);
    if (include_department) {
        format_display_cell(department_name != NULL ? department_name : "未知", dept, (int)sizeof(dept), NURSE_COL_DEPT);
        printf("| %s | %s | %s | %s | %s | %s |\n",
               id, name, gender, title, dept, phone);
    } else {
        printf("| %s | %s | %s | %s | %s |\n",
               id, name, gender, title, phone);
    }
}

/* 统计某个科室下当前有效护士人数 */
int count_nurses_in_department(HospitalSystem *system, int department_id) {
    int count = 0;
    Nurse *current = system->nurses;
    while (current != NULL) {
        if (current->department_id == department_id && current->is_deleted == 0) {
            count++;
        }
        current = current->next;
    }
    return count;
}

/* 向护士链表尾部追加护士节点 */
void add_nurse(HospitalSystem *system, Nurse nurse) {
    Nurse *node = (Nurse *)malloc(sizeof(Nurse));
    Nurse *tail = NULL;
    if (node == NULL) {
        return;
    }
    *node = nurse;
    node->next = NULL;
    if (system->nurses == NULL) {
        system->nurses = node;
    } else {
        tail = system->nurses;
        while (tail->next != NULL) {
            tail = tail->next;
        }
        tail->next = node;
    }
}

/* 交互式新增护士 */
void add_nurse_interactive(HospitalSystem *system) {
    Nurse nurse;
    memset(&nurse, 0, sizeof(nurse));
    nurse.nurse_id = system->next_nurse_id++;
    /* 表单取消时回收预分配编号，避免生成空护士资料 */
    if (!fill_nurse_form(system, &nurse)) {
        system->next_nurse_id--;
        printf("新增护士已取消\n");
        return;
    }
    generate_initial_password("N", nurse.nurse_id, nurse.password, PASSWORD_LEN);
    nurse.must_change_password = 1;
    nurse.is_deleted = 0;
    add_nurse(system, nurse);
    append_log(system, admin_log_role(system), "新增护士", nurse.name);
    printf("新增护士成功，护士编号为 %d\n", nurse.nurse_id);
    printf("系统生成初始密码：%s，请提醒护士首次登录后修改。\n", nurse.password);
}

/* 修改护士信息 */
void modify_nurse_interactive(HospitalSystem *system) {
    int nurse_id = 0;
    list_nurses(system);
    if (read_int_step("请输入要修改的护士编号（输入 b 返回上一级）: ", &nurse_id, 1, 999999, 1) == -1) {
        printf("护士信息修改已取消\n");
        return;
    }
    Nurse *nurse = find_nurse_by_id(system, nurse_id);
    Nurse old_nurse;
    Department *department = NULL;
    if (nurse == NULL) {
        printf("未找到对应护士\n");
        return;
    }
    department = find_department_by_id(system, nurse->department_id);
    print_title("当前护士信息");
    print_nurse_header(1);
    print_nurse_row(nurse, department != NULL ? department->name : "未知", 1);
    print_table_border(nurse_table_width(1), '=');
    printf("修改时直接按回车可保留原值，输入 b 可取消修改\n");
    old_nurse = *nurse;
    /* 修改中断时恢复原护士资料，避免留下半更新数据 */
    if (!fill_nurse_form(system, nurse)) {
        *nurse = old_nurse;
        printf("护士信息修改已取消\n");
        return;
    }
    append_log(system, admin_log_role(system), "修改护士", nurse->name);
    printf("护士信息修改成功\n");
}

/* 删除护士，逻辑删除 */
void delete_nurse_interactive(HospitalSystem *system) {
    int nurse_id = read_int("请输入要删除的护士编号: ");
    Nurse *nurse = find_nurse_by_id(system, nurse_id);
    if (nurse == NULL) {
        printf("未找到对应护士\n");
        return;
    }
    nurse->is_deleted = 1;
    append_log(system, admin_log_role(system), "删除护士", nurse->name);
    printf("护士已逻辑删除\n");
}

/* 多方式模糊查询护士 */
void query_nurse_interactive(HospitalSystem *system) {
    char keyword[NAME_LEN];
    Nurse *current = system->nurses;
    int found = 0;
    read_string("请输入护士编号/姓名/职称/科室/电话关键字: ", keyword, NAME_LEN);
    print_nurse_header(1);
    while (current != NULL) {
        char id_text[32];
        Department *department = find_department_by_id(system, current->department_id);
        snprintf(id_text, sizeof(id_text), "%d", current->nurse_id);
        if (current->is_deleted == 0 &&
            (str_contains_ignore_case(id_text, keyword) ||
             str_contains_ignore_case(current->name, keyword) ||
             str_contains_ignore_case(current->gender, keyword) ||
             str_contains_ignore_case(current->title, keyword) ||
             str_contains_ignore_case(current->phone, keyword) ||
             (department != NULL && str_contains_ignore_case(department->name, keyword)))) {
            print_nurse_row(current, department != NULL ? department->name : "未知", 1);
            found = 1;
        }
        current = current->next;
    }
    print_table_border(nurse_table_width(1), '=');
    if (!found) {
        printf("未找到匹配的护士\n");
    }
}

/* 列出全部有效护士 */
void list_nurses(HospitalSystem *system) {
    Nurse *current = system->nurses;
    int found = 0;
    print_title("护士列表");
    print_nurse_header(1);
    while (current != NULL) {
        if (current->is_deleted == 0) {
            Department *department = find_department_by_id(system, current->department_id);
            print_nurse_row(current, department != NULL ? department->name : "未知", 1);
            found = 1;
        }
        current = current->next;
    }
    print_table_border(nurse_table_width(1), '=');
    if (!found) {
        printf("当前暂无护士数据\n");
    }
}

/* 按科室列出护士 */
void list_nurses_by_department(HospitalSystem *system, int department_id) {
    Nurse *current = system->nurses;
    int found = 0;
    print_nurse_header(0);
    while (current != NULL) {
        if (current->department_id == department_id && current->is_deleted == 0) {
            print_nurse_row(current, NULL, 0);
            found = 1;
        }
        current = current->next;
    }
    print_table_border(nurse_table_width(0), '=');
    if (!found) {
        printf("该科室下暂无护士数据\n");
    }
}

#define NURSE_INP_COL_ADMISSION 10
#define NURSE_INP_COL_PATIENT 8
#define NURSE_INP_COL_NAME 12
#define NURSE_INP_COL_GENDER 6
#define NURSE_INP_COL_AGE 6
#define NURSE_INP_COL_BED 8
#define NURSE_INP_COL_PHONE 14

/* 当前护士本科室在院患者表格总宽度 */
static int nurse_inpatient_table_width(void) {
    return NURSE_INP_COL_ADMISSION + NURSE_INP_COL_PATIENT + NURSE_INP_COL_NAME +
           NURSE_INP_COL_GENDER + NURSE_INP_COL_AGE + NURSE_INP_COL_BED +
           NURSE_INP_COL_PHONE + 3 * 7 + 1;
}

/* 打印当前护士本科室在院患者表头 */
static void print_nurse_inpatient_header(void) {
    char admission[32], patient[32], name[32], gender[32], age[32], bed[32], phone[32];
    print_table_border(nurse_inpatient_table_width(), '=');
    format_display_cell("住院ID", admission, (int)sizeof(admission), NURSE_INP_COL_ADMISSION);
    format_display_cell("患者ID", patient, (int)sizeof(patient), NURSE_INP_COL_PATIENT);
    format_display_cell("姓名", name, (int)sizeof(name), NURSE_INP_COL_NAME);
    format_display_cell("性别", gender, (int)sizeof(gender), NURSE_INP_COL_GENDER);
    format_display_cell("年龄", age, (int)sizeof(age), NURSE_INP_COL_AGE);
    format_display_cell("床位", bed, (int)sizeof(bed), NURSE_INP_COL_BED);
    format_display_cell("电话", phone, (int)sizeof(phone), NURSE_INP_COL_PHONE);
    printf("| %s | %s | %s | %s | %s | %s | %s |\n",
           admission, patient, name, gender, age, bed, phone);
    print_table_border(nurse_inpatient_table_width(), '-');
}

/* 打印当前护士本科室在院患者单行数据 */
static void print_nurse_inpatient_row(const AdmissionHistory *admission, const Patient *patient) {
    char raw_admission[32], admission_id[32], raw_patient[32], patient_id[32], name[NAME_LEN * 2];
    char gender[32], raw_age[32], age[32], raw_bed[32], bed[32], phone[40];
    if (admission == NULL || patient == NULL) {
        return;
    }
    snprintf(raw_admission, sizeof(raw_admission), "%d", admission->admission_id);
    snprintf(raw_patient, sizeof(raw_patient), "%d", patient->patient_id);
    snprintf(raw_age, sizeof(raw_age), "%d", patient->age);
    snprintf(raw_bed, sizeof(raw_bed), "%d", admission->bed_id);
    format_display_cell(raw_admission, admission_id, (int)sizeof(admission_id), NURSE_INP_COL_ADMISSION);
    format_display_cell(raw_patient, patient_id, (int)sizeof(patient_id), NURSE_INP_COL_PATIENT);
    format_display_cell(patient->name, name, (int)sizeof(name), NURSE_INP_COL_NAME);
    format_display_cell(patient->gender, gender, (int)sizeof(gender), NURSE_INP_COL_GENDER);
    format_display_cell(raw_age, age, (int)sizeof(age), NURSE_INP_COL_AGE);
    format_display_cell(raw_bed, bed, (int)sizeof(bed), NURSE_INP_COL_BED);
    format_display_cell(patient->phone, phone, (int)sizeof(phone), NURSE_INP_COL_PHONE);
    printf("| %s | %s | %s | %s | %s | %s | %s |\n",
           admission_id, patient_id, name, gender, age, bed, phone);
}

/* 列出当前护士所在科室的在院患者 */
void list_current_nurse_inpatients(HospitalSystem *system) {
    Nurse *nurse = get_current_nurse(system);
    AdmissionHistory *admission = NULL;
    int found = 0;
    if (nurse == NULL) {
        printf("当前护士身份无效，请重新登录\n");
        return;
    }
    print_title("本科室在院患者");
    print_nurse_inpatient_header();
    for (admission = system->admissions; admission != NULL; admission = admission->next) {
        Patient *patient = NULL;
        if (admission->department_id != nurse->department_id || admission->status != ADMISSION_ACTIVE) {
            continue;
        }
        patient = find_patient_by_id(system, admission->patient_id);
        if (patient == NULL) {
            continue;
        }
        print_nurse_inpatient_row(admission, patient);
        found = 1;
    }
    print_table_border(nurse_inpatient_table_width(), '=');
    if (!found) {
        printf("当前本科室没有在院患者\n");
    }
}

#define NURSE_ADM_COL_ID 10
#define NURSE_ADM_COL_PATIENT 8
#define NURSE_ADM_COL_DOCTOR 8
#define NURSE_ADM_COL_WARD 8
#define NURSE_ADM_COL_BED 8
#define NURSE_ADM_COL_STATUS 10
#define NURSE_ADM_COL_DIAG 20

/* 当前护士本科室住院记录表格总宽度 */
static int nurse_admission_table_width(void) {
    return NURSE_ADM_COL_ID + NURSE_ADM_COL_PATIENT + NURSE_ADM_COL_DOCTOR +
           NURSE_ADM_COL_WARD + NURSE_ADM_COL_BED + NURSE_ADM_COL_STATUS +
           NURSE_ADM_COL_DIAG + 3 * 7 + 1;
}

/* 打印当前护士本科室住院记录表头 */
static void print_nurse_admission_header(void) {
    char admission[32], patient[32], doctor[32], ward[32], bed[32], status[32], diagnosis[48];
    print_table_border(nurse_admission_table_width(), '=');
    format_display_cell("住院ID", admission, (int)sizeof(admission), NURSE_ADM_COL_ID);
    format_display_cell("患者", patient, (int)sizeof(patient), NURSE_ADM_COL_PATIENT);
    format_display_cell("医生", doctor, (int)sizeof(doctor), NURSE_ADM_COL_DOCTOR);
    format_display_cell("病房", ward, (int)sizeof(ward), NURSE_ADM_COL_WARD);
    format_display_cell("床位", bed, (int)sizeof(bed), NURSE_ADM_COL_BED);
    format_display_cell("状态", status, (int)sizeof(status), NURSE_ADM_COL_STATUS);
    format_display_cell("诊断", diagnosis, (int)sizeof(diagnosis), NURSE_ADM_COL_DIAG);
    printf("| %s | %s | %s | %s | %s | %s | %s |\n",
           admission, patient, doctor, ward, bed, status, diagnosis);
    print_table_border(nurse_admission_table_width(), '-');
}

/* 打印当前护士本科室住院记录单行数据 */
static void print_nurse_admission_row(const AdmissionHistory *admission) {
    char raw_admission[32], admission_id[32], raw_patient[32], patient[32], raw_doctor[32], doctor[32];
    char raw_ward[32], ward[32], raw_bed[32], bed[32], status[32], diagnosis[TEXT_LEN * 2];
    if (admission == NULL) {
        return;
    }
    snprintf(raw_admission, sizeof(raw_admission), "%d", admission->admission_id);
    snprintf(raw_patient, sizeof(raw_patient), "%d", admission->patient_id);
    snprintf(raw_doctor, sizeof(raw_doctor), "%d", admission->doctor_id);
    snprintf(raw_ward, sizeof(raw_ward), "%d", admission->ward_id);
    snprintf(raw_bed, sizeof(raw_bed), "%d", admission->bed_id);
    format_display_cell(raw_admission, admission_id, (int)sizeof(admission_id), NURSE_ADM_COL_ID);
    format_display_cell(raw_patient, patient, (int)sizeof(patient), NURSE_ADM_COL_PATIENT);
    format_display_cell(raw_doctor, doctor, (int)sizeof(doctor), NURSE_ADM_COL_DOCTOR);
    format_display_cell(raw_ward, ward, (int)sizeof(ward), NURSE_ADM_COL_WARD);
    format_display_cell(raw_bed, bed, (int)sizeof(bed), NURSE_ADM_COL_BED);
    format_display_cell(admission->status == ADMISSION_ACTIVE ? "住院中" : "已出院", status, (int)sizeof(status), NURSE_ADM_COL_STATUS);
    format_display_cell(admission->diagnosis, diagnosis, (int)sizeof(diagnosis), NURSE_ADM_COL_DIAG);
    printf("| %s | %s | %s | %s | %s | %s | %s |\n",
           admission_id, patient, doctor, ward, bed, status, diagnosis);
}

/* 列出当前护士所在科室的住院记录 */
void list_current_nurse_admissions(HospitalSystem *system) {
    Nurse *nurse = get_current_nurse(system);
    AdmissionHistory *admission = NULL;
    int found = 0;
    if (nurse == NULL) {
        printf("当前护士身份无效，请重新登录\n");
        return;
    }
    print_title("本科室住院记录");
    print_nurse_admission_header();
    for (admission = system->admissions; admission != NULL; admission = admission->next) {
        if (admission->department_id != nurse->department_id) {
            continue;
        }
        print_nurse_admission_row(admission);
        found = 1;
    }
    print_table_border(nurse_admission_table_width(), '=');
    if (!found) {
        printf("当前本科室暂无住院记录\n");
    }
}

#endif
