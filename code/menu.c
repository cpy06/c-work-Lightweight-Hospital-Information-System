/*
 * menu.c
 * 这里主要管菜单调度
 * 不同身份进系统以后 都是从这里分到各自入口
 */

#include "system.h"
#include "fileio.h"

#if defined(HIS_BUILD_FROM_MAIN) || !HIS_MAIN_ONE_CLICK_BUILD

static void patient_menu(HospitalSystem *system) {
    int choice = 0;
    while (1) {
        print_title("患者管理");
        printf("1. 新增患者\n");
        printf("2. 修改患者\n");
        printf("3. 删除患者\n");
        printf("4. 查询患者\n");
        printf("5. 显示所有患者\n");
        printf("0. 返回上一级\n");
        choice = read_int_in_range("请选择: ", 0, 5);
        if (choice == 0) {
            return;
        } else if (choice == 1) {
            add_patient_interactive(system);
        } else if (choice == 2) {
            modify_patient_interactive(system);
        } else if (choice == 3) {
            delete_patient_interactive(system);
        } else if (choice == 4) {
            query_patient_interactive(system);
        } else if (choice == 5) {
            list_patients(system);
        }
        pause_screen();
    }
}

static void doctor_menu(HospitalSystem *system) {
    int choice = 0;
    while (1) {
        print_title("医生管理");
        printf("1. 新增医生\n");
        printf("2. 修改医生\n");
        printf("3. 删除医生\n");
        printf("4. 查询医生\n");
        printf("5. 显示所有医生\n");
        printf("0. 返回上一级\n");
        choice = read_int_in_range("请选择: ", 0, 5);
        if (choice == 0) {
            return;
        } else if (choice == 1) {
            add_doctor_interactive(system);
        } else if (choice == 2) {
            modify_doctor_interactive(system);
        } else if (choice == 3) {
            delete_doctor_interactive(system);
        } else if (choice == 4) {
            query_doctor_interactive(system);
        } else if (choice == 5) {
            list_doctors(system);
        }
        pause_screen();
    }
}

static void nurse_menu(HospitalSystem *system) {
    int choice = 0;
    while (1) {
        print_title("护士管理");
        printf("1. 新增护士\n");
        printf("2. 修改护士\n");
        printf("3. 删除护士\n");
        printf("4. 查询护士\n");
        printf("5. 显示所有护士\n");
        printf("0. 返回上一级\n");
        choice = read_int_in_range("请选择: ", 0, 5);
        if (choice == 0) {
            return;
        } else if (choice == 1) {
            add_nurse_interactive(system);
        } else if (choice == 2) {
            modify_nurse_interactive(system);
        } else if (choice == 3) {
            delete_nurse_interactive(system);
        } else if (choice == 4) {
            query_nurse_interactive(system);
        } else if (choice == 5) {
            list_nurses(system);
        }
        pause_screen();
    }
}

static void department_menu(HospitalSystem *system) {
    int choice = 0;
    while (1) {
        print_title("科室管理");
        printf("1. 新增科室\n");
        printf("2. 修改科室\n");
        printf("3. 删除科室\n");
        printf("4. 查询科室\n");
        printf("5. 显示所有科室\n");
        printf("0. 返回上一级\n");
        choice = read_int_in_range("请选择: ", 0, 5);
        if (choice == 0) {
            return;
        } else if (choice == 1) {
            add_department_interactive(system);
        } else if (choice == 2) {
            modify_department_interactive(system);
        } else if (choice == 3) {
            delete_department_interactive(system);
        } else if (choice == 4) {
            query_department_interactive(system);
        } else if (choice == 5) {
            list_departments(system);
        }
        pause_screen();
    }
}

static void ward_menu(HospitalSystem *system) {
    int choice = 0;
    while (1) {
        print_title("病房与床位管理");
        printf("1. 新增病房\n");
        printf("2. 新增床位\n");
        printf("3. 修改床位状态\n");
        printf("4. 显示病房列表\n");
        printf("5. 显示床位列表\n");
        printf("6. 办理住院\n");
        printf("7. 办理出院\n");
        printf("8. 显示住院历史\n");
        printf("9. 停用病房\n");
        printf("10. 移动床位到新病房\n");
        printf("0. 返回上一级\n");
        choice = read_int_in_range("请选择: ", 0, 10);
        if (choice == 0) {
            return;
        } else if (choice == 1) {
            add_ward_interactive(system);
        } else if (choice == 2) {
            add_bed_interactive(system);
        } else if (choice == 3) {
            modify_bed_status_interactive(system);
        } else if (choice == 4) {
            list_wards(system);
        } else if (choice == 5) {
            list_beds(system);
        } else if (choice == 6) {
            handle_admission_interactive(system);
        } else if (choice == 7) {
            handle_discharge_interactive(system);
        } else if (choice == 8) {
            list_admissions(system);
        } else if (choice == 9) {
            deactivate_ward_interactive(system);
        } else if (choice == 10) {
            move_bed_to_ward_interactive(system);
        }
        pause_screen();
    }
}

static void drug_admin_menu(HospitalSystem *system) {
    int choice = 0;
    while (1) {
        print_title("药品基础与规则管理");
        printf("1. 新增药品\n");
        printf("2. 修改药品\n");
        printf("3. 删除药品\n");
        printf("4. 查询药品\n");
        printf("5. 显示所有药品\n");
        printf("6. 设置药品科室可用性\n");
        printf("0. 返回上一级\n");
        choice = read_int_in_range("请选择: ", 0, 6);
        if (choice == 0) {
            return;
        } else if (choice == 1) {
            add_drug_interactive(system);
        } else if (choice == 2) {
            modify_drug_interactive(system);
        } else if (choice == 3) {
            delete_drug_interactive(system);
        } else if (choice == 4) {
            query_drug_interactive(system);
        } else if (choice == 5) {
            list_drugs(system);
        } else if (choice == 6) {
            manage_drug_department_access_interactive(system);
        }
        pause_screen();
    }
}

static void record_menu(HospitalSystem *system) {
    int choice = 0;
    while (1) {
        print_title("医疗记录管理");
        printf("1. 新增医疗记录\n");
        printf("2. 显示全部记录\n");
        printf("3. 按患者查看历史\n");
        printf("4. 显示门诊挂号记录\n");
        printf("5. 取消门诊挂号\n");
        printf("6. 挂号收费异常对账\n");
        printf("0. 返回上一级\n");
        choice = read_int_in_range("请选择: ", 0, 6);
        if (choice == 0) {
            return;
        } else if (choice == 1) {
            add_record_interactive(system);
        } else if (choice == 2) {
            list_records(system);
        } else if (choice == 3) {
            patient_history_interactive(system);
        } else if (choice == 4) {
            list_outpatient_registrations(system);
        } else if (choice == 5) {
            cancel_registration_interactive(system);
        } else if (choice == 6) {
            show_registration_reconciliation_report(system);
        }
        pause_screen();
    }
}

static void query_menu(HospitalSystem *system) {
    int choice = 0;
    while (1) {
        print_title("查询与统计");
        printf("1. 按患者查询\n");
        printf("2. 按医生查询\n");
        printf("3. 按科室查询\n");
        printf("4. 按药品查询\n");
        printf("5. 医护视角统计\n");
        printf("6. 管理员视角统计\n");
        printf("7. 患者视角统计\n");
        printf("8. 挂号收费异常对账\n");
        printf("0. 返回上一级\n");
        choice = read_int_in_range("请选择: ", 0, 8);
        if (choice == 0) {
            return;
        } else if (choice == 1) {
            show_patient_query_report(system);
        } else if (choice == 2) {
            show_doctor_query_report(system);
        } else if (choice == 3) {
            show_department_query_report(system);
        } else if (choice == 4) {
            show_drug_query_report(system);
        } else if (choice == 5) {
            show_medical_statistics(system);
        } else if (choice == 6) {
            show_management_statistics(system);
        } else if (choice == 7) {
            show_patient_statistics(system);
        } else if (choice == 8) {
            show_registration_reconciliation_report(system);
        }
        pause_screen();
    }
}

static void analysis_menu(HospitalSystem *system) {
    int choice = 0;
    while (1) {
        print_title("历史数据分析与优化");
        printf("1. 科室历史住院分析\n");
        printf("2. 科室住院需求预测\n");
        printf("3. 病房优化建议\n");
        printf("4. 药品趋势分析\n");
        printf("5. 药品需求预测\n");
        printf("6. 导出分析报告\n");
        printf("0. 返回上一级\n");
        choice = read_int_in_range("请选择: ", 0, 6);
        if (choice == 0) {
            return;
        } else if (choice == 1) {
            show_department_history_analysis(system, read_int_in_range("请输入分析窗口天数: ", 3, 30));
        } else if (choice == 2) {
            show_department_forecast(system);
        } else if (choice == 3) {
            show_ward_optimization_suggestions(system, read_int_in_range("请输入分析窗口天数: ", 3, 30));
        } else if (choice == 4) {
            show_drug_trend_analysis(system, read_int_in_range("请输入分析窗口天数: ", 3, 30));
        } else if (choice == 5) {
            show_drug_forecast(system);
        } else if (choice == 6) {
            export_analysis_report_interactive(system);
        }
        pause_screen();
    }
}

static int login_menu(void) {
    print_title("轻量级医院 HIS 系统");
    printf("1. 管理员\n");
    printf("2. 医生\n");
    printf("3. 护士\n");
    printf("4. 药房工作人员\n");
    printf("5. 患者\n");
    printf("0. 退出系统\n");
    return read_int_in_range("请选择身份: ", 0, 5);
}

static int patient_entry_menu(void) {
    print_title("患者入口");
    printf("1. 已有患者登录\n");
    printf("2. 新患者自助登记\n");
    printf("0. 返回上一级\n");
    return read_int_in_range("请选择: ", 0, 2);
}

static void save_data_interactive(HospitalSystem *system) {
    if (save_all_data(system)) {
        printf("数据保存成功。\n");
    } else {
        printf("数据保存失败。\n");
    }
}

static void ward_resource_menu(HospitalSystem *system) {
    int choice = 0;
    while (1) {
        print_title("病房与床位资源管理");
        printf("1. 新增病房\n");
        printf("2. 新增床位\n");
        printf("3. 修改床位状态\n");
        printf("4. 显示病房列表\n");
        printf("5. 显示床位列表\n");
        printf("6. 停用病房\n");
        printf("7. 移动床位到新病房\n");
        printf("0. 返回上一级\n");
        choice = read_int_in_range("请选择: ", 0, 7);
        if (choice == 0) {
            return;
        } else if (choice == 1) {
            add_ward_interactive(system);
        } else if (choice == 2) {
            add_bed_interactive(system);
        } else if (choice == 3) {
            modify_bed_status_interactive(system);
        } else if (choice == 4) {
            list_wards(system);
        } else if (choice == 5) {
            list_beds(system);
        } else if (choice == 6) {
            deactivate_ward_interactive(system);
        } else if (choice == 7) {
            move_bed_to_ward_interactive(system);
        }
        pause_screen();
    }
}

static void inpatient_business_menu(HospitalSystem *system) {
    int choice = 0;
    while (1) {
        print_title("住院出院业务");
        printf("1. 办理住院\n");
        printf("2. 办理出院\n");
        printf("3. 显示住院历史\n");
        printf("0. 返回上一级\n");
        choice = read_int_in_range("请选择: ", 0, 3);
        if (choice == 0) {
            return;
        } else if (choice == 1) {
            handle_admission_interactive(system);
        } else if (choice == 2) {
            handle_discharge_interactive(system);
        } else if (choice == 3) {
            list_admissions(system);
        }
        pause_screen();
    }
}

static void pharmacy_staff_management_menu(HospitalSystem *system) {
    int choice = 0;
    while (1) {
        print_title("药房工作人员管理");
        printf("1. 新增药房工作人员\n");
        printf("2. 修改药房工作人员\n");
        printf("3. 停用药房工作人员\n");
        printf("4. 查询药房工作人员\n");
        printf("5. 显示全部药房工作人员\n");
        printf("0. 返回上一级\n");
        choice = read_int_in_range("请选择: ", 0, 5);
        if (choice == 0) {
            return;
        } else if (choice == 1) {
            add_pharmacy_staff_interactive(system);
        } else if (choice == 2) {
            modify_pharmacy_staff_interactive(system);
        } else if (choice == 3) {
            deactivate_pharmacy_staff_interactive(system);
        } else if (choice == 4) {
            query_pharmacy_staff_interactive(system);
        } else if (choice == 5) {
            list_pharmacy_staffs(system);
        }
        pause_screen();
    }
}

static void run_super_admin(HospitalSystem *system) {
    int choice = 0;
    while (1) {
        print_title("总管理员菜单");
        printf("1. 患者管理\n");
        printf("2. 医生管理\n");
        printf("3. 护士管理\n");
        printf("4. 药房工作人员管理\n");
        printf("5. 科室管理\n");
        printf("6. 病房与床位管理\n");
        printf("7. 药品基础与规则管理\n");
        printf("8. 医疗记录管理\n");
        printf("9. 查询与统计\n");
        printf("10. 历史数据分析\n");
        printf("11. 保存数据\n");
        printf("0. 退出登录\n");
        choice = read_int_in_range("请选择: ", 0, 11);
        if (choice == 0) {
            return;
        } else if (choice == 1) {
            patient_menu(system);
        } else if (choice == 2) {
            doctor_menu(system);
        } else if (choice == 3) {
            nurse_menu(system);
        } else if (choice == 4) {
            pharmacy_staff_management_menu(system);
        } else if (choice == 5) {
            department_menu(system);
        } else if (choice == 6) {
            ward_menu(system);
        } else if (choice == 7) {
            drug_admin_menu(system);
        } else if (choice == 8) {
            record_menu(system);
        } else if (choice == 9) {
            query_menu(system);
        } else if (choice == 10) {
            analysis_menu(system);
        } else if (choice == 11) {
            save_data_interactive(system);
            pause_screen();
        }
    }
}

static void run_staff_data_admin(HospitalSystem *system) {
    int choice = 0;
    while (1) {
        print_title("医护资料管理员菜单");
        printf("1. 医生管理\n");
        printf("2. 护士管理\n");
        printf("3. 药房工作人员管理\n");
        printf("4. 保存数据\n");
        printf("0. 退出登录\n");
        choice = read_int_in_range("请选择: ", 0, 4);
        if (choice == 0) {
            return;
        } else if (choice == 1) {
            doctor_menu(system);
        } else if (choice == 2) {
            nurse_menu(system);
        } else if (choice == 3) {
            pharmacy_staff_management_menu(system);
        } else if (choice == 4) {
            save_data_interactive(system);
            pause_screen();
        }
    }
}

static void run_resource_admin(HospitalSystem *system) {
    int choice = 0;
    while (1) {
        print_title("科室资源管理员菜单");
        printf("1. 科室管理\n");
        printf("2. 病房与床位资源管理\n");
        printf("3. 保存数据\n");
        printf("0. 退出登录\n");
        choice = read_int_in_range("请选择: ", 0, 3);
        if (choice == 0) {
            return;
        } else if (choice == 1) {
            department_menu(system);
        } else if (choice == 2) {
            ward_resource_menu(system);
        } else if (choice == 3) {
            save_data_interactive(system);
            pause_screen();
        }
    }
}

static void run_medical_business_admin(HospitalSystem *system) {
    int choice = 0;
    while (1) {
        print_title("医疗业务管理员菜单");
        printf("1. 患者管理\n");
        printf("2. 医疗记录管理\n");
        printf("3. 住院出院业务\n");
        printf("4. 查询与统计\n");
        printf("5. 历史数据分析\n");
        printf("6. 保存数据\n");
        printf("0. 退出登录\n");
        choice = read_int_in_range("请选择: ", 0, 6);
        if (choice == 0) {
            return;
        } else if (choice == 1) {
            patient_menu(system);
        } else if (choice == 2) {
            record_menu(system);
        } else if (choice == 3) {
            inpatient_business_menu(system);
        } else if (choice == 4) {
            query_menu(system);
        } else if (choice == 5) {
            analysis_menu(system);
        } else if (choice == 6) {
            save_data_interactive(system);
            pause_screen();
        }
    }
}

static void run_pharmacy_role(HospitalSystem *system) {
    int choice = 0;
    while (1) {
        if (get_current_pharmacy_staff(system) == NULL) {
            printf("当前药房工作人员身份无效，请重新登录。\n");
            return;
        }
        print_title("药房工作人员菜单");
        printf("1. 查询药品\n");
        printf("2. 查看全部药品\n");
        printf("3. 药品入库\n");
        printf("4. 药品出库\n");
        printf("5. 查看处方\n");
        printf("6. 按处方一键发药\n");
        printf("7. 查看发药记录\n");
        printf("8. 修改密码\n");
        printf("9. 保存数据\n");
        printf("0. 退出登录\n");
        choice = read_int_in_range("请选择: ", 0, 9);
        if (choice == 0) {
            return;
        } else if (choice == 1) {
            query_drug_interactive(system);
        } else if (choice == 2) {
            list_drugs(system);
        } else if (choice == 3) {
            stock_in_interactive(system);
        } else if (choice == 4) {
            stock_out_interactive(system);
        } else if (choice == 5) {
            list_prescriptions(system);
        } else if (choice == 6) {
            dispense_prescription_interactive(system);
        } else if (choice == 7) {
            list_drug_usage_records(system);
        } else if (choice == 8) {
            change_current_password_interactive(system);
        } else if (choice == 9) {
            save_data_interactive(system);
        }
        pause_screen();
    }
}

static void run_admin(HospitalSystem *system) {
    if (system == NULL || system->current_role != ROLE_ADMIN) {
        printf("当前管理员身份无效，请重新登录。\n");
        return;
    }

    switch (system->current_admin_type) {
    case ADMIN_SUPER:
        run_super_admin(system);
        break;
    case ADMIN_STAFF_DATA:
        run_staff_data_admin(system);
        break;
    case ADMIN_RESOURCE:
        run_resource_admin(system);
        break;
    case ADMIN_MEDICAL_BUSINESS:
        run_medical_business_admin(system);
        break;
    default:
        printf("管理员类型无效，请重新登录。\n");
        break;
    }
}

static void run_doctor_role(HospitalSystem *system) {
    int choice = 0;
    while (1) {
        print_title("医生菜单");
        printf("1. 查询本科室患者\n");
        printf("2. 查看本科室患者病史\n");
        printf("3. 办理本科室住院\n");
        printf("4. 办理本科室出院\n");
        printf("5. 查看本科室统计\n");
        printf("6. 查看候诊队列\n");
        printf("7. 叫号接诊（支持跳号）\n");
        printf("8. 开具处方\n");
        printf("9. 查看处方\n");
        printf("10. 查看本科室发药记录\n");
        printf("11. 修改密码\n");
        printf("0. 退出登录\n");
        choice = read_int_in_range("请选择: ", 0, 11);
        if (choice == 0) {
            return;
        } else if (choice == 1) {
            query_patient_interactive(system);
        } else if (choice == 2) {
            patient_history_interactive(system);
        } else if (choice == 3) {
            handle_admission_interactive(system);
        } else if (choice == 4) {
            handle_discharge_interactive(system);
        } else if (choice == 5) {
            show_medical_statistics(system);
        } else if (choice == 6) {
            list_doctor_waiting_queue(system);
        } else if (choice == 7) {
            doctor_call_and_finish_registration(system);
        } else if (choice == 8) {
            prescribe_drug_interactive(system);
        } else if (choice == 9) {
            list_prescriptions(system);
        } else if (choice == 10) {
            list_drug_usage_records(system);
        } else if (choice == 11) {
            change_current_password_interactive(system);
        }
        pause_screen();
    }
}

static void run_nurse_role(HospitalSystem *system) {
    int choice = 0;
    Nurse *nurse = NULL;
    while (1) {
        nurse = get_current_nurse(system);
        if (nurse == NULL) {
            printf("当前护士身份无效，请重新登录。\n");
            return;
        }
        print_title("护士菜单");
        printf("1. 查看本科室在院患者\n");
        printf("2. 查看本科室住院记录\n");
        printf("3. 查看本科室病房\n");
        printf("4. 查看本科室床位\n");
        printf("5. 查看本科室可用床位\n");
        printf("6. 本科室床位报修/恢复\n");
        printf("7. 本科室床位移动\n");
        printf("8. 查询本科室患者\n");
        printf("9. 查看本科室患者病史\n");
        printf("10. 查看本科室处方\n");
        printf("11. 查看本科室发药记录\n");
        printf("12. 修改密码\n");
        printf("0. 退出登录\n");
        choice = read_int_in_range("请选择: ", 0, 12);
        if (choice == 0) {
            return;
        } else if (choice == 1) {
            list_current_nurse_inpatients(system);
        } else if (choice == 2) {
            list_current_nurse_admissions(system);
        } else if (choice == 3) {
            list_wards_by_department(system, nurse->department_id);
        } else if (choice == 4) {
            list_beds_by_department(system, nurse->department_id);
        } else if (choice == 5) {
            print_title("本科室可用床位");
            list_available_beds_by_department(system, nurse->department_id);
        } else if (choice == 6) {
            modify_bed_status_interactive(system);
        } else if (choice == 7) {
            move_bed_to_ward_interactive(system);
        } else if (choice == 8) {
            query_patient_interactive(system);
        } else if (choice == 9) {
            patient_history_interactive(system);
        } else if (choice == 10) {
            list_prescriptions(system);
        } else if (choice == 11) {
            list_drug_usage_records(system);
        } else if (choice == 12) {
            change_current_password_interactive(system);
        }
        pause_screen();
    }
}

static void run_patient_role(HospitalSystem *system) {
    int choice = 0;
    while (1) {
        print_title("患者菜单");
        printf("1. 查看我的基本信息与病史\n");
        printf("2. 查看我的住院信息\n");
        printf("3. 门诊挂号（预约/现场）\n");
        printf("4. 查看我的挂号记录\n");
        printf("5. 取消我的挂号\n");
        printf("6. 查看我的处方\n");
        printf("7. 查看我的发药记录\n");
        printf("8. 修改密码\n");
        printf("0. 退出登录\n");
        choice = read_int_in_range("请选择: ", 0, 8);
        if (choice == 0) {
            return;
        } else if (choice == 1) {
            show_patient_query_report(system);
        } else if (choice == 2) {
            show_current_patient_admission(system);
        } else if (choice == 3) {
            patient_registration_interactive(system);
        } else if (choice == 4) {
            show_current_patient_registrations(system);
        } else if (choice == 5) {
            cancel_registration_interactive(system);
        } else if (choice == 6) {
            list_prescriptions(system);
        } else if (choice == 7) {
            list_drug_usage_records(system);
        } else if (choice == 8) {
            change_current_password_interactive(system);
        }
        pause_screen();
    }
}

/* 系统总调度入口
 * 先把登录菜单放出来，让用户选当前身份，
 * 然后再跳进不同角色对应的菜单循环，
 * 最后用户退出系统前，还可以顺手决定要不要把内存里的修改存到文件
 */
void run_system(HospitalSystem *system) {
    while (1) {
        int role = login_menu();
        reset_current_session(system);
        if (role == 0) {
            if (read_yes_no("是否在退出前保存数据？(y/n): ")) {
                save_all_data(system);
            }
            break;
        } else if (role == 1) {
            if (authenticate_admin(system)) {
                run_admin(system);
            }
        } else if (role == 2) {
            if (authenticate_doctor(system)) {
                run_doctor_role(system);
            }
        } else if (role == 3) {
            if (authenticate_nurse(system)) {
                run_nurse_role(system);
            }
        } else if (role == 4) {
            if (authenticate_pharmacy_staff(system)) {
                run_pharmacy_role(system);
            }
        } else if (role == 5) {
            int patient_choice = patient_entry_menu();
            if (patient_choice == 1) {
                if (authenticate_patient(system)) {
                    run_patient_role(system);
                }
            } else if (patient_choice == 2) {
                int patient_id = register_patient_self_interactive(system);
                if (patient_id > 0) {
                    system->current_role = ROLE_PATIENT;
                    system->current_patient_id = patient_id;
                    run_patient_role(system);
                }
            }
        }
        reset_current_session(system);
    }
}

#endif

