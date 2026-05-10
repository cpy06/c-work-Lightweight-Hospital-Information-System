/*
 * testdata.c
 * 这里是示例数据生成模块
 * 如果系统一开始没有完整数据文件，就自动补一套能直接演示的默认数据，
 * 这样老师验收或者组里联调都会方便很多
 */

#include "testdata.h"
#include "system.h"

#if defined(HIS_BUILD_FROM_MAIN) || !HIS_MAIN_ONE_CLICK_BUILD

static void add_demo_department(HospitalSystem *system, const char *name, const char *manager, const char *desc) {
    Department department;
    memset(&department, 0, sizeof(department));
    department.department_id = system->next_department_id++;
    safe_copy(department.name, name, NAME_LEN);
    safe_copy(department.manager, manager, NAME_LEN);
    safe_copy(department.description, desc, TEXT_LEN);
    department.is_deleted = 0;
    add_department(system, department);
}

static void build_demo_id_card(int index, int year, int month, int day, const char *area, char *buffer, int size) {
    static const int weights[17] = {7, 9, 10, 5, 8, 4, 2, 1, 6, 3, 7, 9, 10, 5, 8, 4, 2};
    static const char checks[11] = {'1', '0', 'X', '9', '8', '7', '6', '5', '4', '3', '2'};
    char first17[18];
    int sum = 0;
    int i = 0;
    snprintf(first17, sizeof(first17), "%s%04d%02d%02d%03d", area, year, month, day, (index % 998) + 1);
    for (i = 0; i < 17; i++) {
        sum += (first17[i] - '0') * weights[i];
    }
    snprintf(buffer, (size_t)size, "%s%c", first17, checks[sum % 11]);
}

static void add_demo_doctor(HospitalSystem *system, const char *name, const char *gender, const char *title, int department_id, const char *specialty, const char *phone, const char *id_card) {
    Doctor doctor;
    memset(&doctor, 0, sizeof(doctor));
    doctor.doctor_id = system->next_doctor_id++;
    safe_copy(doctor.name, name, NAME_LEN);
    safe_copy(doctor.gender, gender, SMALL_LEN);
    safe_copy(doctor.id_card, id_card, ID_LEN);
    doctor.age = age_from_id_card(doctor.id_card);
    safe_copy(doctor.title, title, NAME_LEN);
    doctor.department_id = department_id;
    safe_copy(doctor.specialty, specialty, TEXT_LEN);
    safe_copy(doctor.phone, phone, PHONE_LEN);
    generate_initial_password("D", doctor.doctor_id, doctor.password, PASSWORD_LEN);
    doctor.must_change_password = 1;
    doctor.is_deleted = 0;
    add_doctor(system, doctor);
}

static void add_demo_nurse(HospitalSystem *system, const char *name, const char *gender, const char *title, int department_id, const char *phone) {
    Nurse nurse;
    memset(&nurse, 0, sizeof(nurse));
    nurse.nurse_id = system->next_nurse_id++;
    safe_copy(nurse.name, name, NAME_LEN);
    safe_copy(nurse.gender, gender, SMALL_LEN);
    safe_copy(nurse.title, title, NAME_LEN);
    nurse.department_id = department_id;
    safe_copy(nurse.phone, phone, PHONE_LEN);
    generate_initial_password("N", nurse.nurse_id, nurse.password, PASSWORD_LEN);
    nurse.must_change_password = 1;
    nurse.is_deleted = 0;
    add_nurse(system, nurse);
}

static void add_demo_pharmacy_staff(HospitalSystem *system, const char *name, const char *gender, const char *phone) {
    PharmacyStaff staff;
    memset(&staff, 0, sizeof(staff));
    staff.staff_id = system->next_pharmacy_staff_id++;
    safe_copy(staff.name, name, NAME_LEN);
    safe_copy(staff.gender, gender, SMALL_LEN);
    safe_copy(staff.phone, phone, PHONE_LEN);
    generate_initial_password("S", staff.staff_id, staff.password, PASSWORD_LEN);
    staff.must_change_password = 1;
    staff.is_deleted = 0;
    add_pharmacy_staff(system, staff);
}

static void add_demo_patient(HospitalSystem *system, const char *name, const char *gender, const char *phone, const char *id_card, const char *blood) {
    Patient patient;
    memset(&patient, 0, sizeof(patient));
    patient.patient_id = system->next_patient_id++;
    safe_copy(patient.name, name, NAME_LEN);
    safe_copy(patient.gender, gender, SMALL_LEN);
    safe_copy(patient.phone, phone, PHONE_LEN);
    safe_copy(patient.id_card, id_card, ID_LEN);
    patient.age = age_from_id_card(patient.id_card);
    safe_copy(patient.blood_type, blood, SMALL_LEN);
    patient.is_inpatient = 0;
    patient.current_bed_id = 0;
    generate_initial_password("P", patient.patient_id, patient.password, PASSWORD_LEN);
    patient.must_change_password = 1;
    patient.is_deleted = 0;
    add_patient(system, patient);
}

static void add_demo_record(HospitalSystem *system, int patient_id, int doctor_id, int department_id, const char *type, const char *date_text, const char *diagnosis, double cost, const char *note) {
    MedicalRecord record;
    memset(&record, 0, sizeof(record));
    record.record_id = system->next_record_id++;
    record.patient_id = patient_id;
    record.doctor_id = doctor_id;
    record.department_id = department_id;
    safe_copy(record.record_type, type, TYPE_LEN);
    safe_copy(record.date, date_text, DATE_LEN);
    safe_copy(record.diagnosis, diagnosis, TEXT_LEN);
    record.cost = cost;
    safe_copy(record.note, note, TEXT_LEN);
    add_record(system, record);
}

static void add_demo_registration(HospitalSystem *system, int patient_id, int doctor_id, int department_id,
                                  int registration_type, const char *created_time, const char *visit_time,
                                  int status, const char *symptom) {
    OutpatientRegistration registration;
    MedicalRecord charge_record;
    char note[TEXT_LEN];
    memset(&registration, 0, sizeof(registration));
    registration.registration_id = system->next_registration_id++;
    registration.patient_id = patient_id;
    registration.doctor_id = doctor_id;
    registration.department_id = department_id;
    registration.registration_type = registration_type;
    safe_copy(registration.created_time, created_time, DATE_LEN);
    safe_copy(registration.visit_time, visit_time, DATE_LEN);
    registration.status = status;
    safe_copy(registration.symptom, symptom, TEXT_LEN);
    add_outpatient_registration(system, registration);

    memset(&charge_record, 0, sizeof(charge_record));
    charge_record.record_id = system->next_record_id++;
    charge_record.patient_id = patient_id;
    charge_record.doctor_id = doctor_id;
    charge_record.department_id = department_id;
    safe_copy(charge_record.record_type,
              registration_type == REG_TYPE_APPOINTMENT ? "预约挂号" : "现场挂号",
              TYPE_LEN);
    safe_copy(charge_record.date, created_time, DATE_LEN);
    safe_copy(charge_record.diagnosis, symptom, TEXT_LEN);
    charge_record.cost = status == REG_STATUS_CANCELED ? 0.0 : 20.0;
    snprintf(note, sizeof(note), "门诊挂号ID:%d，就诊时间:%s", registration.registration_id, registration.visit_time);
    if (status == REG_STATUS_CANCELED) {
        safe_copy(note + strlen(note), "（已取消，不计费）", (int)(sizeof(note) - strlen(note)));
    }
    safe_copy(charge_record.note, note, TEXT_LEN);
    add_record(system, charge_record);
}

static void append_admission(HospitalSystem *system, AdmissionHistory admission) {
    AdmissionHistory *node = (AdmissionHistory *)malloc(sizeof(AdmissionHistory));
    AdmissionHistory *tail = NULL;
    if (node == NULL) {
        return;
    }
    *node = admission;
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

static void append_usage(HospitalSystem *system, DrugUsageHistory usage) {
    DrugUsageHistory *node = (DrugUsageHistory *)malloc(sizeof(DrugUsageHistory));
    DrugUsageHistory *tail = NULL;
    if (node == NULL) {
        return;
    }
    *node = usage;
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

void generate_demo_data(HospitalSystem *system) {
    static const char *department_names[] = {"内科", "外科", "儿科", "骨科", "妇科", "急诊科"};
    static const char *department_desc[] = {
        "负责常见内科疾病诊疗", "负责普通外科与术后管理", "负责儿童门诊与住院管理",
        "负责骨折创伤与康复管理", "负责妇科门诊与病房", "负责急诊分诊与急救观察"
    };
    static const char *doctor_names[] = {
        "张医生", "李医生", "王医生", "赵医生", "刘医生", "孙医生",
        "周医生", "吴医生", "郑医生", "钱医生", "冯医生", "陈医生",
        "朱医生", "徐医生", "胡医生", "郭医生", "马医生", "高医生",
        "林医生", "何医生", "宋医生", "谢医生", "唐医生", "韩医生",
        "陶医生", "孟医生", "方医生", "邓医生", "许医生", "潘医生"
    };
    static const char *nurse_names[] = {
        "张护士", "李护士", "王护士", "赵护士", "刘护士", "孙护士",
        "周护士", "吴护士", "郑护士", "钱护士", "冯护士", "陈护士"
    };
    static const char *pharmacy_staff_names[] = {
        "张药师", "李药师", "王药师", "赵药师"
    };
    static const char *drug_generics[] = {
        "阿莫西林", "阿莫西林", "头孢克肟", "头孢克肟", "对乙酰氨基酚", "对乙酰氨基酚", "氯化钠注射液",
        "氯化钠注射液", "维生素C", "维生素C", "奥美拉唑", "奥美拉唑", "左氧氟沙星",
        "甲硝唑", "多潘立酮", "氨溴索", "氯雷他定", "云南白药", "双氯芬酸钠", "乳酸菌素片",
        "维生素B6", "肠溶阿司匹林", "甲钴胺", "硫酸镁", "地塞米松",
        "诺氟沙星", "赖氨酸", "葡萄糖酸钙", "硝酸甘油片", "板蓝根颗粒"
    };
    static const char *drug_brands[] = {
        "阿莫灵", "联邦阿莫仙", "世福素", "达力芬", "泰诺林", "百服宁", "科伦",
        "双鹤", "白云山", "东北制药", "洛赛克", "奥克", "来立信",
        "太极", "西安杨森", "勃林格", "拜耳", "云南白药", "诺华", "整肠生",
        "东北制药", "拜耳", "卫材", "扬子江", "仙琚",
        "白云山", "三精", "哈药", "信谊", "白云山"
    };
    static const char *drug_common_names[] = {
        "阿莫仙", "阿莫仙", "世福素", "世福素", "扑热息痛", "扑热息痛", "生理盐水",
        "生理盐水", "VC", "VC", "奥美", "奥美", "左氧",
        "灭滴灵", "吗丁啉", "沐舒坦", "开瑞坦", "白药", "扶他林", "整肠生",
        "VB6", "拜阿司匹灵", "弥可保", "泻盐", "氟美松",
        "氟哌酸", "赖氨酸B12", "葡钙", "救心丸", "板蓝根"
    };
    int i = 0;

    free_system(system);
    init_system(system);

    for (i = 0; i < 6; i++) {
        add_demo_department(system, department_names[i], doctor_names[i], department_desc[i]);
    }

    for (i = 0; i < 30; i++) {
        char phone[PHONE_LEN];
        char id_card[ID_LEN];
        sprintf(phone, "1380000%04d", i + 1);
        build_demo_id_card(i + 1, 1970 + (i % 25), (i % 12) + 1, (i % 28) + 1, "370100", id_card, ID_LEN);
        add_demo_doctor(system,
                        doctor_names[i],
                        (i % 2 == 0) ? "男" : "女",
                        (i % 3 == 0) ? "主任医师" : ((i % 3 == 1) ? "主治医师" : "住院医师"),
                        3001 + (i / 5),
                        (i % 2 == 0) ? "常见病诊治" : "慢病管理",
                        phone,
                        id_card);
    }

    for (i = 0; i < 12; i++) {
        char phone[PHONE_LEN];
        sprintf(phone, "1372000%04d", i + 1);
        add_demo_nurse(system,
                       nurse_names[i],
                       (i % 2 == 0) ? "女" : "男",
                       (i % 3 == 0) ? "主管护师" : ((i % 3 == 1) ? "护师" : "护士"),
                       3001 + (i / 2),
                       phone);
    }

    for (i = 0; i < 4; i++) {
        char phone[PHONE_LEN];
        sprintf(phone, "1369000%04d", i + 1);
        add_demo_pharmacy_staff(system,
                                pharmacy_staff_names[i],
                                (i % 2 == 0) ? "女" : "男",
                                phone);
    }

    for (i = 0; i < 130; i++) {
        char name[NAME_LEN];
        char phone[PHONE_LEN];
        char id_card[ID_LEN];
        const char *gender = (i % 2 == 0) ? "男" : "女";
        const char *blood = (i % 4 == 0) ? "A" : ((i % 4 == 1) ? "B" : ((i % 4 == 2) ? "O" : "AB"));
        if (i % 25 == 0) {
            safe_copy(name, "张伟", NAME_LEN);
        } else if (i % 33 == 0) {
            safe_copy(name, "王芳", NAME_LEN);
        } else {
            sprintf(name, "患者%03d", i + 1);
        }
        sprintf(phone, "1391000%04d", i + 1);
        build_demo_id_card(i + 1, 1948 + (i % 60), (i % 12) + 1, (i % 28) + 1, "370100", id_card, ID_LEN);
        add_demo_patient(system, name, gender, phone, id_card, blood);
    }

    for (i = 0; i < 30; i++) {
        Ward ward;
        memset(&ward, 0, sizeof(ward));
        ward.ward_id = system->next_ward_id++;
        sprintf(ward.name, "%s病房%d", department_names[i / 5], (i % 5) + 1);
        safe_copy(ward.ward_type, (i % 3 == 0) ? "普通" : ((i % 3 == 1) ? "重症" : "普通病房"), TYPE_LEN);
        safe_copy(ward.ward_type, (i % 2 == 0) ? "普通病房" : "重症", TYPE_LEN);
        ward.department_id = 3001 + (i / 5);
        ward.floor_no = 2 + (i / 6);
        ward.capacity = 5;
        ward.is_active = 1;
        add_ward(system, ward);
    }

    for (i = 0; i < 150; i++) {
        Bed bed;
        memset(&bed, 0, sizeof(bed));
        bed.bed_id = system->next_bed_id++;
        bed.ward_id = 4001 + (i / 5);
        bed.department_id = 3001 + ((i / 25) % 6);
        sprintf(bed.bed_no, "%d-%02d", bed.ward_id, (i % 5) + 1);
        bed.status = BED_STATUS_FREE;
        bed.patient_id = 0;
        add_bed(system, bed);
    }

    for (i = 0; i < 30; i++) {
        Drug drug;
        char production_date[DATE_LEN];
        char expiry_date[DATE_LEN];
        memset(&drug, 0, sizeof(drug));
        drug.drug_id = system->next_drug_id++;
        safe_copy(drug.generic_name, drug_generics[i], DRUG_NAME_LEN);
        safe_copy(drug.common_name, drug_common_names[i], DRUG_NAME_LEN);
        safe_copy(drug.brand_name, drug_brands[i], DRUG_NAME_LEN);
        if (!add_days_to_date("2026-01-01", i * 3, production_date, DATE_LEN)) {
            safe_copy(production_date, "2026-01-01", DATE_LEN);
        }
        if (!add_years_to_date(production_date, 2, expiry_date, DATE_LEN)) {
            safe_copy(expiry_date, "2028-01-01", DATE_LEN);
        }
        safe_copy(drug.production_date, production_date, DATE_LEN);
        safe_copy(drug.expiry_date, expiry_date, DATE_LEN);
        drug.purchase_price = 8.0 + (i % 7) * 2.5 + (i % 3) * 1.2;
        drug.sale_price = drug.purchase_price * DRUG_SALE_RATE;
        drug.stock = 80 + (i % 10) * 12;
        drug.total_used = 0;
        drug.is_deleted = 0;
        add_drug(system, drug);
    }

    for (i = 0; i < 240; i++) {
        int patient_id = 1001 + (i % 130);
        int doctor_id = 2001 + (i % 30);
        int department_id = 3001 + ((doctor_id - 2001) / 5);
        char date_text[DATE_LEN];
        char diagnosis[TEXT_LEN];
        char note[TEXT_LEN];
        const char *type = (i % 4 == 0) ? "挂号" : ((i % 4 == 1) ? "看诊" : ((i % 4 == 2) ? "检查" : "住院"));
        sprintf(date_text, "2026-03-%02d", (i % 28) + 1);
        sprintf(diagnosis, "%s诊断样例%d", department_names[(department_id - 3001) % 6], i + 1);
        sprintf(note, "演示病历备注%d", i + 1);
        add_demo_record(system, patient_id, doctor_id, department_id, type, date_text, diagnosis, 20.0 + (i % 15) * 8.0, note);
    }

    for (i = 0; i < 24; i++) {
        int doctor_id = 2001 + (i % 12);
        int department_id = 3001 + ((doctor_id - 2001) / 5);
        char created_time[DATE_LEN];
        char visit_time[DATE_LEN];
        char symptom[TEXT_LEN];
        int registration_type = (i % 3 == 0) ? REG_TYPE_WALKIN : REG_TYPE_APPOINTMENT;
        int status = (i % 8 == 0) ? REG_STATUS_CALLED :
                     ((i % 8 == 1) ? REG_STATUS_COMPLETED :
                     ((i % 8 == 2) ? REG_STATUS_CANCELED : REG_STATUS_WAITING));
        sprintf(created_time, "2026-04-%02d 08:%02d:00", (i % 20) + 1, (i * 3) % 60);
        if (registration_type == REG_TYPE_APPOINTMENT) {
            sprintf(visit_time, "2026-05-%02d %02d:%02d:00", (i % 20) + 2, 8 + (i % 8), (i * 5) % 60);
        } else {
            sprintf(visit_time, "2026-04-%02d 09:%02d:00", (i % 20) + 1, (i * 4) % 60);
        }
        sprintf(symptom, "演示门诊挂号诉求%d", i + 1);
        add_demo_registration(system, 1001 + (i % 40), doctor_id, department_id,
                              registration_type, created_time, visit_time, status, symptom);
    }

    for (i = 0; i < 60; i++) {
        AdmissionHistory admission;
        int department_index = i % 6;
        int doctor_id = 2001 + department_index * 5 + (i % 5);
        int ward_id = 4001 + department_index * 5 + (i % 5);
        int bed_id = 5001 + department_index * 25 + (i % 25);
        Bed *bed = find_bed_by_id(system, bed_id);
        Patient *patient = find_patient_by_id(system, 1001 + i);
        memset(&admission, 0, sizeof(admission));
        admission.admission_id = system->next_admission_id++;
        admission.patient_id = 1001 + i;
        admission.department_id = 3001 + department_index;
        admission.doctor_id = doctor_id;
        admission.ward_id = ward_id;
        admission.bed_id = bed_id;
        sprintf(admission.admit_date, "2026-03-%02d", (i % 20) + 1);
        if (i < 32) {
            safe_copy(admission.discharge_date, "-", DATE_LEN);
            admission.stay_days = 0;
            admission.status = ADMISSION_ACTIVE;
            if (bed != NULL) {
                bed->status = BED_STATUS_OCCUPIED;
                bed->patient_id = admission.patient_id;
            }
            if (patient != NULL) {
                patient->is_inpatient = 1;
                patient->current_bed_id = admission.bed_id;
            }
        } else {
            sprintf(admission.discharge_date, "2026-03-%02d", (i % 20) + 5);
            admission.stay_days = days_between(admission.admit_date, admission.discharge_date);
            if (admission.stay_days < 1) {
                admission.stay_days = 1;
            }
            admission.status = ADMISSION_DISCHARGED;
        }
        admission.deposit = 1500.0 + (i % 6) * 300.0;
        sprintf(admission.diagnosis, "%s住院样例%d", department_names[i % 6], i + 1);
        append_admission(system, admission);
    }

    for (i = 0; i < 120; i++) {
        DrugUsageHistory usage;
        Drug *drug = find_drug_by_id(system, 6001 + (i % 30));
        Doctor *doctor = find_doctor_by_id(system, 2001 + (i % 30));
        int quantity = 1 + (i % 5);
        memset(&usage, 0, sizeof(usage));
        usage.usage_id = system->next_usage_id++;
        usage.drug_id = 6001 + (i % 30);
        usage.patient_id = 1001 + (i % 130);
        usage.doctor_id = 2001 + (i % 30);
        usage.department_id = doctor != NULL ? doctor->department_id : 3001 + (i % 6);
        sprintf(usage.date, "2026-03-%02d", (i % 28) + 1);
        usage.quantity = quantity;
        usage.unit_price = drug != NULL ? drug->sale_price : 10.0;
        usage.purchase_price = drug != NULL ? drug->purchase_price : 0.0;
        usage.amount = usage.unit_price * quantity;
        sprintf(usage.note, "演示发药记录%d", i + 1);
        if (drug != NULL) {
            drug->stock -= quantity;
            if (drug->stock < 0) {
                drug->stock = 0;
            }
            drug->total_used += quantity;
        }
        append_usage(system, usage);
    }

    for (i = 0; i < 40; i++) {
        char detail[TEXT_LEN];
        sprintf(detail, "系统初始化日志%d", i + 1);
        append_log(system, (i % 2 == 0) ? "总管理员" : "药房工作人员", (i % 3 == 0) ? "导入数据" : "初始化", detail);
    }

    rebuild_next_ids(system);
}

#endif
