#ifndef COMMON_H
#define COMMON_H

/* 本头文件定义整个医院信息系统的公共数据结构、通用常量和基础工具函数
 * 1. 统一结构体定义，保证患者、医生、科室、病房、药品等对象格式一致
 * 2. 提供安全输入、字符串处理、日期处理等通用能力
 * 3. 提供日志、表格显示等基础支撑，便于其他模块直接复用
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <locale.h>//设置区域为中文
#include <limits.h>//定义整数范围

#ifdef _WIN32
#include <windows.h>
#ifndef PATH_MAX
#define PATH_MAX MAX_PATH
#endif
#endif

/* 启用 main.c 一键运行模式：直接运行 main.c 时由它统一包含其余实现文件 */
#ifndef HIS_MAIN_ONE_CLICK_BUILD
#define HIS_MAIN_ONE_CLICK_BUILD 1
#endif

/* 各类文本字段长度上限 */
#define NAME_LEN 64//姓名长度上限
#define DRUG_NAME_LEN 96//药品名称长度上限
#define DRUG_DISPLAY_NAME_LEN (DRUG_NAME_LEN * 3 + 16)//药品完整显示名长度
#define DRUG_NAME_COL_WIDTH 44//药品名称表格列宽
#define PHONE_LEN 32//手机号长度上限
#define ID_LEN 32//身份证号长度上限
#define DATE_LEN 20//日期长度上限
#define TYPE_LEN 32 //类型长度上限
#define TEXT_LEN 256//文本长度上限
#define SMALL_LEN 16//小文本长度上限
#define PASSWORD_LEN 32//登录密码长度上限

/* 药品售价倍率：售价 = 进价 * 1.3*/
#define DRUG_SALE_RATE 1.3
#define REGISTRATION_FEE 20.0
#define DRUG_ACCESS_ALLOW_ALL 0
#define DRUG_ACCESS_DENY_ALL 1
#define DRUG_RULE_DENY 0
#define DRUG_RULE_ALLOW 1

/* 床位状态常量，便于病房模块和住院模块统一判断*/
#define BED_STATUS_FREE 0//空闲
#define BED_STATUS_OCCUPIED 1//已占用
#define BED_STATUS_REPAIR 2//维修中

/* 住院记录状态常量 */
#define ADMISSION_ACTIVE 1//住院中
#define ADMISSION_DISCHARGED 0//已出院

/* 门诊挂号类型常量 */
#define REG_TYPE_APPOINTMENT 1//预约挂号
#define REG_TYPE_WALKIN 2//现场挂号

/* 门诊挂号状态常量 */
#define REG_STATUS_WAITING 0//等待挂号
#define REG_STATUS_CALLED 1//已挂号
#define REG_STATUS_COMPLETED 2//已完成挂号
#define REG_STATUS_CANCELED 3//已取消挂号

/* 处方状态常量 */
#define PRESCRIPTION_PENDING 0//待发药
#define PRESCRIPTION_DISPENSED 1//已发药
#define PRESCRIPTION_CANCELED 2//已取消

/* 当前登录角色常量 */
#define ROLE_NONE 0//无角色
#define ROLE_ADMIN 1//管理员角色
#define ROLE_DOCTOR 2//医生角色
#define ROLE_PHARMACY 3//药房工作人员角色
#define ROLE_PATIENT 4//患者角色
#define ROLE_NURSE 5//护士角色

/* 管理员细分类型 */
#define ADMIN_NONE 0//无管理员身份
#define ADMIN_SUPER 1//总管理员
#define ADMIN_STAFF_DATA 2//医护资料管理员
#define ADMIN_RESOURCE 3//科室资源管理员
#define ADMIN_MEDICAL_BUSINESS 4//医疗业务管理员

/* 患者结点：用于患者主数据链表 */
typedef struct Patient {
    int patient_id;//患者ID
    char name[NAME_LEN];//姓名
    char gender[SMALL_LEN];//性别
    int age;//年龄
    char phone[PHONE_LEN];//手机号
    char id_card[ID_LEN];//身份证号
    char blood_type[SMALL_LEN];//血型
    char password[PASSWORD_LEN];//登录密码
    int must_change_password;//是否需要修改初始密码
    int is_inpatient;//是否住院
    int current_bed_id;//当前床位ID
    int is_deleted;//是否删除
    struct Patient *next;
} Patient;

/* 医生结点：用于医生主数据链表 */
typedef struct Doctor {
    int doctor_id;//医生ID
    char name[NAME_LEN];//姓名
    char gender[SMALL_LEN];//性别
    int age;//年龄
    char id_card[ID_LEN];//身份证号
    char title[NAME_LEN];//职称
    int department_id;//科室ID
    char specialty[TEXT_LEN];//专业
    char phone[PHONE_LEN];//手机号
    char password[PASSWORD_LEN];//登录密码
    int must_change_password;//是否需要修改初始密码
    int is_deleted;//是否删除
    struct Doctor *next;
} Doctor;

/* 护士结点：用于护士主数据链表 */
typedef struct Nurse {
    int nurse_id;//护士ID
    char name[NAME_LEN];//姓名
    char gender[SMALL_LEN];//性别
    char title[NAME_LEN];//职称
    int department_id;//科室ID
    char phone[PHONE_LEN];//手机号
    char password[PASSWORD_LEN];//登录密码
    int must_change_password;//是否需要修改初始密码
    int is_deleted;//是否删除
    struct Nurse *next;
} Nurse;

/* 药房工作人员结点：用于药师/药房人员主数据链表 */
typedef struct PharmacyStaff {
    int staff_id;//药房工作人员编号
    char name[NAME_LEN];//姓名
    char gender[SMALL_LEN];//性别
    char phone[PHONE_LEN];//联系电话
    char password[PASSWORD_LEN];//登录密码
    int must_change_password;//是否需要修改初始密码
    int is_deleted;//是否停用
    struct PharmacyStaff *next;
} PharmacyStaff;

/* 科室结点：用于科室主数据链表 */
typedef struct Department {
    int department_id;//科室ID
    char name[NAME_LEN];//科室名称
    char manager[NAME_LEN];//科室主任
    char description[TEXT_LEN];//科室描述
    int is_deleted;//是否删除
    struct Department *next;
} Department;

/* 病房结点：描述病房基本信息 */
typedef struct Ward {
    int ward_id;//病房ID
    char name[NAME_LEN];//病房名称
    char ward_type[TYPE_LEN];//病房类型
    int department_id;//科室ID
    int floor_no;//楼层
    int capacity;//床位容量
    int is_active;//是否激活
    struct Ward *next;
} Ward;

/* 床位结点：描述具体床位及占用情况 */
typedef struct Bed {
    int bed_id;//床位ID
    int ward_id;//病房ID
    int department_id;//科室ID
    char bed_no[NAME_LEN];//床位号
    int status;//床位状态
    int patient_id;//患者ID
    struct Bed *next;
} Bed;

/* 药品结点：保存全院共享药品的库存与价格信息 */
typedef struct Drug {
    int drug_id;//药品ID
    char generic_name[DRUG_NAME_LEN];//通用名称
    char common_name[DRUG_NAME_LEN];//俗称
    char brand_name[DRUG_NAME_LEN];//品牌名称
    char production_date[DATE_LEN];//生产日期
    char expiry_date[DATE_LEN];//有效期至
    double purchase_price;//进价
    double sale_price;//售价
    int stock;//库存
    int total_used;//已用量
    int access_mode;//药品可用策略
    int is_deleted;//是否删除
    struct Drug *next;
} Drug;

typedef struct DrugDepartmentRule {
    int drug_id;
    int department_id;
    int rule_type;
    struct DrugDepartmentRule *next;
} DrugDepartmentRule;

/* 医疗记录结点：记录挂号、看诊、检查、住院等业务信息*/
typedef struct MedicalRecord {
    int record_id;//记录ID
    int patient_id;//患者ID
    int doctor_id;//医生ID
    int department_id;//科室ID
    char record_type[TYPE_LEN];//记录类型
    char date[DATE_LEN];//日期
    char diagnosis[TEXT_LEN];//诊断
    double cost;//费用
    char note[TEXT_LEN];//备注
    struct MedicalRecord *next;
} MedicalRecord;

/* 门诊挂号记录结点：独立管理预约/现场挂号、候诊状态和医生叫号队列 */
typedef struct OutpatientRegistration {
    int registration_id;//挂号ID
    int patient_id;//患者ID
    int doctor_id;//医生ID
    int department_id;//科室ID
    int registration_type;//挂号类型
    char created_time[DATE_LEN];//创建时间
    char visit_time[DATE_LEN];//就诊时间
    int status;//挂号状态
    char symptom[TEXT_LEN];//症状
    struct OutpatientRegistration *next;
} OutpatientRegistration;

/* 住院历史结点：用于追踪患者住院全过程 */
typedef struct AdmissionHistory {
    int admission_id;//住院ID
    int patient_id;//患者ID
    int department_id;//科室ID
    int doctor_id;//医生ID
    int ward_id;//病房ID
    int bed_id;//床位ID
    char admit_date[DATE_LEN];//入院日期
    char discharge_date[DATE_LEN];//出院日期
    int stay_days;//住院天数
    double deposit;//押金
    char diagnosis[TEXT_LEN];//诊断
    int status;//住院状态
    struct AdmissionHistory *next;
} AdmissionHistory;

/* 发药历史结点：记录药品发放明细，用于统计和预测
 * 这里的 department_id 表示“本次发药发生在哪个科室”
 */
typedef struct DrugUsageHistory {
    int usage_id;//发药ID
    int drug_id;//药品ID
    int patient_id;//患者ID
    int doctor_id;//医生ID
    int department_id;//科室ID
    char date[DATE_LEN];//日期
    int quantity;//数量
    double unit_price;//发药时单价
    double purchase_price;//发药时进价
    double amount;//金额
    char note[TEXT_LEN];//备注
    struct DrugUsageHistory *next;
} DrugUsageHistory;

/* 处方主表：医生开具，药房按处方发药 */
typedef struct Prescription {
    int prescription_id;//处方ID
    int patient_id;//患者ID
    int doctor_id;//医生ID
    int department_id;//科室ID
    char created_time[DATE_LEN];//开具时间
    int status;//处方状态
    char note[TEXT_LEN];//备注
    struct Prescription *next;
} Prescription;

/* 处方明细：一张处方可包含多种药品 */
typedef struct PrescriptionItem {
    int prescription_id;//处方ID
    int drug_id;//药品ID
    int quantity;//数量
    char note[TEXT_LEN];//用药说明
    struct PrescriptionItem *next;
} PrescriptionItem;

/* 操作日志结点，记录系统关键操作 */
typedef struct OperationLog {
    int log_id;//日志ID
    char date[DATE_LEN];//日期
    char role[NAME_LEN];//角色
    char action[NAME_LEN];//操作
    char detail[TEXT_LEN];//详情
    struct OperationLog *next;
} OperationLog;

/* 系统总控结构体：
 * 汇总所有链表头、编号生成器和当前会话状态
 */
typedef struct HospitalSystem {
    Patient *patients;//患者链表头
    Doctor *doctors;//医生链表头
    Nurse *nurses;//护士链表头
    PharmacyStaff *pharmacy_staffs;//药房工作人员链表头
    Department *departments;//科室链表头
    Ward *wards;//病房链表头
    Bed *beds;//床位链表头
    Drug *drugs;//药品链表头
    MedicalRecord *records;//医疗记录链表头
    OutpatientRegistration *registrations;//门诊挂号记录链表头
    AdmissionHistory *admissions;//住院历史链表头
    DrugUsageHistory *drug_usages;//发药历史链表头
    Prescription *prescriptions;//处方链表头
    PrescriptionItem *prescription_items;//处方明细链表头
    DrugDepartmentRule *drug_department_rules;//药品-科室可用性规则
    OperationLog *logs;//操作日志链表头
    int next_patient_id;//患者ID
    int next_doctor_id;//医生ID
    int next_nurse_id;//护士ID
    int next_pharmacy_staff_id;//药房工作人员ID
    int next_department_id;//科室ID 
    int next_ward_id;//病房ID
    int next_bed_id;//床位ID
    int next_drug_id;//药品ID
    int next_record_id;//医疗记录ID
    int next_registration_id;//挂号ID
    int next_admission_id;//住院ID
    int next_usage_id;//发药ID
    int next_prescription_id;//处方ID
    int next_log_id;//日志ID
    int current_role;//当前角色
    int current_admin_type;//当前管理员类型
    int current_doctor_id;//当前医生ID
    int current_nurse_id;//当前护士ID
    int current_pharmacy_staff_id;//当前药房工作人员ID
    int current_patient_id;//当前患者ID
} HospitalSystem;

/* 初始化系统结构体，设置链表头为空并给各类编号赋初值 */
void init_system(HospitalSystem *system);

/* 释放系统所有链表结点 */
void free_system(HospitalSystem *system);

/* 根据当前链表中的最大编号重新计算 next_id，保证读档后仍能继续自动编号 */
void rebuild_next_ids(HospitalSystem *system);

/* 初始化控制台编码，保证中文在不同平台下显示正常 */
void init_console_encoding(void);

/* 去掉字符串末尾的换行符，常用于处理 fgets 的输入结果 */
void trim_newline(char *text);

/* 去掉 UTF-8 BOM 头，避免文本文件首行被解析出异常字符 */
void strip_utf8_bom(char *text);

/* 安全读取非空字符串输入 */
void read_string(const char *prompt, char *buffer, int size);

/* 安全读取整数输入 */
int read_int(const char *prompt);

/* 安全读取指定范围内的整数输入 */
int read_int_in_range(const char *prompt, int min_value, int max_value);

/* 安全读取浮点数输入 */
double read_double(const char *prompt);

/* 读取 y/n 型确认输入，用于二次确认 */
int read_yes_no(const char *prompt);

/* 读取一个步骤式字符串输入；allow_back 为真时允许用 b 返回上一步 */
int read_string_step(const char *prompt, char *buffer, int size, int allow_back);

/* 读取一个步骤式整数输入，并限制输入范围 */
int read_int_step(const char *prompt, int *value, int min_value, int max_value, int allow_back);

/* 读取一个步骤式浮点数输入，并限制最小值 */
int read_double_step(const char *prompt, double *value, double min_value, int allow_back);

/* 读取并校验手机号 */
int read_phone_step(const char *prompt, char *buffer, int size, int allow_back);

/* 读取并校验身份证号 */
int read_id_card_step(const char *prompt, char *buffer, int size, int allow_back);

/* 判断字符串是否为空串或仅由空白字符组成 */
int is_blank_string(const char *text);

/* 校验手机号是否合法 */
int is_valid_phone(const char *text);

/* 校验身份证号格式是否合法 */
int is_valid_id_card(const char *text);

/* 根据身份证号计算年龄，身份证不合法时返回 -1 */
int age_from_id_card(const char *id_card);

/* 生成系统初始密码 */
void generate_initial_password(const char *prefix, int id, char *buffer, int size);

/* 不区分大小写地判断 text 中是否包含 keyword */
int str_contains_ignore_case(const char *text, const char *keyword);

/* 将字符串转换为小写形式，常用于模糊匹配*/
void to_lower_string(const char *src, char *dst, int size);

/* 安全复制字符串，避免 strcpy 导致越界 */
void safe_copy(char *dst, const char *src, int size);

/* 生成药品显示名，统一显示通用名、俗称和品牌 */
void build_drug_display_name(const Drug *drug, char *buffer, int size);

/* 规范化性别输入，仅接受“男”或“女” */
int normalize_gender(const char *input, char *output, int size);

/* 规范化血型输入，仅接受 A/B/O/AB */
int normalize_blood_type(const char *input, char *output, int size);

/* 获取当前日期，格式一般为 YYYY-MM-DD */
void get_current_date(char *buffer);

/* 获取当前完整日期时间，格式一般为 YYYY-MM-DD HH:MM:SS */
void get_current_datetime(char *buffer);

/* 校验日期字符串是否为合法 YYYY-MM-DD */
int is_valid_date_text(const char *date_text);

/* 在日期上增加指定天数，成功返回 1 */
int add_days_to_date(const char *date_text, int days, char *buffer, int size);

/* 在日期上增加指定年数，成功返回 1 */
int add_years_to_date(const char *date_text, int years, char *buffer, int size);

/* 把日期字符串转换为可比较的序列值，用于日期差值运算 */
int date_to_serial(const char *date_text);

/* 计算两个日期之间相差的天数 */
int days_between(const char *date_a, const char *date_b);

/* 打印指定字符组成的分隔线 */
void print_line(char ch, int count);

/* 打印统一格式的标题 */
void print_title(const char *title);

/* 停顿等待用户按回车，便于控制台查看输出 */
void pause_screen(void);

/* 判断 UTF-8 字符占用的字节数，主要服务于表格对齐 */
int utf8_char_size(unsigned char ch);

/* 按显示宽度格式化单元格文本，解决中英文混排时表格错位问题 */
void format_display_cell(const char *src, char *dst, int dst_size, int max_width);

/* 打印表格边框 */
void print_table_border(int total_width, char ch);

/* 把床位状态编号转换成中文文本 */
const char *bed_status_to_text(int status);

/* 统计某个科室配置的床位总数 */
int count_department_beds(HospitalSystem *system, int department_id);

/* 统计某个科室当前已占用床位数 */
int count_department_occupied_beds(HospitalSystem *system, int department_id);

/* 统计某个科室在指定日期序列范围内的入院人数 */
int count_department_admissions_in_range(HospitalSystem *system, int department_id, int start_serial, int end_serial);

/* 统计某个科室近 N 天入院人数 */
int count_department_admissions_in_recent_days(HospitalSystem *system, int department_id, int window_days);

/* 统计某个科室近 N 天出院人数 */
int count_department_discharges_in_recent_days(HospitalSystem *system, int department_id, int window_days);

/* 计算某个科室已出院患者的平均住院天数，无历史数据时返回默认 3 天 */
double average_department_stay_days(HospitalSystem *system, int department_id);

/* 追加一条操作日志到日志链表 */
int append_log(HospitalSystem *system, const char *role, const char *action, const char *detail);

#endif
