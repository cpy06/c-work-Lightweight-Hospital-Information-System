/*
 * record.c
 * ------------------------------------------------------------
 * 医疗记录与住院处理模块
 * 负责病历新增、患者挂号、病史查看、住院办理、出院办理
 * 和住院记录展示等核心业务流程
 */

#include "system.h"

#if defined(HIS_BUILD_FROM_MAIN) || !HIS_MAIN_ONE_CLICK_BUILD

/* 按记录编号查找病历 */
MedicalRecord *find_record_by_id(HospitalSystem *system, int record_id) {
    MedicalRecord *current = system->records;
    while (current != NULL) {
        if (current->record_id == record_id) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

#define RECORD_COL_ID 8
#define RECORD_COL_PATIENT 8
#define RECORD_COL_DOCTOR 8
#define RECORD_COL_DEPT 8
#define RECORD_COL_TYPE 8
#define RECORD_COL_DATE 19
#define RECORD_COL_DIAG 18
#define RECORD_COL_COST 10
#define RECORD_COL_NOTE 20
#define ADMISSION_COL_STATUS 10
#define ADMISSION_COL_DIAG 18

/* 病历表格总宽度 */
static int record_table_width(int include_patient) {
    return RECORD_COL_ID + (include_patient ? RECORD_COL_PATIENT : 0) + RECORD_COL_DOCTOR + RECORD_COL_DEPT +
           RECORD_COL_TYPE + RECORD_COL_DATE + RECORD_COL_DIAG + RECORD_COL_COST + RECORD_COL_NOTE +
           (include_patient ? 3 * 9 + 1 : 3 * 8 + 1);
}

/* 住院历史表格总宽度 */
static int admission_table_width(void) {
    return RECORD_COL_ID + RECORD_COL_PATIENT + RECORD_COL_DOCTOR + RECORD_COL_DEPT + RECORD_COL_PATIENT +
           RECORD_COL_PATIENT + 12 + 12 + RECORD_COL_PATIENT + ADMISSION_COL_STATUS + ADMISSION_COL_DIAG + 3 * 11 + 1;
}

/* 打印病历表头 */
static void print_record_header(int include_patient) {
    char id[32], patient[32], doctor[32], dept[32], type[32], date[32], diag[48], cost[32], note[48];
    print_table_border(record_table_width(include_patient), '=');
    format_display_cell("记录ID", id, (int)sizeof(id), RECORD_COL_ID);
    format_display_cell("医生", doctor, (int)sizeof(doctor), RECORD_COL_DOCTOR);
    format_display_cell("科室", dept, (int)sizeof(dept), RECORD_COL_DEPT);
    format_display_cell("类型", type, (int)sizeof(type), RECORD_COL_TYPE);
    format_display_cell("日期时间", date, (int)sizeof(date), RECORD_COL_DATE);
    format_display_cell("诊断", diag, (int)sizeof(diag), RECORD_COL_DIAG);
    format_display_cell("费用", cost, (int)sizeof(cost), RECORD_COL_COST);
    format_display_cell("备注", note, (int)sizeof(note), RECORD_COL_NOTE);
    if (include_patient) {
        format_display_cell("患者", patient, (int)sizeof(patient), RECORD_COL_PATIENT);
        printf("| %s | %s | %s | %s | %s | %s | %s | %s | %s |\n",
               id, patient, doctor, dept, type, date, diag, cost, note);
    } else {
        printf("| %s | %s | %s | %s | %s | %s | %s | %s |\n",
               id, doctor, dept, type, date, diag, cost, note);
    }
    print_table_border(record_table_width(include_patient), '-');
}

/* 打印单行病历 */
static void print_record_row(const MedicalRecord *record, int include_patient) {
    char raw_id[32], id[32], raw_patient[32], patient[32], raw_doctor[32], doctor[32], raw_dept[32], dept[32];
    char type[TYPE_LEN * 2], date[32], diag[TEXT_LEN * 2], raw_cost[32], cost[32], note[TEXT_LEN * 2];
    if (record == NULL) {
        return;
    }
    snprintf(raw_id, sizeof(raw_id), "%d", record->record_id);
    snprintf(raw_patient, sizeof(raw_patient), "%d", record->patient_id);
    snprintf(raw_doctor, sizeof(raw_doctor), "%d", record->doctor_id);
    snprintf(raw_dept, sizeof(raw_dept), "%d", record->department_id);
    snprintf(raw_cost, sizeof(raw_cost), "%.2f", record->cost);
    format_display_cell(raw_id, id, (int)sizeof(id), RECORD_COL_ID);
    format_display_cell(raw_patient, patient, (int)sizeof(patient), RECORD_COL_PATIENT);
    format_display_cell(raw_doctor, doctor, (int)sizeof(doctor), RECORD_COL_DOCTOR);
    format_display_cell(raw_dept, dept, (int)sizeof(dept), RECORD_COL_DEPT);
    format_display_cell(record->record_type, type, (int)sizeof(type), RECORD_COL_TYPE);
    format_display_cell(record->date, date, (int)sizeof(date), RECORD_COL_DATE);
    format_display_cell(record->diagnosis, diag, (int)sizeof(diag), RECORD_COL_DIAG);
    format_display_cell(raw_cost, cost, (int)sizeof(cost), RECORD_COL_COST);
    format_display_cell(record->note, note, (int)sizeof(note), RECORD_COL_NOTE);
    if (include_patient) {
        printf("| %s | %s | %s | %s | %s | %s | %s | %s | %s |\n",
               id, patient, doctor, dept, type, date, diag, cost, note);
    } else {
        printf("| %s | %s | %s | %s | %s | %s | %s | %s |\n",
               id, doctor, dept, type, date, diag, cost, note);
    }
}

/* 打印住院历史表头 */
static void print_admission_header(void) {
    char admission[32], patient[32], doctor[32], dept[32], ward[32], bed[32], admit[32], discharge[32], stay[32], status[32], diag[48];
    print_table_border(admission_table_width(), '=');
    format_display_cell("住院ID", admission, (int)sizeof(admission), RECORD_COL_ID);
    format_display_cell("患者", patient, (int)sizeof(patient), RECORD_COL_PATIENT);
    format_display_cell("医生", doctor, (int)sizeof(doctor), RECORD_COL_DOCTOR);
    format_display_cell("科室", dept, (int)sizeof(dept), RECORD_COL_DEPT);
    format_display_cell("病房", ward, (int)sizeof(ward), RECORD_COL_PATIENT);
    format_display_cell("床位", bed, (int)sizeof(bed), RECORD_COL_PATIENT);
    format_display_cell("入院日期", admit, (int)sizeof(admit), 12);
    format_display_cell("出院日期", discharge, (int)sizeof(discharge), 12);
    format_display_cell("住院天数", stay, (int)sizeof(stay), RECORD_COL_PATIENT);
    format_display_cell("状态", status, (int)sizeof(status), ADMISSION_COL_STATUS);
    format_display_cell("诊断", diag, (int)sizeof(diag), ADMISSION_COL_DIAG);
    printf("| %s | %s | %s | %s | %s | %s | %s | %s | %s | %s | %s |\n",
           admission, patient, doctor, dept, ward, bed, admit, discharge, stay, status, diag);
    print_table_border(admission_table_width(), '-');
}

/* 打印单行住院历史记录 */
static void print_admission_row(const AdmissionHistory *admission) {
    char raw_admission[32], admission_id[32], raw_patient[32], patient[32], raw_doctor[32], doctor[32], raw_dept[32], dept[32];
    char raw_ward[32], ward[32], raw_bed[32], bed[32], admit[32], discharge[32], raw_stay[32], stay[32], status[32], diag[TEXT_LEN * 2];
    if (admission == NULL) {
        return;
    }
    snprintf(raw_admission, sizeof(raw_admission), "%d", admission->admission_id);
    snprintf(raw_patient, sizeof(raw_patient), "%d", admission->patient_id);
    snprintf(raw_doctor, sizeof(raw_doctor), "%d", admission->doctor_id);
    snprintf(raw_dept, sizeof(raw_dept), "%d", admission->department_id);
    snprintf(raw_ward, sizeof(raw_ward), "%d", admission->ward_id);
    snprintf(raw_bed, sizeof(raw_bed), "%d", admission->bed_id);
    snprintf(raw_stay, sizeof(raw_stay), "%d", admission->stay_days);
    format_display_cell(raw_admission, admission_id, (int)sizeof(admission_id), RECORD_COL_ID);
    format_display_cell(raw_patient, patient, (int)sizeof(patient), RECORD_COL_PATIENT);
    format_display_cell(raw_doctor, doctor, (int)sizeof(doctor), RECORD_COL_DOCTOR);
    format_display_cell(raw_dept, dept, (int)sizeof(dept), RECORD_COL_DEPT);
    format_display_cell(raw_ward, ward, (int)sizeof(ward), RECORD_COL_PATIENT);
    format_display_cell(raw_bed, bed, (int)sizeof(bed), RECORD_COL_PATIENT);
    format_display_cell(admission->admit_date, admit, (int)sizeof(admit), 12);
    format_display_cell(admission->discharge_date, discharge, (int)sizeof(discharge), 12);
    format_display_cell(raw_stay, stay, (int)sizeof(stay), RECORD_COL_PATIENT);
    format_display_cell(admission->status == ADMISSION_ACTIVE ? "住院中" : "已出院", status, (int)sizeof(status), ADMISSION_COL_STATUS);
    format_display_cell(admission->diagnosis, diag, (int)sizeof(diag), ADMISSION_COL_DIAG);
    printf("| %s | %s | %s | %s | %s | %s | %s | %s | %s | %s | %s |\n",
           admission_id, patient, doctor, dept, ward, bed, admit, discharge, stay, status, diag);
}

/* 向病历链表尾部追加一条病历 */
void add_record(HospitalSystem *system, MedicalRecord record) {
    MedicalRecord *node = (MedicalRecord *)malloc(sizeof(MedicalRecord));
    MedicalRecord *tail = NULL;
    if (node == NULL) {
        return;
    }
    *node = record;
    node->next = NULL;
    if (system->records == NULL) {
        system->records = node;
    } else {
        tail = system->records;
        while (tail->next != NULL) {
            tail = tail->next;
        }
        tail->next = node;
    }
}

#define REG_COL_ID 8
#define REG_COL_PATIENT 8
#define REG_COL_DOCTOR 8
#define REG_COL_DEPT 8
#define REG_COL_TYPE 8
#define REG_COL_VISIT 19
#define REG_COL_STATUS 8
#define REG_COL_POS 6
#define REG_COL_SYMPTOM 18

static const char *registration_type_to_text(int registration_type) {
    if (registration_type == REG_TYPE_APPOINTMENT) {
        return "预约";
    }
    if (registration_type == REG_TYPE_WALKIN) {
        return "现场";
    }
    return "未知";
}

static const char *registration_status_to_text(int status) {
    if (status == REG_STATUS_WAITING) {
        return "待诊";
    }
    if (status == REG_STATUS_CALLED) {
        return "已叫号";
    }
    if (status == REG_STATUS_COMPLETED) {
        return "已完成";
    }
    if (status == REG_STATUS_CANCELED) {
        return "已取消";
    }
    return "未知";
}

static int registration_table_width(void) {
    return REG_COL_ID + REG_COL_PATIENT + REG_COL_DOCTOR + REG_COL_DEPT + REG_COL_TYPE +
           REG_COL_VISIT + REG_COL_STATUS + REG_COL_POS + REG_COL_SYMPTOM + 3 * 9 + 1;
}

static void print_registration_header(void) {
    char id[32], patient[32], doctor[32], dept[32], type[32], visit[32], status[32], position[32], symptom[48];
    print_table_border(registration_table_width(), '=');
    format_display_cell("挂号ID", id, (int)sizeof(id), REG_COL_ID);
    format_display_cell("患者", patient, (int)sizeof(patient), REG_COL_PATIENT);
    format_display_cell("医生", doctor, (int)sizeof(doctor), REG_COL_DOCTOR);
    format_display_cell("科室", dept, (int)sizeof(dept), REG_COL_DEPT);
    format_display_cell("类型", type, (int)sizeof(type), REG_COL_TYPE);
    format_display_cell("就诊时间", visit, (int)sizeof(visit), REG_COL_VISIT);
    format_display_cell("状态", status, (int)sizeof(status), REG_COL_STATUS);
    format_display_cell("排队", position, (int)sizeof(position), REG_COL_POS);
    format_display_cell("诉求", symptom, (int)sizeof(symptom), REG_COL_SYMPTOM);
    printf("| %s | %s | %s | %s | %s | %s | %s | %s | %s |\n",
           id, patient, doctor, dept, type, visit, status, position, symptom);
    print_table_border(registration_table_width(), '-');
}

static int registration_sort_before(const OutpatientRegistration *left, const OutpatientRegistration *right) {
    int time_cmp = 0;
    if (left == NULL || right == NULL) {
        return 0;
    }
    time_cmp = strcmp(left->visit_time, right->visit_time);
    if (time_cmp != 0) {
        return time_cmp < 0;
    }
    return left->registration_id < right->registration_id;
}

static int registration_queue_position(HospitalSystem *system, const OutpatientRegistration *target) {
    int position = 1;
    OutpatientRegistration *current = NULL;
    if (system == NULL || target == NULL || target->status != REG_STATUS_WAITING) {
        return 0;
    }
    for (current = system->registrations; current != NULL; current = current->next) {
        if (current->doctor_id == target->doctor_id &&
            current->status == REG_STATUS_WAITING &&
            registration_sort_before(current, target)) {
            position++;
        }
    }
    return position;
}

static void print_registration_row(HospitalSystem *system, const OutpatientRegistration *registration) {
    char raw_id[32], id[32], raw_patient[32], patient[32], raw_doctor[32], doctor[32], raw_dept[32], dept[32];
    char type[32], visit[32], status[32], raw_position[32], position[32], symptom[TEXT_LEN * 2];
    int queue_position = 0;
    if (registration == NULL) {
        return;
    }
    queue_position = registration_queue_position(system, registration);
    snprintf(raw_id, sizeof(raw_id), "%d", registration->registration_id);
    snprintf(raw_patient, sizeof(raw_patient), "%d", registration->patient_id);
    snprintf(raw_doctor, sizeof(raw_doctor), "%d", registration->doctor_id);
    snprintf(raw_dept, sizeof(raw_dept), "%d", registration->department_id);
    if (queue_position > 0) {
        snprintf(raw_position, sizeof(raw_position), "%d", queue_position);
    } else {
        safe_copy(raw_position, "-", (int)sizeof(raw_position));
    }
    format_display_cell(raw_id, id, (int)sizeof(id), REG_COL_ID);
    format_display_cell(raw_patient, patient, (int)sizeof(patient), REG_COL_PATIENT);
    format_display_cell(raw_doctor, doctor, (int)sizeof(doctor), REG_COL_DOCTOR);
    format_display_cell(raw_dept, dept, (int)sizeof(dept), REG_COL_DEPT);
    format_display_cell(registration_type_to_text(registration->registration_type), type, (int)sizeof(type), REG_COL_TYPE);
    format_display_cell(registration->visit_time, visit, (int)sizeof(visit), REG_COL_VISIT);
    format_display_cell(registration_status_to_text(registration->status), status, (int)sizeof(status), REG_COL_STATUS);
    format_display_cell(raw_position, position, (int)sizeof(position), REG_COL_POS);
    format_display_cell(registration->symptom, symptom, (int)sizeof(symptom), REG_COL_SYMPTOM);
    printf("| %s | %s | %s | %s | %s | %s | %s | %s | %s |\n",
           id, patient, doctor, dept, type, visit, status, position, symptom);
}

static int normalize_visit_time(const char *input, char *output, int size) {
    int year = 0, month = 0, day = 0, hour = 0, minute = 0, second = 0;
    int consumed = 0;
    struct tm tm_value;
    time_t time_value;
    struct tm *checked_time = NULL;
    if (input == NULL || output == NULL || size <= 0) {
        return 0;
    }
    if (sscanf(input, "%d-%d-%d %d:%d:%d%n", &year, &month, &day, &hour, &minute, &second, &consumed) == 6) {
        if (input[consumed] != '\0') {
            return 0;
        }
    } else if (sscanf(input, "%d-%d-%d %d:%d%n", &year, &month, &day, &hour, &minute, &consumed) == 5) {
        if (input[consumed] != '\0') {
            return 0;
        }
        second = 0;
    } else if (sscanf(input, "%d-%d-%d %d-%d:%d%n", &year, &month, &day, &hour, &minute, &second, &consumed) == 6) {
        if (input[consumed] != '\0') {
            return 0;
        }
    } else if (sscanf(input, "%d-%d-%d %d-%d%n", &year, &month, &day, &hour, &minute, &consumed) == 5) {
        if (input[consumed] != '\0') {
            return 0;
        }
        second = 0;
    } else {
        return 0;
    }
    if (year < 2000 || month < 1 || month > 12 || day < 1 || day > 31 ||
        hour < 0 || hour > 23 || minute < 0 || minute > 59 || second < 0 || second > 59) {
        return 0;
    }
    memset(&tm_value, 0, sizeof(tm_value));
    tm_value.tm_year = year - 1900;
    tm_value.tm_mon = month - 1;
    tm_value.tm_mday = day;
    tm_value.tm_hour = hour;
    tm_value.tm_min = minute;
    tm_value.tm_sec = second;
    tm_value.tm_isdst = -1;
    time_value = mktime(&tm_value);
    if (time_value == (time_t)-1) {
        return 0;
    }
    checked_time = localtime(&time_value);
    if (checked_time == NULL ||
        checked_time->tm_year != year - 1900 ||
        checked_time->tm_mon != month - 1 ||
        checked_time->tm_mday != day ||
        checked_time->tm_hour != hour ||
        checked_time->tm_min != minute ||
        checked_time->tm_sec != second) {
        return 0;
    }
    snprintf(output, (size_t)size, "%04d-%02d-%02d %02d:%02d:%02d", year, month, day, hour, minute, second);
    return 1;
}

static int visit_time_is_after_now(const char *visit_time) {
    char current_time[DATE_LEN];
    if (visit_time == NULL) {
        return 0;
    }
    get_current_datetime(current_time);
    return strcmp(visit_time, current_time) > 0;
}

static OutpatientRegistration *find_current_called_registration(HospitalSystem *system, int doctor_id) {
    OutpatientRegistration *current = NULL;
    for (current = system->registrations; current != NULL; current = current->next) {
        if (current->doctor_id == doctor_id && current->status == REG_STATUS_CALLED) {
            return current;
        }
    }
    return NULL;
}

static OutpatientRegistration *find_next_waiting_registration(HospitalSystem *system, int doctor_id) {
    OutpatientRegistration *current = NULL;
    OutpatientRegistration *best = NULL;
    for (current = system->registrations; current != NULL; current = current->next) {
        if (current->doctor_id != doctor_id || current->status != REG_STATUS_WAITING) {
            continue;
        }
        if (best == NULL || registration_sort_before(current, best)) {
            best = current;
        }
    }
    return best;
}

static int has_pending_registration(HospitalSystem *system, int patient_id, int doctor_id) {
    OutpatientRegistration *current = NULL;
    for (current = system->registrations; current != NULL; current = current->next) {
        if (current->patient_id == patient_id &&
            current->doctor_id == doctor_id &&
            (current->status == REG_STATUS_WAITING || current->status == REG_STATUS_CALLED)) {
            return 1;
        }
    }
    return 0;
}

static int is_registration_charge_record(const MedicalRecord *record) {
    if (record == NULL) {
        return 0;
    }
    return strcmp(record->record_type, "预约挂号") == 0 ||
           strcmp(record->record_type, "现场挂号") == 0;
}

static const char *registration_charge_record_type(int registration_type) {
    if (registration_type == REG_TYPE_APPOINTMENT) {
        return "预约挂号";
    }
    if (registration_type == REG_TYPE_WALKIN) {
        return "现场挂号";
    }
    return "";
}

static int extract_registration_id_from_marker(const char *text, const char *marker, int *registration_id) {
    const char *start = NULL;
    char *end = NULL;
    long parsed = 0;
    if (text == NULL || marker == NULL || registration_id == NULL) {
        return 0;
    }
    start = strstr(text, marker);
    if (start == NULL) {
        return 0;
    }
    start += strlen(marker);
    parsed = strtol(start, &end, 10);
    if (end == start || parsed <= 0 || parsed > 999999999L) {
        return 0;
    }
    *registration_id = (int)parsed;
    return 1;
}

static int extract_charge_registration_id(const MedicalRecord *record, int *registration_id) {
    if (record == NULL) {
        return 0;
    }
    return extract_registration_id_from_marker(record->note, "门诊挂号ID:", registration_id);
}

static int charge_record_matches_registration_fields(const MedicalRecord *record, const OutpatientRegistration *registration) {
    if (record == NULL || registration == NULL) {
        return 0;
    }
    return record->patient_id == registration->patient_id &&
           record->doctor_id == registration->doctor_id &&
           record->department_id == registration->department_id &&
           strcmp(record->record_type, registration_charge_record_type(registration->registration_type)) == 0;
}

static int charge_record_date_matches_registration(const MedicalRecord *record, const OutpatientRegistration *registration) {
    if (record == NULL || registration == NULL) {
        return 0;
    }
    if (strcmp(record->date, registration->created_time) == 0) {
        return 1;
    }
    return strlen(record->date) >= 10 &&
           strlen(registration->created_time) >= 10 &&
           strncmp(record->date, registration->created_time, 10) == 0;
}

static int is_waived_registration_charge(const MedicalRecord *record) {
    if (record == NULL) {
        return 0;
    }
    return record->cost <= 0.0001 ||
           strstr(record->note, "已取消") != NULL ||
           strstr(record->note, "不计费") != NULL;
}

static int count_registration_charge_records(HospitalSystem *system, const OutpatientRegistration *registration, MedicalRecord **first_record) {
    MedicalRecord *current = NULL;
    int linked_registration_id = 0;
    int count = 0;
    if (system == NULL || registration == NULL) {
        return 0;
    }
    if (first_record != NULL) {
        *first_record = NULL;
    }
    for (current = system->records; current != NULL; current = current->next) {
        if (!is_registration_charge_record(current)) {
            continue;
        }
        if (extract_charge_registration_id(current, &linked_registration_id) &&
            linked_registration_id == registration->registration_id &&
            charge_record_matches_registration_fields(current, registration)) {
            if (first_record != NULL && *first_record == NULL) {
                *first_record = current;
            }
            count++;
        }
    }
    return count;
}

static int count_active_registration_charge_records(HospitalSystem *system, const OutpatientRegistration *registration) {
    MedicalRecord *current = NULL;
    int linked_registration_id = 0;
    int count = 0;
    if (system == NULL || registration == NULL) {
        return 0;
    }
    for (current = system->records; current != NULL; current = current->next) {
        if (!is_registration_charge_record(current)) {
            continue;
        }
        if (extract_charge_registration_id(current, &linked_registration_id) &&
            linked_registration_id == registration->registration_id &&
            charge_record_matches_registration_fields(current, registration) &&
            !is_waived_registration_charge(current)) {
            count++;
        }
    }
    return count;
}

static int has_registration_charge_relationship_issue(HospitalSystem *system, const OutpatientRegistration *registration) {
    MedicalRecord *current = NULL;
    int linked_registration_id = 0;
    if (system == NULL || registration == NULL) {
        return 0;
    }
    for (current = system->records; current != NULL; current = current->next) {
        if (!is_registration_charge_record(current)) {
            continue;
        }
        if (extract_charge_registration_id(current, &linked_registration_id)) {
            if (linked_registration_id == registration->registration_id &&
                !charge_record_matches_registration_fields(current, registration)) {
                return 1;
            }
        } else if (charge_record_matches_registration_fields(current, registration) &&
                   charge_record_date_matches_registration(current, registration)) {
            return 1;
        }
    }
    return 0;
}

static void waive_registration_charge(MedicalRecord *record) {
    const char *suffix = "（已取消，不计费）";
    char updated_note[TEXT_LEN];
    size_t length = 0;
    if (record == NULL) {
        return;
    }
    record->cost = 0.0;
    if (strstr(record->note, suffix) != NULL) {
        return;
    }
    safe_copy(updated_note, record->note, (int)sizeof(updated_note));
    length = strlen(updated_note);
    if (length < sizeof(updated_note) - 1) {
        safe_copy(updated_note + length, suffix, (int)(sizeof(updated_note) - length));
    }
    safe_copy(record->note, updated_note, TEXT_LEN);
}

static int waive_registration_charge_records(HospitalSystem *system, const OutpatientRegistration *registration) {
    MedicalRecord *current = NULL;
    int linked_registration_id = 0;
    int waived = 0;
    if (system == NULL || registration == NULL) {
        return 0;
    }
    for (current = system->records; current != NULL; current = current->next) {
        if (!is_registration_charge_record(current)) {
            continue;
        }
        if (extract_charge_registration_id(current, &linked_registration_id) &&
            linked_registration_id == registration->registration_id &&
            charge_record_matches_registration_fields(current, registration)) {
            if (!is_waived_registration_charge(current)) {
                waive_registration_charge(current);
                waived++;
            }
        }
    }
    return waived;
}

static void print_registration_list(HospitalSystem *system, int doctor_id, int patient_id, int active_only) {
    OutpatientRegistration *last = NULL;
    int found = 0;
    print_registration_header();
    while (1) {
        OutpatientRegistration *current = NULL;
        OutpatientRegistration *best = NULL;
        for (current = system->registrations; current != NULL; current = current->next) {
            if (doctor_id > 0 && current->doctor_id != doctor_id) {
                continue;
            }
            if (patient_id > 0 && current->patient_id != patient_id) {
                continue;
            }
            if (active_only && current->status != REG_STATUS_WAITING && current->status != REG_STATUS_CALLED) {
                continue;
            }
            if (last != NULL && !registration_sort_before(last, current)) {
                continue;
            }
            if (best == NULL || registration_sort_before(current, best)) {
                best = current;
            }
        }
        if (best == NULL) {
            break;
        }
        print_registration_row(system, best);
        found = 1;
        last = best;
    }
    print_table_border(registration_table_width(), '=');
    if (!found) {
        printf("当前暂无符合条件的门诊挂号记录\n");
    }
}

OutpatientRegistration *find_registration_by_id(HospitalSystem *system, int registration_id) {
    OutpatientRegistration *current = NULL;
    if (system == NULL) {
        return NULL;
    }
    for (current = system->registrations; current != NULL; current = current->next) {
        if (current->registration_id == registration_id) {
            return current;
        }
    }
    return NULL;
}

void add_outpatient_registration(HospitalSystem *system, OutpatientRegistration registration) {
    OutpatientRegistration *node = NULL;
    OutpatientRegistration *tail = NULL;
    if (system == NULL) {
        return;
    }
    node = (OutpatientRegistration *)malloc(sizeof(OutpatientRegistration));
    if (node == NULL) {
        return;
    }
    *node = registration;
    node->next = NULL;
    if (system->registrations == NULL) {
        system->registrations = node;
        return;
    }
    tail = system->registrations;
    while (tail->next != NULL) {
        tail = tail->next;
    }
    tail->next = node;
}

/* 医生端/管理员端交互式新增病历
 * 该流程会逐步采集患者、医生、科室、记录类型、诊断和费用等信息，
 * 并根据当前角色决定哪些字段可自动带出
 */
/* 返回上一步后清空当前步骤字段，避免旧输入残留到下一次提交。 */
static void record_clear_step_data(MedicalRecord *record, int step) {
    switch (step) {
        case 0:
            record->patient_id = 0;
            break;
        case 1:
            record->doctor_id = 0;
            break;
        case 2:
            record->department_id = 0;
            break;
        case 3:
            memset(record->record_type, 0, TYPE_LEN);
            break;
        case 4:
            memset(record->diagnosis, 0, TEXT_LEN);
            break;
        case 5:
            record->cost = -1.0;
            break;
    }
}

static const char *medical_record_operator_role(HospitalSystem *system) {
    if (system != NULL && system->current_role == ROLE_DOCTOR) {
        return "医生";
    }
    if (system != NULL && system->current_role == ROLE_ADMIN) {
        return admin_log_role(system);
    }
    return "系统";
}

/* 处理新增病历中的返回动作；如果已经在第一步，则直接取消并回收编号。 */
static int record_handle_back(int *step, HospitalSystem *system) {
    if (*step == 0) {
        printf("已取消新增病历\n");
        system->next_record_id--;
        return -1;
    }
    (*step)--;
    return 0;
}

void add_record_interactive(HospitalSystem *system) {
    MedicalRecord record;
    int type_choice = 0;
    int step = 0;
    Doctor *current_doctor = get_current_doctor(system);

    if (system != NULL && system->current_role == ROLE_DOCTOR) {
        printf("医生不能手工新增门诊病历，请通过“叫号接诊”生成看诊记录。\n");
        return;
    }

    memset(&record, 0, sizeof(record));
    record.cost = -1.0;
    record.record_id = system->next_record_id++;

    while (step < 6) {

        /*  Step 0: 患者  */
        if (step == 0) {
            int result = read_int_step("请输入患者编号（输入 b 退出）: ",
                                      &record.patient_id, 1, 999999, 1);

            if (result == -1) {
                system->next_record_id--;
                printf("已取消新增病历\n");
                return;
            }

            if (find_patient_by_id(system, record.patient_id) == NULL) {
                printf("患者不存在\n");
                continue;
            }

            if (system->current_role == ROLE_DOCTOR &&
                doctor_can_access_patient(system, record.patient_id) == 0 &&
                find_active_admission_by_patient(system, record.patient_id) != NULL &&
                find_active_admission_by_patient(system, record.patient_id)->department_id != current_doctor->department_id) {
                printf("无权限为其他科室患者建立记录\n");
                continue;
            }

            step++;
        }

        /*  Step 1: 医生  */
        else if (step == 1) {

            if (system->current_role == ROLE_DOCTOR) {
                printf("当前医生编号：%d（输入 b 返回）\n", current_doctor->doctor_id);

                /* 医生身份的医生编号自动带出，这里只读取确认或返回指令。 */
                char buf[8];
                if (fgets(buf, sizeof(buf), stdin) == NULL) {
                    system->next_record_id--;
                    printf("已取消新增病历\n");
                    return;
                }
                if (buf[0] == 'b' || buf[0] == 'B') {
                    if (record_handle_back(&step, system) == -1) return;
                    record_clear_step_data(&record, step);
                    continue;
                }

                record.doctor_id = current_doctor->doctor_id;
                step++;
            } else {
                int result = read_int_step("请输入医生编号（输入 b 返回）: ",
                                          &record.doctor_id, 1, 999999, 1);

                if (result == -1) {
                    if (record_handle_back(&step, system) == -1) return;
                    record_clear_step_data(&record, step);
                    continue;
                }

                if (find_doctor_by_id(system, record.doctor_id) == NULL) {
                    printf("医生不存在\n");
                    continue;
                }

                step++;
            }
        }

        /*  Step 2: 科室  */
        else if (step == 2) {

            if (system->current_role == ROLE_DOCTOR) {
                printf("当前科室编号：%d（输入 b 返回）\n", current_doctor->department_id);

                /* 医生身份的科室编号自动带出，EOF 时按取消新增病历处理。 */
                char buf[8];
                if (fgets(buf, sizeof(buf), stdin) == NULL) {
                    system->next_record_id--;
                    printf("已取消新增病历\n");
                    return;
                }
                if (buf[0] == 'b' || buf[0] == 'B') {
                    if (record_handle_back(&step, system) == -1) return;
                    record_clear_step_data(&record, step);
                    continue;
                }

                record.department_id = current_doctor->department_id;
                step++;
            } else {
                int result = read_int_step("请输入科室编号（输入 b 返回）: ",
                                          &record.department_id, 1, 999999, 1);

                if (result == -1) {
                    if (record_handle_back(&step, system) == -1) return;
                    record_clear_step_data(&record, step);
                    continue;
                }

                if (find_department_by_id(system, record.department_id) == NULL) {
                    printf("科室不存在\n");
                    continue;
                }

                if (find_doctor_by_id(system, record.doctor_id)->department_id != record.department_id) {
                    printf("所选科室与医生所属科室不一致，请重新输入\n");
                    continue;
                }

                step++;
            }
        }

        /*  Step 3: 类型  */
        else if (step == 3) {
            int result = read_int_step(
                "请选择记录类型 1.挂号 2.看诊 3.检查 4.住院（输入 b 返回）: ",
                &type_choice, 1, 4, 1);

            if (result == -1) {
                if (record_handle_back(&step, system) == -1) return;
                record_clear_step_data(&record, step);
                continue;
            }

            switch (type_choice) {
                case 1: safe_copy(record.record_type, "挂号", TYPE_LEN); break;
                case 2: safe_copy(record.record_type, "看诊", TYPE_LEN); break;
                case 3: safe_copy(record.record_type, "检查", TYPE_LEN); break;
                case 4: safe_copy(record.record_type, "住院", TYPE_LEN); break;
            }

            step++;
        }

        /*  Step 4: 诊断  */
        else if (step == 4) {
            int result = read_string_step(
                "请输入诊断结果（输入 b 返回）: ",
                record.diagnosis, TEXT_LEN, 1);

            if (result == -1) {
                if (record_handle_back(&step, system) == -1) return;
                record_clear_step_data(&record, step);
                continue;
            }

            step++;
        }

        /*  Step 5: 费用  */
        else if (step == 5) {
            int result = read_double_step(
                "请输入费用（输入 b 返回）: ",
                &record.cost, 0.0, 1);

            if (result == -1) {
                if (record_handle_back(&step, system) == -1) return;
                record_clear_step_data(&record, step);
                continue;
            }

            step++;
        }
    }

    /*  完成后补充信息  */
    get_current_datetime(record.date);

    /* 备注属于补充信息，EOF 时由步骤输入函数返回取消状态，不再阻塞流程。 */
    read_string_step("请输入备注（可简单说明用途）: ",
                     record.note, TEXT_LEN, 0);

    /*  权限校验  */
    if (system->current_role == ROLE_DOCTOR &&
        doctor_can_access_department(system, record.department_id) == 0) {
        printf("无权限为其他科室创建病历\n");
        system->next_record_id--;
        return;
    }

    add_record(system, record);
    append_log(system, medical_record_operator_role(system), "新增病历", record.record_type);

    printf("医疗记录新增成功，记录编号为 %d\n", record.record_id);
}
/* 患者自助挂号
 * 预约挂号使用患者选择的就诊时间，现场挂号使用当前时间；
 * 两者都会进入同一候诊队列，由医生按就诊时间顺序叫号
 */
void patient_registration_interactive(HospitalSystem *system) {
    Patient *patient = get_current_patient(system);
    int department_id = 0;
    int doctor_id = 0;
    int type_choice = 0;
    int step = 0;
    char symptom[TEXT_LEN];
    char visit_time[DATE_LEN];
    char input_time[DATE_LEN];
    char note[TEXT_LEN];
    OutpatientRegistration registration;
    MedicalRecord record;
    Doctor *doctor = NULL;
    Department *department = NULL;

    if (patient == NULL) {
        printf("当前患者身份无效，请重新登录\n");
        return;
    }

    memset(&registration, 0, sizeof(registration));
    memset(&record, 0, sizeof(record));
    memset(symptom, 0, sizeof(symptom));
    memset(visit_time, 0, sizeof(visit_time));
    memset(input_time, 0, sizeof(input_time));

    while (step < 5) {
        if (step == 0) {
            int result = read_int_step("请选择挂号方式 1.预约挂号 2.现场挂号（输入 b 取消）: ", &type_choice, 1, 2, 1);
            if (result == 1) {
                step++;
            } else if (result == -1) {
                /* 输入流结束时取消挂号，避免生成缺字段的挂号记录。 */
                printf("挂号已取消\n");
                return;
            }
        } else if (step == 1) {
            list_departments(system);
            if (read_int_step("请输入挂号科室编号（输入 b 返回上一项）: ", &department_id, 1, 999999, 1) == -1) {
                step--;
            } else {
                department = find_department_by_id(system, department_id);
                if (department == NULL) {
                    printf("科室不存在，请重新输入\n");
                } else {
                    step++;
                }
            }
        } else if (step == 2) {
            print_title("该科室医生列表");
            list_doctors_by_department(system, department_id);
            if (read_int_step("请输入挂号医生编号（输入 b 返回上一项）: ", &doctor_id, 1, 999999, 1) == -1) {
                step--;
            } else {
                doctor = find_doctor_by_id(system, doctor_id);
                if (doctor == NULL || doctor->department_id != department_id) {
                    printf("医生不存在，或不属于所选科室，请重新输入\n");
                } else if (has_pending_registration(system, patient->patient_id, doctor_id)) {
                    printf("你在该医生处已有待诊或已叫号挂号，请先完成或取消后再挂号\n");
                } else {
                    step++;
                }
            }
        } else if (step == 3) {
            if (type_choice == REG_TYPE_APPOINTMENT) {
                int result = read_string_step("请输入预约就诊时间（请使用英文半角输入法，格式 YYYY-MM-DD HH:MM，例如 2026-05-02 09:30，输入 b 返回上一项）: ",
                                               input_time, DATE_LEN, 1);
                if (result == -1) {
                    step--;
                } else if (!normalize_visit_time(input_time, visit_time, DATE_LEN)) {
                    printf("时间格式不正确，请使用英文半角数字、减号和冒号，按 YYYY-MM-DD HH:MM 输入\n");
                } else if (!visit_time_is_after_now(visit_time)) {
                    printf("预约时间必须晚于当前系统时间，请重新输入\n");
                } else {
                    step++;
                }
            } else {
                get_current_datetime(visit_time);
                step++;
            }
        } else {
            int result = read_string_step("请输入就诊诉求/症状（输入 b 返回上一项）: ", symptom, TEXT_LEN, 1);
            if (result == -1) {
                step--;
            } else {
                step++;
            }
        }
    }

    registration.registration_id = system->next_registration_id++;
    registration.patient_id = patient->patient_id;
    registration.doctor_id = doctor_id;
    registration.department_id = department_id;
    registration.registration_type = type_choice;
    get_current_datetime(registration.created_time);
    safe_copy(registration.visit_time, visit_time, DATE_LEN);
    registration.status = REG_STATUS_WAITING;
    safe_copy(registration.symptom, symptom, TEXT_LEN);
    add_outpatient_registration(system, registration);

    record.record_id = system->next_record_id++;
    record.patient_id = patient->patient_id;
    record.doctor_id = doctor_id;
    record.department_id = department_id;
    safe_copy(record.record_type, type_choice == REG_TYPE_APPOINTMENT ? "预约挂号" : "现场挂号", TYPE_LEN);
    safe_copy(record.date, registration.created_time, DATE_LEN);
    safe_copy(record.diagnosis, symptom, TEXT_LEN);
    record.cost = REGISTRATION_FEE;
    snprintf(note, sizeof(note), "门诊挂号ID:%d，就诊时间:%s", registration.registration_id, registration.visit_time);
    safe_copy(record.note, note, TEXT_LEN);
    add_record(system, record);

    append_log(system, "患者", type_choice == REG_TYPE_APPOINTMENT ? "预约挂号" : "现场挂号", patient->name);

    printf("挂号成功，已进入候诊队列\n");
    printf("挂号编号：%d\n", registration.registration_id);
    printf("患者编号：%d\n", patient->patient_id);
    printf("科室：%s\n", department != NULL ? department->name : "未知");
    printf("医生：%s\n", doctor != NULL ? doctor->name : "未知");
    printf("就诊时间：%s\n", registration.visit_time);
    printf("当前排队序号：%d\n", registration_queue_position(system, find_registration_by_id(system, registration.registration_id)));
    printf("挂号收费记录编号：%d，挂号费用：%.2f 元\n", record.record_id, record.cost);
}

void list_outpatient_registrations(HospitalSystem *system) {
    print_title("门诊挂号记录");
    print_registration_list(system, 0, 0, 0);
}

void list_doctor_waiting_queue(HospitalSystem *system) {
    Doctor *doctor = get_current_doctor(system);
    if (doctor == NULL) {
        printf("当前医生身份无效，请重新登录\n");
        return;
    }
    print_title("本科室候诊队列");
    print_registration_list(system, doctor->doctor_id, 0, 1);
}

void show_current_patient_registrations(HospitalSystem *system) {
    Patient *patient = get_current_patient(system);
    if (patient == NULL) {
        printf("当前患者身份无效，请重新登录\n");
        return;
    }
    print_title("我的挂号记录");
    print_registration_list(system, 0, patient->patient_id, 0);
}

void doctor_call_next_registration(HospitalSystem *system) {
    Doctor *doctor = get_current_doctor(system);
    OutpatientRegistration *current_called = NULL;
    OutpatientRegistration *next = NULL;
    Patient *patient = NULL;
    if (doctor == NULL) {
        printf("当前医生身份无效，请重新登录\n");
        return;
    }
    current_called = find_current_called_registration(system, doctor->doctor_id);
    if (current_called != NULL) {
        printf("当前已有已叫号患者，请先完成接诊挂号编号：%d，患者编号：%d\n",
               current_called->registration_id, current_called->patient_id);
        return;
    }
    next = find_next_waiting_registration(system, doctor->doctor_id);
    if (next == NULL) {
        printf("当前没有待叫号患者\n");
        return;
    }
    next->status = REG_STATUS_CALLED;
    patient = find_patient_by_id(system, next->patient_id);
    print_title("叫号信息");
    printf("请 %s（患者编号：%d，挂号编号：%d）进入诊室就诊\n",
           patient != NULL ? patient->name : "患者", next->patient_id, next->registration_id);
    printf("挂号类型：%s\n", registration_type_to_text(next->registration_type));
    printf("预约/挂号时间：%s\n", next->visit_time);
    printf("就诊诉求：%s\n", next->symptom);
    append_log(system, "医生", "叫号", patient != NULL ? patient->name : "未知患者");
}

void doctor_finish_current_registration(HospitalSystem *system) {
    Doctor *doctor = get_current_doctor(system);
    OutpatientRegistration *registration = NULL;
    MedicalRecord record;
    char diagnosis[TEXT_LEN];
    char note[TEXT_LEN];
    char merged_note[TEXT_LEN];
    if (doctor == NULL) {
        printf("当前医生身份无效，请重新登录\n");
        return;
    }
    registration = find_current_called_registration(system, doctor->doctor_id);
    if (registration == NULL) {
        printf("当前没有已叫号待完成的患者，请先叫号\n");
        return;
    }
    memset(&record, 0, sizeof(record));
    read_string("请输入诊断结果: ", diagnosis, TEXT_LEN);
    record.cost = 0.0;
    read_string("请输入诊疗备注: ", note, TEXT_LEN);

    record.record_id = system->next_record_id++;
    record.patient_id = registration->patient_id;
    record.doctor_id = registration->doctor_id;
    record.department_id = registration->department_id;
    safe_copy(record.record_type, "看诊", TYPE_LEN);
    get_current_datetime(record.date);
    safe_copy(record.diagnosis, diagnosis, TEXT_LEN);
    snprintf(merged_note, sizeof(merged_note), "叫号挂号ID:%d；%s", registration->registration_id, note);
    safe_copy(record.note, merged_note, TEXT_LEN);
    add_record(system, record);
    registration->status = REG_STATUS_COMPLETED;
    append_log(system, "医生", "完成接诊", diagnosis);
    printf("接诊完成，已生成看诊记录，记录编号为 %d\n", record.record_id);
}

void doctor_call_and_finish_registration(HospitalSystem *system) {
    Doctor *doctor = get_current_doctor(system);
    OutpatientRegistration *registration = NULL;
    Patient *patient = NULL;
    MedicalRecord record;
    char diagnosis[TEXT_LEN];
    char note[TEXT_LEN];
    char merged_note[TEXT_LEN];
    int registration_id = 0;
    if (doctor == NULL) {
        printf("当前医生身份无效，请重新登录\n");
        return;
    }
    print_title("候诊队列");
    print_registration_list(system, doctor->doctor_id, 0, 1);
    registration_id = read_int("请输入要接诊的挂号编号（输入 0 接诊下一位）: ");
    if (registration_id == 0) {
        registration = find_next_waiting_registration(system, doctor->doctor_id);
    } else {
        registration = find_registration_by_id(system, registration_id);
        if (registration != NULL &&
            (registration->doctor_id != doctor->doctor_id || registration->status != REG_STATUS_WAITING)) {
            registration = NULL;
        }
    }
    if (registration == NULL) {
        printf("未找到可接诊的待诊挂号。\n");
        return;
    }
    registration->status = REG_STATUS_CALLED;
    patient = find_patient_by_id(system, registration->patient_id);
    print_title("叫号信息");
    printf("请 %s（患者编号：%d，挂号编号：%d）进入诊室就诊\n",
           patient != NULL ? patient->name : "患者", registration->patient_id, registration->registration_id);
    printf("就诊诉求：%s\n", registration->symptom);
    memset(&record, 0, sizeof(record));
    read_string("请输入诊断结果: ", diagnosis, TEXT_LEN);
    read_string("请输入诊疗备注: ", note, TEXT_LEN);
    record.record_id = system->next_record_id++;
    record.patient_id = registration->patient_id;
    record.doctor_id = registration->doctor_id;
    record.department_id = registration->department_id;
    safe_copy(record.record_type, "看诊", TYPE_LEN);
    get_current_datetime(record.date);
    safe_copy(record.diagnosis, diagnosis, TEXT_LEN);
    record.cost = 0.0;
    snprintf(merged_note, sizeof(merged_note), "接诊挂号ID:%d；%s", registration->registration_id, note);
    safe_copy(record.note, merged_note, TEXT_LEN);
    add_record(system, record);
    registration->status = REG_STATUS_COMPLETED;
    append_log(system, "医生", "叫号接诊", diagnosis);
    printf("叫号接诊完成，已生成看诊记录，记录编号为 %d\n", record.record_id);
}

void cancel_registration_interactive(HospitalSystem *system) {
    int registration_id = read_int("请输入要取消的挂号编号: ");
    OutpatientRegistration *registration = find_registration_by_id(system, registration_id);
    int charge_record_count = 0;
    int active_charge_count = 0;
    int relationship_issue = 0;
    int waived_count = 0;
    char log_detail[TEXT_LEN];
    if (registration == NULL) {
        printf("挂号记录不存在\n");
        return;
    }
    if (system->current_role == ROLE_PATIENT && registration->patient_id != system->current_patient_id) {
        printf("患者只能取消自己的挂号\n");
        return;
    }
    if (registration->status == REG_STATUS_COMPLETED) {
        printf("该挂号已完成接诊，不能取消\n");
        return;
    }
    if (registration->status == REG_STATUS_CALLED) {
        printf("该挂号已被医生叫号，请先完成接诊，不能直接取消\n");
        return;
    }
    if (registration->status == REG_STATUS_CANCELED) {
        printf("该挂号已取消，无需重复操作\n");
        return;
    }
    charge_record_count = count_registration_charge_records(system, registration, NULL);
    active_charge_count = count_active_registration_charge_records(system, registration);
    relationship_issue = charge_record_count == 0 && has_registration_charge_relationship_issue(system, registration);
    snprintf(log_detail, sizeof(log_detail), "挂号ID:%d，患者ID:%d，收费记录数:%d，症状:%s",
             registration->registration_id, registration->patient_id, charge_record_count, registration->symptom);
    if (!append_log(system, system->current_role == ROLE_PATIENT ? "患者" : admin_log_role(system), "取消挂号", log_detail)) {
        printf("取消挂号失败：无法写入取消记录，请稍后重试\n");
        return;
    }
    registration->status = REG_STATUS_CANCELED;
    waived_count = waive_registration_charge_records(system, registration);
    if (charge_record_count > 0) {
        if (active_charge_count > 0) {
            printf("挂号已取消，对应挂号费已冲销\n");
            if (charge_record_count > 1) {
                printf("提示：该挂号存在重复收费记录，已一并冲销，请在异常对账中复核\n");
            }
        } else {
            printf("挂号已取消，该挂号费此前已冲销\n");
        }
        if (active_charge_count > 0 && waived_count == 0) {
            printf("提示：收费记录状态异常，请人工核对历史数据\n");
        }
    } else if (relationship_issue) {
        printf("挂号已取消，但收费记录异常，请人工核对历史数据\n");
    } else {
        printf("挂号已取消，该挂号未产生收费记录\n");
    }
}

static int count_registration_cancel_logs(HospitalSystem *system, int registration_id) {
    OperationLog *current = NULL;
    int logged_registration_id = 0;
    int count = 0;
    if (system == NULL) {
        return 0;
    }
    for (current = system->logs; current != NULL; current = current->next) {
        if (strcmp(current->action, "取消挂号") != 0) {
            continue;
        }
        if (extract_registration_id_from_marker(current->detail, "挂号ID:", &logged_registration_id) &&
            logged_registration_id == registration_id) {
            count++;
        }
    }
    return count;
}

static void print_reconciliation_detail(const char *category, int registration_id, int record_id, int patient_id, const char *detail) {
    char registration_text[32];
    char record_text[32];
    safe_copy(registration_text, "-", (int)sizeof(registration_text));
    safe_copy(record_text, "-", (int)sizeof(record_text));
    if (registration_id > 0) {
        snprintf(registration_text, sizeof(registration_text), "%d", registration_id);
    }
    if (record_id > 0) {
        snprintf(record_text, sizeof(record_text), "%d", record_id);
    }
    printf("[%s] 挂号ID:%s 收费记录ID:%s 患者ID:%d %s\n",
           category, registration_text, record_text, patient_id, detail != NULL ? detail : "");
}

void show_registration_reconciliation_report(HospitalSystem *system) {
    OutpatientRegistration *registration = NULL;
    MedicalRecord *record = NULL;
    int found = 0;
    if (system == NULL) {
        return;
    }
    print_title("挂号收费异常对账");
    for (registration = system->registrations; registration != NULL; registration = registration->next) {
        MedicalRecord *first_charge = NULL;
        int charge_count = count_registration_charge_records(system, registration, &first_charge);
        int active_charge_count = count_active_registration_charge_records(system, registration);
        int cancel_log_count = count_registration_cancel_logs(system, registration->registration_id);
        if (charge_count == 0) {
            print_reconciliation_detail(
                "有挂号无收费",
                registration->registration_id,
                0,
                registration->patient_id,
                has_registration_charge_relationship_issue(system, registration)
                    ? "存在疑似收费记录，但挂号ID、患者、医生、科室或类型关联异常"
                    : "未发现门诊挂号收费记录"
            );
            found = 1;
        }
        if (registration->status == REG_STATUS_CANCELED && active_charge_count > 0) {
            print_reconciliation_detail(
                "已取消但未退费",
                registration->registration_id,
                first_charge != NULL ? first_charge->record_id : 0,
                registration->patient_id,
                "挂号已取消，但仍存在未冲销的挂号费"
            );
            found = 1;
        }
        if (charge_count > 1) {
            print_reconciliation_detail(
                "重复收费",
                registration->registration_id,
                first_charge != NULL ? first_charge->record_id : 0,
                registration->patient_id,
                "同一挂号ID存在多条收费记录"
            );
            found = 1;
        }
        if (cancel_log_count > 1) {
            print_reconciliation_detail(
                "重复取消",
                registration->registration_id,
                0,
                registration->patient_id,
                "同一挂号ID存在多条取消日志"
            );
            found = 1;
        }
    }
    for (record = system->records; record != NULL; record = record->next) {
        OutpatientRegistration *registration_for_charge = NULL;
        int linked_registration_id = 0;
        if (!is_registration_charge_record(record)) {
            continue;
        }
        if (!extract_charge_registration_id(record, &linked_registration_id)) {
            print_reconciliation_detail(
                "有收费无挂号",
                0,
                record->record_id,
                record->patient_id,
                "收费记录缺少门诊挂号ID关联字段"
            );
            found = 1;
            continue;
        }
        registration_for_charge = find_registration_by_id(system, linked_registration_id);
        if (registration_for_charge == NULL) {
            print_reconciliation_detail(
                "有收费无挂号",
                linked_registration_id,
                record->record_id,
                record->patient_id,
                "收费记录关联的挂号ID不存在"
            );
            found = 1;
        } else if (!charge_record_matches_registration_fields(record, registration_for_charge)) {
            print_reconciliation_detail(
                "有收费无挂号",
                linked_registration_id,
                record->record_id,
                record->patient_id,
                "收费记录与挂号的患者、医生、科室或挂号类型不一致"
            );
            found = 1;
        }
    }
    if (!found) {
        printf("未发现挂号收费异常数据\n");
    }
}

/* 列出全部病历 */
void list_records(HospitalSystem *system) {
    MedicalRecord *current = system->records;
    int found = 0;
    print_title("医疗记录列表");
    print_record_header(1);
    while (current != NULL) {
        print_record_row(current, 1);
        found = 1;
        current = current->next;
    }
    print_table_border(record_table_width(1), '=');
    if (!found) {
        printf("当前暂无医疗记录\n");
    }
}

/* 按患者筛选病历 */
void list_records_by_patient(HospitalSystem *system, int patient_id) {
    MedicalRecord *current = system->records;
    int found = 0;
    print_record_header(0);
    while (current != NULL) {
        if (current->patient_id == patient_id) {
            print_record_row(current, 0);
            found = 1;
        }
        current = current->next;
    }
    print_table_border(record_table_width(0), '=');
    if (!found) {
        printf("该患者暂无就诊记录\n");
    }
}

/* 查看患者完整病史
 * 会根据当前角色限制可查看的患者范围
 */
void patient_history_interactive(HospitalSystem *system) {
    char keyword[NAME_LEN];
    Patient *current = system->patients;
    int found = 0;
    if (system->current_role == ROLE_PATIENT) {
        print_title("患者就诊历史");
        list_records_by_patient(system, system->current_patient_id);
        return;
    }
    read_string("请输入患者编号/姓名/电话/身份证关键字: ", keyword, NAME_LEN);
    while (current != NULL) {
        char id_text[32];
        snprintf(id_text, sizeof(id_text), "%d", current->patient_id);
        if (current->is_deleted == 0 &&
            (str_contains_ignore_case(id_text, keyword) ||
             str_contains_ignore_case(current->name, keyword) ||
             str_contains_ignore_case(current->phone, keyword) ||
             str_contains_ignore_case(current->id_card, keyword)) &&
            (system->current_role != ROLE_DOCTOR || doctor_can_access_patient(system, current->patient_id)) &&
            (system->current_role != ROLE_NURSE || nurse_can_access_patient(system, current->patient_id))) {
            printf("\n患者 %s（%d）就诊历史：\n", current->name, current->patient_id);
            list_records_by_patient(system, current->patient_id);
            found = 1;
        }
        current = current->next;
    }
    if (!found) {
        printf("未找到可查看的患者病史\n");
    }
}

/* 办理住院：
 * 该流程会按步骤采集患者、医生、科室、床位和押金信息，
 * 最终同时生成住院记录、更新床位状态，并自动补一条病历
 */
void handle_admission_interactive(HospitalSystem *system) {
    int patient_id = 0;
    int doctor_id = 0;
    int department_id = 0;
    int bed_id = 0;
    char diagnosis[TEXT_LEN];
    double deposit = -1.0;
    AdmissionHistory *admission = NULL;
    MedicalRecord record;
    int step = 0;
    Doctor *current_doctor = get_current_doctor(system);
    memset(diagnosis, 0, sizeof(diagnosis));

    while (step < 6) {
        if (step == 0) {
            int result = read_int_step("请输入患者编号（输入 b 取消）: ", &patient_id, 1, 999999, 1);
            if (result == -1) {
                /* 第一步就遇到 EOF，直接取消住院流程，不占用床位和编号。 */
                printf("办理住院已取消\n");
                return;
            }
            if (result == 1) {
                if (find_patient_by_id(system, patient_id) == NULL) {
                    printf("患者不存在\n");
                } else if (find_active_admission_by_patient(system, patient_id) != NULL) {
                    printf("该患者已有未出院记录\n");
                } else {
                    step++;
                }
            }
        } else if (step == 1) {
            if (system->current_role == ROLE_DOCTOR) {
                doctor_id = current_doctor->doctor_id;
                department_id = current_doctor->department_id;
                step++;
            } else {
                int result = read_int_step("请输入接诊医生编号（输入 b 返回上一项）: ", &doctor_id, 1, 999999, 1);
                if (result == -1) {
                    step--;
                } else if (find_doctor_by_id(system, doctor_id) == NULL) {
                    printf("医生不存在\n");
                } else {
                    step++;
                }
            }
        } else if (step == 2) {
            if (system->current_role != ROLE_DOCTOR) {
                int result = read_int_step("请输入科室编号（输入 b 返回上一项）: ", &department_id, 1, 999999, 1);
                if (result == -1) {
                    step--;
                    continue;
                }
                if (find_department_by_id(system, department_id) == NULL) {
                    printf("科室不存在\n");
                    continue;
                }
                if (find_doctor_by_id(system, doctor_id)->department_id != department_id) {
                    printf("所选科室与医生所属科室不一致，请重新输入\n");
                    continue;
                }
            }
            print_title("可用床位");
            list_available_beds_by_department(system, department_id);
            step++;
        } else if (step == 3) {
            int result = read_int_step("请输入分配床位编号（输入 b 返回上一项）: ", &bed_id, 1, 999999, 1);
            if (result == -1) {
                step--;
            } else if (find_bed_by_id(system, bed_id) == NULL || find_bed_by_id(system, bed_id)->department_id != department_id) {
                printf("床位不存在或与科室不匹配\n");
            } else if (find_bed_by_id(system, bed_id)->status != BED_STATUS_FREE) {
                printf("该床位当前不是空闲状态\n");
            } else {
                step++;
            }
        } else if (step == 4) {
            int result = read_double_step("请输入住院押金（输入 b 返回上一项）: ", &deposit, 0.0, 1);
            if (result == -1) {
                step--;
            } else {
                step++;
            }
        } else {
            int result = read_string_step("请输入住院诊断（输入 b 返回上一项）: ", diagnosis, TEXT_LEN, 1);
            if (result == -1) {
                step--;
            } else {
                step++;
            }
        }
    }

    if (system->current_role == ROLE_DOCTOR && doctor_can_access_department(system, department_id) == 0) {
        printf("无权限为其他科室办理住院\n");
        return;
    }

    if (!assign_bed(system, patient_id, bed_id)) {
        printf("床位分配失败，可能该床位已被占用\n");
        return;
    }

    admission = (AdmissionHistory *)malloc(sizeof(AdmissionHistory));
    if (admission == NULL) {
        release_bed(system, bed_id);
        return;
    }
    admission->admission_id = system->next_admission_id++;
    admission->patient_id = patient_id;
    admission->department_id = department_id;
    admission->doctor_id = doctor_id;
    admission->ward_id = find_bed_by_id(system, bed_id)->ward_id;
    admission->bed_id = bed_id;
    get_current_date(admission->admit_date);
    safe_copy(admission->discharge_date, "-", DATE_LEN);
    admission->stay_days = 0;
    admission->deposit = deposit;
    safe_copy(admission->diagnosis, diagnosis, TEXT_LEN);
    admission->status = ADMISSION_ACTIVE;
    admission->next = NULL;

    if (system->admissions == NULL) {
        system->admissions = admission;
    } else {
        AdmissionHistory *tail = system->admissions;
        while (tail->next != NULL) {
            tail = tail->next;
        }
        tail->next = admission;
    }

    memset(&record, 0, sizeof(record));
    record.record_id = system->next_record_id++;
    record.patient_id = patient_id;
    record.doctor_id = doctor_id;
    record.department_id = department_id;
    safe_copy(record.record_type, "住院", TYPE_LEN);
    get_current_date(record.date);
    safe_copy(record.diagnosis, diagnosis, TEXT_LEN);
    record.cost = deposit;
    safe_copy(record.note, "自动生成住院记录", TEXT_LEN);
    add_record(system, record);

    append_log(system, medical_record_operator_role(system), "办理住院", diagnosis);
    printf("住院办理成功，住院编号为 %d\n", admission->admission_id);
}

/* 办理出院：
 * 该流程会更新出院日期和住院天数，并释放床位，
 * 从而让床位资源重新回到可分配状态
 */
void handle_discharge_interactive(HospitalSystem *system) {
    int admission_id = read_int("请输入住院编号: ");
    AdmissionHistory *admission = find_admission_by_id(system, admission_id);
    char current_date[DATE_LEN];
    if (admission == NULL) {
        printf("住院记录不存在\n");
        return;
    }
    if (system->current_role == ROLE_DOCTOR && !doctor_can_access_admission(system, admission)) {
        printf("无权限为其他科室患者办理出院\n");
        return;
    }
    if (admission->status != ADMISSION_ACTIVE) {
        printf("该住院记录已出院，不能重复办理\n");
        return;
    }
    get_current_date(current_date);
    safe_copy(admission->discharge_date, current_date, DATE_LEN);
    admission->stay_days = days_between(admission->admit_date, admission->discharge_date);
    if (admission->stay_days < 1) {
        admission->stay_days = 1;
    }
    admission->status = ADMISSION_DISCHARGED;
    if (!release_bed(system, admission->bed_id)) {
        printf("警告：床位释放失败，请检查床位状态\n");
    }
    append_log(system, medical_record_operator_role(system), "办理出院", admission->diagnosis);
    printf("出院办理成功，住院时长 %d 天\n", admission->stay_days);
}

/* 列出全部住院历史记录 */
void list_admissions(HospitalSystem *system) {
    AdmissionHistory *current = system->admissions;
    int found = 0;
    print_title("住院历史");
    print_admission_header();
    while (current != NULL) {
        print_admission_row(current);
        found = 1;
        current = current->next;
    }
    print_table_border(admission_table_width(), '=');
    if (!found) {
        printf("当前暂无住院记录\n");
    }
}

#endif
