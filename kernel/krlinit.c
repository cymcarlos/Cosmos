/****************************************************************
        Cosmos kernel全局初始化文件krlinit.c
*****************************************************************
                彭东 
****************************************************************/
#include "cosmostypes.h"
#include "cosmosmctrl.h"
// 初始化内核 
void init_krl()
{
    init_krlmm();
	init_krldevice();
    init_krldriver();
	init_krlsched();            // 内核进程调度
    init_krlcpuidle();
    //STI();
    die(0);
    return;
}
