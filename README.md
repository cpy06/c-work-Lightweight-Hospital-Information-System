# 轻量级医院 HIS 管理系统

这是一个使用 C 语言实现的控制台版医院信息管理系统，围绕患者、医生、护士、药房工作人员、科室、病房、床位、药品、处方、挂号、住院和统计报表等对象完成基础 HIS 业务流程。项目采用多源文件模块化组织，并通过 txt 文件进行本地数据持久化，适合作为程序设计基础课程设计或 C 语言综合实践项目。

## 功能特性

- 多角色入口：管理员、医生、护士、药房工作人员、患者。
- 权限分级管理：总管理员、医护资料管理员、科室资源管理员、医疗业务管理员。
- 人员登录密码：医生、护士、药房工作人员和患者均使用“编号 + 密码”登录；新增人员自动生成初始密码，首次登录后可修改。
- 身份证校验：患者和医生录入身份证号时校验 18 位身份证出生日期与校验码，并自动计算年龄。
- 医生职称限制：医生职称只能从系统给定列表中选择，不能自由输入。
- 挂号与接诊：挂号费固定为 `20.00` 元；医生端合并叫号和接诊，支持按挂号编号跳号接诊。
- 处方与发药：医生开具处方，药房工作人员查询处方后可按处方一键发药并扣减库存。
- 护士病房管理：护士可查看本科室住院记录、病房、床位和可用床位，并维护床位状态。
- 床位自动编号：新增床位时按病房自动生成床位号；床位移动到新病房后自动重算床位号。
- 模糊查询：患者、医生、护士、药房工作人员、科室、药品等查询入口支持编号、姓名/名称、电话、身份证号等多字段模糊匹配。
- 统计报表：支持整体分析报告导出，也支持科室分析、药品分析和利润统计分段导出。
- 利润统计：可统计药品总利润，并计算包含挂号费等收入在内的医院总体利润。

## 目录结构

```text
.
├── README.md               # 项目说明
├── TEST_DATA.md            # 测试数据汇总文档
├── code/                   # 源码与 Windows 可执行文件
│   ├── main.c              # 程序入口，默认一键包含各业务模块
│   ├── common.h / common.c # 公共结构、常量、工具函数、输入校验
│   ├── system.h            # 系统结构与业务函数声明
│   ├── fileio.h / fileio.c # txt 数据文件读写
│   ├── auth.c              # 登录认证、角色权限、密码修改
│   ├── menu.c              # 菜单调度
│   ├── patient.c           # 患者资料管理
│   ├── doctor.c            # 医生资料管理
│   ├── nurse.c             # 护士资料与护士端功能
│   ├── pharmacy_staff.c    # 药房工作人员资料管理
│   ├── department.c        # 科室管理
│   ├── ward.c              # 病房、床位、住院资源管理
│   ├── drug.c              # 药品、处方、发药管理
│   ├── record.c            # 挂号、接诊、医疗记录、住院出院
│   ├── analysis.c          # 查询统计与报表导出
│   ├── forecast.c          # 趋势预测
│   ├── optimize.c          # 优化建议
│   ├── testdata.c/.h       # 默认演示数据
│   └── his.exe             # Windows 64 位控制台程序
└── test_data/              # txt 测试数据与导出报表
    ├── patients.txt
    ├── doctors.txt
    ├── nurses.txt
    ├── pharmacy_staff.txt
    ├── departments.txt
    ├── wards.txt
    ├── beds.txt
    ├── drugs.txt
    ├── records.txt
    ├── registrations.txt
    ├── admissions.txt
    ├── prescriptions.txt
    ├── prescription_items.txt
    ├── drug_usage.txt
    ├── log.txt
    └── *_report.txt
```

## 编译运行

本项目默认开启 `HIS_MAIN_ONE_CLICK_BUILD`，因此直接编译 `code/main.c` 即可构建完整系统。程序启动时会自动寻找当前目录、`test_data/` 或 `../test_data/` 中的数据文件。

### macOS / Linux

```bash
cd code
cc -std=c99 -Wall -Wextra -pedantic main.c -o his
./his
```

也可以在仓库根目录运行：

```bash
./code/his
```

### Windows

仓库已提供预编译的 Windows 64 位控制台程序：

```text
code/his.exe
```

在 Windows 终端中进入 `code` 目录后运行：

```bat
his.exe
```

如果需要自行编译，可使用 MinGW-w64：

```bash
cd code
gcc -std=c99 -Wall -Wextra -pedantic main.c -o his.exe
```

如果在 macOS 上交叉编译 Windows 版本，可使用：

```bash
cd code
x86_64-w64-mingw32-gcc -std=c99 -Wall -Wextra -pedantic -static main.c -o his.exe
```

## 登录说明

主菜单入口如下：

```text
1. 管理员
2. 医生
3. 护士
4. 药房工作人员
5. 患者
0. 退出系统
```

管理员默认密码：

```text
总管理员：0001
医护资料管理员：1001
科室资源管理员：1003
医疗业务管理员：1004
```

患者资料管理员入口已合并到医疗业务管理员中。

医生、护士、药房工作人员、患者使用“编号 + 密码”登录。新增人员时系统会生成初始密码：

```text
患者：P + 患者编号，例如 P1001
医生：D + 医生编号，例如 D2001
护士：N + 护士编号，例如 N2501
药房工作人员：S + 工作人员编号，例如 S9001
```

初始密码登录后，系统会提示修改密码；各角色菜单中也提供“修改密码”功能。

## 编号规则

新增人员时，系统会自动生成编号。读取已有数据后，会从当前同类人员最大编号继续递增。

```text
患者编号：从 1001 开始
医生编号：从 2001 开始
护士编号：从 2501 开始
药房工作人员编号：从 9001 开始
```

管理员不使用人员编号登录，按管理员类型输入对应密码。

## 数据文件

测试数据统一放在 `test_data/` 目录中。系统启动时会读取该目录下的 txt 文件；如果没有检测到完整数据，会自动生成默认演示数据并保存。

主要数据文件包括：

```text
test_data/patients.txt                  # 患者数据
test_data/doctors.txt                   # 医生数据
test_data/nurses.txt                    # 护士数据
test_data/pharmacy_staff.txt            # 药房工作人员数据
test_data/departments.txt               # 科室数据
test_data/wards.txt                     # 病房数据
test_data/beds.txt                      # 床位数据
test_data/drugs.txt                     # 药品数据
test_data/drug_department_rules.txt     # 药品科室规则
test_data/records.txt                   # 医疗记录
test_data/registrations.txt             # 挂号记录
test_data/admissions.txt                # 住院记录
test_data/prescriptions.txt             # 处方主表
test_data/prescription_items.txt        # 处方明细
test_data/drug_usage.txt                # 发药与用药记录
test_data/log.txt                       # 操作日志
```

程序运行过程中修改的数据会先保存在内存中，请在菜单中选择“保存数据”，或退出系统时选择保存，确保修改写回 `.txt` 文件。

## 报表导出

分析报告默认导出为：

```text
analysis_report.txt
```

分段导出支持：

```text
department_analysis_report.txt # 科室分析报告
drug_analysis_report.txt       # 药品分析报告
profit_report.txt              # 利润统计报告
```

程序会把报表导出到当前识别到的数据目录中，本仓库中的示例报表位于 `test_data/`。

## 测试数据说明

完整测试数据字段、记录数和内容见 [TEST_DATA.md](TEST_DATA.md)。

## 适用环境

- C 标准：C99
- 推荐编译器：GCC / Clang / MinGW-w64
- 推荐运行环境：Windows Terminal、PowerShell、macOS Terminal 或 Linux Terminal

Windows 控制台下程序会尝试设置中文编码；如果仍出现乱码，建议使用 Windows Terminal，并将终端编码设置为 UTF-8。
