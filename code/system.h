#ifndef SYSTEM_H
#define SYSTEM_H

/*本头文件声明整个项目的核心业务接口。
 * 包含认证、主数据管理、业务处理、统计分析、预测优化和主菜单调度等主要函数。
 */

#include "common.h"

/* 清空当前登录会话，常用于退出登录或切换角色 */
void reset_current_session(HospitalSystem *system);

/* 获取当前登录医生对象；若当前不是医生身份则返回 NULL */
Doctor *get_current_doctor(HospitalSystem *system);

/* 获取当前登录护士对象；若当前不是护士身份则返回 NULL */
Nurse *get_current_nurse(HospitalSystem *system);

/* 获取当前登录药房工作人员对象；若当前不是药房工作人员身份则返回 NULL */
PharmacyStaff *get_current_pharmacy_staff(HospitalSystem *system);

/* 获取当前登录患者对象；若当前不是患者身份则返回 NULL。*/
Patient *get_current_patient(HospitalSystem *system);

/* 管理员登录认证 */
int authenticate_admin(HospitalSystem *system);

/* 获取管理员类型名称 */
const char *admin_type_to_text(int admin_type);

/* 获取管理员操作日志中的角色名称 */
const char *admin_log_role(HospitalSystem *system);

/* 医生登录认证 */
int authenticate_doctor(HospitalSystem *system);

/* 护士登录认证 */
int authenticate_nurse(HospitalSystem *system);

/* 药房工作人员登录认证 */
int authenticate_pharmacy_staff(HospitalSystem *system);

/* 患者登录认证 */
int authenticate_patient(HospitalSystem *system);

/* 判断当前登录医生是否有权限访问指定科室 */
int doctor_can_access_department(HospitalSystem *system, int department_id);

/* 判断当前登录医生是否有权限访问指定患者 */
int doctor_can_access_patient(HospitalSystem *system, int patient_id);

/* 判断当前登录医生是否有权限访问指定住院记录 */
int doctor_can_access_admission(HospitalSystem *system, AdmissionHistory *admission);

/* 判断当前登录护士是否有权限访问指定科室 */
int nurse_can_access_department(HospitalSystem *system, int department_id);

/* 判断当前登录护士是否有权限访问指定患者 */
int nurse_can_access_patient(HospitalSystem *system, int patient_id);

/* 判断当前登录护士是否有权限访问指定住院记录 */
int nurse_can_access_admission(HospitalSystem *system, AdmissionHistory *admission);

/* 患者身份下查看自己的住院信息 */
void show_current_patient_admission(HospitalSystem *system);

/* 交互式填写科室表单 */
int fill_department_form(Department *department);

/* 交互式填写医生表单 */
int fill_doctor_form(HospitalSystem *system, Doctor *doctor);

/* 交互式填写护士表单 */
int fill_nurse_form(HospitalSystem *system, Nurse *nurse);

/* 交互式填写患者表单 */
int fill_patient_form(Patient *patient);

/* 交互式填写药品表单 */
int fill_drug_form(Drug *drug);

/* 按编号查找科室*/
Department *find_department_by_id(HospitalSystem *system, int department_id);

/* 按名称查找科室。 */
Department *find_department_by_name(HospitalSystem *system, const char *name);

/* 统计当前有效科室数量。 */
int count_departments(HospitalSystem *system);

/* 向科室链表中新增一个科室。 */
void add_department(HospitalSystem *system, Department department);

/* 交互式新增科室。 */
void add_department_interactive(HospitalSystem *system);

/* 交互式修改科室。 */
void modify_department_interactive(HospitalSystem *system);

/* 交互式删除科室。 */
void delete_department_interactive(HospitalSystem *system);

/* 交互式查询科室。 */
void query_department_interactive(HospitalSystem *system);

/* 列出所有有效科室。 */
void list_departments(HospitalSystem *system);

/* 当前登录人员修改自己的登录密码。 */
void change_current_password_interactive(HospitalSystem *system);

/* 按编号查找医生。 */
Doctor *find_doctor_by_id(HospitalSystem *system, int doctor_id);

/* 统计指定科室下的医生人数。 */
int count_doctors_in_department(HospitalSystem *system, int department_id);

/* 向医生链表中新增一个医生。 */
void add_doctor(HospitalSystem *system, Doctor doctor);

/* 交互式新增医生。 */
void add_doctor_interactive(HospitalSystem *system);

/* 交互式修改医生。 */
void modify_doctor_interactive(HospitalSystem *system);

/* 交互式删除医生。 */
void delete_doctor_interactive(HospitalSystem *system);

/* 交互式查询医生。 */
void query_doctor_interactive(HospitalSystem *system);

/* 列出所有有效医生。 */
void list_doctors(HospitalSystem *system);

/* 按科室列出医生。 */
void list_doctors_by_department(HospitalSystem *system, int department_id);

/* 按编号查找患者。 */
Patient *find_patient_by_id(HospitalSystem *system, int patient_id);

/* 按编号查找护士。 */
Nurse *find_nurse_by_id(HospitalSystem *system, int nurse_id);

/* 统计指定科室下的护士人数。 */
int count_nurses_in_department(HospitalSystem *system, int department_id);

/* 向护士链表中新增一个护士。 */
void add_nurse(HospitalSystem *system, Nurse nurse);

/* 交互式新增护士。 */
void add_nurse_interactive(HospitalSystem *system);

/* 交互式修改护士。 */
void modify_nurse_interactive(HospitalSystem *system);

/* 交互式删除护士。 */
void delete_nurse_interactive(HospitalSystem *system);

/* 交互式查询护士。 */
void query_nurse_interactive(HospitalSystem *system);

/* 列出所有有效护士。 */
void list_nurses(HospitalSystem *system);

/* 按科室列出护士。 */
void list_nurses_by_department(HospitalSystem *system, int department_id);

/* 列出当前护士所在科室的在院患者。 */
void list_current_nurse_inpatients(HospitalSystem *system);

/* 列出当前护士所在科室的住院记录。 */
void list_current_nurse_admissions(HospitalSystem *system);

/* 按编号查找有效药房工作人员。 */
PharmacyStaff *find_pharmacy_staff_by_id(HospitalSystem *system, int staff_id);

/* 向药房工作人员链表中新增人员。 */
void add_pharmacy_staff(HospitalSystem *system, PharmacyStaff staff);

/* 交互式新增药房工作人员。 */
void add_pharmacy_staff_interactive(HospitalSystem *system);

/* 交互式修改药房工作人员。 */
void modify_pharmacy_staff_interactive(HospitalSystem *system);

/* 交互式停用药房工作人员。 */
void deactivate_pharmacy_staff_interactive(HospitalSystem *system);

/* 交互式查询药房工作人员。 */
void query_pharmacy_staff_interactive(HospitalSystem *system);

/* 列出全部有效药房工作人员。 */
void list_pharmacy_staffs(HospitalSystem *system);

/* 向患者链表中新增一个患者。 */
void add_patient(HospitalSystem *system, Patient patient);

/* 交互式新增患者。 */
void add_patient_interactive(HospitalSystem *system);

/* 患者自助建档入口，成功时返回新患者编号。 */
int register_patient_self_interactive(HospitalSystem *system);

/* 交互式修改患者。 */
void modify_patient_interactive(HospitalSystem *system);

/* 交互式删除患者。 */
void delete_patient_interactive(HospitalSystem *system);

/* 交互式查询患者。 */
void query_patient_interactive(HospitalSystem *system);

/* 列出所有有效患者。 */
void list_patients(HospitalSystem *system);

/* 列出当前住院中的患者。 */
void list_inpatients(HospitalSystem *system);

/* 按编号查找病房。 */
Ward *find_ward_by_id(HospitalSystem *system, int ward_id);

/* 按编号查找床位。 */
Bed *find_bed_by_id(HospitalSystem *system, int bed_id);

/* 按编号查找住院记录。 */
AdmissionHistory *find_admission_by_id(HospitalSystem *system, int admission_id);

/* 查找某位患者当前仍在进行中的住院记录。 */
AdmissionHistory *find_active_admission_by_patient(HospitalSystem *system, int patient_id);

/* 向病房链表中新增病房。 */
void add_ward(HospitalSystem *system, Ward ward);

/* 向床位链表中新增床位。 */
void add_bed(HospitalSystem *system, Bed bed);

/* 交互式新增病房。 */
void add_ward_interactive(HospitalSystem *system);

/* 交互式新增床位。 */
void add_bed_interactive(HospitalSystem *system);

/* 交互式修改床位状态。 */
void modify_bed_status_interactive(HospitalSystem *system);

/* 交互式停用病房，停用前要求病房内没有占用床位。 */
void deactivate_ward_interactive(HospitalSystem *system);

/* 列出所有病房。 */
void list_wards(HospitalSystem *system);

/* 列出所有床位。 */
void list_beds(HospitalSystem *system);

/* 按科室列出病房。 */
void list_wards_by_department(HospitalSystem *system, int department_id);

/* 按科室列出床位。 */
void list_beds_by_department(HospitalSystem *system, int department_id);

/* 移动床位到新病房并自动重算床位号。 */
void move_bed_to_ward_interactive(HospitalSystem *system);

/* 按病房列出床位。 */
void list_beds_by_ward_interactive(HospitalSystem *system);

/* 按科室列出当前可分配床位。 */
void list_available_beds_by_department(HospitalSystem *system, int department_id);

/* 为患者分配床位。 */
int assign_bed(HospitalSystem *system, int patient_id, int bed_id);

/* 释放床位。 */
int release_bed(HospitalSystem *system, int bed_id);

/* 统计当前住院人数。 */
int current_inpatient_count(HospitalSystem *system);

/* 按编号查找药品。 */
Drug *find_drug_by_id(HospitalSystem *system, int drug_id);

/* 向药品链表中新增药品。 */
void add_drug(HospitalSystem *system, Drug drug);

/* 交互式新增药品。 */
void add_drug_interactive(HospitalSystem *system);

/* 交互式修改药品。 */
void modify_drug_interactive(HospitalSystem *system);

/* 交互式删除药品。 */
void delete_drug_interactive(HospitalSystem *system);

/* 交互式查询药品。 */
void query_drug_interactive(HospitalSystem *system);

/* 列出所有有效药品。 */
void list_drugs(HospitalSystem *system);

/* 药品入库。 */
void stock_in_interactive(HospitalSystem *system);

/* 药品出库。 */
void stock_out_interactive(HospitalSystem *system);

/* 执行发药逻辑并生成发药记录。发药科室由医生所属科室决定。 */
int dispense_drug(HospitalSystem *system, int drug_id, int patient_id, int doctor_id, int quantity, const char *date_text, const char *note);

/* 交互式发药入口。 */
void dispense_drug_interactive(HospitalSystem *system);

/* 医生开具处方。 */
void prescribe_drug_interactive(HospitalSystem *system);

/* 药房按处方一键发药。 */
void dispense_prescription_interactive(HospitalSystem *system);

/* 按当前角色权限查看处方。 */
void list_prescriptions(HospitalSystem *system);

/* 按当前角色权限查看发药记录。 */
void list_drug_usage_records(HospitalSystem *system);

/* 维护药品的可用科室和不可用科室规则。 */
void manage_drug_department_access_interactive(HospitalSystem *system);

/* 按编号查找医疗记录。 */
MedicalRecord *find_record_by_id(HospitalSystem *system, int record_id);

/* 向病历链表中新增一条记录。 */
void add_record(HospitalSystem *system, MedicalRecord record);

/* 按编号查找门诊挂号记录。 */
OutpatientRegistration *find_registration_by_id(HospitalSystem *system, int registration_id);

/* 向门诊挂号链表中新增一条记录。 */
void add_outpatient_registration(HospitalSystem *system, OutpatientRegistration registration);

/* 医生端交互式新增病历。 */
void add_record_interactive(HospitalSystem *system);

/* 患者端自助挂号，支持预约挂号和现场挂号。 */
void patient_registration_interactive(HospitalSystem *system);

/* 查看门诊挂号与候诊队列。 */
void list_outpatient_registrations(HospitalSystem *system);

/* 查看当前医生的候诊队列。 */
void list_doctor_waiting_queue(HospitalSystem *system);

/* 医生旧版叫号下一位患者。 */
void doctor_call_next_registration(HospitalSystem *system);

/* 医生旧版完成当前已叫号患者接诊。 */
void doctor_finish_current_registration(HospitalSystem *system);

/* 医生叫号并完成接诊，支持选择候诊编号跳号。 */
void doctor_call_and_finish_registration(HospitalSystem *system);

/* 患者查看自己的挂号记录。 */
void show_current_patient_registrations(HospitalSystem *system);

/* 取消待诊挂号。 */
void cancel_registration_interactive(HospitalSystem *system);

/* 挂号收费异常对账。 */
void show_registration_reconciliation_report(HospitalSystem *system);

/* 列出全部病历。 */
void list_records(HospitalSystem *system);

/* 按患者列出病历。 */
void list_records_by_patient(HospitalSystem *system, int patient_id);

/* 交互式查看患者完整病史。 */
void patient_history_interactive(HospitalSystem *system);

/* 办理住院。 */
void handle_admission_interactive(HospitalSystem *system);

/* 办理出院。 */
void handle_discharge_interactive(HospitalSystem *system);

/* 列出全部住院记录。 */
void list_admissions(HospitalSystem *system);

/* 输出患者查询报表。 */
void show_patient_query_report(HospitalSystem *system);

/* 输出医生查询报表。 */
void show_doctor_query_report(HospitalSystem *system);

/* 输出科室查询报表。 */
void show_department_query_report(HospitalSystem *system);

/* 输出药品查询报表。 */
void show_drug_query_report(HospitalSystem *system);

/* 输出管理视角的综合统计信息。 */
void show_management_statistics(HospitalSystem *system);

/* 输出医疗业务视角的统计信息。 */
void show_medical_statistics(HospitalSystem *system);

/* 输出患者相关统计信息。 */
void show_patient_statistics(HospitalSystem *system);

/* 将统计分析结果导出为文本报告。 */
void export_analysis_report(HospitalSystem *system, const char *file_name);

/* 交互式分段导出分析报告。 */
void export_analysis_report_interactive(HospitalSystem *system);

/* 检查并修复患者、床位、住院、药品用量等链表之间的数据一致性。 */

/* 预测指定科室未来的住院人数。 */
double forecast_department_admission(HospitalSystem *system, int department_id, int window_days);

/* 预测指定科室未来需要的床位数量。 */
double forecast_department_bed_need(HospitalSystem *system, int department_id, int window_days);

/* 预测指定药品未来需求量。 */
double forecast_drug_demand(HospitalSystem *system, int drug_id, int window_days);

/* 显示科室预测结果。 */
void show_department_forecast(HospitalSystem *system);

/* 显示药品预测结果。 */
void show_drug_forecast(HospitalSystem *system);

/* 显示科室历史趋势分析。 */
void show_department_history_analysis(HospitalSystem *system, int window_days);

/* 显示病房资源优化建议。 */
void show_ward_optimization_suggestions(HospitalSystem *system, int window_days);

/* 显示药品趋势分析。 */
void show_drug_trend_analysis(HospitalSystem *system, int window_days);

/* 启动整个系统主循环。 */
void run_system(HospitalSystem *system);

#endif
