/*
 * ward.c
 * ------------------------------------------------------------
 * 病房与床位管理模块
 * 负责病房、床位的维护，以及床位分配、释放和住院人数统计
 */

#include "system.h"

#if defined(HIS_BUILD_FROM_MAIN) || !HIS_MAIN_ONE_CLICK_BUILD

/* 按病房编号查找当前仍处于启用状态的病房 */
Ward *find_ward_by_id(HospitalSystem *system, int ward_id) {
    Ward *current = system->wards;
    while (current != NULL) {
        if (current->ward_id == ward_id && current->is_active == 1) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

#define WARD_COL_ID 8
#define WARD_COL_NAME 14
#define WARD_COL_TYPE 12
#define WARD_COL_DEPT 8
#define WARD_COL_FLOOR 8
#define WARD_COL_CAP 8
#define WARD_COL_BUILT 8
#define WARD_COL_FREE 8
#define WARD_COL_OCCUPIED 8
#define WARD_COL_REPAIR 8
#define BED_COL_ID 8
#define BED_COL_WARD 8
#define BED_COL_NO 12
#define BED_COL_STATUS 10
#define BED_COL_PATIENT 8

static int count_beds_in_ward(HospitalSystem *system, int ward_id);
static int count_beds_in_ward_by_status(HospitalSystem *system, int ward_id, int status);
static int bed_no_exists_in_ward(HospitalSystem *system, int ward_id, const char *bed_no);
static int ward_has_occupied_bed(HospitalSystem *system, int ward_id);

/* 病房列表表格总宽度 */
static int ward_table_width(void) {
    return WARD_COL_ID + WARD_COL_NAME + WARD_COL_TYPE + WARD_COL_DEPT + WARD_COL_FLOOR +
           WARD_COL_CAP + WARD_COL_BUILT + WARD_COL_FREE + WARD_COL_OCCUPIED + WARD_COL_REPAIR +
           3 * 10 + 1;
}

/* 床位列表表格总宽度 */
static int bed_table_width(int include_patient) {
    return BED_COL_ID + BED_COL_WARD + BED_COL_NO + WARD_COL_DEPT + BED_COL_STATUS +
           (include_patient ? BED_COL_PATIENT : 0) + (include_patient ? 3 * 6 + 1 : 3 * 5 + 1);
}

/* 打印病房表头 */
static void print_ward_header(void) {
    char id[32], name[32], type[32], dept[32], floor[32], cap[32], built[32], free_count[32], occupied[32], repair[32];
    print_table_border(ward_table_width(), '=');
    format_display_cell("编号", id, (int)sizeof(id), WARD_COL_ID);
    format_display_cell("名称", name, (int)sizeof(name), WARD_COL_NAME);
    format_display_cell("类型", type, (int)sizeof(type), WARD_COL_TYPE);
    format_display_cell("科室", dept, (int)sizeof(dept), WARD_COL_DEPT);
    format_display_cell("楼层", floor, (int)sizeof(floor), WARD_COL_FLOOR);
    format_display_cell("容量", cap, (int)sizeof(cap), WARD_COL_CAP);
    format_display_cell("已建", built, (int)sizeof(built), WARD_COL_BUILT);
    format_display_cell("空闲", free_count, (int)sizeof(free_count), WARD_COL_FREE);
    format_display_cell("占用", occupied, (int)sizeof(occupied), WARD_COL_OCCUPIED);
    format_display_cell("维修", repair, (int)sizeof(repair), WARD_COL_REPAIR);
    printf("| %s | %s | %s | %s | %s | %s | %s | %s | %s | %s |\n",
           id, name, type, dept, floor, cap, built, free_count, occupied, repair);
    print_table_border(ward_table_width(), '-');
}

/* 打印单行病房信息 */
static void print_ward_row(HospitalSystem *system, const Ward *ward) {
    char raw_id[32], id[32], name[NAME_LEN * 2], type[TYPE_LEN * 2], raw_dept[32], dept[32], raw_floor[32], floor[32], raw_cap[32], cap[32];
    char raw_built[32], built[32], raw_free[32], free_count[32], raw_occupied[32], occupied[32], raw_repair[32], repair[32];
    if (ward == NULL) {
        return;
    }
    snprintf(raw_id, sizeof(raw_id), "%d", ward->ward_id);
    snprintf(raw_dept, sizeof(raw_dept), "%d", ward->department_id);
    snprintf(raw_floor, sizeof(raw_floor), "%d", ward->floor_no);
    snprintf(raw_cap, sizeof(raw_cap), "%d", ward->capacity);
    snprintf(raw_built, sizeof(raw_built), "%d", count_beds_in_ward(system, ward->ward_id));
    snprintf(raw_free, sizeof(raw_free), "%d", count_beds_in_ward_by_status(system, ward->ward_id, BED_STATUS_FREE));
    snprintf(raw_occupied, sizeof(raw_occupied), "%d", count_beds_in_ward_by_status(system, ward->ward_id, BED_STATUS_OCCUPIED));
    snprintf(raw_repair, sizeof(raw_repair), "%d", count_beds_in_ward_by_status(system, ward->ward_id, BED_STATUS_REPAIR));
    format_display_cell(raw_id, id, (int)sizeof(id), WARD_COL_ID);
    format_display_cell(ward->name, name, (int)sizeof(name), WARD_COL_NAME);
    format_display_cell(ward->ward_type, type, (int)sizeof(type), WARD_COL_TYPE);
    format_display_cell(raw_dept, dept, (int)sizeof(dept), WARD_COL_DEPT);
    format_display_cell(raw_floor, floor, (int)sizeof(floor), WARD_COL_FLOOR);
    format_display_cell(raw_cap, cap, (int)sizeof(cap), WARD_COL_CAP);
    format_display_cell(raw_built, built, (int)sizeof(built), WARD_COL_BUILT);
    format_display_cell(raw_free, free_count, (int)sizeof(free_count), WARD_COL_FREE);
    format_display_cell(raw_occupied, occupied, (int)sizeof(occupied), WARD_COL_OCCUPIED);
    format_display_cell(raw_repair, repair, (int)sizeof(repair), WARD_COL_REPAIR);
    printf("| %s | %s | %s | %s | %s | %s | %s | %s | %s | %s |\n",
           id, name, type, dept, floor, cap, built, free_count, occupied, repair);
}

/* 打印床位表头 */
static void print_bed_header(int include_patient) {
    char id[32], ward[32], bed_no[32], dept[32], status[32], patient[32];
    print_table_border(bed_table_width(include_patient), '=');
    format_display_cell("床位ID", id, (int)sizeof(id), BED_COL_ID);
    format_display_cell("病房ID", ward, (int)sizeof(ward), BED_COL_WARD);
    format_display_cell("床位号", bed_no, (int)sizeof(bed_no), BED_COL_NO);
    format_display_cell("科室", dept, (int)sizeof(dept), WARD_COL_DEPT);
    format_display_cell("状态", status, (int)sizeof(status), BED_COL_STATUS);
    if (include_patient) {
        format_display_cell("患者", patient, (int)sizeof(patient), BED_COL_PATIENT);
        printf("| %s | %s | %s | %s | %s | %s |\n", id, ward, bed_no, dept, status, patient);
    } else {
        printf("| %s | %s | %s | %s | %s |\n", id, ward, bed_no, dept, status);
    }
    print_table_border(bed_table_width(include_patient), '-');
}

/* 打印单行床位信息 */
static void print_bed_row(const Bed *bed, int include_patient) {
    char raw_id[32], id[32], raw_ward[32], ward[32], bed_no[NAME_LEN * 2], raw_dept[32], dept[32], status[32], raw_patient[32], patient[32];
    if (bed == NULL) {
        return;
    }
    snprintf(raw_id, sizeof(raw_id), "%d", bed->bed_id);
    snprintf(raw_ward, sizeof(raw_ward), "%d", bed->ward_id);
    snprintf(raw_dept, sizeof(raw_dept), "%d", bed->department_id);
    snprintf(raw_patient, sizeof(raw_patient), "%d", bed->patient_id);
    format_display_cell(raw_id, id, (int)sizeof(id), BED_COL_ID);
    format_display_cell(raw_ward, ward, (int)sizeof(ward), BED_COL_WARD);
    format_display_cell(bed->bed_no, bed_no, (int)sizeof(bed_no), BED_COL_NO);
    format_display_cell(raw_dept, dept, (int)sizeof(dept), WARD_COL_DEPT);
    format_display_cell(bed_status_to_text(bed->status), status, (int)sizeof(status), BED_COL_STATUS);
    if (include_patient) {
        format_display_cell(raw_patient, patient, (int)sizeof(patient), BED_COL_PATIENT);
        printf("| %s | %s | %s | %s | %s | %s |\n", id, ward, bed_no, dept, status, patient);
    } else {
        printf("| %s | %s | %s | %s | %s |\n", id, ward, bed_no, dept, status);
    }
}

/* 规范化病房类型输入
 * 把用户可能输入的简称统一转换成系统内部使用的标准文本
 */
static int normalize_ward_type(const char *input, char *output, int size) {
    if (input == NULL || output == NULL || size <= 0) {
        return 0;
    }
    if (strcmp(input, "普通") == 0 || strcmp(input, "普通病房") == 0) {
        safe_copy(output, "普通病房", size);
        return 1;
    }
    if (strcmp(input, "重症") == 0 || strcmp(input, "重症病房") == 0) {
        safe_copy(output, "重症", size);
        return 1;
    }
    return 0;
}

static int count_beds_in_ward(HospitalSystem *system, int ward_id) {
    int count = 0;
    Bed *current = system->beds;
    while (current != NULL) {
        if (current->ward_id == ward_id) {
            count++;
        }
        current = current->next;
    }
    return count;
}

static int count_beds_in_ward_by_status(HospitalSystem *system, int ward_id, int status) {
    int count = 0;
    Bed *current = system->beds;
    while (current != NULL) {
        if (current->ward_id == ward_id && current->status == status) {
            count++;
        }
        current = current->next;
    }
    return count;
}

static int bed_no_exists_in_ward(HospitalSystem *system, int ward_id, const char *bed_no) {
    Bed *current = system->beds;
    while (current != NULL) {
        if (current->ward_id == ward_id && strcmp(current->bed_no, bed_no) == 0) {
            return 1;
        }
        current = current->next;
    }
    return 0;
}

static void generate_bed_no_for_ward(HospitalSystem *system, int ward_id, char *buffer, int size) {
    int index = 1;
    if (buffer == NULL || size <= 0) {
        return;
    }
    while (index <= 999) {
        char candidate[NAME_LEN];
        snprintf(candidate, sizeof(candidate), "%d-%02d", ward_id, index);
        if (!bed_no_exists_in_ward(system, ward_id, candidate)) {
            safe_copy(buffer, candidate, size);
            return;
        }
        index++;
    }
    snprintf(buffer, (size_t)size, "%d-%03d", ward_id, index);
}

static int ward_has_occupied_bed(HospitalSystem *system, int ward_id) {
    return count_beds_in_ward_by_status(system, ward_id, BED_STATUS_OCCUPIED) > 0;
}

/* 按床位编号查找床位 */
Bed *find_bed_by_id(HospitalSystem *system, int bed_id) {
    Bed *current = system->beds;
    while (current != NULL) {
        if (current->bed_id == bed_id) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

/* 按住院编号查找住院记录 */
AdmissionHistory *find_admission_by_id(HospitalSystem *system, int admission_id) {
    AdmissionHistory *current = system->admissions;
    while (current != NULL) {
        if (current->admission_id == admission_id) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

/* 查找某个患者当前尚未出院的住院记录 */
AdmissionHistory *find_active_admission_by_patient(HospitalSystem *system, int patient_id) {
    AdmissionHistory *current = system->admissions;
    while (current != NULL) {
        if (current->patient_id == patient_id && current->status == ADMISSION_ACTIVE) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

/* 向病房链表尾部追加病房 */
void add_ward(HospitalSystem *system, Ward ward) {
    Ward *node = (Ward *)malloc(sizeof(Ward));
    Ward *tail = NULL;
    if (node == NULL) {
        return;
    }
    *node = ward;
    node->next = NULL;
    if (system->wards == NULL) {
        system->wards = node;
    } else {
        tail = system->wards;
        while (tail->next != NULL) {
            tail = tail->next;
        }
        tail->next = node;
    }
}

/* 向床位链表尾部追加床位 */
void add_bed(HospitalSystem *system, Bed bed) {
    Bed *node = (Bed *)malloc(sizeof(Bed));
    Bed *tail = NULL;
    if (node == NULL) {
        return;
    }
    *node = bed;
    node->next = NULL;
    if (system->beds == NULL) {
        system->beds = node;
    } else {
        tail = system->beds;
        while (tail->next != NULL) {
            tail = tail->next;
        }
        tail->next = node;
    }
}

/* 交互式新增病房 */
void add_ward_interactive(HospitalSystem *system) {
    Ward ward;
    char ward_type[TYPE_LEN];
    memset(&ward, 0, sizeof(ward));
    ward.ward_id = system->next_ward_id++;
    read_string("请输入病房名称: ", ward.name, NAME_LEN);
    while (1) {
        read_string("请输入病房类型(普通病房/重症): ", ward_type, TYPE_LEN);
        if (normalize_ward_type(ward_type, ward.ward_type, TYPE_LEN)) {
            break;
        }
        printf("病房类型输入不合法，只能输入 普通病房 或 重症\n");
    }
    ward.department_id = read_int("请输入关联科室编号: ");
    if (find_department_by_id(system, ward.department_id) == NULL) {
        printf("科室不存在，病房新增已取消\n");
        system->next_ward_id--;
        return;
    }
    ward.floor_no = read_int("请输入楼层: ");
    ward.capacity = read_int_in_range("请输入病房容量: ", 1, 100);
    ward.is_active = 1;
    add_ward(system, ward);
    append_log(system, admin_log_role(system), "新增病房", ward.name);
    printf("病房新增成功，编号为 %d\n", ward.ward_id);
}

/* 交互式新增床位
 * 新增床位前会先验证所属病房是否存在，并自动继承科室编号
 */
void add_bed_interactive(HospitalSystem *system) {
    Ward *ward = NULL;
    Bed bed;
    memset(&bed, 0, sizeof(bed));
    bed.bed_id = system->next_bed_id++;
    bed.ward_id = read_int("请输入病房编号: ");
    ward = find_ward_by_id(system, bed.ward_id);
    if (ward == NULL) {
        printf("病房不存在\n");
        system->next_bed_id--;
        return;
    }
    if (count_beds_in_ward(system, bed.ward_id) >= ward->capacity) {
        printf("该病房床位数量已达到容量上限，不能继续新增床位\n");
        system->next_bed_id--;
        return;
    }
    bed.department_id = ward->department_id;
    generate_bed_no_for_ward(system, bed.ward_id, bed.bed_no, NAME_LEN);
    bed.status = BED_STATUS_FREE;
    bed.patient_id = 0;
    add_bed(system, bed);
    append_log(system, admin_log_role(system), "新增床位", bed.bed_no);
    printf("床位新增成功，编号为 %d，床位号为 %s\n", bed.bed_id, bed.bed_no);
}

static const char *bed_status_operator_role(HospitalSystem *system) {
    if (system != NULL && system->current_role == ROLE_NURSE) {
        return "护士";
    }
    return admin_log_role(system);
}

/* 交互式修改床位状态
 * 护士端只作为本科室床位报修/恢复入口，已占用床位必须通过出院流程释放
 */
void modify_bed_status_interactive(HospitalSystem *system) {
    int bed_id = read_int("请输入床位编号: ");
    int status = 0;
    Bed *bed = find_bed_by_id(system, bed_id);
    Nurse *nurse = NULL;
    int is_nurse_role = system != NULL && system->current_role == ROLE_NURSE;
    if (bed == NULL) {
        printf("床位不存在\n");
        return;
    }
    if (is_nurse_role) {
        nurse = get_current_nurse(system);
        if (nurse == NULL) {
            printf("当前护士身份无效，请重新登录\n");
            return;
        }
        if (bed->department_id != nurse->department_id) {
            printf("无权限修改其他科室的床位状态\n");
            return;
        }
    }
    if (bed->status == BED_STATUS_OCCUPIED) {
        printf("该床位当前已占用，不能直接修改，请先办理出院\n");
        return;
    }
    if (is_nurse_role) {
        printf("0. 恢复空闲  2. 报修维修\n");
        status = read_int_in_range("请选择床位报修/恢复状态: ", 0, 2);
    } else {
        printf("0. 空闲  2. 维修\n");
        status = read_int_in_range("请输入新的床位状态: ", 0, 2);
    }
    if (status == BED_STATUS_OCCUPIED) {
        printf("不能手工把床位改为已占用，请通过办理住院流程分配患者\n");
        return;
    }
    bed->status = status;
    bed->patient_id = 0;
    append_log(system, bed_status_operator_role(system),
               is_nurse_role ? "床位报修/恢复" : "修改床位状态",
               bed->bed_no);
    printf(is_nurse_role ? "床位报修/恢复成功\n" : "床位状态修改成功\n");
}

void deactivate_ward_interactive(HospitalSystem *system) {
    int ward_id = read_int("请输入要停用的病房编号: ");
    Ward *ward = find_ward_by_id(system, ward_id);
    if (ward == NULL) {
        printf("病房不存在或已经停用\n");
        return;
    }
    if (ward_has_occupied_bed(system, ward_id)) {
        printf("该病房仍有占用床位，请先办理出院或转出后再停用\n");
        return;
    }
    ward->is_active = 0;
    append_log(system, admin_log_role(system), "停用病房", ward->name);
    printf("病房已停用，该病房下空闲/维修床位不会再参与分配\n");
}

/* 列出全部启用中的病房 */
void list_wards(HospitalSystem *system) {
    Ward *current = system->wards;
    int found = 0;
    print_title("病房列表");
    print_ward_header();
    while (current != NULL) {
        if (current->is_active == 1) {
            print_ward_row(system, current);
            found = 1;
        }
        current = current->next;
    }
    print_table_border(ward_table_width(), '=');
    if (!found) {
        printf("当前暂无病房数据\n");
    }
}

/* 列出全部床位 */
void list_beds(HospitalSystem *system) {
    Bed *current = system->beds;
    int found = 0;
    print_title("床位列表");
    print_bed_header(1);
    while (current != NULL) {
        print_bed_row(current, 1);
        found = 1;
        current = current->next;
    }
    print_table_border(bed_table_width(1), '=');
    if (!found) {
        printf("当前暂无床位数据\n");
    }
}

void list_wards_by_department(HospitalSystem *system, int department_id) {
    Ward *current = system->wards;
    int found = 0;
    print_title("本科室病房列表");
    print_ward_header();
    while (current != NULL) {
        if (current->is_active == 1 && current->department_id == department_id) {
            print_ward_row(system, current);
            found = 1;
        }
        current = current->next;
    }
    print_table_border(ward_table_width(), '=');
    if (!found) {
        printf("当前科室暂无病房数据\n");
    }
}

void list_beds_by_department(HospitalSystem *system, int department_id) {
    Bed *current = system->beds;
    int found = 0;
    print_title("本科室床位列表");
    print_bed_header(1);
    while (current != NULL) {
        if (current->department_id == department_id) {
            print_bed_row(current, 1);
            found = 1;
        }
        current = current->next;
    }
    print_table_border(bed_table_width(1), '=');
    if (!found) {
        printf("当前科室暂无床位数据\n");
    }
}

void move_bed_to_ward_interactive(HospitalSystem *system) {
    int bed_id = read_int("请输入要移动的床位编号: ");
    int ward_id = read_int("请输入目标病房编号: ");
    Bed *bed = find_bed_by_id(system, bed_id);
    Ward *ward = find_ward_by_id(system, ward_id);
    Nurse *nurse = NULL;
    int is_nurse_role = system != NULL && system->current_role == ROLE_NURSE;
    char old_bed_no[NAME_LEN];
    if (bed == NULL) {
        printf("床位不存在\n");
        return;
    }
    if (ward == NULL) {
        printf("目标病房不存在或已停用\n");
        return;
    }
    if (bed->status == BED_STATUS_OCCUPIED) {
        printf("占用中的床位不能移动，请先办理出院或转床释放\n");
        return;
    }
    if (count_beds_in_ward(system, ward_id) >= ward->capacity) {
        printf("目标病房床位数量已达到容量上限\n");
        return;
    }
    if (is_nurse_role) {
        nurse = get_current_nurse(system);
        if (nurse == NULL) {
            printf("当前护士身份无效，请重新登录\n");
            return;
        }
        if (bed->department_id != nurse->department_id || ward->department_id != nurse->department_id) {
            printf("护士只能在本科室内移动床位\n");
            return;
        }
    }
    safe_copy(old_bed_no, bed->bed_no, NAME_LEN);
    bed->ward_id = ward_id;
    bed->department_id = ward->department_id;
    generate_bed_no_for_ward(system, ward_id, bed->bed_no, NAME_LEN);
    append_log(system, bed_status_operator_role(system), "移动床位", old_bed_no);
    printf("床位移动成功，新床位号为 %s\n", bed->bed_no);
}

/* 按病房查看其包含的床位，体现病房与床位的一对多关系 */
void list_beds_by_ward_interactive(HospitalSystem *system) {
    int ward_id = read_int("请输入病房编号: ");
    Ward *ward = find_ward_by_id(system, ward_id);
    Bed *current = NULL;
    int found = 0;
    if (ward == NULL) {
        printf("病房不存在或已经停用\n");
        return;
    }
    printf("病房：%s  容量：%d  已建床位：%d\n", ward->name, ward->capacity, count_beds_in_ward(system, ward_id));
    print_bed_header(1);
    current = system->beds;
    while (current != NULL) {
        if (current->ward_id == ward_id) {
            print_bed_row(current, 1);
            found = 1;
        }
        current = current->next;
    }
    print_table_border(bed_table_width(1), '=');
    if (!found) {
        printf("该病房下暂无床位\n");
    }
}
/* 按科室列出可分配的空闲床位 */
void list_available_beds_by_department(HospitalSystem *system, int department_id) {
    Bed *current = system->beds;
    int found = 0;
    print_bed_header(0);
    while (current != NULL) {
        if (current->department_id == department_id &&
            current->status == BED_STATUS_FREE &&
            find_ward_by_id(system, current->ward_id) != NULL) {
            print_bed_row(current, 0);
            found = 1;
        }
        current = current->next;
    }
    print_table_border(bed_table_width(0), '=');
    if (!found) {
        printf("该科室当前没有空闲床位\n");
    }
}

/* 分配床位给患者
 * 只有当患者当前未住院、床位处于空闲状态时，分配才会成功
 */
int assign_bed(HospitalSystem *system, int patient_id, int bed_id) {
    Patient *patient = find_patient_by_id(system, patient_id);
    Bed *bed = find_bed_by_id(system, bed_id);
    if (patient == NULL || bed == NULL) {
        return 0;
    }
    if (patient->is_inpatient == 1 || find_active_admission_by_patient(system, patient_id) != NULL) {
        return 0;
    }
    if (bed->status != BED_STATUS_FREE) {
        return 0;
    }
    if (find_ward_by_id(system, bed->ward_id) == NULL) {
        return 0;
    }
    bed->status = BED_STATUS_OCCUPIED;
    bed->patient_id = patient_id;
    patient->is_inpatient = 1;
    patient->current_bed_id = bed_id;
    return 1;
}

/* 释放床位
 * 会同步把患者状态从“住院中”改回“未住院”，并清空当前床位编号
 */
int release_bed(HospitalSystem *system, int bed_id) {
    Bed *bed = find_bed_by_id(system, bed_id);
    Patient *patient = NULL;
    if (bed == NULL) {
        return 0;
    }
    if (bed->status != BED_STATUS_OCCUPIED) {
        return 0;
    }
    patient = find_patient_by_id(system, bed->patient_id);
    if (patient != NULL) {
        patient->is_inpatient = 0;
        patient->current_bed_id = 0;
    }
    bed->status = BED_STATUS_FREE;
    bed->patient_id = 0;
    return 1;
}

/* 统计当前系统中处于住院状态的有效患者数量 */
int current_inpatient_count(HospitalSystem *system) {
    int count = 0;
    Patient *current = system->patients;
    while (current != NULL) {
        if (current->is_deleted == 0 && current->is_inpatient == 1) {
            count++;
        }
        current = current->next;
    }
    return count;
}

#endif
