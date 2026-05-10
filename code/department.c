/*
 * department.c
 * ------------------------------------------------------------
 * 科室管理模块
 * 负责科室对象的增删改查，以及科室列表展示
 */

#include "system.h"

#if defined(HIS_BUILD_FROM_MAIN) || !HIS_MAIN_ONE_CLICK_BUILD

/* 按科室编号查找有效科室
 * 这里只返回未被逻辑删除的科室结点，避免业务层误操作无效数据
 */
Department *find_department_by_id(HospitalSystem *system, int department_id) {
    Department *current = system->departments;
    while (current != NULL) {
        if (current->department_id == department_id && current->is_deleted == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

#define DEPT_COL_ID 8
#define DEPT_COL_NAME 14
#define DEPT_COL_MANAGER 12
#define DEPT_COL_DESC 30

/* 计算科室表格总宽度，便于统一输出边框 */
static int department_table_width(void) {
    return DEPT_COL_ID + DEPT_COL_NAME + DEPT_COL_MANAGER + DEPT_COL_DESC + 3 * 4 + 1;
}

/* 打印科室列表表头 */
static void print_department_header(void) {
    char id[32], name[32], manager[32], desc[64];
    print_table_border(department_table_width(), '=');
    format_display_cell("编号", id, (int)sizeof(id), DEPT_COL_ID);
    format_display_cell("名称", name, (int)sizeof(name), DEPT_COL_NAME);
    format_display_cell("负责人", manager, (int)sizeof(manager), DEPT_COL_MANAGER);
    format_display_cell("简介", desc, (int)sizeof(desc), DEPT_COL_DESC);
    printf("| %s | %s | %s | %s |\n", id, name, manager, desc);
    print_table_border(department_table_width(), '-');
}

/* 打印单行科室数据
 * 使用 format_display_cell 处理后，可以让中英文混排时列宽更整齐
 */
static void print_department_row(const Department *department) {
    char raw_id[32], id[32], name[NAME_LEN * 2], manager[NAME_LEN * 2], desc[TEXT_LEN * 2];
    if (department == NULL) {
        return;
    }
    snprintf(raw_id, sizeof(raw_id), "%d", department->department_id);
    format_display_cell(raw_id, id, (int)sizeof(id), DEPT_COL_ID);
    format_display_cell(department->name, name, (int)sizeof(name), DEPT_COL_NAME);
    format_display_cell(department->manager, manager, (int)sizeof(manager), DEPT_COL_MANAGER);
    format_display_cell(department->description, desc, (int)sizeof(desc), DEPT_COL_DESC);
    printf("| %s | %s | %s | %s |\n", id, name, manager, desc);
}

/* 按科室名称查找有效科室
 * 新增科室前会调用它，避免出现重名科室
 */
Department *find_department_by_name(HospitalSystem *system, const char *name) {
    Department *current = system->departments;
    while (current != NULL) {
        if (current->is_deleted == 0 && strcmp(current->name, name) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

/* 统计当前系统中有效科室数量 */
int count_departments(HospitalSystem *system) {
    int count = 0;
    Department *current = system->departments;
    while (current != NULL) {
        if (current->is_deleted == 0) {
            count++;
        }
        current = current->next;
    }
    return count;
}

/* 向科室链表尾部追加一个新科室 */
void add_department(HospitalSystem *system, Department department) {
    Department *node = (Department *)malloc(sizeof(Department));
    Department *tail = NULL;
    if (node == NULL) {
        return;
    }
    *node = department;
    node->next = NULL;
    if (system->departments == NULL) {
        system->departments = node;
    } else {
        tail = system->departments;
        while (tail->next != NULL) {
            tail = tail->next;
        }
        tail->next = node;
    }
}

/* 交互式新增科室：
 * 先分配新编号，再通过表单函数收集信息，最后做重名检查并入链表
 */
void add_department_interactive(HospitalSystem *system) {
    Department department;
    memset(&department, 0, sizeof(department));
    department.department_id = system->next_department_id++;
    /* 表单被取消时回收预分配编号，避免下次新增跳号 */
    if (!fill_department_form(&department)) {
        system->next_department_id--;
        printf("新增科室已取消\n");
        return;
    }
    if (find_department_by_name(system, department.name) != NULL) {
        printf("已存在同名科室，请先查询后再决定是否新增\n");
        system->next_department_id--;
        return;
    }
    department.is_deleted = 0;
    add_department(system, department);
    append_log(system, admin_log_role(system), "新增科室", department.name);
    printf("新增科室成功，科室编号为 %d\n", department.department_id);
}

/* 交互式修改科室：
 * 先按编号定位目标科室，再复用表单函数直接覆盖字段内容
 */
void modify_department_interactive(HospitalSystem *system) {
    int department_id = 0;
    list_departments(system);
    if (read_int_step("请输入要修改的科室编号（输入 b 返回上一级）: ", &department_id, 1, 999999, 1) == -1) {
        printf("科室信息修改已取消\n");
        return;
    }
    Department *department = find_department_by_id(system, department_id);
    Department old_department;
    if (department == NULL) {
        printf("未找到对应科室\n");
        return;
    }
    print_title("当前科室信息");
    print_department_header();
    print_department_row(department);
    print_table_border(department_table_width(), '=');
    printf("修改时直接按回车可保留原值，输入 b 可取消修改\n");
    old_department = *department;
    /* 修改中断时恢复原值，避免只改了一半的资料留在内存里 */
    if (!fill_department_form(department)) {
        *department = old_department;
        printf("科室信息修改已取消\n");
        return;
    }
    append_log(system, admin_log_role(system), "修改科室", department->name);
    printf("科室信息修改成功\n");
}

/* 交互式删除科室
 * 这里采用逻辑删除，不直接 free 内存；
 * 同时会检查该科室下是否仍有医生或病房，防止删除后留下悬挂关联
 */
void delete_department_interactive(HospitalSystem *system) {
    int department_id = read_int("请输入要删除的科室编号: ");
    Department *department = find_department_by_id(system, department_id);
    Ward *ward = system->wards;
    if (department == NULL) {
        printf("未找到对应科室\n");
        return;
    }
    if (count_doctors_in_department(system, department_id) > 0) {
        printf("该科室下仍有医生，不能删除\n");
        return;
    }
    if (count_nurses_in_department(system, department_id) > 0) {
        printf("该科室下仍有护士，不能删除\n");
        return;
    }
    while (ward != NULL) {
        if (ward->department_id == department_id && ward->is_active == 1) {
            printf("该科室下仍有关联病房，不能删除\n");
            return;
        }
        ward = ward->next;
    }
    department->is_deleted = 1;
    append_log(system, admin_log_role(system), "删除科室", department->name);
    printf("科室已逻辑删除\n");
}

/* 多方式模糊查询科室 */
void query_department_interactive(HospitalSystem *system) {
    char keyword[NAME_LEN];
    Department *current = system->departments;
    int found = 0;
    read_string("请输入科室编号/名称/负责人/简介关键字: ", keyword, NAME_LEN);
    print_department_header();
    while (current != NULL) {
        char id_text[32];
        snprintf(id_text, sizeof(id_text), "%d", current->department_id);
        if (current->is_deleted == 0 &&
            (str_contains_ignore_case(id_text, keyword) ||
             str_contains_ignore_case(current->name, keyword) ||
             str_contains_ignore_case(current->manager, keyword) ||
             str_contains_ignore_case(current->description, keyword))) {
            print_department_row(current);
            found = 1;
        }
        current = current->next;
    }
    print_table_border(department_table_width(), '=');
    if (!found) {
        printf("未找到匹配的科室\n");
    }
}

/* 列出全部有效科室 */
void list_departments(HospitalSystem *system) {
    Department *current = system->departments;
    int found = 0;
    print_title("科室列表");
    print_department_header();
    while (current != NULL) {
        if (current->is_deleted == 0) {
            print_department_row(current);
            found = 1;
        }
        current = current->next;
    }
    print_table_border(department_table_width(), '=');
    if (!found) {
        printf("当前暂无科室数据\n");
    }
}

#endif
