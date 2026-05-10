/*
 * pharmacy_staff.c
 * 药房工作人员资料管理模块
 * 只维护药师/药房人员主数据，不处理药品入库、出库、发药等业务操作
 */

#include "system.h"

#if defined(HIS_BUILD_FROM_MAIN) || !HIS_MAIN_ONE_CLICK_BUILD

/* 按药房工作人员编号查找有效人员 */
PharmacyStaff *find_pharmacy_staff_by_id(HospitalSystem *system, int staff_id) {
    PharmacyStaff *current = system->pharmacy_staffs;
    while (current != NULL) {
        if (current->staff_id == staff_id && current->is_deleted == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

#define PHARMACY_STAFF_COL_ID 8
#define PHARMACY_STAFF_COL_NAME 12
#define PHARMACY_STAFF_COL_GENDER 6
#define PHARMACY_STAFF_COL_PHONE 14

static int pharmacy_staff_table_width(void) {
    return PHARMACY_STAFF_COL_ID + PHARMACY_STAFF_COL_NAME +
           PHARMACY_STAFF_COL_GENDER + PHARMACY_STAFF_COL_PHONE + 3 * 5 + 1;
}

static void print_pharmacy_staff_header(void) {
    char id[32], name[32], gender[32], phone[32];
    print_table_border(pharmacy_staff_table_width(), '=');
    format_display_cell("编号", id, (int)sizeof(id), PHARMACY_STAFF_COL_ID);
    format_display_cell("姓名", name, (int)sizeof(name), PHARMACY_STAFF_COL_NAME);
    format_display_cell("性别", gender, (int)sizeof(gender), PHARMACY_STAFF_COL_GENDER);
    format_display_cell("电话", phone, (int)sizeof(phone), PHARMACY_STAFF_COL_PHONE);
    printf("| %s | %s | %s | %s |\n", id, name, gender, phone);
    print_table_border(pharmacy_staff_table_width(), '-');
}

static void print_pharmacy_staff_row(const PharmacyStaff *staff) {
    char raw_id[32], id[32], name[NAME_LEN * 2], gender[32], phone[40];
    if (staff == NULL) {
        return;
    }
    snprintf(raw_id, sizeof(raw_id), "%d", staff->staff_id);
    format_display_cell(raw_id, id, (int)sizeof(id), PHARMACY_STAFF_COL_ID);
    format_display_cell(staff->name, name, (int)sizeof(name), PHARMACY_STAFF_COL_NAME);
    format_display_cell(staff->gender, gender, (int)sizeof(gender), PHARMACY_STAFF_COL_GENDER);
    format_display_cell(staff->phone, phone, (int)sizeof(phone), PHARMACY_STAFF_COL_PHONE);
    printf("| %s | %s | %s | %s |\n", id, name, gender, phone);
}

static int fill_pharmacy_staff_form(PharmacyStaff *staff) {
    int step = 0;
    char gender[SMALL_LEN];
    safe_copy(gender, staff->gender, SMALL_LEN);
    while (step < 3) {
        if (step == 0) {
            int result = read_string_step("请输入药房工作人员姓名（输入 b 取消）: ", staff->name, NAME_LEN, 1);
            if (result == -1) {
                return 0;
            }
            step++;
        } else if (step == 1) {
            int result = read_string_step("请输入性别 男/女（输入 b 返回上一项）: ", gender, SMALL_LEN, 1);
            if (result == -1) {
                step--;
            } else if (!normalize_gender(gender, staff->gender, SMALL_LEN)) {
                printf("性别输入不合法，只能输入 男 或 女\n");
            } else {
                step++;
            }
        } else {
            int result = read_phone_step("请输入联系电话（输入 b 返回上一项）: ", staff->phone, PHONE_LEN, 1);
            if (result == -1) {
                step--;
            } else {
                step++;
            }
        }
    }
    return 1;
}

/* 向药房工作人员链表尾部追加人员节点 */
void add_pharmacy_staff(HospitalSystem *system, PharmacyStaff staff) {
    PharmacyStaff *node = (PharmacyStaff *)malloc(sizeof(PharmacyStaff));
    PharmacyStaff *tail = NULL;
    if (node == NULL) {
        return;
    }
    *node = staff;
    node->next = NULL;
    if (system->pharmacy_staffs == NULL) {
        system->pharmacy_staffs = node;
    } else {
        tail = system->pharmacy_staffs;
        while (tail->next != NULL) {
            tail = tail->next;
        }
        tail->next = node;
    }
}

/* 交互式新增药房工作人员 */
void add_pharmacy_staff_interactive(HospitalSystem *system) {
    PharmacyStaff staff;
    memset(&staff, 0, sizeof(staff));
    staff.staff_id = system->next_pharmacy_staff_id++;
    /* 表单取消时回收预分配编号，避免生成空药房工作人员 */
    if (!fill_pharmacy_staff_form(&staff)) {
        system->next_pharmacy_staff_id--;
        printf("新增药房工作人员已取消\n");
        return;
    }
    generate_initial_password("S", staff.staff_id, staff.password, PASSWORD_LEN);
    staff.must_change_password = 1;
    staff.is_deleted = 0;
    add_pharmacy_staff(system, staff);
    append_log(system, admin_log_role(system), "新增药房工作人员", staff.name);
    printf("新增药房工作人员成功，编号为 %d\n", staff.staff_id);
    printf("系统生成初始密码：%s，请提醒工作人员首次登录后修改。\n", staff.password);
}

/* 交互式修改药房工作人员 */
void modify_pharmacy_staff_interactive(HospitalSystem *system) {
    int staff_id = 0;
    list_pharmacy_staffs(system);
    if (read_int_step("请输入要修改的药房工作人员编号（输入 b 返回上一级）: ", &staff_id, 1, 999999, 1) == -1) {
        printf("药房工作人员信息修改已取消\n");
        return;
    }
    PharmacyStaff *staff = find_pharmacy_staff_by_id(system, staff_id);
    PharmacyStaff old_staff;
    if (staff == NULL) {
        printf("未找到对应药房工作人员，或该人员已停用\n");
        return;
    }
    print_title("当前药房工作人员信息");
    print_pharmacy_staff_header();
    print_pharmacy_staff_row(staff);
    print_table_border(pharmacy_staff_table_width(), '=');
    printf("修改时直接按回车可保留原值，输入 b 可取消修改\n");
    old_staff = *staff;
    /* 修改中断时恢复原资料，避免停用/登录判断引用到半更新人员 */
    if (!fill_pharmacy_staff_form(staff)) {
        *staff = old_staff;
        printf("药房工作人员信息修改已取消\n");
        return;
    }
    append_log(system, admin_log_role(system), "修改药房工作人员", staff->name);
    printf("药房工作人员信息修改成功\n");
}

/* 交互式停用药房工作人员 */
void deactivate_pharmacy_staff_interactive(HospitalSystem *system) {
    int staff_id = read_int("请输入要停用的药房工作人员编号: ");
    PharmacyStaff *staff = find_pharmacy_staff_by_id(system, staff_id);
    if (staff == NULL) {
        printf("未找到对应药房工作人员，或该人员已停用\n");
        return;
    }
    staff->is_deleted = 1;
    append_log(system, admin_log_role(system), "停用药房工作人员", staff->name);
    printf("药房工作人员已停用\n");
}

/* 交互式查询药房工作人员 */
void query_pharmacy_staff_interactive(HospitalSystem *system) {
    char keyword[NAME_LEN];
    PharmacyStaff *current = system->pharmacy_staffs;
    int found = 0;
    read_string("请输入药房工作人员编号/姓名/电话关键字: ", keyword, NAME_LEN);
    print_pharmacy_staff_header();
    while (current != NULL) {
        char id_text[32];
        snprintf(id_text, sizeof(id_text), "%d", current->staff_id);
        if (current->is_deleted == 0 &&
            (str_contains_ignore_case(id_text, keyword) ||
             str_contains_ignore_case(current->name, keyword) ||
             str_contains_ignore_case(current->gender, keyword) ||
             str_contains_ignore_case(current->phone, keyword))) {
            print_pharmacy_staff_row(current);
            found = 1;
        }
        current = current->next;
    }
    print_table_border(pharmacy_staff_table_width(), '=');
    if (!found) {
        printf("未找到匹配的药房工作人员\n");
    }
}

/* 列出全部有效药房工作人员 */
void list_pharmacy_staffs(HospitalSystem *system) {
    PharmacyStaff *current = system->pharmacy_staffs;
    int found = 0;
    print_title("药房工作人员列表");
    print_pharmacy_staff_header();
    while (current != NULL) {
        if (current->is_deleted == 0) {
            print_pharmacy_staff_row(current);
            found = 1;
        }
        current = current->next;
    }
    print_table_border(pharmacy_staff_table_width(), '=');
    if (!found) {
        printf("当前暂无药房工作人员数据\n");
    }
}

#endif
