/****************************************************************
        Cosmos HAL全局数据结构头文件halglobal_t.h
*****************************************************************
                彭东
****************************************************************/
#ifndef _HALGLOBAL_T_H
#define _HALGLOBAL_T_H
//全局变量定义变量放在data段 放到init_entry.asm的data段中 ， 实际上都是同一个段， 毕竟64位没有段地址， 都是0开始
#define HAL_DEFGLOB_VARIABLE(vartype,varname) \
EXTERN  __attribute__((section(".data"))) vartype varname  

#endif // HALGLOBAL_T_H
