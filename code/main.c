/*
 * main.c
 * ------------------------------------------------------------
 * 这是完整项目的总入口文件
 *
 * 程序运行时大致按下面的顺序执行：
 * 1. 先处理控制台编码，尽量保证中文能正常显示；
 * 2. 再找到数据文件所在目录，避免从别的位置运行时读不到 txt；
 * 3. 初始化 HospitalSystem 总结构体，把各个链表头和编号都设置好；
 * 4. 读取 patients.txt、doctors.txt 等数据文件；
 * 5. 如果没有读到数据，就自动生成一套演示数据；
 * 6. 最后进入菜单系统，由用户选择管理员、医生、护士、药房工作人员或患者入口
 *
 * 这个文件不要拆开运行，它负责把整个项目串起来
 */

#include "common.h"
#include "system.h"
#include "fileio.h"
#include "testdata.h"

/* main 函数是 C 程序真正开始执行的位置
 *
 * argc 和 argv 是命令行参数：
 * argc 表示参数个数，argv[0] 一般是当前程序的路径
 * 本项目主要用 argv[0] 来辅助定位数据文件所在目录，
 * 所以 argc 本身暂时用不到，下面用 (void)argc 消除未使用警告
 */
int main(int argc, char *argv[]) {
    /* system 是整个医院信息系统的总数据对象
     * 患者链表、医生链表、科室链表、病房链表、药品链表、
     * 病历链表、住院记录链表等，都挂在这个结构体里面
     */
    HospitalSystem system;
    (void)argc;

    /* 设置控制台编码，主要是为了在 Windows 控制台中显示中文 */
    init_console_encoding();

    /* 尝试切换到数据文件所在目录
     * 这样不管是双击运行，还是在 IDE 里直接运行 main.c，
     * 程序都更容易找到 patients.txt、doctors.txt 等文件
     */
    prepare_data_directory(argv[0], __FILE__);

    /* 初始化系统总结构体
     * 这一步会把所有链表头设置为 NULL，
     * 并把患者编号、医生编号、科室编号等设置成初始值
     */
    init_system(&system);

    /* 从 txt 文件读取数据
     * load_all_data 返回 0 时，说明没有读到可用数据，
     * 一般是第一次运行，或者数据文件被删除了
     */
    if (load_all_data(&system) == 0) {
        printf("未检测到完整数据文件，系统将自动生成默认示例数据\n");

        /* 生成一套默认演示数据，方便第一次运行时直接看效果 */
        generate_demo_data(&system);

        /* 生成后马上保存到 txt 文件，下次启动就可以直接读取 */
        save_all_data(&system);
    } else {
        printf("数据文件读取成功\n");
    }

    /* 进入完整菜单
     * 后面的管理员菜单、医生菜单、护士菜单、药房工作人员菜单、患者菜单，
     * 都是在 run_system 里面根据用户选择继续分发的
     */
    run_system(&system);

    /* 程序退出前释放所有链表结点，避免内存泄漏 */
    free_system(&system);
    return 0;
}

#if HIS_MAIN_ONE_CLICK_BUILD
/*
 * 下面这一段是为了“一键运行 main.c”准备的
 *
 * 平时 C 项目一般会把 common.c、menu.c、patient.c 等文件一起交给编译器
 * 但是有些同学在 Dev-C++、Code::Blocks 或 VS Code 里只点开 main.c 运行，
 * 如果不把其他 .c 文件带进来，就会出现“函数未定义”的错误
 *
 * 所以这里在 HIS_MAIN_ONE_CLICK_BUILD 打开时，
 * 直接把各个模块的 .c 文件包含进 main.c
 * 这样只编译 main.c，也能把整个系统跑起来
 */
#define HIS_BUILD_FROM_MAIN 1

/* 公共工具：初始化、字符串处理、日期计算、表格打印、日志等 */
#include "common.c"

/* 文件读写：负责把链表数据保存到 txt，也负责启动时读取 txt */
#include "fileio.c"

/* 登录认证和权限判断：管理员、医生、患者身份都在这里处理 */
#include "auth.c"

    /* 科室、医生、护士、药房工作人员、患者等基础资料管理 */
    #include "department.c"
    #include "doctor.c"
    #include "nurse.c"
    #include "pharmacy_staff.c"
    #include "patient.c"

/* 病房和床位管理，包括床位分配、释放、状态显示等 */
#include "ward.c"

/* 药品库存和发药 */
#include "drug.c"

/* 病历、挂号、住院和出院记录 */
#include "record.c"

/* 统计报表 */
#include "analysis.c"

/* 预测和优化建议 */
#include "forecast.c"
#include "optimize.c"

/* 菜单调度，把各个功能模块按角色组织起来 */
#include "menu.c"

/* 第一次运行时使用的默认演示数据 */
#include "testdata.c"
#endif
