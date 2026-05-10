/* 公共基础模块
 * 这里实现系统初始化、输入校验、字符串处理、日期处理、表格显示、日志记录等函数
 */

#include "common.h"

#if defined(HIS_BUILD_FROM_MAIN) || !HIS_MAIN_ONE_CLICK_BUILD


/* 释放患者链表 */
static void free_patient_list(Patient *head) {
    Patient *current = head;
    while (current != NULL) {
        Patient *next = current->next;
        free(current);
        current = next;
    }
}

/* 释放医生链表 */
static void free_doctor_list(Doctor *head) {
    Doctor *current = head;
    while (current != NULL) {
        Doctor *next = current->next;
        free(current);
        current = next;
    }
}

/* 释放护士链表 */
static void free_nurse_list(Nurse *head) {
    Nurse *current = head;
    while (current != NULL) {
        Nurse *next = current->next;
        free(current);
        current = next;
    }
}

/* 释放药房工作人员链表 */
static void free_pharmacy_staff_list(PharmacyStaff *head) {
    PharmacyStaff *current = head;
    while (current != NULL) {
        PharmacyStaff *next = current->next;
        free(current);
        current = next;
    }
}

/* 释放科室链表 */
static void free_department_list(Department *head) {
    Department *current = head;
    while (current != NULL) {
        Department *next = current->next;
        free(current);
        current = next;
    }
}

/* 释放病房链表 */
static void free_ward_list(Ward *head) {
    Ward *current = head;
    while (current != NULL) {
        Ward *next = current->next;
        free(current);
        current = next;
    }
}

/* 释放床位链表 */
static void free_bed_list(Bed *head) {
    Bed *current = head;
    while (current != NULL) {
        Bed *next = current->next;
        free(current);
        current = next;
    }
}

/* 释放药品链表 */
static void free_drug_list(Drug *head) {
    Drug *current = head;
    while (current != NULL) {
        Drug *next = current->next;
        free(current);
        current = next;
    }
}

/* 释放病历链表 */
static void free_record_list(MedicalRecord *head) {
    MedicalRecord *current = head;
    while (current != NULL) {
        MedicalRecord *next = current->next;
        free(current);
        current = next;
    }
}

/* 释放门诊挂号链表 */
static void free_registration_list(OutpatientRegistration *head) {
    OutpatientRegistration *current = head;
    while (current != NULL) {
        OutpatientRegistration *next = current->next;
        free(current);
        current = next;
    }
}

/* 释放住院历史链表 */
static void free_admission_list(AdmissionHistory *head) {
    AdmissionHistory *current = head;
    while (current != NULL) {
        AdmissionHistory *next = current->next;
        free(current);
        current = next;
    }
}

/* 释放发药历史链表 */
static void free_usage_list(DrugUsageHistory *head) {
    DrugUsageHistory *current = head;
    while (current != NULL) {
        DrugUsageHistory *next = current->next;
        free(current);
        current = next;
    }
}

/* 释放处方链表 */
static void free_prescription_list(Prescription *head) {
    Prescription *current = head;
    while (current != NULL) {
        Prescription *next = current->next;
        free(current);
        current = next;
    }
}

/* 释放处方明细链表 */
static void free_prescription_item_list(PrescriptionItem *head) {
    PrescriptionItem *current = head;
    while (current != NULL) {
        PrescriptionItem *next = current->next;
        free(current);
        current = next;
    }
}

static void free_drug_department_rule_list(DrugDepartmentRule *head) {
    DrugDepartmentRule *current = head;
    while (current != NULL) {
        DrugDepartmentRule *next = current->next;
        free(current);
        current = next;
    }
}

/* 释放日志链表 */
static void free_log_list(OperationLog *head) {
    OperationLog *current = head;
    while (current != NULL) {
        OperationLog *next = current->next;
        free(current);
        current = next;
    }
}

/* 初始化系统结构体和各类自动编号起点 */
void init_system(HospitalSystem *system) {
    if (system == NULL) {
        return;
    }
    memset(system, 0, sizeof(HospitalSystem));
    system->next_patient_id = 1001;
    system->next_doctor_id = 2001;
    system->next_nurse_id = 2501;
    system->next_pharmacy_staff_id = 9001;
    system->next_department_id = 3001;
    system->next_ward_id = 4001;
    system->next_bed_id = 5001;
    system->next_drug_id = 6001;
    system->next_record_id = 7001;
    system->next_registration_id = 11001;
    system->next_admission_id = 8001;
    system->next_usage_id = 9001;
    system->next_prescription_id = 12001;
    system->next_log_id = 10001;
    system->current_role = ROLE_NONE;
    system->current_admin_type = ADMIN_NONE;
    system->current_doctor_id = 0;
    system->current_nurse_id = 0;
    system->current_pharmacy_staff_id = 0;
    system->current_patient_id = 0;
}

/* 初始化控制台编码，尽量保证中文输出正常 */
void init_console_encoding(void) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    system("chcp 65001 > nul");
    if (setlocale(LC_ALL, ".UTF-8") == NULL &&
        setlocale(LC_ALL, ".65001") == NULL) {
        setlocale(LC_ALL, "");
    }
#else
    setlocale(LC_ALL, "");
#endif
}

/* 释放系统中所有链表，并重置系统状态 */
void free_system(HospitalSystem *system) {
    if (system == NULL) {
        return;
    }
    free_patient_list(system->patients);
    free_doctor_list(system->doctors);
    free_nurse_list(system->nurses);
    free_pharmacy_staff_list(system->pharmacy_staffs);
    free_department_list(system->departments);
    free_ward_list(system->wards);
    free_bed_list(system->beds);
    free_drug_list(system->drugs);
    free_record_list(system->records);
    free_registration_list(system->registrations);
    free_admission_list(system->admissions);
    free_usage_list(system->drug_usages);
    free_prescription_list(system->prescriptions);
    free_prescription_item_list(system->prescription_items);
    free_drug_department_rule_list(system->drug_department_rules);
    free_log_list(system->logs);
    init_system(system);
}

/* 返回两个整数中的较大值 */
static int max_int(int a, int b) {
    return a > b ? a : b;
}

/* 重建所有 next_id：
 * 系统从文件读取完历史数据后，需要重新扫描每条链表
 * 找到当前最大编号，从而保证新数据的自动编号不会重复。
 */
void rebuild_next_ids(HospitalSystem *system) {
    Patient *p = NULL;
    Doctor *d = NULL;
    Nurse *n = NULL;
    PharmacyStaff *staff = NULL;
    Department *dept = NULL;
    Ward *ward = NULL;
    Bed *bed = NULL;
    Drug *drug = NULL;
    MedicalRecord *record = NULL;
    OutpatientRegistration *registration = NULL;
    AdmissionHistory *admission = NULL;
    DrugUsageHistory *usage = NULL;
    Prescription *prescription = NULL;
    OperationLog *log_node = NULL;

    if (system == NULL) {
        return;
    }

    system->next_patient_id = 1001;
    system->next_doctor_id = 2001;
    system->next_nurse_id = 2501;
    system->next_pharmacy_staff_id = 9001;
    system->next_department_id = 3001;
    system->next_ward_id = 4001;
    system->next_bed_id = 5001;
    system->next_drug_id = 6001;
    system->next_record_id = 7001;
    system->next_registration_id = 11001;
    system->next_admission_id = 8001;
    system->next_usage_id = 9001;
    system->next_prescription_id = 12001;
    system->next_log_id = 10001;

    for (p = system->patients; p != NULL; p = p->next) {
        system->next_patient_id = max_int(system->next_patient_id, p->patient_id + 1);
    }
    for (d = system->doctors; d != NULL; d = d->next) {
        system->next_doctor_id = max_int(system->next_doctor_id, d->doctor_id + 1);
    }
    for (n = system->nurses; n != NULL; n = n->next) {
        system->next_nurse_id = max_int(system->next_nurse_id, n->nurse_id + 1);
    }
    for (staff = system->pharmacy_staffs; staff != NULL; staff = staff->next) {
        system->next_pharmacy_staff_id = max_int(system->next_pharmacy_staff_id, staff->staff_id + 1);
    }
    for (dept = system->departments; dept != NULL; dept = dept->next) {
        system->next_department_id = max_int(system->next_department_id, dept->department_id + 1);
    }
    for (ward = system->wards; ward != NULL; ward = ward->next) {
        system->next_ward_id = max_int(system->next_ward_id, ward->ward_id + 1);
    }
    for (bed = system->beds; bed != NULL; bed = bed->next) {
        system->next_bed_id = max_int(system->next_bed_id, bed->bed_id + 1);
    }
    for (drug = system->drugs; drug != NULL; drug = drug->next) {
        system->next_drug_id = max_int(system->next_drug_id, drug->drug_id + 1);
    }
    for (record = system->records; record != NULL; record = record->next) {
        system->next_record_id = max_int(system->next_record_id, record->record_id + 1);
    }
    for (registration = system->registrations; registration != NULL; registration = registration->next) {
        system->next_registration_id = max_int(system->next_registration_id, registration->registration_id + 1);
    }
    for (admission = system->admissions; admission != NULL; admission = admission->next) {
        system->next_admission_id = max_int(system->next_admission_id, admission->admission_id + 1);
    }
    for (usage = system->drug_usages; usage != NULL; usage = usage->next) {
        system->next_usage_id = max_int(system->next_usage_id, usage->usage_id + 1);
    }
    for (prescription = system->prescriptions; prescription != NULL; prescription = prescription->next) {
        system->next_prescription_id = max_int(system->next_prescription_id, prescription->prescription_id + 1);
    }
    for (log_node = system->logs; log_node != NULL; log_node = log_node->next) {
        system->next_log_id = max_int(system->next_log_id, log_node->log_id + 1);
    }
}

/* 去掉字符串末尾的回车换行 */
void trim_newline(char *text) {
    size_t len = 0;
    if (text == NULL) {
        return;
    }
    len = strlen(text);
    while (len > 0 && (text[len - 1] == '\n' || text[len - 1] == '\r')) {
        text[len - 1] = '\0';
        len--;
    }
}

/* 去掉文本开头可能存在的 UTF-8 BOM */
void strip_utf8_bom(char *text) {
    unsigned char *bytes = (unsigned char *)text;
    if (text == NULL) {
        return;
    }
    if (bytes[0] == 0xEF && bytes[1] == 0xBB && bytes[2] == 0xBF) {
        memmove(text, text + 3, strlen(text + 3) + 1);
    }//如果前三个字节是UTF-8 BOM，就去掉它们
}

/* 底层按行读取函数，统一处理 fgets 和换行符 */
static int read_line_prompt(const char *prompt, char *buffer, int size) {
    if (buffer == NULL || size <= 0) {
        return 0;
    }
    while (1) {
        printf("%s", prompt);
        if (fgets(buffer, size, stdin) == NULL) {
            if (feof(stdin)) {
                buffer[0] = '\0';
                return 0;
            }
            clearerr(stdin);
            continue;
        }
        trim_newline(buffer);
        return 1;
    }
}

/* 读取非空字符串 */
void read_string(const char *prompt, char *buffer, int size) {
    char temp[TEXT_LEN * 2];
    if (buffer == NULL || size <= 0) {
        return;
    }
    while (1) {
        printf("%s", prompt);
        if (fgets(temp, sizeof(temp), stdin) == NULL) {
            if (feof(stdin)) {
                buffer[0] = '\0';
                return;
            }
            clearerr(stdin);
            continue;
        }
        trim_newline(temp);
        if (!is_blank_string(temp)) {
            safe_copy(buffer, temp, size);
            return;
        }
        printf("输入不能为空，请重新输入\n");
    }
}

/* 读取合法整数 */
int read_int(const char *prompt) {
    char buffer[128];
    int value = 0;
    char extra = '\0';
    while (1) {
        printf("%s", prompt);
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            if (feof(stdin)) {
                return INT_MIN;
            }
            clearerr(stdin);
            continue;
        }
        if (sscanf(buffer, "%d %c", &value, &extra) == 1) {
            return value;
        }
        printf("请输入合法整数\n");
    }
}

/* 读取指定范围内的整数 */
int read_int_in_range(const char *prompt, int min_value, int max_value) {
    int value = 0;
    while (1) {
        value = read_int(prompt);
        if (value == INT_MIN) {
            return min_value;
        }
        if (value >= min_value && value <= max_value) {
            return value;
        }
        printf("输入超出范围，请输入 %d ~ %d 之间的数值\n", min_value, max_value);
    }
}

/* 读取合法浮点数 */
double read_double(const char *prompt) {
    char buffer[128];
    double value = 0.0;
    char extra = '\0';
    while (1) {
        printf("%s", prompt);
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            if (feof(stdin)) {
                return 0.0;
            }
            clearerr(stdin);
            continue;
        }
        if (sscanf(buffer, "%lf %c", &value, &extra) == 1) {
            return value;
        }
        printf("请输入合法数值\n");
    }
}

/* 读取 yes/no 类型确认输入 */
int read_yes_no(const char *prompt) {
    char buffer[32];
    while (1) {
        printf("%s", prompt);
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            if (feof(stdin)) {
                return 0;
            }
            clearerr(stdin);
            continue;
        }
        trim_newline(buffer);
        if (buffer[0] == 'y' || buffer[0] == 'Y' || strcmp(buffer, "1") == 0) {
            return 1;
        }
        if (buffer[0] == 'n' || buffer[0] == 'N' || strcmp(buffer, "0") == 0) {
            return 0;
        }
        printf("请输入 y/n\n");
    }
}

/* 判断输入是否表示“返回上一步” */
static int is_back_command(const char *text) {
    const char *start = text;
    const char *end = NULL;
    size_t len = 0;
    if (text == NULL) {
        return 0;
    }
    while (*start != '\0' && isspace((unsigned char)*start)) {
        start++;
    }
    end = start + strlen(start);
    while (end > start && isspace((unsigned char)*(end - 1))) {
        end--;
    }
    len = (size_t)(end - start);
    return (len == 1 && (start[0] == 'b' || start[0] == 'B')) ||
           (len == 4 &&
            tolower((unsigned char)start[0]) == 'b' &&
            tolower((unsigned char)start[1]) == 'a' &&
            tolower((unsigned char)start[2]) == 'c' &&
            tolower((unsigned char)start[3]) == 'k');
}

/* 读取步骤式字符串输入。
 * 返回 -1 表示用户返回上一步，或输入流已经结束，需要由上层取消当前流程。
 */
int read_string_step(const char *prompt, char *buffer, int size, int allow_back) {
    char temp[TEXT_LEN * 2];
    while (1) {
        if (!read_line_prompt(prompt, temp, (int)sizeof(temp))) {
            return -1;
        }
        if (allow_back && is_back_command(temp)) {
            return -1;
        }
        if (is_blank_string(temp)) {
            if (buffer[0] != '\0') {
                return 1;
            }
            printf("输入不能为空，请重新输入\n");
            continue;
        }
        safe_copy(buffer, temp, size);
        return 1;
    }
}

/* 读取步骤式整数输入。
 * EOF 和返回上一步都统一返回 -1，避免自动化测试输入不足时反复提示。
 */
int read_int_step(const char *prompt, int *value, int min_value, int max_value, int allow_back) {
    char temp[128];
    int parsed_value = 0;
    char extra = '\0';
    while (1) {
        if (!read_line_prompt(prompt, temp, (int)sizeof(temp))) {
            return -1;
        }
        if (allow_back && is_back_command(temp)) {
            return -1;
        }
        if (is_blank_string(temp)) {
            if (value != NULL && *value >= min_value && *value <= max_value) {
                return 1;
            }
            printf("输入不能为空，请重新输入\n");
            continue;
        }
        if (sscanf(temp, "%d %c", &parsed_value, &extra) != 1) {
            printf("请输入合法整数\n");
            continue;
        }
        if (parsed_value < min_value || parsed_value > max_value) {
            printf("输入超出范围，请输入 %d ~ %d 之间的数值\n", min_value, max_value);
            continue;
        }
        *value = parsed_value;
        return 1;
    }
}

/* 读取步骤式浮点数输入。
 * 这里不直接退出程序，而是把取消状态交给具体业务流程处理。
 */
int read_double_step(const char *prompt, double *value, double min_value, int allow_back) {
    char temp[128];
    double parsed_value = 0.0;
    char extra = '\0';
    while (1) {
        if (!read_line_prompt(prompt, temp, (int)sizeof(temp))) {
            return -1;
        }
        if (allow_back && is_back_command(temp)) {
            return -1;
        }
        if (is_blank_string(temp)) {
            if (value != NULL && *value >= min_value) {
                return 1;
            }
            printf("输入不能为空，请重新输入\n");
            continue;
        }
        if (sscanf(temp, "%lf %c", &parsed_value, &extra) != 1) {
            printf("请输入合法数值。\n");
            continue;
        }
        if (parsed_value < min_value) {
            printf("输入值不能小于 %.2f。\n", min_value);
            continue;
        }
        *value = parsed_value;
        return 1;
    }
}

/* 读取并校验手机号，输入结束时让上层回滚当前表单。 */
int read_phone_step(const char *prompt, char *buffer, int size, int allow_back) {
    char temp[PHONE_LEN];
    while (1) {
        if (!read_line_prompt(prompt, temp, (int)sizeof(temp))) {
            return -1;
        }
        if (allow_back && is_back_command(temp)) {
            return -1;
        }
        if (is_blank_string(temp) && buffer[0] != '\0') {
            return 1;
        }
        if (!is_valid_phone(temp)) {
            printf("手机号格式非法，请输入 11 位数字\n");
            continue;
        }
        safe_copy(buffer, temp, size);
        return 1;
    }
}

/* 读取并校验身份证号，输入结束时让上层回滚当前表单。 */
int read_id_card_step(const char *prompt, char *buffer, int size, int allow_back) {
    char temp[ID_LEN];
    while (1) {
        if (!read_line_prompt(prompt, temp, (int)sizeof(temp))) {
            return -1;
        }
        if (allow_back && is_back_command(temp)) {
            return -1;
        }
        if (is_blank_string(temp) && buffer[0] != '\0') {
            return 1;
        }
        if (!is_valid_id_card(temp)) {
            printf("身份证号非法，请确认 18 位号码、出生日期和校验码是否正确。\n");
            continue;
        }
        safe_copy(buffer, temp, size);
        return 1;
    }
}

/* 判断字符串是否为空白 */
int is_blank_string(const char *text) {
    while (text != NULL && *text != '\0') {
        if (!isspace((unsigned char)*text)) {
            return 0;
        }
        text++;
    }
    return 1;
}

/* 校验手机号格式：要求为 11 位数字 */
int is_valid_phone(const char *text) {
    int i = 0;
    if (text == NULL || strlen(text) != 11) {
        return 0;
    }
    for (i = 0; i < 11; i++) {
        if (!isdigit((unsigned char)text[i])) {
            return 0;
        }
    }
    return 1;
}

static int id_card_birth_parts(const char *text, int *year, int *month, int *day) {
    char temp[5];
    if (text == NULL || strlen(text) != 18) {
        return 0;
    }
    memcpy(temp, text + 6, 4);
    temp[4] = '\0';
    *year = atoi(temp);
    memcpy(temp, text + 10, 2);
    temp[2] = '\0';
    *month = atoi(temp);
    memcpy(temp, text + 12, 2);
    temp[2] = '\0';
    *day = atoi(temp);
    return 1;
}

static int id_card_birth_is_valid(int year, int month, int day) {
    struct tm tm_value;
    time_t time_value;
    struct tm *checked_time = NULL;
    if (year < 1900 || month < 1 || month > 12 || day < 1 || day > 31) {
        return 0;
    }
    memset(&tm_value, 0, sizeof(tm_value));
    tm_value.tm_year = year - 1900;
    tm_value.tm_mon = month - 1;
    tm_value.tm_mday = day;
    tm_value.tm_hour = 12;
    tm_value.tm_isdst = -1;
    time_value = mktime(&tm_value);
    if (time_value == (time_t)-1) {
        return 0;
    }
    checked_time = localtime(&time_value);
    return checked_time != NULL &&
           checked_time->tm_year == year - 1900 &&
           checked_time->tm_mon == month - 1 &&
           checked_time->tm_mday == day;
}

/* 校验身份证号：18 位、出生日期合法，并通过校验码 */
int is_valid_id_card(const char *text) {
    static const int weights[17] = {7, 9, 10, 5, 8, 4, 2, 1, 6, 3, 7, 9, 10, 5, 8, 4, 2};
    static const char checks[11] = {'1', '0', 'X', '9', '8', '7', '6', '5', '4', '3', '2'};
    int i = 0;
    int sum = 0;
    int year = 0;
    int month = 0;
    int day = 0;
    char expected = '\0';
    char actual = '\0';
    if (text == NULL || strlen(text) != 18) {
        return 0;
    }
    for (i = 0; i < 17; i++) {
        if (!isdigit((unsigned char)text[i])) {
            return 0;
        }
        sum += (text[i] - '0') * weights[i];
    }
    actual = (char)toupper((unsigned char)text[17]);
    if (!(isdigit((unsigned char)actual) || actual == 'X')) {
        return 0;
    }
    if (!id_card_birth_parts(text, &year, &month, &day) ||
        !id_card_birth_is_valid(year, month, day)) {
        return 0;
    }
    expected = checks[sum % 11];
    return actual == expected;
}

int age_from_id_card(const char *id_card) {
    int year = 0;
    int month = 0;
    int day = 0;
    time_t now;
    struct tm *today = NULL;
    int age = 0;
    if (!is_valid_id_card(id_card) || !id_card_birth_parts(id_card, &year, &month, &day)) {
        return -1;
    }
    now = time(NULL);
    today = localtime(&now);
    if (today == NULL) {
        return -1;
    }
    age = today->tm_year + 1900 - year;
    if (today->tm_mon + 1 < month ||
        (today->tm_mon + 1 == month && today->tm_mday < day)) {
        age--;
    }
    return age;
}

void generate_initial_password(const char *prefix, int id, char *buffer, int size) {
    if (buffer == NULL || size <= 0) {
        return;
    }
    snprintf(buffer, (size_t)size, "%s%d", prefix != NULL ? prefix : "U", id);
}

/* 把字符串转换为小写 */
void to_lower_string(const char *src, char *dst, int size) {
    int i = 0;
    if (src == NULL || dst == NULL || size <= 0) {
        return;
    }
    while (src[i] != '\0' && i < size - 1) {
        dst[i] = (char)tolower((unsigned char)src[i]);
        i++;
    }
    dst[i] = '\0';
}

/* 不区分大小写判断关键字是否出现 */
int str_contains_ignore_case(const char *text, const char *keyword) {
    char lower_text[TEXT_LEN * 2];
    char lower_keyword[TEXT_LEN];
    if (text == NULL || keyword == NULL) {
        return 0;
    }
    to_lower_string(text, lower_text, (int)sizeof(lower_text));
    to_lower_string(keyword, lower_keyword, (int)sizeof(lower_keyword));
    return strstr(lower_text, lower_keyword) != NULL;
}

/* 安全复制字符串，保证目标串一定以 '\0' 结尾 */
void safe_copy(char *dst, const char *src, int size) {
    if (dst == NULL || size <= 0) {
        return;
    }
    if (src == NULL) {
        dst[0] = '\0';
        return;
    }
    strncpy(dst, src, (size_t)(size - 1));
    dst[size - 1] = '\0';
}

/* 生成药品显示名 */
void build_drug_display_name(const Drug *drug, char *buffer, int size) {
    int has_common = 0;
    int has_brand = 0;
    if (buffer == NULL || size <= 0) {
        return;
    }
    buffer[0] = '\0';
    if (drug == NULL) {
        safe_copy(buffer, "未知药品", size);
        return;
    }
    has_common = !is_blank_string(drug->common_name) && strcmp(drug->common_name, drug->generic_name) != 0;
    has_brand = !is_blank_string(drug->brand_name) &&
                strcmp(drug->brand_name, drug->generic_name) != 0 &&
                (!has_common || strcmp(drug->brand_name, drug->common_name) != 0);
    if (has_common && has_brand) {
        snprintf(buffer, (size_t)size, "%s（%s/%s）", drug->generic_name, drug->common_name, drug->brand_name);
    } else if (has_common) {
        snprintf(buffer, (size_t)size, "%s（%s）", drug->generic_name, drug->common_name);
    } else if (has_brand) {
        snprintf(buffer, (size_t)size, "%s（%s）", drug->generic_name, drug->brand_name);
    } else {
        safe_copy(buffer, drug->generic_name, size);
    }
    buffer[size - 1] = '\0';
}

/* 规范化性别输入 */
int normalize_gender(const char *input, char *output, int size) {
    if (input == NULL || output == NULL || size <= 0) {
        return 0;
    }
    if (strcmp(input, "男") == 0) {
        safe_copy(output, "男", size);
        return 1;
    }
    if (strcmp(input, "女") == 0) {
        safe_copy(output, "女", size);
        return 1;
    }
    return 0;
}

/* 规范化血型输入 */
int normalize_blood_type(const char *input, char *output, int size) {
    char lower[SMALL_LEN];
    if (input == NULL || output == NULL || size <= 0) {
        return 0;
    }
    to_lower_string(input, lower, SMALL_LEN);
    if (strcmp(lower, "a") == 0) {
        safe_copy(output, "A", size);
        return 1;
    }
    if (strcmp(lower, "b") == 0) {
        safe_copy(output, "B", size);
        return 1;
    }
    if (strcmp(lower, "o") == 0) {
        safe_copy(output, "O", size);
        return 1;
    }
    if (strcmp(lower, "ab") == 0) {
        safe_copy(output, "AB", size);
        return 1;
    }
    return 0;
}

/* 获取当前日期 */
void get_current_date(char *buffer) {
    time_t now;
    struct tm *tm_info;
    now = time(NULL);
    tm_info = localtime(&now);
    if (buffer == NULL) {
        return;
    }
    sprintf(buffer, "%04d-%02d-%02d", tm_info->tm_year + 1900, tm_info->tm_mon + 1, tm_info->tm_mday);
}

/* 获取当前完整时间 */
void get_current_datetime(char *buffer) {
    time_t now;
    struct tm *tm_info;
    now = time(NULL);
    tm_info = localtime(&now);
    if (buffer == NULL) {
        return;
    }
    sprintf(buffer, "%04d-%02d-%02d %02d:%02d:%02d",
            tm_info->tm_year + 1900,
            tm_info->tm_mon + 1,
            tm_info->tm_mday,
            tm_info->tm_hour,
            tm_info->tm_min,
            tm_info->tm_sec);
}

static int parse_date_parts(const char *date_text, int *year, int *month, int *day) {
    int consumed = 0;
    int y = 0;
    int m = 0;
    int d = 0;
    if (date_text == NULL || strlen(date_text) != 10 ||
        date_text[4] != '-' || date_text[7] != '-') {
        return 0;
    }
    if (sscanf(date_text, "%4d-%2d-%2d%n", &y, &m, &d, &consumed) != 3 ||
        consumed != 10) {
        return 0;
    }
    if (year != NULL) {
        *year = y;
    }
    if (month != NULL) {
        *month = m;
    }
    if (day != NULL) {
        *day = d;
    }
    return 1;
}

static int date_parts_are_valid(int year, int month, int day) {
    struct tm tm_value;
    time_t time_value;
    struct tm *checked_time = NULL;
    if (year < 1900 || month < 1 || month > 12 || day < 1 || day > 31) {
        return 0;
    }
    memset(&tm_value, 0, sizeof(tm_value));
    tm_value.tm_year = year - 1900;
    tm_value.tm_mon = month - 1;
    tm_value.tm_mday = day;
    tm_value.tm_hour = 12;
    tm_value.tm_isdst = -1;
    time_value = mktime(&tm_value);
    if (time_value == (time_t)-1) {
        return 0;
    }
    checked_time = localtime(&time_value);
    return checked_time != NULL &&
           checked_time->tm_year == year - 1900 &&
           checked_time->tm_mon == month - 1 &&
           checked_time->tm_mday == day;
}

int is_valid_date_text(const char *date_text) {
    int year = 0;
    int month = 0;
    int day = 0;
    if (!parse_date_parts(date_text, &year, &month, &day)) {
        return 0;
    }
    return date_parts_are_valid(year, month, day);
}

int add_days_to_date(const char *date_text, int days, char *buffer, int size) {
    int year = 0;
    int month = 0;
    int day = 0;
    struct tm tm_value;
    time_t time_value;
    struct tm *result_time = NULL;
    if (buffer == NULL || size <= 0 || !parse_date_parts(date_text, &year, &month, &day) ||
        !date_parts_are_valid(year, month, day)) {
        return 0;
    }
    memset(&tm_value, 0, sizeof(tm_value));
    tm_value.tm_year = year - 1900;
    tm_value.tm_mon = month - 1;
    tm_value.tm_mday = day + days;
    tm_value.tm_hour = 12;
    tm_value.tm_isdst = -1;
    time_value = mktime(&tm_value);
    if (time_value == (time_t)-1) {
        return 0;
    }
    result_time = localtime(&time_value);
    if (result_time == NULL) {
        return 0;
    }
    snprintf(buffer, (size_t)size, "%04d-%02d-%02d",
             result_time->tm_year + 1900, result_time->tm_mon + 1, result_time->tm_mday);
    buffer[size - 1] = '\0';
    return 1;
}

int add_years_to_date(const char *date_text, int years, char *buffer, int size) {
    int year = 0;
    int month = 0;
    int day = 0;
    int target_year = 0;
    if (buffer == NULL || size <= 0 || !parse_date_parts(date_text, &year, &month, &day) ||
        !date_parts_are_valid(year, month, day)) {
        return 0;
    }
    target_year = year + years;
    while (day > 1 && !date_parts_are_valid(target_year, month, day)) {
        day--;
    }
    if (!date_parts_are_valid(target_year, month, day)) {
        return 0;
    }
    snprintf(buffer, (size_t)size, "%04d-%02d-%02d", target_year, month, day);
    buffer[size - 1] = '\0';
    return 1;
}

/* 把日期字符串转成天数序列值，便于做区间比较和差值计算 */
int date_to_serial(const char *date_text) {
    int year = 0;
    int month = 0;
    int day = 0;
    struct tm tm_value;
    time_t time_value;
    if (date_text == NULL || sscanf(date_text, "%d-%d-%d", &year, &month, &day) != 3) {
        return 0;
    }
    memset(&tm_value, 0, sizeof(tm_value));
    tm_value.tm_year = year - 1900;
    tm_value.tm_mon = month - 1;
    tm_value.tm_mday = day;
    time_value = mktime(&tm_value);
    if (time_value == (time_t)-1) {
        return 0;
    }
    return (int)(time_value / (24 * 3600));
}

/* 计算两个日期之间相差的天数 */
int days_between(const char *date_a, const char *date_b) {
    int serial_a = date_to_serial(date_a);
    int serial_b = date_to_serial(date_b);
    return serial_b - serial_a;
}

/* 打印指定字符组成的一条横线 */
void print_line(char ch, int count) {
    int i = 0;
    for (i = 0; i < count; i++) {
        putchar(ch);
    }
    putchar('\n');
}

/* 打印统一风格的标题栏 */
void print_title(const char *title) {
    print_line('=', 72);
    printf("%s\n", title);
    print_line('-', 72);
}

/* 计算 UTF-8 字符的字节长度 */
int utf8_char_size(unsigned char ch) {
    if ((ch & 0x80) == 0x00) {
        return 1;
    }//情况1：0xxxxxxx → ASCII字符，占1字节
    if ((ch & 0xE0) == 0xC0) {
        return 2;
    }//情况2：110xxxx → 2字节字符，占2字节
    if ((ch & 0xF0) == 0xE0) {
        return 3;
    }//情况3：1110xxxx → 3字节字符，占3字节
    if ((ch & 0xF8) == 0xF0) {
        return 4;
    }//情况4：11110xxxx → 4字节字符，占4字节
    return 1;//默认返回1字节
}

/* 格式化表格单元格内容
 * src 是原来的文字，dst 是处理后要放进去的地方
 * dst_size 是 dst 真正能装多少字节，防止写超
 * max_width 是这一格想显示多宽，不是字节数
 * 这个函数主要是为了让表格排得整齐
 * 英文和数字一般算 1 格宽
 * 中文一般算 2 格宽
 * UTF-8 里的中文会占多个字节，所以复制时要按一个完整字符来复制
 * 内容太长就截掉，再加两个点表示后面省略了
 * 内容不够宽就补空格，让每一列看起来一样宽
 */
void format_display_cell(const char *src, char *dst, int dst_size, int max_width) {
    // src_index 表示现在读到 src 的哪个字节
    int src_index = 0;
    // dst_index 表示现在写到 dst 的哪个字节
    int dst_index = 0;
    // width 表示现在已经占了多少显示宽度
    int width = 0;
    // truncated 用来记一下文字有没有被截断
    int truncated = 0;
    // source 是真正要读取的字符串
    const char *source = src;
    // temp 用来处理 src 和 dst 是同一个地址的情况，避免边读边写把原内容盖掉
    char temp[TEXT_LEN * 2];

    // dst 不能用时就直接返回
    if (dst == NULL || dst_size <= 0) {
        return;
    }
    // src 为空时按空字符串处理
    if (source == NULL) {
        source = "";
    }
    // 如果 src 和 dst 是同一个地址，先备份一份再处理
    if (source == dst) {
        safe_copy(temp, source, (int)sizeof(temp));
        source = temp;
    }

    // 先把 dst 置空，后面即使没有写入也不会出问题
    dst[0] = '\0';
    // 一个字符一个字符地处理 source
    while (source[src_index] != '\0' && dst_index < dst_size - 1) {
        // char_size 是当前字符实际占几个字节
        int char_size = utf8_char_size((unsigned char)source[src_index]);
        // char_width 是当前字符显示出来占几格宽
        int char_width = ((unsigned char)source[src_index] < 0x80) ? 1 : 2;
        // 再放这个字符就超宽的话，就停下来并标记为截断
        if (width + char_width > max_width) {
            truncated = 1;
            break;
        }
        // dst 剩余空间放不下这个完整字符时也要停
        if (dst_index + char_size >= dst_size - 1) {
            break;
        }
        // 一次复制完整字符，避免中文被截成乱码
        memcpy(dst + dst_index, source + src_index, (size_t)char_size);
        // 读写位置都往后移动这个字符的字节数
        dst_index += char_size;
        src_index += char_size;
        // 显示宽度也跟着增加
        width += char_width;
    }
    // 如果前面截断了，并且格子放得下两个点，就在末尾加省略标记
    if (truncated && max_width >= 2) {
        // 先往回退一点位置，给两个点留空间
        while (width > max_width - 2 && dst_index > 0) {
            int char_start = dst_index - 1;
            while (char_start > 0 && ((unsigned char)dst[char_start] & 0xC0) == 0x80) {
                char_start--;
            }
            width -= ((unsigned char)dst[char_start] < 0x80) ? 1 : 2;
            dst_index = char_start;
        }
        // 空间够的话就写入两个点
        if (dst_index < dst_size - 2) {
            dst[dst_index++] = '.';
            dst[dst_index++] = '.';
            // 两个点一共补回 2 格显示宽度
            width += 2;
        }
    }
    // 不够宽的地方用空格补齐
    while (width < max_width && dst_index < dst_size - 1) {
        dst[dst_index++] = ' ';
        width++;
    }
    // 最后补上字符串结束符
    dst[dst_index] = '\0';
}

/* 打印表格边框 */
void print_table_border(int total_width, char ch) {
    print_line(ch, total_width);
}

/* 等待用户按回车继续 */
void pause_screen(void) {
    char buffer[8];
    printf("按回车键继续...");
    if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
        clearerr(stdin);
    }
}

/* 把床位状态编号转换为中文文字 */
const char *bed_status_to_text(int status) {
    if (status == BED_STATUS_FREE) {
        return "空闲";
    }
    if (status == BED_STATUS_OCCUPIED) {
        return "已占用";
    }
    if (status == BED_STATUS_REPAIR) {
        return "维修";
    }
    return "未知";
}

/* 统计某个科室配置的床位总数 */
int count_department_beds(HospitalSystem *system, int department_id) {
    int count = 0;
    Bed *current = NULL;
    if (system == NULL) {
        return 0;
    }
    current = system->beds;
    while (current != NULL) {
        if (current->department_id == department_id) {
            count++;
        }
        current = current->next;
    }
    return count;
}

/* 统计某个科室当前已占用床位数 */
int count_department_occupied_beds(HospitalSystem *system, int department_id) {
    int count = 0;
    Bed *current = NULL;
    if (system == NULL) {
        return 0;
    }
    current = system->beds;
    while (current != NULL) {
        if (current->department_id == department_id && current->status == BED_STATUS_OCCUPIED) {
            count++;
        }
        current = current->next;
    }
    return count;
}

/* 统计某个科室在指定日期范围内的入院人数 */
int count_department_admissions_in_range(HospitalSystem *system, int department_id, int start_serial, int end_serial) {
    int count = 0;
    AdmissionHistory *current = NULL;
    if (system == NULL) {
        return 0;
    }
    current = system->admissions;
    while (current != NULL) {
        int serial = date_to_serial(current->admit_date);
        if (current->department_id == department_id && serial >= start_serial && serial <= end_serial) {
            count++;
        }
        current = current->next;
    }
    return count;
}

/* 返回今天对应的日期序列值 */
static int today_serial(void) {
    char today[DATE_LEN];
    get_current_date(today);
    return date_to_serial(today);
}

/* 统计某个科室近 N 天入院人数 */
int count_department_admissions_in_recent_days(HospitalSystem *system, int department_id, int window_days) {
    int latest = today_serial();
    if (window_days <= 0) {
        return 0;
    }
    return count_department_admissions_in_range(system, department_id, latest - window_days + 1, latest);
}

/* 统计某个科室近 N 天出院人数 */
int count_department_discharges_in_recent_days(HospitalSystem *system, int department_id, int window_days) {
    int count = 0;
    int latest = today_serial();
    AdmissionHistory *current = NULL;
    if (system == NULL || window_days <= 0) {
        return 0;
    }
    current = system->admissions;
    while (current != NULL) {
        if (current->department_id == department_id && strcmp(current->discharge_date, "-") != 0) {
            int serial = date_to_serial(current->discharge_date);
            if (serial >= latest - window_days + 1 && serial <= latest) {
                count++;
            }
        }
        current = current->next;
    }
    return count;
}

/* 计算某个科室已出院患者的平均住院天数，没有历史数据时默认按 3 天算 */
double average_department_stay_days(HospitalSystem *system, int department_id) {
    int total_days = 0;
    int total_count = 0;
    AdmissionHistory *current = NULL;
    if (system == NULL) {
        return 3.0;
    }
    current = system->admissions;
    while (current != NULL) {
        if (current->department_id == department_id && current->status == ADMISSION_DISCHARGED) {
            total_days += current->stay_days;
            total_count++;
        }
        current = current->next;
    }
    if (total_count == 0) {
        return 3.0;
    }
    return (double)total_days / total_count;
}

/* 追加日志到日志链表尾部
 * 日志不会立即写文件，而是由保存流程统一落盘
 */
int append_log(HospitalSystem *system, const char *role, const char *action, const char *detail) {
    OperationLog *node = NULL;
    OperationLog *tail = NULL;
    if (system == NULL) {
        return 0;
    }
    node = (OperationLog *)malloc(sizeof(OperationLog));
    if (node == NULL) {
        return 0;
    }
    node->log_id = system->next_log_id++;
    get_current_date(node->date);
    safe_copy(node->role, role, NAME_LEN);
    safe_copy(node->action, action, NAME_LEN);
    safe_copy(node->detail, detail, TEXT_LEN);
    node->next = NULL;

    if (system->logs == NULL) {
        system->logs = node;
        return 1;
    }
    tail = system->logs;
    while (tail->next != NULL) {
        tail = tail->next;
    }
    tail->next = node;
    return 1;
}

#endif
