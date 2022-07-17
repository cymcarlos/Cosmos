/**********************************************************
        线程管理头文件krlthread_t.h
***********************************************************
                彭东
**********************************************************/
#ifndef _KRLTHREAD_T_H
#define _KRLTHREAD_T_H
#define TDSTUS_RUN 0
#define TDSTUS_SLEEP 3
#define TDSTUS_WAIT 4
#define TDSTUS_NEW 5
#define TDSTUS_ZOMB 6
#define TDSTUS_EXIT 12

#define TDFLAG_FREE (1)
#define TDFLAG_BUSY (2)


#define TDRUN_TICK 20

#define PRITY_MAX 64
#define PRITY_MIN 0
#define PRILG_SYS 0
#define PRILG_USR 5

#define MICRSTK_MAX 4

#define THREAD_MAX (4)

#define THREAD_NAME_MAX (64)
#define KERNTHREAD_FLG 0                        // 内核进程
#define USERTHREAD_FLG 3                        // 用户进程

#if((defined CFG_X86_PLATFORM)) 
#define DAFT_TDUSRSTKSZ 0x8000                  // 程序栈默认大小
#define DAFT_TDKRLSTKSZ 0x8000                  // 内核栈默认大小
#endif


#if((defined CFG_X86_PLATFORM)) 
#define TD_HAND_MAX 8
#define DAFT_SPSR 0x10
#define DAFT_CPSR 0xd3
#define DAFT_CIDLESPSR 0x13   
#endif

#define K_CS_IDX    0x08
#define K_DS_IDX    0x10
#define U_CS_IDX    0x1b
#define U_DS_IDX    0x23
#define K_TAR_IDX   0x28
#define UMOD_EFLAGS 0x1202                          // 用户
#define KMOD_EFLAGS	0x202                           // 内核                

typedef struct s_MICRSTK
{
    uint_t msk_val[MICRSTK_MAX];
}micrstk_t;



typedef struct s_CONTEXT
{  
    uint_t       ctx_nextrip;                                   //保存下一次运行的地址    
    uint_t       ctx_nextrsp;                                   //保存下一次运行时内核栈的地址 
    x64tss_t*    ctx_nexttss;                                   //指向tss结构        
}context_t;

typedef struct s_THREAD
{
    spinlock_t  td_lock;                                        //进程的自旋锁
    list_h_t    td_list;                                        //进程链表 
    uint_t      td_flgs;                                        //进程的标志
    uint_t      td_stus;                                        //进程的状态
    uint_t      td_cpuid;                                       //进程所在的CPU的id
    uint_t      td_id;                                          //进程的id
    uint_t      td_tick;                                        //进程运行了多少tick   
    uint_t      td_sumtick;         
    uint_t      td_privilege;                                   //进程的权限                                         
    uint_t      td_priority;                                    //进程的优先级
    uint_t      td_runmode;                                     //进程的运行模式        
    adr_t       td_krlstktop;                                   //应用程序内核栈顶地址  相当于栈段地址
    adr_t       td_krlstkstart;                                 //应用程序内核栈开始地址
    adr_t       td_usrstktop;                                   //应用程序栈顶地址        
    adr_t       td_usrstkstart;                                 //应用程序栈开始地址            
    mmadrsdsc_t* td_mmdsc;                                      //地址空间结构
    void*       td_resdsc;
    void*       td_privtep;
    void*       td_extdatap;
    char_t*     td_appfilenm;
    uint_t      td_appfilenmlen;
    context_t   td_context;                                    //机器上下文件结构      
    objnode_t*  td_handtbl[TD_HAND_MAX];
    char_t      td_name[THREAD_NAME_MAX];                      //打开的对象数组 
}thread_t;

#endif // KRLTHREAD_T_H
