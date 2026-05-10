/*
 * fileio.c
 * 数据读写模块
 * 项目能不能把数据留下来 主要看这块
 */

#include "fileio.h"

#if defined(HIS_BUILD_FROM_MAIN) || !HIS_MAIN_ONE_CLICK_BUILD

#ifdef _WIN32
#include <direct.h>
#include <io.h>
#ifndef F_OK
#define F_OK 0
#endif
#define ACCESS _access
#define CHDIR _chdir
#else
#include <unistd.h>
#define ACCESS access
#define CHDIR chdir
#endif

/* 用于启动时定位数据目录；新增持久化文件时也要同步加入这里 */
static const char *DATA_FILES[] = {
    "patients.txt",
    "doctors.txt",
    "nurses.txt",
    "pharmacy_staff.txt",
    "departments.txt",
    "wards.txt",
    "beds.txt",
    "drugs.txt",
    "drug_department_rules.txt",
    "records.txt",
    "registrations.txt",
    "admissions.txt",
    "drug_usage.txt",
    "prescriptions.txt",
    "prescription_items.txt",
    "log.txt"
};

static const int DATA_FILE_COUNT = (int)(sizeof(DATA_FILES) / sizeof(DATA_FILES[0]));
static int extract_directory(const char *path, char *directory, int size);

static void write_utf8_bom(FILE *fp) {
    if (fp != NULL) {
        fputs("\xEF\xBB\xBF", fp);
    }
}

static int file_exists_in_directory(const char *directory, const char *filename) {
    char path[PATH_MAX];
    int written = 0;
    if (directory == NULL || filename == NULL) {
        return 0;
    }
    if (directory[0] == '\0' || strcmp(directory, ".") == 0) {
        written = snprintf(path, sizeof(path), "%s", filename);
    } else {
        written = snprintf(path, sizeof(path), "%s/%s", directory, filename);
    }
    if (written < 0 || written >= (int)sizeof(path)) {
        return 0;
    }
    return ACCESS(path, F_OK) == 0;
}

static int directory_has_data_files(const char *directory) {
    int index = 0;
    for (index = 0; index < DATA_FILE_COUNT; index++) {
        if (!file_exists_in_directory(directory, DATA_FILES[index])) {
            return 0;
        }
    }
    return 1;
}

static int extract_directory(const char *path, char *directory, int size) {
    const char *last_slash = NULL;
    size_t length = 0;
    if (path == NULL || directory == NULL || size <= 0 || path[0] == '\0') {
        return 0;
    }
    last_slash = strrchr(path, '/');
#ifdef _WIN32
    {
        const char *last_backslash = strrchr(path, '\\');
        if (last_backslash != NULL && (last_slash == NULL || last_backslash > last_slash)) {
            last_slash = last_backslash;
        }
    }
#endif
    if (last_slash == NULL) {
        safe_copy(directory, ".", size);
        return 1;
    }
    length = (size_t)(last_slash - path);
    if (length == 0) {
        length = 1;
    }
    if ((int)length >= size) {
        return 0;
    }
    memcpy(directory, path, length);
    directory[length] = '\0';
    return 1;
}

static int try_change_to_directory(const char *directory) {
    if (directory == NULL || directory[0] == '\0') {
        return 0;
    }
    if (!directory_has_data_files(directory)) {
        return 0;
    }
    return CHDIR(directory) == 0;
}

static int try_change_to_known_data_directory(void) {
    const char *candidates[] = {
        "test_data",
        "../test_data",
        "data",
        "../data"
    };
    int index = 0;
    for (index = 0; index < (int)(sizeof(candidates) / sizeof(candidates[0])); index++) {
        if (try_change_to_directory(candidates[index])) {
            return 1;
        }
    }
    return 0;
}

static int try_change_to_related_data_directory(const char *path) {
    char directory[PATH_MAX];
    char candidate[PATH_MAX];
    int written = 0;
    if (!extract_directory(path, directory, sizeof(directory))) {
        return 0;
    }
    written = snprintf(candidate, sizeof(candidate), "%s/test_data", directory);
    if (written > 0 && written < (int)sizeof(candidate) && try_change_to_directory(candidate)) {
        return 1;
    }
    written = snprintf(candidate, sizeof(candidate), "%s/../test_data", directory);
    if (written > 0 && written < (int)sizeof(candidate) && try_change_to_directory(candidate)) {
        return 1;
    }
    written = snprintf(candidate, sizeof(candidate), "%s/data", directory);
    if (written > 0 && written < (int)sizeof(candidate) && try_change_to_directory(candidate)) {
        return 1;
    }
    written = snprintf(candidate, sizeof(candidate), "%s/../data", directory);
    if (written > 0 && written < (int)sizeof(candidate) && try_change_to_directory(candidate)) {
        return 1;
    }
    return 0;
}

static void trim_last_path_component(char *directory) {
    char *last_slash = NULL;
    if (directory == NULL || directory[0] == '\0') {
        return;
    }
    last_slash = strrchr(directory, '/');
#ifdef _WIN32
    {
        char *last_backslash = strrchr(directory, '\\');
        if (last_backslash != NULL && (last_slash == NULL || last_backslash > last_slash)) {
            last_slash = last_backslash;
        }
    }
#endif
    if (last_slash == NULL) {
        safe_copy(directory, ".", PATH_MAX);
        return;
    }
    if (last_slash == directory) {
        directory[1] = '\0';
        return;
    }
    *last_slash = '\0';
}

static int try_change_to_directory_or_parent(const char *path) {
    char directory[PATH_MAX];
    int depth = 0;
    if (!extract_directory(path, directory, sizeof(directory))) {
        return 0;
    }
    for (depth = 0; depth < 6; depth++) {
        if (try_change_to_directory(directory)) {
            return 1;
        }
        if (strcmp(directory, ".") == 0 || strcmp(directory, "/") == 0) {
            break;
        }
        trim_last_path_component(directory);
    }
    return 0;
}

void prepare_data_directory(const char *argv0, const char *source_file) {
    if (directory_has_data_files(".")) {
        return;
    }
    if (try_change_to_known_data_directory()) {
        return;
    }
    if (argv0 != NULL && try_change_to_related_data_directory(argv0)) {
        return;
    }
    if (argv0 != NULL && try_change_to_directory_or_parent(argv0)) {
        return;
    }
    if (source_file != NULL && try_change_to_related_data_directory(source_file)) {
        return;
    }
    if (source_file != NULL) {
        try_change_to_directory_or_parent(source_file);
    }
}

static char *next_field(void) {
    return strtok(NULL, "|");
}

static int parse_int_field(const char *text, int *value) {
    char *end = NULL;
    long parsed = 0;
    if (text == NULL || value == NULL || *text == '\0') {
        return 0;
    }
    parsed = strtol(text, &end, 10);
    if (end == text || (*end != '\0' && !isspace((unsigned char)*end))) {
        return 0;
    }
    *value = (int)parsed;
    return 1;
}

static int parse_double_field(const char *text, double *value) {
    char *end = NULL;
    double parsed = 0.0;
    if (text == NULL || value == NULL || *text == '\0') {
        return 0;
    }
    parsed = strtod(text, &end);
    if (end == text || (*end != '\0' && !isspace((unsigned char)*end))) {
        return 0;
    }
    *value = parsed;
    return 1;
}

static int read_string_field(char *buffer, int size) {
    char *field = next_field();
    if (field == NULL) {
        return 0;
    }
    safe_copy(buffer, field, size);
    return 1;
}

static int read_int_field(int *value) {
    return parse_int_field(next_field(), value);
}

static int read_double_field(double *value) {
    return parse_double_field(next_field(), value);
}

static void ensure_patient_login_defaults(Patient *patient) {
    if (patient == NULL) {
        return;
    }
    if (patient->password[0] == '\0') {
        generate_initial_password("P", patient->patient_id, patient->password, PASSWORD_LEN);
        patient->must_change_password = 1;
    }
    if (is_valid_id_card(patient->id_card)) {
        patient->age = age_from_id_card(patient->id_card);
    }
}

static void ensure_doctor_login_defaults(Doctor *doctor) {
    if (doctor == NULL) {
        return;
    }
    if (doctor->password[0] == '\0') {
        generate_initial_password("D", doctor->doctor_id, doctor->password, PASSWORD_LEN);
        doctor->must_change_password = 1;
    }
    if (is_valid_id_card(doctor->id_card)) {
        doctor->age = age_from_id_card(doctor->id_card);
    }
}

static void ensure_nurse_login_defaults(Nurse *nurse) {
    if (nurse == NULL) {
        return;
    }
    if (nurse->password[0] == '\0') {
        generate_initial_password("N", nurse->nurse_id, nurse->password, PASSWORD_LEN);
        nurse->must_change_password = 1;
    }
}

static void ensure_pharmacy_staff_login_defaults(PharmacyStaff *staff) {
    if (staff == NULL) {
        return;
    }
    if (staff->password[0] == '\0') {
        generate_initial_password("S", staff->staff_id, staff->password, PASSWORD_LEN);
        staff->must_change_password = 1;
    }
}

static void append_patient_loaded(HospitalSystem *system, Patient *patient) {
    Patient *node = (Patient *)malloc(sizeof(Patient));
    Patient *tail = NULL;
    if (node == NULL) {
        return;
    }
    *node = *patient;
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

/* 正式患者读档入口，保留逻辑删除字段，跳过格式不完整的旧坏行 */
static int load_patients(HospitalSystem *system) {
    FILE *fp = fopen("patients.txt", "r");
    char line[1024];
    int loaded = 0;
    if (fp == NULL) {
        return 0;
    }
    while (fgets(line, sizeof(line), fp) != NULL) {
        Patient patient;
        char *fields[14];
        char *token = NULL;
        int count = 0;
        memset(&patient, 0, sizeof(patient));
        trim_newline(line);
        strip_utf8_bom(line);
        token = strtok(line, "|");
        while (token != NULL && count < 14) {
            fields[count++] = token;
            token = strtok(NULL, "|");
        }
        if (count == 10) {
            if (!parse_int_field(fields[0], &patient.patient_id) ||
                !parse_int_field(fields[3], &patient.age) ||
                !parse_int_field(fields[7], &patient.is_inpatient) ||
                !parse_int_field(fields[8], &patient.current_bed_id) ||
                !parse_int_field(fields[9], &patient.is_deleted)) {
                continue;
            }
            safe_copy(patient.name, fields[1], NAME_LEN);
            safe_copy(patient.gender, fields[2], SMALL_LEN);
            safe_copy(patient.phone, fields[4], PHONE_LEN);
            safe_copy(patient.id_card, fields[5], ID_LEN);
            safe_copy(patient.blood_type, fields[6], SMALL_LEN);
        } else if (count == 12) {
            if (!parse_int_field(fields[0], &patient.patient_id) ||
                !parse_int_field(fields[3], &patient.age) ||
                !parse_int_field(fields[8], &patient.must_change_password) ||
                !parse_int_field(fields[9], &patient.is_inpatient) ||
                !parse_int_field(fields[10], &patient.current_bed_id) ||
                !parse_int_field(fields[11], &patient.is_deleted)) {
                continue;
            }
            safe_copy(patient.name, fields[1], NAME_LEN);
            safe_copy(patient.gender, fields[2], SMALL_LEN);
            safe_copy(patient.phone, fields[4], PHONE_LEN);
            safe_copy(patient.id_card, fields[5], ID_LEN);
            safe_copy(patient.blood_type, fields[6], SMALL_LEN);
            safe_copy(patient.password, fields[7], PASSWORD_LEN);
        } else {
            continue;
        }
        ensure_patient_login_defaults(&patient);
        append_patient_loaded(system, &patient);
        loaded++;
    }
    fclose(fp);
    return loaded;
}

static void append_doctor_loaded(HospitalSystem *system, Doctor *doctor) {
    Doctor *node = (Doctor *)malloc(sizeof(Doctor));
    Doctor *tail = NULL;
    if (node == NULL) {
        return;
    }
    *node = *doctor;
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

static int load_doctors(HospitalSystem *system) {
    FILE *fp = fopen("doctors.txt", "r");
    char line[1024];
    int loaded = 0;
    if (fp == NULL) {
        return 0;
    }
    while (fgets(line, sizeof(line), fp) != NULL) {
        Doctor doctor;
        char *fields[14];
        char *token = NULL;
        int count = 0;
        memset(&doctor, 0, sizeof(doctor));
        trim_newline(line);
        strip_utf8_bom(line);
        token = strtok(line, "|");
        while (token != NULL && count < 14) {
            fields[count++] = token;
            token = strtok(NULL, "|");
        }
        if (count == 8) {
            if (!parse_int_field(fields[0], &doctor.doctor_id) ||
                !parse_int_field(fields[4], &doctor.department_id) ||
                !parse_int_field(fields[7], &doctor.is_deleted)) {
                continue;
            }
            safe_copy(doctor.name, fields[1], NAME_LEN);
            safe_copy(doctor.gender, fields[2], SMALL_LEN);
            safe_copy(doctor.title, fields[3], NAME_LEN);
            safe_copy(doctor.specialty, fields[5], TEXT_LEN);
            safe_copy(doctor.phone, fields[6], PHONE_LEN);
        } else if (count == 12) {
            if (!parse_int_field(fields[0], &doctor.doctor_id) ||
                !parse_int_field(fields[3], &doctor.age) ||
                !parse_int_field(fields[6], &doctor.department_id) ||
                !parse_int_field(fields[10], &doctor.must_change_password) ||
                !parse_int_field(fields[11], &doctor.is_deleted)) {
                continue;
            }
            safe_copy(doctor.name, fields[1], NAME_LEN);
            safe_copy(doctor.gender, fields[2], SMALL_LEN);
            safe_copy(doctor.id_card, fields[4], ID_LEN);
            safe_copy(doctor.title, fields[5], NAME_LEN);
            safe_copy(doctor.specialty, fields[7], TEXT_LEN);
            safe_copy(doctor.phone, fields[8], PHONE_LEN);
            safe_copy(doctor.password, fields[9], PASSWORD_LEN);
        } else {
            continue;
        }
        ensure_doctor_login_defaults(&doctor);
        append_doctor_loaded(system, &doctor);
        loaded++;
    }
    fclose(fp);
    return loaded;
}

static void append_department_loaded(HospitalSystem *system, Department *department) {
    Department *node = (Department *)malloc(sizeof(Department));
    Department *tail = NULL;
    if (node == NULL) {
        return;
    }
    *node = *department;
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

static int load_departments(HospitalSystem *system) {
    FILE *fp = fopen("departments.txt", "r");
    char line[1024];
    int loaded = 0;
    if (fp == NULL) {
        return 0;
    }
    while (fgets(line, sizeof(line), fp) != NULL) {
        Department department;
        memset(&department, 0, sizeof(department));
        trim_newline(line);
        strip_utf8_bom(line);
        if (!parse_int_field(strtok(line, "|"), &department.department_id) ||
            !read_string_field(department.name, NAME_LEN) ||
            !read_string_field(department.manager, NAME_LEN) ||
            !read_string_field(department.description, TEXT_LEN) ||
            !read_int_field(&department.is_deleted)) {
            continue;
        }
        append_department_loaded(system, &department);
        loaded++;
    }
    fclose(fp);
    return loaded;
}

static void append_ward_loaded(HospitalSystem *system, Ward *ward) {
    Ward *node = (Ward *)malloc(sizeof(Ward));
    Ward *tail = NULL;
    if (node == NULL) {
        return;
    }
    *node = *ward;
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

static int load_wards(HospitalSystem *system) {
    FILE *fp = fopen("wards.txt", "r");
    char line[1024];
    int loaded = 0;
    if (fp == NULL) {
        return 0;
    }
    while (fgets(line, sizeof(line), fp) != NULL) {
        Ward ward;
        memset(&ward, 0, sizeof(ward));
        trim_newline(line);
        strip_utf8_bom(line);
        if (!parse_int_field(strtok(line, "|"), &ward.ward_id) ||
            !read_string_field(ward.name, NAME_LEN) ||
            !read_string_field(ward.ward_type, TYPE_LEN) ||
            !read_int_field(&ward.department_id) ||
            !read_int_field(&ward.floor_no) ||
            !read_int_field(&ward.capacity) ||
            !read_int_field(&ward.is_active)) {
            continue;
        }
        append_ward_loaded(system, &ward);
        loaded++;
    }
    fclose(fp);
    return loaded;
}

static void append_bed_loaded(HospitalSystem *system, Bed *bed) {
    Bed *node = (Bed *)malloc(sizeof(Bed));
    Bed *tail = NULL;
    if (node == NULL) {
        return;
    }
    *node = *bed;
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

static int load_beds(HospitalSystem *system) {
    FILE *fp = fopen("beds.txt", "r");
    char line[1024];
    int loaded = 0;
    if (fp == NULL) {
        return 0;
    }
    while (fgets(line, sizeof(line), fp) != NULL) {
        Bed bed;
        memset(&bed, 0, sizeof(bed));
        trim_newline(line);
        strip_utf8_bom(line);
        if (!parse_int_field(strtok(line, "|"), &bed.bed_id) ||
            !read_int_field(&bed.ward_id) ||
            !read_int_field(&bed.department_id) ||
            !read_string_field(bed.bed_no, NAME_LEN) ||
            !read_int_field(&bed.status) ||
            !read_int_field(&bed.patient_id)) {
            continue;
        }
        append_bed_loaded(system, &bed);
        loaded++;
    }
    fclose(fp);
    return loaded;
}

static void append_drug_loaded(HospitalSystem *system, Drug *drug) {
    Drug *node = (Drug *)malloc(sizeof(Drug));
    Drug *tail = NULL;
    if (node == NULL) {
        return;
    }
    *node = *drug;
    node->next = NULL;
    if (system->drugs == NULL) {
        system->drugs = node;
    } else {
        tail = system->drugs;
        while (tail->next != NULL) {
            tail = tail->next;
        }
        tail->next = node;
    }
}

static Drug *find_drug_loaded_by_id(HospitalSystem *system, int drug_id) {
    Drug *current = system->drugs;
    while (current != NULL) {
        if (current->drug_id == drug_id) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

static void append_drug_department_rule_loaded(HospitalSystem *system, int drug_id, int department_id, int rule_type) {
    DrugDepartmentRule *node = (DrugDepartmentRule *)malloc(sizeof(DrugDepartmentRule));
    DrugDepartmentRule *tail = NULL;
    if (node == NULL) {
        return;
    }
    node->drug_id = drug_id;
    node->department_id = department_id;
    node->rule_type = rule_type;
    node->next = NULL;
    if (system->drug_department_rules == NULL) {
        system->drug_department_rules = node;
        return;
    }
    tail = system->drug_department_rules;
    while (tail->next != NULL) {
        tail = tail->next;
    }
    tail->next = node;
}

static void set_default_drug_dates(Drug *drug, int index) {
    char production_date[DATE_LEN];
    char expiry_date[DATE_LEN];
    if (drug == NULL) {
        return;
    }
    if (!add_days_to_date("2026-01-01", index * 3, production_date, DATE_LEN)) {
        safe_copy(production_date, "2026-01-01", DATE_LEN);
    }
    if (!add_years_to_date(production_date, 2, expiry_date, DATE_LEN)) {
        safe_copy(expiry_date, "2028-01-01", DATE_LEN);
    }
    safe_copy(drug->production_date, production_date, DATE_LEN);
    safe_copy(drug->expiry_date, expiry_date, DATE_LEN);
}

static void normalize_loaded_drug_dates(Drug *drug, int index) {
    if (drug == NULL) {
        return;
    }
    if (!is_valid_date_text(drug->production_date) ||
        !is_valid_date_text(drug->expiry_date) ||
        date_to_serial(drug->expiry_date) <= date_to_serial(drug->production_date)) {
        set_default_drug_dates(drug, index);
    }
}

static int load_drugs(HospitalSystem *system) {
    FILE *fp = fopen("drugs.txt", "r");
    char line[1024];
    int loaded = 0;
    if (fp == NULL) {
        return 0;
    }
    while (fgets(line, sizeof(line), fp) != NULL) {
        Drug drug;
        char *fields[14];
        int count = 0;
        char *token = NULL;
        memset(&drug, 0, sizeof(drug));
        trim_newline(line);
        strip_utf8_bom(line);
        token = strtok(line, "|");
        while (token != NULL && count < 12) {
            fields[count++] = token;
            token = strtok(NULL, "|");
        }
        if (count == 8) {
            /* 旧格式：编号|通用名|品牌|进价|售价|库存|已用量|删除标记 */
            safe_copy(drug.generic_name, fields[1], DRUG_NAME_LEN);
            safe_copy(drug.common_name, fields[1], DRUG_NAME_LEN);
            safe_copy(drug.brand_name, fields[2], DRUG_NAME_LEN);
            set_default_drug_dates(&drug, loaded);
            if (!parse_int_field(fields[0], &drug.drug_id) ||
                !parse_double_field(fields[3], &drug.purchase_price) ||
                !parse_int_field(fields[5], &drug.stock) ||
                !parse_int_field(fields[6], &drug.total_used) ||
                !parse_int_field(fields[7], &drug.is_deleted)) {
                continue;
            }
        } else if (count == 9) {
            int old_department_id = 0;
            safe_copy(drug.generic_name, fields[1], DRUG_NAME_LEN);
            if (parse_int_field(fields[3], &old_department_id)) {
                /* 旧格式：编号|通用名|品牌|科室|进价|售价|库存|已用量|删除标记 */
                safe_copy(drug.common_name, fields[1], DRUG_NAME_LEN);
                safe_copy(drug.brand_name, fields[2], DRUG_NAME_LEN);
                set_default_drug_dates(&drug, loaded);
                if (!parse_int_field(fields[0], &drug.drug_id) ||
                    !parse_double_field(fields[4], &drug.purchase_price) ||
                    !parse_int_field(fields[6], &drug.stock) ||
                    !parse_int_field(fields[7], &drug.total_used) ||
                    !parse_int_field(fields[8], &drug.is_deleted)) {
                    continue;
                }
            } else {
                /* 新格式：编号|通用名|俗称|品牌|进价|售价|库存|已用量|删除标记 */
                safe_copy(drug.common_name, fields[2], DRUG_NAME_LEN);
                safe_copy(drug.brand_name, fields[3], DRUG_NAME_LEN);
                set_default_drug_dates(&drug, loaded);
                if (!parse_int_field(fields[0], &drug.drug_id) ||
                    !parse_double_field(fields[4], &drug.purchase_price) ||
                    !parse_int_field(fields[6], &drug.stock) ||
                    !parse_int_field(fields[7], &drug.total_used) ||
                    !parse_int_field(fields[8], &drug.is_deleted)) {
                    continue;
                }
            }
        } else if (count == 10) {
            /* 这个是兼容更早的旧数据格式，当时还带过别名、预警这些列 */
            safe_copy(drug.generic_name, fields[1], DRUG_NAME_LEN);
            safe_copy(drug.brand_name, fields[2], DRUG_NAME_LEN);
            safe_copy(drug.common_name, fields[3], DRUG_NAME_LEN);
            set_default_drug_dates(&drug, loaded);
            if (!parse_int_field(fields[0], &drug.drug_id) ||
                !parse_double_field(fields[5], &drug.purchase_price) ||
                !parse_int_field(fields[6], &drug.stock) ||
                !parse_int_field(fields[8], &drug.total_used) ||
                !parse_int_field(fields[9], &drug.is_deleted)) {
                continue;
            }
            drug.sale_price = drug.purchase_price * DRUG_SALE_RATE;
        } else if (count == 11) {
            /* 当前格式：编号|通用名|俗称|品牌|进价|售价|库存|已用量|删除标记|生产日期|有效期至 */
            safe_copy(drug.generic_name, fields[1], DRUG_NAME_LEN);
            safe_copy(drug.common_name, fields[2], DRUG_NAME_LEN);
            safe_copy(drug.brand_name, fields[3], DRUG_NAME_LEN);
            safe_copy(drug.production_date, fields[9], DATE_LEN);
            safe_copy(drug.expiry_date, fields[10], DATE_LEN);
            if (!parse_int_field(fields[0], &drug.drug_id) ||
                !parse_double_field(fields[4], &drug.purchase_price) ||
                !parse_int_field(fields[6], &drug.stock) ||
                !parse_int_field(fields[7], &drug.total_used) ||
                !parse_int_field(fields[8], &drug.is_deleted)) {
                continue;
            }
        } else {
            continue;
        }
        if (is_blank_string(drug.common_name)) {
            safe_copy(drug.common_name, drug.generic_name, DRUG_NAME_LEN);
        }
        normalize_loaded_drug_dates(&drug, loaded);
        drug.sale_price = drug.purchase_price * DRUG_SALE_RATE;
        append_drug_loaded(system, &drug);
        loaded++;
    }
    fclose(fp);
    return loaded;
}

static void append_nurse_loaded(HospitalSystem *system, Nurse *nurse) {
    Nurse *node = (Nurse *)malloc(sizeof(Nurse));
    Nurse *tail = NULL;
    if (node == NULL) {
        return;
    }
    *node = *nurse;
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

static int load_nurses(HospitalSystem *system) {
    FILE *fp = fopen("nurses.txt", "r");
    char line[1024];
    int loaded = 0;
    if (fp == NULL) {
        return 0;
    }
    while (fgets(line, sizeof(line), fp) != NULL) {
        Nurse nurse;
        char *fields[12];
        char *token = NULL;
        int count = 0;
        memset(&nurse, 0, sizeof(nurse));
        trim_newline(line);
        strip_utf8_bom(line);
        token = strtok(line, "|");
        while (token != NULL && count < 12) {
            fields[count++] = token;
            token = strtok(NULL, "|");
        }
        if (count == 7) {
            if (!parse_int_field(fields[0], &nurse.nurse_id) ||
                !parse_int_field(fields[4], &nurse.department_id) ||
                !parse_int_field(fields[6], &nurse.is_deleted)) {
                continue;
            }
            safe_copy(nurse.name, fields[1], NAME_LEN);
            safe_copy(nurse.gender, fields[2], SMALL_LEN);
            safe_copy(nurse.title, fields[3], NAME_LEN);
            safe_copy(nurse.phone, fields[5], PHONE_LEN);
        } else if (count == 9) {
            if (!parse_int_field(fields[0], &nurse.nurse_id) ||
                !parse_int_field(fields[4], &nurse.department_id) ||
                !parse_int_field(fields[7], &nurse.must_change_password) ||
                !parse_int_field(fields[8], &nurse.is_deleted)) {
                continue;
            }
            safe_copy(nurse.name, fields[1], NAME_LEN);
            safe_copy(nurse.gender, fields[2], SMALL_LEN);
            safe_copy(nurse.title, fields[3], NAME_LEN);
            safe_copy(nurse.phone, fields[5], PHONE_LEN);
            safe_copy(nurse.password, fields[6], PASSWORD_LEN);
        } else {
            continue;
        }
        ensure_nurse_login_defaults(&nurse);
        append_nurse_loaded(system, &nurse);
        loaded++;
    }
    fclose(fp);
    return loaded;
}

static void append_pharmacy_staff_loaded(HospitalSystem *system, PharmacyStaff *staff) {
    PharmacyStaff *node = (PharmacyStaff *)malloc(sizeof(PharmacyStaff));
    PharmacyStaff *tail = NULL;
    if (node == NULL) {
        return;
    }
    *node = *staff;
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

static int load_pharmacy_staffs(HospitalSystem *system) {
    FILE *fp = fopen("pharmacy_staff.txt", "r");
    char line[1024];
    int loaded = 0;
    if (fp == NULL) {
        return 0;
    }
    while (fgets(line, sizeof(line), fp) != NULL) {
        PharmacyStaff staff;
        char *fields[10];
        char *token = NULL;
        int count = 0;
        memset(&staff, 0, sizeof(staff));
        trim_newline(line);
        strip_utf8_bom(line);
        token = strtok(line, "|");
        while (token != NULL && count < 10) {
            fields[count++] = token;
            token = strtok(NULL, "|");
        }
        if (count == 5) {
            if (!parse_int_field(fields[0], &staff.staff_id) ||
                !parse_int_field(fields[4], &staff.is_deleted)) {
                continue;
            }
            safe_copy(staff.name, fields[1], NAME_LEN);
            safe_copy(staff.gender, fields[2], SMALL_LEN);
            safe_copy(staff.phone, fields[3], PHONE_LEN);
        } else if (count == 7) {
            if (!parse_int_field(fields[0], &staff.staff_id) ||
                !parse_int_field(fields[5], &staff.must_change_password) ||
                !parse_int_field(fields[6], &staff.is_deleted)) {
                continue;
            }
            safe_copy(staff.name, fields[1], NAME_LEN);
            safe_copy(staff.gender, fields[2], SMALL_LEN);
            safe_copy(staff.phone, fields[3], PHONE_LEN);
            safe_copy(staff.password, fields[4], PASSWORD_LEN);
        } else {
            continue;
        }
        ensure_pharmacy_staff_login_defaults(&staff);
        append_pharmacy_staff_loaded(system, &staff);
        loaded++;
    }
    fclose(fp);
    return loaded;
}

static int load_drug_department_rules(HospitalSystem *system) {
    FILE *fp = fopen("drug_department_rules.txt", "r");
    char line[256];
    int loaded = 0;
    if (fp == NULL) {
        return 0;
    }
    while (fgets(line, sizeof(line), fp) != NULL) {
        char *tag = NULL;
        trim_newline(line);
        strip_utf8_bom(line);
        tag = strtok(line, "|");
        if (tag == NULL) {
            continue;
        }
        if (strcmp(tag, "MODE") == 0) {
            int drug_id = 0;
            int access_mode = 0;
            Drug *drug = NULL;
            if (!read_int_field(&drug_id) || !read_int_field(&access_mode)) {
                continue;
            }
            drug = find_drug_loaded_by_id(system, drug_id);
            if (drug == NULL) {
                continue;
            }
            drug->access_mode = access_mode == DRUG_ACCESS_DENY_ALL ? DRUG_ACCESS_DENY_ALL : DRUG_ACCESS_ALLOW_ALL;
            loaded++;
        } else if (strcmp(tag, "RULE") == 0) {
            int drug_id = 0;
            int department_id = 0;
            int rule_type = 0;
            if (!read_int_field(&drug_id) ||
                !read_int_field(&department_id) ||
                !read_int_field(&rule_type)) {
                continue;
            }
            append_drug_department_rule_loaded(
                system,
                drug_id,
                department_id,
                rule_type == DRUG_RULE_ALLOW ? DRUG_RULE_ALLOW : DRUG_RULE_DENY
            );
            loaded++;
        }
    }
    fclose(fp);
    return loaded;
}

static void append_record_loaded(HospitalSystem *system, MedicalRecord *record) {
    MedicalRecord *node = (MedicalRecord *)malloc(sizeof(MedicalRecord));
    MedicalRecord *tail = NULL;
    if (node == NULL) {
        return;
    }
    *node = *record;
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

static int load_records(HospitalSystem *system) {
    FILE *fp = fopen("records.txt", "r");
    char line[2048];
    int loaded = 0;
    if (fp == NULL) {
        return 0;
    }
    while (fgets(line, sizeof(line), fp) != NULL) {
        MedicalRecord record;
        memset(&record, 0, sizeof(record));
        trim_newline(line);
        strip_utf8_bom(line);
        if (!parse_int_field(strtok(line, "|"), &record.record_id) ||
            !read_int_field(&record.patient_id) ||
            !read_int_field(&record.doctor_id) ||
            !read_int_field(&record.department_id) ||
            !read_string_field(record.record_type, TYPE_LEN) ||
            !read_string_field(record.date, DATE_LEN) ||
            !read_string_field(record.diagnosis, TEXT_LEN) ||
            !read_double_field(&record.cost) ||
            !read_string_field(record.note, TEXT_LEN)) {
            continue;
        }
        append_record_loaded(system, &record);
        loaded++;
    }
    fclose(fp);
    return loaded;
}

static void append_registration_loaded(HospitalSystem *system, OutpatientRegistration *registration) {
    OutpatientRegistration *node = (OutpatientRegistration *)malloc(sizeof(OutpatientRegistration));
    OutpatientRegistration *tail = NULL;
    if (node == NULL) {
        return;
    }
    *node = *registration;
    node->next = NULL;
    if (system->registrations == NULL) {
        system->registrations = node;
    } else {
        tail = system->registrations;
        while (tail->next != NULL) {
            tail = tail->next;
        }
        tail->next = node;
    }
}

static int load_registrations(HospitalSystem *system) {
    FILE *fp = fopen("registrations.txt", "r");
    char line[2048];
    int loaded = 0;
    if (fp == NULL) {
        return 0;
    }
    while (fgets(line, sizeof(line), fp) != NULL) {
        OutpatientRegistration registration;
        memset(&registration, 0, sizeof(registration));
        trim_newline(line);
        strip_utf8_bom(line);
        if (!parse_int_field(strtok(line, "|"), &registration.registration_id) ||
            !read_int_field(&registration.patient_id) ||
            !read_int_field(&registration.doctor_id) ||
            !read_int_field(&registration.department_id) ||
            !read_int_field(&registration.registration_type) ||
            !read_string_field(registration.created_time, DATE_LEN) ||
            !read_string_field(registration.visit_time, DATE_LEN) ||
            !read_int_field(&registration.status) ||
            !read_string_field(registration.symptom, TEXT_LEN)) {
            continue;
        }
        append_registration_loaded(system, &registration);
        loaded++;
    }
    fclose(fp);
    return loaded;
}

static void append_admission_loaded(HospitalSystem *system, AdmissionHistory *admission) {
    AdmissionHistory *node = (AdmissionHistory *)malloc(sizeof(AdmissionHistory));
    AdmissionHistory *tail = NULL;
    if (node == NULL) {
        return;
    }
    *node = *admission;
    node->next = NULL;
    if (system->admissions == NULL) {
        system->admissions = node;
    } else {
        tail = system->admissions;
        while (tail->next != NULL) {
            tail = tail->next;
        }
        tail->next = node;
    }
}

static int load_admissions(HospitalSystem *system) {
    FILE *fp = fopen("admissions.txt", "r");
    char line[2048];
    int loaded = 0;
    if (fp == NULL) {
        return 0;
    }
    while (fgets(line, sizeof(line), fp) != NULL) {
        AdmissionHistory admission;
        memset(&admission, 0, sizeof(admission));
        trim_newline(line);
        strip_utf8_bom(line);
        if (!parse_int_field(strtok(line, "|"), &admission.admission_id) ||
            !read_int_field(&admission.patient_id) ||
            !read_int_field(&admission.department_id) ||
            !read_int_field(&admission.doctor_id) ||
            !read_int_field(&admission.ward_id) ||
            !read_int_field(&admission.bed_id) ||
            !read_string_field(admission.admit_date, DATE_LEN) ||
            !read_string_field(admission.discharge_date, DATE_LEN) ||
            !read_int_field(&admission.stay_days) ||
            !read_double_field(&admission.deposit) ||
            !read_string_field(admission.diagnosis, TEXT_LEN) ||
            !read_int_field(&admission.status)) {
            continue;
        }
        append_admission_loaded(system, &admission);
        loaded++;
    }
    fclose(fp);
    return loaded;
}

static void append_usage_loaded(HospitalSystem *system, DrugUsageHistory *usage) {
    DrugUsageHistory *node = (DrugUsageHistory *)malloc(sizeof(DrugUsageHistory));
    DrugUsageHistory *tail = NULL;
    if (node == NULL) {
        return;
    }
    *node = *usage;
    node->next = NULL;
    if (system->drug_usages == NULL) {
        system->drug_usages = node;
    } else {
        tail = system->drug_usages;
        while (tail->next != NULL) {
            tail = tail->next;
        }
        tail->next = node;
    }
}

static int load_drug_usages(HospitalSystem *system) {
    FILE *fp = fopen("drug_usage.txt", "r");
    char line[2048];
    int loaded = 0;
    if (fp == NULL) {
        return 0;
    }
    while (fgets(line, sizeof(line), fp) != NULL) {
        DrugUsageHistory usage;
        char *unit_or_amount = NULL;
        char *purchase_or_amount = NULL;
        char *amount_or_note = NULL;
        char *note_field = NULL;
        Drug *drug = NULL;
        memset(&usage, 0, sizeof(usage));
        trim_newline(line);
        strip_utf8_bom(line);
        if (!parse_int_field(strtok(line, "|"), &usage.usage_id) ||
            !read_int_field(&usage.drug_id) ||
            !read_int_field(&usage.patient_id) ||
            !read_int_field(&usage.doctor_id) ||
            !read_int_field(&usage.department_id) ||
            !read_string_field(usage.date, DATE_LEN) ||
            !read_int_field(&usage.quantity)) {
            continue;
        }
        unit_or_amount = next_field();
        purchase_or_amount = next_field();
        amount_or_note = next_field();
        note_field = next_field();
        if (unit_or_amount == NULL || purchase_or_amount == NULL) {
            continue;
        }
        if (amount_or_note == NULL) {
            if (!parse_double_field(unit_or_amount, &usage.amount)) {
                continue;
            }
            usage.unit_price = usage.quantity > 0 ? usage.amount / usage.quantity : 0.0;
            safe_copy(usage.note, purchase_or_amount, TEXT_LEN);
        } else if (note_field == NULL) {
            if (!parse_double_field(unit_or_amount, &usage.unit_price) ||
                !parse_double_field(purchase_or_amount, &usage.amount)) {
                continue;
            }
            safe_copy(usage.note, amount_or_note, TEXT_LEN);
        } else {
            if (!parse_double_field(unit_or_amount, &usage.unit_price) ||
                !parse_double_field(purchase_or_amount, &usage.purchase_price) ||
                !parse_double_field(amount_or_note, &usage.amount)) {
                continue;
            }
            safe_copy(usage.note, note_field, TEXT_LEN);
        }
        if (usage.unit_price <= 0.0 && usage.quantity > 0) {
            usage.unit_price = usage.amount / usage.quantity;
        }
        if (usage.purchase_price <= 0.0) {
            drug = find_drug_loaded_by_id(system, usage.drug_id);
            usage.purchase_price = drug != NULL ? drug->purchase_price : 0.0;
        }
        if (usage.amount <= 0.0 && usage.quantity > 0) {
            usage.amount = usage.unit_price * usage.quantity;
        }
        if (usage.note[0] == '\0') {
            continue;
        }
        append_usage_loaded(system, &usage);
        loaded++;
    }
    fclose(fp);
    return loaded;
}

static void append_prescription_loaded(HospitalSystem *system, Prescription *prescription) {
    Prescription *node = (Prescription *)malloc(sizeof(Prescription));
    Prescription *tail = NULL;
    if (node == NULL) {
        return;
    }
    *node = *prescription;
    node->next = NULL;
    if (system->prescriptions == NULL) {
        system->prescriptions = node;
    } else {
        tail = system->prescriptions;
        while (tail->next != NULL) {
            tail = tail->next;
        }
        tail->next = node;
    }
}

static int load_prescriptions(HospitalSystem *system) {
    FILE *fp = fopen("prescriptions.txt", "r");
    char line[1024];
    int loaded = 0;
    if (fp == NULL) {
        return 0;
    }
    while (fgets(line, sizeof(line), fp) != NULL) {
        Prescription prescription;
        memset(&prescription, 0, sizeof(prescription));
        trim_newline(line);
        strip_utf8_bom(line);
        if (!parse_int_field(strtok(line, "|"), &prescription.prescription_id) ||
            !read_int_field(&prescription.patient_id) ||
            !read_int_field(&prescription.doctor_id) ||
            !read_int_field(&prescription.department_id) ||
            !read_string_field(prescription.created_time, DATE_LEN) ||
            !read_int_field(&prescription.status) ||
            !read_string_field(prescription.note, TEXT_LEN)) {
            continue;
        }
        append_prescription_loaded(system, &prescription);
        loaded++;
    }
    fclose(fp);
    return loaded;
}

static void append_prescription_item_loaded(HospitalSystem *system, PrescriptionItem *item) {
    PrescriptionItem *node = (PrescriptionItem *)malloc(sizeof(PrescriptionItem));
    PrescriptionItem *tail = NULL;
    if (node == NULL) {
        return;
    }
    *node = *item;
    node->next = NULL;
    if (system->prescription_items == NULL) {
        system->prescription_items = node;
    } else {
        tail = system->prescription_items;
        while (tail->next != NULL) {
            tail = tail->next;
        }
        tail->next = node;
    }
}

static int load_prescription_items(HospitalSystem *system) {
    FILE *fp = fopen("prescription_items.txt", "r");
    char line[1024];
    int loaded = 0;
    if (fp == NULL) {
        return 0;
    }
    while (fgets(line, sizeof(line), fp) != NULL) {
        PrescriptionItem item;
        memset(&item, 0, sizeof(item));
        trim_newline(line);
        strip_utf8_bom(line);
        if (!parse_int_field(strtok(line, "|"), &item.prescription_id) ||
            !read_int_field(&item.drug_id) ||
            !read_int_field(&item.quantity) ||
            !read_string_field(item.note, TEXT_LEN)) {
            continue;
        }
        append_prescription_item_loaded(system, &item);
        loaded++;
    }
    fclose(fp);
    return loaded;
}

static void append_log_loaded(HospitalSystem *system, OperationLog *log_node) {
    OperationLog *node = (OperationLog *)malloc(sizeof(OperationLog));
    OperationLog *tail = NULL;
    if (node == NULL) {
        return;
    }
    *node = *log_node;
    node->next = NULL;
    if (system->logs == NULL) {
        system->logs = node;
    } else {
        tail = system->logs;
        while (tail->next != NULL) {
            tail = tail->next;
        }
        tail->next = node;
    }
}

static int load_logs(HospitalSystem *system) {
    FILE *fp = fopen("log.txt", "r");
    char line[2048];
    int loaded = 0;
    if (fp == NULL) {
        return 0;
    }
    while (fgets(line, sizeof(line), fp) != NULL) {
        OperationLog log_node;
        memset(&log_node, 0, sizeof(log_node));
        trim_newline(line);
        strip_utf8_bom(line);
        if (!parse_int_field(strtok(line, "|"), &log_node.log_id) ||
            !read_string_field(log_node.date, DATE_LEN) ||
            !read_string_field(log_node.role, NAME_LEN) ||
            !read_string_field(log_node.action, NAME_LEN) ||
            !read_string_field(log_node.detail, TEXT_LEN)) {
            continue;
        }
        append_log_loaded(system, &log_node);
        loaded++;
    }
    fclose(fp);
    return loaded;
}

static int save_patients(HospitalSystem *system) {
    FILE *fp = fopen("patients.txt", "w");
    Patient *current = system->patients;
    if (fp == NULL) {
        return 0;
    }
    write_utf8_bom(fp);
    while (current != NULL) {
        fprintf(fp, "%d|%s|%s|%d|%s|%s|%s|%s|%d|%d|%d|%d\n",
                current->patient_id, current->name, current->gender, current->age,
                current->phone, current->id_card, current->blood_type,
                current->password, current->must_change_password,
                current->is_inpatient, current->current_bed_id, current->is_deleted);
        current = current->next;
    }
    fclose(fp);
    return 1;
}

static int save_doctors(HospitalSystem *system) {
    FILE *fp = fopen("doctors.txt", "w");
    Doctor *current = system->doctors;
    if (fp == NULL) {
        return 0;
    }
    write_utf8_bom(fp);
    while (current != NULL) {
        fprintf(fp, "%d|%s|%s|%d|%s|%s|%d|%s|%s|%s|%d|%d\n",
                current->doctor_id, current->name, current->gender, current->age,
                current->id_card, current->title, current->department_id,
                current->specialty, current->phone, current->password,
                current->must_change_password, current->is_deleted);
        current = current->next;
    }
    fclose(fp);
    return 1;
}

static int save_departments(HospitalSystem *system) {
    FILE *fp = fopen("departments.txt", "w");
    Department *current = system->departments;
    if (fp == NULL) {
        return 0;
    }
    write_utf8_bom(fp);
    while (current != NULL) {
        fprintf(fp, "%d|%s|%s|%s|%d\n",
                current->department_id, current->name, current->manager,
                current->description, current->is_deleted);
        current = current->next;
    }
    fclose(fp);
    return 1;
}

static int save_wards(HospitalSystem *system) {
    FILE *fp = fopen("wards.txt", "w");
    Ward *current = system->wards;
    if (fp == NULL) {
        return 0;
    }
    write_utf8_bom(fp);
    while (current != NULL) {
        fprintf(fp, "%d|%s|%s|%d|%d|%d|%d\n",
                current->ward_id, current->name, current->ward_type, current->department_id,
                current->floor_no, current->capacity, current->is_active);
        current = current->next;
    }
    fclose(fp);
    return 1;
}

static int save_beds(HospitalSystem *system) {
    FILE *fp = fopen("beds.txt", "w");
    Bed *current = system->beds;
    if (fp == NULL) {
        return 0;
    }
    write_utf8_bom(fp);
    while (current != NULL) {
        fprintf(fp, "%d|%d|%d|%s|%d|%d\n",
                current->bed_id, current->ward_id, current->department_id,
                current->bed_no, current->status, current->patient_id);
        current = current->next;
    }
    fclose(fp);
    return 1;
}

static int save_drugs(HospitalSystem *system) {
    FILE *fp = fopen("drugs.txt", "w");
    Drug *current = system->drugs;
    if (fp == NULL) {
        return 0;
    }
    write_utf8_bom(fp);
    while (current != NULL) {
        current->sale_price = current->purchase_price * DRUG_SALE_RATE;
        fprintf(fp, "%d|%s|%s|%s|%.2f|%.2f|%d|%d|%d|%s|%s\n",
                current->drug_id, current->generic_name, current->common_name, current->brand_name,
                current->purchase_price, current->sale_price,
                current->stock, current->total_used, current->is_deleted,
                current->production_date, current->expiry_date);
        current = current->next;
    }
    fclose(fp);
    return 1;
}

static int save_nurses(HospitalSystem *system) {
    FILE *fp = fopen("nurses.txt", "w");
    Nurse *current = system->nurses;
    if (fp == NULL) {
        return 0;
    }
    write_utf8_bom(fp);
    while (current != NULL) {
        fprintf(fp, "%d|%s|%s|%s|%d|%s|%s|%d|%d\n",
                current->nurse_id, current->name, current->gender, current->title,
                current->department_id, current->phone, current->password,
                current->must_change_password, current->is_deleted);
        current = current->next;
    }
    fclose(fp);
    return 1;
}

static int save_pharmacy_staffs(HospitalSystem *system) {
    FILE *fp = fopen("pharmacy_staff.txt", "w");
    PharmacyStaff *current = system->pharmacy_staffs;
    if (fp == NULL) {
        return 0;
    }
    write_utf8_bom(fp);
    while (current != NULL) {
        fprintf(fp, "%d|%s|%s|%s|%s|%d|%d\n",
                current->staff_id, current->name, current->gender,
                current->phone, current->password,
                current->must_change_password, current->is_deleted);
        current = current->next;
    }
    fclose(fp);
    return 1;
}

static int save_drug_department_rules(HospitalSystem *system) {
    FILE *fp = fopen("drug_department_rules.txt", "w");
    Drug *drug = system->drugs;
    DrugDepartmentRule *rule = system->drug_department_rules;
    if (fp == NULL) {
        return 0;
    }
    write_utf8_bom(fp);
    while (drug != NULL) {
        fprintf(fp, "MODE|%d|%d\n", drug->drug_id, drug->access_mode);
        drug = drug->next;
    }
    while (rule != NULL) {
        fprintf(fp, "RULE|%d|%d|%d\n", rule->drug_id, rule->department_id, rule->rule_type);
        rule = rule->next;
    }
    fclose(fp);
    return 1;
}

static int save_records(HospitalSystem *system) {
    FILE *fp = fopen("records.txt", "w");
    MedicalRecord *current = system->records;
    if (fp == NULL) {
        return 0;
    }
    write_utf8_bom(fp);
    while (current != NULL) {
        fprintf(fp, "%d|%d|%d|%d|%s|%s|%s|%.2f|%s\n",
                current->record_id, current->patient_id, current->doctor_id,
                current->department_id, current->record_type, current->date,
                current->diagnosis, current->cost, current->note);
        current = current->next;
    }
    fclose(fp);
    return 1;
}

static int save_registrations(HospitalSystem *system) {
    FILE *fp = fopen("registrations.txt", "w");
    OutpatientRegistration *current = system->registrations;
    if (fp == NULL) {
        return 0;
    }
    write_utf8_bom(fp);
    while (current != NULL) {
        fprintf(fp, "%d|%d|%d|%d|%d|%s|%s|%d|%s\n",
                current->registration_id, current->patient_id, current->doctor_id,
                current->department_id, current->registration_type, current->created_time,
                current->visit_time, current->status, current->symptom);
        current = current->next;
    }
    fclose(fp);
    return 1;
}

static int save_admissions(HospitalSystem *system) {
    FILE *fp = fopen("admissions.txt", "w");
    AdmissionHistory *current = system->admissions;
    if (fp == NULL) {
        return 0;
    }
    write_utf8_bom(fp);
    while (current != NULL) {
        fprintf(fp, "%d|%d|%d|%d|%d|%d|%s|%s|%d|%.2f|%s|%d\n",
                current->admission_id, current->patient_id, current->department_id,
                current->doctor_id, current->ward_id, current->bed_id, current->admit_date,
                current->discharge_date, current->stay_days, current->deposit,
                current->diagnosis, current->status);
        current = current->next;
    }
    fclose(fp);
    return 1;
}

static int save_drug_usages(HospitalSystem *system) {
    FILE *fp = fopen("drug_usage.txt", "w");
    DrugUsageHistory *current = system->drug_usages;
    if (fp == NULL) {
        return 0;
    }
    write_utf8_bom(fp);
    while (current != NULL) {
        fprintf(fp, "%d|%d|%d|%d|%d|%s|%d|%.2f|%.2f|%.2f|%s\n",
                current->usage_id, current->drug_id, current->patient_id,
                current->doctor_id, current->department_id, current->date,
                current->quantity, current->unit_price, current->purchase_price,
                current->amount, current->note);
        current = current->next;
    }
    fclose(fp);
    return 1;
}

static int save_prescriptions(HospitalSystem *system) {
    FILE *fp = fopen("prescriptions.txt", "w");
    Prescription *current = system->prescriptions;
    if (fp == NULL) {
        return 0;
    }
    write_utf8_bom(fp);
    while (current != NULL) {
        fprintf(fp, "%d|%d|%d|%d|%s|%d|%s\n",
                current->prescription_id, current->patient_id, current->doctor_id,
                current->department_id, current->created_time, current->status, current->note);
        current = current->next;
    }
    fclose(fp);
    return 1;
}

static int save_prescription_items(HospitalSystem *system) {
    FILE *fp = fopen("prescription_items.txt", "w");
    PrescriptionItem *current = system->prescription_items;
    if (fp == NULL) {
        return 0;
    }
    write_utf8_bom(fp);
    while (current != NULL) {
        fprintf(fp, "%d|%d|%d|%s\n",
                current->prescription_id, current->drug_id, current->quantity, current->note);
        current = current->next;
    }
    fclose(fp);
    return 1;
}

static int save_logs(HospitalSystem *system) {
    FILE *fp = fopen("log.txt", "w");
    OperationLog *current = system->logs;
    if (fp == NULL) {
        return 0;
    }
    write_utf8_bom(fp);
    while (current != NULL) {
        fprintf(fp, "%d|%s|%s|%s|%s\n",
                current->log_id, current->date, current->role, current->action, current->detail);
        current = current->next;
    }
    fclose(fp);
    return 1;
}

/* 统一读档入口
 * 按“主数据 -> 业务数据 -> 日志数据”的顺序读取文件，
 * 读完以后再把各种 next_id 重算一下，免得后面新增数据编号出问题
 */
int load_all_data(HospitalSystem *system) {
    int total = 0;
    total += load_departments(system);
    total += load_doctors(system);
    total += load_nurses(system);
    total += load_pharmacy_staffs(system);
    total += load_patients(system);
    total += load_wards(system);
    total += load_beds(system);
    total += load_drugs(system);
    total += load_drug_department_rules(system);
    total += load_records(system);
    total += load_registrations(system);
    total += load_admissions(system);
    total += load_drug_usages(system);
    total += load_prescriptions(system);
    total += load_prescription_items(system);
    total += load_logs(system);
    rebuild_next_ids(system);
    return total;
}

/* 统一存档入口
 * 它会按顺序把所有数据文件都存一遍，用 success 一路累积结果，
 * 只要中间有一类没存成功，最后返回值就会变成 0
 */
int save_all_data(HospitalSystem *system) {
    int success = 1;
    success &= save_patients(system);
    success &= save_doctors(system);
    success &= save_nurses(system);
    success &= save_pharmacy_staffs(system);
    success &= save_departments(system);
    success &= save_wards(system);
    success &= save_beds(system);
    success &= save_drugs(system);
    success &= save_drug_department_rules(system);
    success &= save_records(system);
    success &= save_registrations(system);
    success &= save_admissions(system);
    success &= save_drug_usages(system);
    success &= save_prescriptions(system);
    success &= save_prescription_items(system);
    success &= save_logs(system);
    return success;
}

#endif
