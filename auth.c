/* 认证与分步输入模块
 * 负责登录身份识别、当前会话管理、带返回功能的表单录入
 * 医生对科室、患者、住院记录的访问权限判断
 */

#include "system.h"

#if defined(HIS_BUILD_FROM_MAIN) || !HIS_MAIN_ONE_CLICK_BUILD


/* 重置当前登录状态 */
void reset_current_session(HospitalSystem *system) {
    if (system == NULL) {
        return;
    }
    system->current_role = ROLE_NONE;
    system->current_admin_type = ADMIN_NONE;
    system->current_doctor_id = 0;
    system->current_nurse_id = 0;
    system->current_pharmacy_staff_id = 0;
    system->current_patient_id = 0;
}

/* 获取当前登录医生信息*/
Doctor *get_current_doctor(HospitalSystem *system) {
    if (system == NULL || system->current_role != ROLE_DOCTOR) {
        return NULL;
    }
    return find_doctor_by_id(system, system->current_doctor_id);
}

/* 获取当前登录护士信息*/
Nurse *get_current_nurse(HospitalSystem *system) {
    if (system == NULL || system->current_role != ROLE_NURSE) {
        return NULL;
    }
    return find_nurse_by_id(system, system->current_nurse_id);
}

/* 获取当前登录药房工作人员信息*/
PharmacyStaff *get_current_pharmacy_staff(HospitalSystem *system) {
    if (system == NULL || system->current_role != ROLE_PHARMACY) {
        return NULL;
    }
    return find_pharmacy_staff_by_id(system, system->current_pharmacy_staff_id);
}

/* 获取当前登录患者信息*/
Patient *get_current_patient(HospitalSystem *system) {
    if (system == NULL || system->current_role != ROLE_PATIENT) {
        return NULL;
    }
    return find_patient_by_id(system, system->current_patient_id);
}

static int password_is_usable(const char *password) {
    return password != NULL && strlen(password) >= 6 && strlen(password) < PASSWORD_LEN;
}

static int read_new_password(char *password, int size) {
    char confirm[PASSWORD_LEN];
    while (1) {
        password[0] = '\0';
        confirm[0] = '\0';
        if (read_string_step("请输入新密码（至少 6 位，输入 b 取消）: ", password, size, 1) == -1) {
            return 0;
        }
        if (!password_is_usable(password)) {
            printf("密码长度需为 6 到 %d 位。\n", PASSWORD_LEN - 1);
            continue;
        }
        if (read_string_step("请再次输入新密码: ", confirm, PASSWORD_LEN, 0) == -1) {
            return 0;
        }
        if (strcmp(password, confirm) != 0) {
            printf("两次输入不一致，请重新设置。\n");
            continue;
        }
        return 1;
    }
}

void change_current_password_interactive(HospitalSystem *system) {
    char old_password[PASSWORD_LEN] = "";
    char new_password[PASSWORD_LEN] = "";
    char *password = NULL;
    int *must_change = NULL;
    if (system == NULL) {
        return;
    }
    if (system->current_role == ROLE_DOCTOR) {
        Doctor *doctor = get_current_doctor(system);
        if (doctor != NULL) {
            password = doctor->password;
            must_change = &doctor->must_change_password;
        }
    } else if (system->current_role == ROLE_NURSE) {
        Nurse *nurse = get_current_nurse(system);
        if (nurse != NULL) {
            password = nurse->password;
            must_change = &nurse->must_change_password;
        }
    } else if (system->current_role == ROLE_PHARMACY) {
        PharmacyStaff *staff = get_current_pharmacy_staff(system);
        if (staff != NULL) {
            password = staff->password;
            must_change = &staff->must_change_password;
        }
    } else if (system->current_role == ROLE_PATIENT) {
        Patient *patient = get_current_patient(system);
        if (patient != NULL) {
            password = patient->password;
            must_change = &patient->must_change_password;
        }
    }
    if (password == NULL || must_change == NULL) {
        printf("当前身份不支持修改密码。\n");
        return;
    }
    if (*must_change == 0) {
        if (read_string_step("请输入原密码（输入 b 取消）: ", old_password, PASSWORD_LEN, 1) == -1) {
            printf("密码修改已取消。\n");
            return;
        }
        if (strcmp(old_password, password) != 0) {
            printf("原密码错误。\n");
            return;
        }
    } else {
        printf("当前为初始密码，请先设置新密码。\n");
    }
    if (!read_new_password(new_password, PASSWORD_LEN)) {
        printf("密码修改已取消。\n");
        return;
    }
    safe_copy(password, new_password, PASSWORD_LEN);
    *must_change = 0;
    printf("密码修改成功。\n");
}

static int finish_initial_password_change(HospitalSystem *system) {
    if (system == NULL) {
        return 0;
    }
    if ((system->current_role == ROLE_DOCTOR && get_current_doctor(system) != NULL && get_current_doctor(system)->must_change_password) ||
        (system->current_role == ROLE_NURSE && get_current_nurse(system) != NULL && get_current_nurse(system)->must_change_password) ||
        (system->current_role == ROLE_PHARMACY && get_current_pharmacy_staff(system) != NULL && get_current_pharmacy_staff(system)->must_change_password) ||
        (system->current_role == ROLE_PATIENT && get_current_patient(system) != NULL && get_current_patient(system)->must_change_password)) {
        change_current_password_interactive(system);
    }
    if (system->current_role == ROLE_DOCTOR && get_current_doctor(system) != NULL) {
        return get_current_doctor(system)->must_change_password == 0;
    }
    if (system->current_role == ROLE_NURSE && get_current_nurse(system) != NULL) {
        return get_current_nurse(system)->must_change_password == 0;
    }
    if (system->current_role == ROLE_PHARMACY && get_current_pharmacy_staff(system) != NULL) {
        return get_current_pharmacy_staff(system)->must_change_password == 0;
    }
    if (system->current_role == ROLE_PATIENT && get_current_patient(system) != NULL) {
        return get_current_patient(system)->must_change_password == 0;
    }
    return 0;
}

const char *admin_type_to_text(int admin_type) {
    switch (admin_type) {
    case ADMIN_SUPER:
        return "总管理员";
    case ADMIN_STAFF_DATA:
        return "医护资料管理员";
    case ADMIN_RESOURCE:
        return "科室资源管理员";
    case ADMIN_MEDICAL_BUSINESS:
        return "医疗业务管理员";
    default:
        return "未知管理员";
    }
}

const char *admin_log_role(HospitalSystem *system) {
    if (system != NULL && system->current_role == ROLE_ADMIN) {
        return admin_type_to_text(system->current_admin_type);
    }
    return "管理员";
}

static const char *admin_type_password(int admin_type) {
    switch (admin_type) {
    case ADMIN_SUPER:
        return "0001";
    case ADMIN_STAFF_DATA:
        return "1001";
    case ADMIN_RESOURCE:
        return "1003";
    case ADMIN_MEDICAL_BUSINESS:
        return "1004";
    default:
        return NULL;
    }
}

static int read_admin_type(void) {
    int choice = -1;
    print_title("管理员身份选择");
    printf("1. 总管理员\n");
    printf("2. 医护资料管理员\n");
    printf("3. 科室资源管理员\n");
    printf("4. 医疗业务管理员\n");
    printf("0. 返回上一级\n");
    if (read_int_step("请选择管理员身份（输入 b 返回上一级）: ", &choice, 0, 4, 1) == -1) {
        return 0;
    }
    return choice;
}

/* 管理员认证：
 * 先选择管理员类型，再校验该类型对应的密码
 */
int authenticate_admin(HospitalSystem *system) {
    char password[32] = "";
    char prompt[80];
    int admin_type = 0;
    int try_count = 0;
    const char *expected_password = NULL;

    if (system == NULL) {
        return 0;
    }

    admin_type = read_admin_type();
    if (admin_type == ADMIN_NONE) {
        return 0;
    }

    expected_password = admin_type_password(admin_type);
    if (expected_password == NULL) {
        printf("管理员身份无效。\n");
        return 0;
    }

    snprintf(prompt, sizeof(prompt), "请输入%s密码（输入 b 返回上一级）: ", admin_type_to_text(admin_type));
    while (try_count < 3) {
        if (read_string_step(prompt, password, (int)sizeof(password), 1) == -1) {
            printf("管理员登录已取消。\n");
            return 0;
        }
        if (strcmp(password, expected_password) == 0) {
            system->current_role = ROLE_ADMIN;
            system->current_admin_type = admin_type;
            printf("%s身份验证通过。\n", admin_type_to_text(admin_type));
            return 1;
        }
        try_count++;
        printf("%s密码错误，请重试！\n", admin_type_to_text(admin_type));
    }
    return 0;
}

/* 医生认证：
 * 根据医生编号确认医生身份，并把当前会话切换为医生角色
 */
int authenticate_doctor(HospitalSystem *system) {
    int doctor_id = 0;
    char password[PASSWORD_LEN] = "";
    if (read_int_step("请输入医生编号进行身份验证（输入 b 返回上一级）: ", &doctor_id, 1, 999999, 1) == -1) {
        printf("医生登录已取消。\n");
        return 0;
    }
    Doctor *doctor = find_doctor_by_id(system, doctor_id);
    if (doctor == NULL) {
        printf("医生编号不存在，请重新输入！\n");
        return 0;
    }
    if (read_string_step("请输入登录密码（输入 b 返回上一级）: ", password, PASSWORD_LEN, 1) == -1) {
        printf("医生登录已取消。\n");
        return 0;
    }
    if (strcmp(password, doctor->password) != 0) {
        printf("密码错误，请重新登录！\n");
        return 0;
    }
    system->current_role = ROLE_DOCTOR;
    system->current_doctor_id = doctor_id;
    printf("医生身份验证通过，欢迎 %s\n", doctor->name);
    return finish_initial_password_change(system);
}

/* 护士认证：
 * 根据护士编号确认护士身份，并把当前会话切换为护士角色
 */
int authenticate_nurse(HospitalSystem *system) {
    int nurse_id = 0;
    char password[PASSWORD_LEN] = "";
    if (read_int_step("请输入护士编号进行身份验证（输入 b 返回上一级）: ", &nurse_id, 1, 999999, 1) == -1) {
        printf("护士登录已取消。\n");
        return 0;
    }
    Nurse *nurse = find_nurse_by_id(system, nurse_id);
    if (nurse == NULL) {
        printf("护士编号不存在，请重新输入！\n");
        return 0;
    }
    if (read_string_step("请输入登录密码（输入 b 返回上一级）: ", password, PASSWORD_LEN, 1) == -1) {
        printf("护士登录已取消。\n");
        return 0;
    }
    if (strcmp(password, nurse->password) != 0) {
        printf("密码错误，请重新登录！\n");
        return 0;
    }
    system->current_role = ROLE_NURSE;
    system->current_nurse_id = nurse_id;
    printf("护士身份验证通过，欢迎 %s\n", nurse->name);
    return finish_initial_password_change(system);
}

/* 药房工作人员认证：
 * 根据药房工作人员编号确认身份，并把当前会话切换为药房工作人员角色
 */
int authenticate_pharmacy_staff(HospitalSystem *system) {
    int staff_id = 0;
    char password[PASSWORD_LEN] = "";
    if (read_int_step("请输入药房工作人员编号进行身份验证（输入 b 返回上一级）: ", &staff_id, 1, 999999, 1) == -1) {
        printf("药房工作人员登录已取消。\n");
        return 0;
    }
    PharmacyStaff *staff = find_pharmacy_staff_by_id(system, staff_id);
    if (staff == NULL) {
        printf("药房工作人员编号不存在或已停用，请重新输入！\n");
        return 0;
    }
    if (read_string_step("请输入登录密码（输入 b 返回上一级）: ", password, PASSWORD_LEN, 1) == -1) {
        printf("药房工作人员登录已取消。\n");
        return 0;
    }
    if (strcmp(password, staff->password) != 0) {
        printf("密码错误，请重新登录！\n");
        return 0;
    }
    system->current_role = ROLE_PHARMACY;
    system->current_pharmacy_staff_id = staff_id;
    printf("药房工作人员身份验证通过，欢迎 %s\n", staff->name);
    return finish_initial_password_change(system);
}

/* 患者认证：
 * 根据患者编号确认患者身份，并把当前会话切换为患者角色
 */
int authenticate_patient(HospitalSystem *system) {
    int patient_id = 0;
    char password[PASSWORD_LEN] = "";
    if (read_int_step("请输入患者编号进行身份验证（输入 b 返回上一级）: ", &patient_id, 1, 999999, 1) == -1) {
        printf("患者登录已取消。\n");
        return 0;
    }
    Patient *patient = find_patient_by_id(system, patient_id);
    if (patient == NULL) {
        printf("患者编号不存在，请重新输入！\n");
        return 0;
    }
    if (read_string_step("请输入登录密码（输入 b 返回上一级）: ", password, PASSWORD_LEN, 1) == -1) {
        printf("患者登录已取消。\n");
        return 0;
    }
    if (strcmp(password, patient->password) != 0) {
        printf("密码错误，请重新登录！\n");
        return 0;
    }
    system->current_role = ROLE_PATIENT;
    system->current_patient_id = patient_id;
    printf("患者身份验证通过，欢迎 %s\n", patient->name);
    return finish_initial_password_change(system);
}

/* 判断当前医生是否属于指定科室 */
int doctor_can_access_department(HospitalSystem *system, int department_id) {
    Doctor *doctor = get_current_doctor(system);
    return doctor != NULL && doctor->department_id == department_id;
}

/* 判断患者是否在指定科室有就诊记录*/
static int patient_has_record_in_department(HospitalSystem *system, int patient_id, int department_id) {
    MedicalRecord *current = system->records;
    while (current != NULL) {
        if (current->patient_id == patient_id && current->department_id == department_id) {
            return 1;
        }
        current = current->next;
    }
    return 0;
}

/* 判断患者是否在指定科室有住院记录 */
static int patient_has_admission_in_department(HospitalSystem *system, int patient_id, int department_id) {
    AdmissionHistory *current = system->admissions;
    while (current != NULL) {
        if (current->patient_id == patient_id && current->department_id == department_id) {
            return 1;
        }
        current = current->next;
    }
    return 0;
}

static int patient_belongs_to_doctor(HospitalSystem *system, int patient_id, int doctor_id) {
    MedicalRecord *record = NULL;
    OutpatientRegistration *registration = NULL;
    AdmissionHistory *admission = NULL;
    for (record = system->records; record != NULL; record = record->next) {
        if (record->patient_id == patient_id && record->doctor_id == doctor_id) {
            return 1;
        }
    }
    for (registration = system->registrations; registration != NULL; registration = registration->next) {
        if (registration->patient_id == patient_id && registration->doctor_id == doctor_id) {
            return 1;
        }
    }
    for (admission = system->admissions; admission != NULL; admission = admission->next) {
        if (admission->patient_id == patient_id && admission->doctor_id == doctor_id) {
            return 1;
        }
    }
    return 0;
}

/* 判断当前医生是否可访问某位患者
 * 只要这位患者在该医生所属科室有病历或住院记录，就允许查看
 */
int doctor_can_access_patient(HospitalSystem *system, int patient_id) {
    Doctor *doctor = get_current_doctor(system);
    if (doctor == NULL) {
        return 0;
    }
    return patient_has_record_in_department(system, patient_id, doctor->department_id) ||
           patient_has_admission_in_department(system, patient_id, doctor->department_id) ||
           patient_belongs_to_doctor(system, patient_id, doctor->doctor_id);
}

/* 判断当前医生是否可以操作某条住院记录 */
int doctor_can_access_admission(HospitalSystem *system, AdmissionHistory *admission) {
    Doctor *doctor = get_current_doctor(system);
    return doctor != NULL && admission != NULL && admission->department_id == doctor->department_id;
}

/* 判断当前护士是否属于指定科室 */
int nurse_can_access_department(HospitalSystem *system, int department_id) {
    Nurse *nurse = get_current_nurse(system);
    return nurse != NULL && nurse->department_id == department_id;
}

/* 判断当前护士是否可访问某位患者 */
int nurse_can_access_patient(HospitalSystem *system, int patient_id) {
    Nurse *nurse = get_current_nurse(system);
    if (nurse == NULL) {
        return 0;
    }
    return patient_has_record_in_department(system, patient_id, nurse->department_id) ||
           patient_has_admission_in_department(system, patient_id, nurse->department_id);
}

/* 判断当前护士是否可以操作某条住院记录 */
int nurse_can_access_admission(HospitalSystem *system, AdmissionHistory *admission) {
    Nurse *nurse = get_current_nurse(system);
    return nurse != NULL && admission != NULL && admission->department_id == nurse->department_id;
}

/* 患者角色查看自己的住院信息 */
void show_current_patient_admission(HospitalSystem *system) {
    Patient *patient = get_current_patient(system);
    AdmissionHistory *admission = NULL;
    if (patient == NULL) {
        printf("当前患者身份无效\n");
        return;
    }
    admission = find_active_admission_by_patient(system, patient->patient_id);
    print_title("我的住院信息");
    if (admission == NULL) {
        printf("当前暂无住院记录\n");
        return;
    }
    printf("住院编号：%d\n", admission->admission_id);
    printf("科室编号：%d\n", admission->department_id);
    printf("医生编号：%d\n", admission->doctor_id);
    printf("病房编号：%d\n", admission->ward_id);
    printf("床位编号：%d\n", admission->bed_id);
    printf("入院日期：%s\n", admission->admit_date);
    printf("住院诊断：%s\n", admission->diagnosis);
}
/* 分步填写科室表单
 * 允许在第二步和第三步输入 b 返回上一项重新修改
 */
int fill_department_form(Department *department) {
    int step = 0;
    while (step < 3) {
        if (step == 0) {
            int result = read_string_step("请输入科室名称（输入 b 取消）: ", department->name, NAME_LEN, 1);
            if (result == -1) {
                return 0;
            }
            step++;
        } else if (step == 1) {
            {
                int result = read_string_step("请输入科室负责人（输入 b 返回上一项）: ",
                                              department->manager, NAME_LEN, 1);
                if (result == -1) {
                    step--;
                } else {
                    step++;
                }
            }
        } else {
            {
                int result = read_string_step("请输入科室简介（输入 b 返回上一项）: ",
                                              department->description, TEXT_LEN, 1);
                if (result == -1) {
                    step--;
                } else {
                    step++;
                }
            }
        }
    }
    return 1;
}

/* 分步填写医生表单 */
static int select_doctor_title(char *title, int size) {
    int choice = 0;
    printf("可选职称：1. 主任医师  2. 副主任医师  3. 主治医师  4. 住院医师  5. 实习医师\n");
    if (read_int_step("请选择医生职称编号（输入 b 返回上一项）: ", &choice, 1, 5, 1) == -1) {
        return 0;
    }
    switch (choice) {
    case 1:
        safe_copy(title, "主任医师", size);
        break;
    case 2:
        safe_copy(title, "副主任医师", size);
        break;
    case 3:
        safe_copy(title, "主治医师", size);
        break;
    case 4:
        safe_copy(title, "住院医师", size);
        break;
    default:
        safe_copy(title, "实习医师", size);
        break;
    }
    return 1;
}

int fill_doctor_form(HospitalSystem *system, Doctor *doctor) {
    int step = 0;
    char gender[SMALL_LEN];
    safe_copy(gender, doctor->gender, SMALL_LEN);
    while (step < 7) {
        if (step == 0) {
            int result = read_string_step("请输入医生姓名（输入 b 取消）: ", doctor->name, NAME_LEN, 1);
            if (result == -1) {
                return 0;
            }
            step++;
        } else if (step == 1) {
            int result = read_string_step("请输入性别 男/女（输入 b 返回上一项）: ", gender, SMALL_LEN, 1);
            if (result == -1) {
                step--;
            } else if (!normalize_gender(gender, doctor->gender, SMALL_LEN)) {
                printf("性别输入不合法，只能输入 男 或 女\n");
            } else {
                step++;
            }
        } else if (step == 2) {
            int result = read_id_card_step("请输入医生身份证号（输入 b 返回上一项）: ", doctor->id_card, ID_LEN, 1);
            if (result == -1) {
                step--;
            } else {
                doctor->age = age_from_id_card(doctor->id_card);
                printf("已根据身份证号计算年龄：%d\n", doctor->age);
                step++;
            }
        } else if (step == 3) {
            if (!select_doctor_title(doctor->title, NAME_LEN)) {
                step--;
            } else {
                step++;
            }
        } else if (step == 4) {
            int department_id = doctor->department_id;
            int result = 0;
            list_departments(system);
            result = read_int_step("请输入所属科室编号（输入 b 返回上一项）: ", &department_id, 1, 999999, 1);
            if (result == -1) {
                step--;
            } else if (find_department_by_id(system, department_id) == NULL) {
                printf("科室编号不存在，请重新输入\n");
            } else {
                doctor->department_id = department_id;
                step++;
            }
        } else if (step == 5) {
            int result = read_string_step("请输入擅长方向（输入 b 返回上一项）: ", doctor->specialty, TEXT_LEN, 1);
            if (result == -1) {
                step--;
            } else {
                step++;
            }
        } else {
            int result = read_phone_step("请输入联系电话（输入 b 返回上一项）: ", doctor->phone, PHONE_LEN, 1);
            if (result == -1) {
                step--;
            } else {
                step++;
            }
        }
    }
    return 1;
}

/* 分步填写护士表单 */
int fill_nurse_form(HospitalSystem *system, Nurse *nurse) {
    int step = 0;
    char gender[SMALL_LEN];
    safe_copy(gender, nurse->gender, SMALL_LEN);
    while (step < 5) {
        if (step == 0) {
            int result = read_string_step("请输入护士姓名（输入 b 取消）: ", nurse->name, NAME_LEN, 1);
            if (result == -1) {
                return 0;
            }
            step++;
        } else if (step == 1) {
            int result = read_string_step("请输入性别 男/女（输入 b 返回上一项）: ", gender, SMALL_LEN, 1);
            if (result == -1) {
                step--;
            } else if (!normalize_gender(gender, nurse->gender, SMALL_LEN)) {
                printf("性别输入不合法，只能输入 男 或 女\n");
            } else {
                step++;
            }
        } else if (step == 2) {
            int result = read_string_step("请输入职称（输入 b 返回上一项）: ", nurse->title, NAME_LEN, 1);
            if (result == -1) {
                step--;
            } else {
                step++;
            }
        } else if (step == 3) {
            int department_id = nurse->department_id;
            int result = 0;
            list_departments(system);
            result = read_int_step("请输入所属科室编号（输入 b 返回上一项）: ", &department_id, 1, 999999, 1);
            if (result == -1) {
                step--;
            } else if (find_department_by_id(system, department_id) == NULL) {
                printf("科室编号不存在，请重新输入\n");
            } else {
                nurse->department_id = department_id;
                step++;
            }
        } else {
            int result = read_phone_step("请输入联系电话（输入 b 返回上一项）: ", nurse->phone, PHONE_LEN, 1);
            if (result == -1) {
                step--;
            } else {
                step++;
            }
        }
    }
    return 1;
}

/* 分步填写患者表单 */
int fill_patient_form(Patient *patient) {
    int step = 0;
    char gender[SMALL_LEN];
    char blood_type[SMALL_LEN];
    safe_copy(gender, patient->gender, SMALL_LEN);
    safe_copy(blood_type, patient->blood_type, SMALL_LEN);
    while (step < 5) {
        if (step == 0) {
            int result = read_string_step("请输入患者姓名（输入 b 取消）: ", patient->name, NAME_LEN, 1);
            if (result == -1) {
                return 0;
            }
            step++;
        } else if (step == 1) {
            int result = read_string_step("请输入性别 男/女（输入 b 返回上一项）: ", gender, SMALL_LEN, 1);
            if (result == -1) {
                step--;
            } else if (!normalize_gender(gender, patient->gender, SMALL_LEN)) {
                printf("性别输入不合法，只能输入 男 或 女\n");
            } else {
                step++;
            }
        } else if (step == 2) {
            int result = read_phone_step("请输入联系电话（输入 b 返回上一项）: ", patient->phone, PHONE_LEN, 1);
            if (result == -1) {
                step--;
            } else {
                step++;
            }
        } else if (step == 3) {
            int result = read_id_card_step("请输入身份证号/证件号（输入 b 返回上一项）: ", patient->id_card, ID_LEN, 1);
            if (result == -1) {
                step--;
            } else {
                patient->age = age_from_id_card(patient->id_card);
                printf("已根据身份证号计算年龄：%d\n", patient->age);
                step++;
            }
        } else {
            int result = read_string_step("请输入血型 A/B/O/AB（输入 b 返回上一项）: ", blood_type, SMALL_LEN, 1);
            if (result == -1) {
                step--;
            } else if (!normalize_blood_type(blood_type, patient->blood_type, SMALL_LEN)) {
                printf("血型输入不合法，只能输入 A、B、O、AB\n");
            } else {
                step++;
            }
        }
    }
    return 1;
}

/* 分步填写药品表单
 */
int fill_drug_form(Drug *drug) {
    int step = 0;
    int editing_existing = drug->generic_name[0] != '\0';
    while (step < 7) {
        if (step == 0) {
            int result = read_string_step("请输入通用名（输入 b 取消）: ", drug->generic_name, DRUG_NAME_LEN, 1);
            if (result == -1) {
                return 0;
            }
            step++;
        } else if (step == 1) {
            int result = read_string_step("请输入俗称（输入 b 返回上一项）: ", drug->common_name, DRUG_NAME_LEN, 1);
            if (result == -1) {
                step--;
            } else {
                step++;
            }
        } else if (step == 2) {
            int result = read_string_step("请输入品牌（输入 b 返回上一项）: ", drug->brand_name, DRUG_NAME_LEN, 1);
            if (result == -1) {
                step--;
            } else {
                step++;
            }
        } else if (step == 3) {
            int result = read_string_step("请输入生产日期 YYYY-MM-DD（输入 b 返回上一项）: ", drug->production_date, DATE_LEN, 1);
            if (result == -1) {
                step--;
            } else if (!is_valid_date_text(drug->production_date)) {
                printf("生产日期格式不合法，请按 YYYY-MM-DD 输入\n");
            } else {
                step++;
            }
        } else if (step == 4) {
            int result = read_string_step("请输入有效期至 YYYY-MM-DD（输入 b 返回上一项）: ", drug->expiry_date, DATE_LEN, 1);
            if (result == -1) {
                step--;
            } else if (!is_valid_date_text(drug->expiry_date)) {
                printf("有效期格式不合法，请按 YYYY-MM-DD 输入\n");
            } else if (date_to_serial(drug->expiry_date) <= date_to_serial(drug->production_date)) {
                printf("有效期至必须晚于生产日期\n");
            } else {
                step++;
            }
        } else if (step == 5) {
            double purchase_price = editing_existing ? drug->purchase_price : -1.0;
            int result = read_double_step("请输入进价（输入 b 返回上一项）: ", &purchase_price, 0.0, 1);
            if (result == -1) {
                step--;
            } else {
                drug->purchase_price = purchase_price;
                drug->sale_price = drug->purchase_price * DRUG_SALE_RATE;
                printf("售价已按进价的 1.3 倍自动设置为 %.2f\n", drug->sale_price);
                step++;
            }
        } else {
            int stock = editing_existing ? drug->stock : -1;
            int result = read_int_step("请输入当前库存（输入 b 返回上一项）: ", &stock, 0, 100000, 1);
            if (result == -1) {
                step--;
            } else {
                drug->stock = stock;
                step++;
            }
        }
    }
    return 1;
}




#endif
