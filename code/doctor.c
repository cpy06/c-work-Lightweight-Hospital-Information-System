/*
 * doctor.c
 * 医生管理模块
 * 负责医生主数据维护，以及按科室查看医生信息
 */

#include "system.h"

#if defined(HIS_BUILD_FROM_MAIN) || !HIS_MAIN_ONE_CLICK_BUILD

static int doctor_has_active_registration(HospitalSystem *system, int doctor_id) {
    OutpatientRegistration *registration = NULL;
    for (registration = system->registrations; registration != NULL; registration = registration->next) {
        if (registration->doctor_id == doctor_id &&
            (registration->status == REG_STATUS_WAITING || registration->status == REG_STATUS_CALLED)) {
            return 1;
        }
    }
    return 0;
}

static int doctor_has_active_admission_in_other_department(HospitalSystem *system, int doctor_id, int department_id) {
    AdmissionHistory *admission = NULL;
    for (admission = system->admissions; admission != NULL; admission = admission->next) {
        if (admission->doctor_id == doctor_id &&
            admission->status == ADMISSION_ACTIVE &&
            admission->department_id != department_id) {
            return 1;
        }
    }
    return 0;
}

/* 按医生编号查找有效医生 */
Doctor *find_doctor_by_id(HospitalSystem *system, int doctor_id) {
    Doctor *current = system->doctors;
    while (current != NULL) {
        if (current->doctor_id == doctor_id && current->is_deleted == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

static Doctor *find_doctor_by_id_card(HospitalSystem *system, const char *id_card) {
    Doctor *current = system->doctors;
    while (current != NULL) {
        if (current->is_deleted == 0 && strcmp(current->id_card, id_card) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

#define DOCTOR_COL_ID 8
#define DOCTOR_COL_NAME 12
#define DOCTOR_COL_GENDER 6
#define DOCTOR_COL_AGE 6
#define DOCTOR_COL_ID_CARD 20
#define DOCTOR_COL_TITLE 12
#define DOCTOR_COL_DEPT 12
#define DOCTOR_COL_SPECIALTY 16
#define DOCTOR_COL_PHONE 14

/* 根据是否显示科室列，动态计算医生表格宽度 */
static int doctor_table_width(int include_department) {
    return DOCTOR_COL_ID + DOCTOR_COL_NAME + DOCTOR_COL_GENDER + DOCTOR_COL_AGE + DOCTOR_COL_ID_CARD + DOCTOR_COL_TITLE +
           (include_department ? DOCTOR_COL_DEPT : 0) + DOCTOR_COL_SPECIALTY + DOCTOR_COL_PHONE +
           (include_department ? 3 * 9 + 1 : 3 * 8 + 1);
}

/* 打印医生表头 */
static void print_doctor_header(int include_department) {
    char id[32], name[32], gender[32], age[32], id_card[40], title[32], dept[32], specialty[48], phone[32];
    print_table_border(doctor_table_width(include_department), '=');
    format_display_cell("编号", id, (int)sizeof(id), DOCTOR_COL_ID);
    format_display_cell("姓名", name, (int)sizeof(name), DOCTOR_COL_NAME);
    format_display_cell("性别", gender, (int)sizeof(gender), DOCTOR_COL_GENDER);
    format_display_cell("年龄", age, (int)sizeof(age), DOCTOR_COL_AGE);
    format_display_cell("身份证号", id_card, (int)sizeof(id_card), DOCTOR_COL_ID_CARD);
    format_display_cell("职称", title, (int)sizeof(title), DOCTOR_COL_TITLE);
    format_display_cell("擅长方向", specialty, (int)sizeof(specialty), DOCTOR_COL_SPECIALTY);
    format_display_cell("电话", phone, (int)sizeof(phone), DOCTOR_COL_PHONE);
    if (include_department) {
        format_display_cell("科室", dept, (int)sizeof(dept), DOCTOR_COL_DEPT);
        printf("| %s | %s | %s | %s | %s | %s | %s | %s | %s |\n",
               id, name, gender, age, id_card, title, dept, specialty, phone);
    } else {
        printf("| %s | %s | %s | %s | %s | %s | %s | %s |\n",
               id, name, gender, age, id_card, title, specialty, phone);
    }
    print_table_border(doctor_table_width(include_department), '-');
}

/* 打印单行医生数据 */
static void print_doctor_row(const Doctor *doctor, const char *department_name, int include_department) {
    char raw_id[32], id[32], name[NAME_LEN * 2], gender[32], raw_age[32], age[32], id_card[ID_LEN * 2], title[NAME_LEN * 2];
    char dept[NAME_LEN * 2], specialty[TEXT_LEN * 2], phone[40];
    if (doctor == NULL) {
        return;
    }
    snprintf(raw_id, sizeof(raw_id), "%d", doctor->doctor_id);
    snprintf(raw_age, sizeof(raw_age), "%d", doctor->age);
    format_display_cell(raw_id, id, (int)sizeof(id), DOCTOR_COL_ID);
    format_display_cell(doctor->name, name, (int)sizeof(name), DOCTOR_COL_NAME);
    format_display_cell(doctor->gender, gender, (int)sizeof(gender), DOCTOR_COL_GENDER);
    format_display_cell(raw_age, age, (int)sizeof(age), DOCTOR_COL_AGE);
    format_display_cell(doctor->id_card, id_card, (int)sizeof(id_card), DOCTOR_COL_ID_CARD);
    format_display_cell(doctor->title, title, (int)sizeof(title), DOCTOR_COL_TITLE);
    format_display_cell(doctor->specialty, specialty, (int)sizeof(specialty), DOCTOR_COL_SPECIALTY);
    format_display_cell(doctor->phone, phone, (int)sizeof(phone), DOCTOR_COL_PHONE);
    if (include_department) {
        format_display_cell(department_name != NULL ? department_name : "未知", dept, (int)sizeof(dept), DOCTOR_COL_DEPT);
        printf("| %s | %s | %s | %s | %s | %s | %s | %s | %s |\n",
               id, name, gender, age, id_card, title, dept, specialty, phone);
    } else {
        printf("| %s | %s | %s | %s | %s | %s | %s | %s |\n",
               id, name, gender, age, id_card, title, specialty, phone);
    }
}

/* 统计某个科室下当前有效医生人数 */
int count_doctors_in_department(HospitalSystem *system, int department_id) {
    int count = 0;
    Doctor *current = system->doctors;
    while (current != NULL) {
        if (current->department_id == department_id && current->is_deleted == 0) {
            count++;
        }
        current = current->next;
    }
    return count;
}

/* 向医生链表尾部追加医生节点 */
void add_doctor(HospitalSystem *system, Doctor doctor) {
    Doctor *node = (Doctor *)malloc(sizeof(Doctor));
    Doctor *tail = NULL;
    if (node == NULL) {
        return;
    }
    *node = doctor;
    node->next = NULL;
    if (system->doctors == NULL) {
        system->doctors = node;
    } else {
        tail = system->doctors;
        while (tail->next != NULL) {
            tail = tail->next;
        }
        tail->next = node;
    }
}

/* 交互式新增医生 */
void add_doctor_interactive(HospitalSystem *system) {
    Doctor doctor;
    memset(&doctor, 0, sizeof(doctor));
    doctor.doctor_id = system->next_doctor_id++;
    /* 表单取消时回收预分配编号，避免生成空医生资料 */
    if (!fill_doctor_form(system, &doctor)) {
        system->next_doctor_id--;
        printf("新增医生已取消\n");
        return;
    }
    if (find_doctor_by_id_card(system, doctor.id_card) != NULL) {
        system->next_doctor_id--;
        printf("系统中已存在相同身份证号的医生，请先查询原有信息。\n");
        return;
    }
    generate_initial_password("D", doctor.doctor_id, doctor.password, PASSWORD_LEN);
    doctor.must_change_password = 1;
    doctor.is_deleted = 0;
    add_doctor(system, doctor);
    append_log(system, admin_log_role(system), "新增医生", doctor.name);
    printf("新增医生成功，医生编号为 %d\n", doctor.doctor_id);
    printf("系统生成初始密码：%s，请提醒医生首次登录后修改。\n", doctor.password);
}

/* 交互式修改医生信息 */
void modify_doctor_interactive(HospitalSystem *system) {
    int doctor_id = 0;
    list_doctors(system);
    if (read_int_step("请输入要修改的医生编号（输入 b 返回上一级）: ", &doctor_id, 1, 999999, 1) == -1) {
        printf("医生信息修改已取消\n");
        return;
    }
    Doctor *doctor = find_doctor_by_id(system, doctor_id);
    Doctor old_doctor;
    Department *department = NULL;
    if (doctor == NULL) {
        printf("未找到对应医生\n");
        return;
    }
    department = find_department_by_id(system, doctor->department_id);
    print_title("当前医生信息");
    print_doctor_header(1);
    print_doctor_row(doctor, department != NULL ? department->name : "未知", 1);
    print_table_border(doctor_table_width(1), '=');
    printf("修改时直接按回车可保留原值，输入 b 可取消修改\n");

    old_doctor = *doctor;
    /* 修改失败或输入结束时回滚，防止医生资料处于半更新状态 */
    if (!fill_doctor_form(system, doctor)) {
        *doctor = old_doctor;
        printf("医生信息修改已取消\n");
        return;
    }
    {
        Doctor *duplicate = find_doctor_by_id_card(system, doctor->id_card);
        if (duplicate != NULL && duplicate->doctor_id != doctor->doctor_id) {
            *doctor = old_doctor;
            printf("系统中已存在相同身份证号的其他医生，本次修改已取消\n");
            return;
        }
    }

    if (doctor_has_active_admission_in_other_department(system, doctor_id, doctor->department_id)) {
        *doctor = old_doctor;
        printf("该医生名下仍有未完成的住院记录，不能直接修改其所属科室\n");
        return;
    }

    append_log(system, admin_log_role(system), "修改医生", doctor->name);
    printf("医生信息修改成功\n");
}

/* 交互式删除医生 */
void delete_doctor_interactive(HospitalSystem *system) {
    int doctor_id = read_int("请输入要删除的医生编号: ");
    Doctor *doctor = find_doctor_by_id(system, doctor_id);
    AdmissionHistory *admission = system->admissions;
    if (doctor == NULL) {
        printf("未找到对应医生\n");
        return;
    }

    while (admission != NULL) {
        if (admission->doctor_id == doctor_id && admission->status == ADMISSION_ACTIVE) {
            printf("该医生名下仍有未出院患者，不能删除\n");
            return;
        }
        admission = admission->next;
    }

    if (doctor_has_active_registration(system, doctor_id)) {
        printf("该医生名下仍有待诊或已叫号的挂号，不能删除\n");
        return;
    }

    doctor->is_deleted = 1;
    append_log(system, admin_log_role(system), "删除医生", doctor->name);
    printf("医生已逻辑删除\n");
}

/* 多方式模糊查询医生 */
void query_doctor_interactive(HospitalSystem *system) {
    char keyword[NAME_LEN];
    Doctor *current = system->doctors;
    int found = 0;
    read_string("请输入医生编号/姓名/职称/科室/电话/身份证关键字: ", keyword, NAME_LEN);
    print_doctor_header(1);
    while (current != NULL) {
        char id_text[32];
        char age_text[32];
        Department *department = find_department_by_id(system, current->department_id);
        snprintf(id_text, sizeof(id_text), "%d", current->doctor_id);
        snprintf(age_text, sizeof(age_text), "%d", current->age);
        if (current->is_deleted == 0 &&
            (str_contains_ignore_case(id_text, keyword) ||
             str_contains_ignore_case(current->name, keyword) ||
             str_contains_ignore_case(current->gender, keyword) ||
             str_contains_ignore_case(age_text, keyword) ||
             str_contains_ignore_case(current->id_card, keyword) ||
             str_contains_ignore_case(current->title, keyword) ||
             str_contains_ignore_case(current->specialty, keyword) ||
             str_contains_ignore_case(current->phone, keyword) ||
             (department != NULL && str_contains_ignore_case(department->name, keyword)))) {
            print_doctor_row(current, department != NULL ? department->name : "未知", 1);
            found = 1;
        }
        current = current->next;
    }
    print_table_border(doctor_table_width(1), '=');
    if (!found) {
        printf("未找到匹配的医生\n");
    }
}

/* 列出全部有效医生 */
void list_doctors(HospitalSystem *system) {
    Doctor *current = system->doctors;
    int found = 0;
    print_title("医生列表");
    print_doctor_header(1);
    while (current != NULL) {
        if (current->is_deleted == 0) {
            Department *department = find_department_by_id(system, current->department_id);
            print_doctor_row(current, department != NULL ? department->name : "未知", 1);
            found = 1;
        }
        current = current->next;
    }
    print_table_border(doctor_table_width(1), '=');
    if (!found) {
        printf("当前暂无医生数据\n");
    }
}

/* 按科室列出医生 */
void list_doctors_by_department(HospitalSystem *system, int department_id) {
    Doctor *current = system->doctors;
    int found = 0;
    print_doctor_header(0);
    while (current != NULL) {
        if (current->department_id == department_id && current->is_deleted == 0) {
            print_doctor_row(current, NULL, 0);
            found = 1;
        }
        current = current->next;
    }
    print_table_border(doctor_table_width(0), '=');
    if (!found) {
        printf("该科室下暂无医生数据\n");
    }
}

#endif
