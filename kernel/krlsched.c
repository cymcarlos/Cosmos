/**********************************************************
        线程调度文件krlsched.c
***********************************************************
                彭东
**********************************************************/
#include "cosmostypes.h"
#include "cosmosmctrl.h"

void thrdlst_t_init(thrdlst_t *initp)
{
    list_init(&initp->tdl_lsth);           //初始化挂载进程的链表
    initp->tdl_curruntd = NULL;
    initp->tdl_nr = 0;
    return;
}

// 每个cpu 对应的数据结构
void schdata_t_init(schdata_t *initp)
{
    krlspinlock_init(&initp->sda_lock);
    initp->sda_cpuid = hal_retn_cpuid();
    initp->sda_schdflgs = NOTS_SCHED_FLGS;
    initp->sda_premptidx = 0;
    initp->sda_threadnr = 0;
    initp->sda_prityidx = 0;
    initp->sda_cpuidle = NULL;                   //开始没有空转进程和运行的进程
    initp->sda_currtd = NULL;
    for (uint_t ti = 0; ti < PRITY_MAX; ti++)
    {   //初始化schdata_t结构中的每个thrdlst_t结构, 各个优先级
        thrdlst_t_init(&initp->sda_thdlst[ti]);
    }
    list_init(&initp->sda_exitlist);
    return;
}

 //初始化osschedcls变量
void schedclass_t_init(schedclass_t *initp)
{
    krlspinlock_init(&initp->scls_lock);
    initp->scls_cpunr = CPUCORE_MAX;                               // cpu 数量
    initp->scls_threadnr = 0;       
    initp->scls_threadid_inc = 0;
    for (uint_t si = 0; si < CPUCORE_MAX; si++)
    {   //初始化osschedcls变量中的每个schdata_t
        schdata_t_init(&initp->scls_schda[si]);
    }
    return;
}

void init_krlsched()
{
    //初始化osschedcls变量
    schedclass_t_init(&osschedcls);
    kprint("进程调度器初始化成功\n");
    // die(0x400);
    return;
}

// 当前运行进程
thread_t *krlsched_retn_currthread()
{
    uint_t cpuid = hal_retn_cpuid();               // 
    schdata_t *schdap = &osschedcls.scls_schda[cpuid];
    if (schdap->sda_currtd == NULL)
    {   //若调度数据结构中当前运行进程的指针为空，就出错死机
        hal_sysdie("schdap->sda_currtd NULL");
    }
    return schdap->sda_currtd;
}

uint_t krlsched_retn_schedflgs()
{
    uint_t cpuid = hal_retn_cpuid();
    schdata_t *schdap = &osschedcls.scls_schda[cpuid];
    return schdap->sda_schdflgs;
}

void krlsched_wait(kwlst_t *wlst)
{
    cpuflg_t cufg, tcufg;
    uint_t cpuid = hal_retn_cpuid();
    schdata_t *schdap = &osschedcls.scls_schda[cpuid];
    thread_t *tdp = krlsched_retn_currthread();
    uint_t pity = tdp->td_priority;

    if (pity >= PRITY_MAX || wlst == NULL)
    {
        goto err_step;
    }
    if (schdap->sda_thdlst[pity].tdl_nr < 1)
    {
        goto err_step;
    }

    krlspinlock_cli(&schdap->sda_lock, &cufg);

    krlspinlock_cli(&tdp->td_lock, &tcufg);
    tdp->td_stus = TDSTUS_WAIT;
    list_del(&tdp->td_list);
    krlspinunlock_sti(&tdp->td_lock, &tcufg);

    if (schdap->sda_thdlst[pity].tdl_curruntd == tdp)
    {
        schdap->sda_thdlst[pity].tdl_curruntd = NULL;
    }
    schdap->sda_thdlst[pity].tdl_nr--;

    krlspinunlock_sti(&schdap->sda_lock, &cufg);
    krlwlst_add_thread(wlst, tdp);

    return;

err_step:
    hal_sysdie("krlsched_wait err");
    return;
}

void krlsched_up(kwlst_t *wlst)
{
    cpuflg_t cufg, tcufg;
    uint_t cpuid = hal_retn_cpuid();
    schdata_t *schdap = &osschedcls.scls_schda[cpuid];
    thread_t *tdp;
    uint_t pity;
    if (wlst == NULL)
    {
        goto err_step;
    }
    tdp = krlwlst_del_thread(wlst);
    if (tdp == NULL)
    {
        goto err_step;
    }
    pity = tdp->td_priority;
    if (pity >= PRITY_MAX)
    {
        goto err_step;
    }
    krlspinlock_cli(&schdap->sda_lock, &cufg);
    krlspinlock_cli(&tdp->td_lock, &tcufg);
    tdp->td_stus = TDSTUS_RUN;
    krlspinunlock_sti(&tdp->td_lock, &tcufg);
    list_add_tail(&tdp->td_list, &(schdap->sda_thdlst[pity].tdl_lsth));
    schdap->sda_thdlst[pity].tdl_nr++;
    krlspinunlock_sti(&schdap->sda_lock, &cufg);

    return;
err_step:
    hal_sysdie("krlsched_up err");
    return;
}

thread_t *krlsched_retn_idlethread()
{
    uint_t cpuid = hal_retn_cpuid();
    schdata_t *schdap = &osschedcls.scls_schda[cpuid];

    if (schdap->sda_cpuidle == NULL)
    {
        hal_sysdie("schdap->sda_cpuidle NULL");
    }
    return schdap->sda_cpuidle;
}

void krlsched_set_schedflgs()
{
    cpuflg_t cpuflg;
    uint_t cpuid = hal_retn_cpuid();
    schdata_t *schdap = &osschedcls.scls_schda[cpuid];

    krlspinlock_cli(&schdap->sda_lock, &cpuflg);
    schdap->sda_schdflgs = NEED_SCHED_FLGS;
    krlspinunlock_sti(&schdap->sda_lock, &cpuflg);
    return;
}

void krlsched_set_schedflgs_ex(uint_t flags)
{
    cpuflg_t cpuflg;
    uint_t cpuid = hal_retn_cpuid();
    schdata_t *schdap = &osschedcls.scls_schda[cpuid];

    krlspinlock_cli(&schdap->sda_lock, &cpuflg);
    schdap->sda_schdflgs = flags;
    krlspinunlock_sti(&schdap->sda_lock, &cpuflg);
    return;
}

void krlsched_chkneed_pmptsched()
{
    cpuflg_t cpuflg;
    uint_t schd = 0, cpuid = hal_retn_cpuid();
    schdata_t *schdap = &osschedcls.scls_schda[cpuid];

    krlspinlock_cli(&schdap->sda_lock, &cpuflg);

    if (schdap->sda_schdflgs == NEED_SCHED_FLGS && schdap->sda_premptidx == PMPT_FLGS)
    {
        schdap->sda_schdflgs = NOTS_SCHED_FLGS;
        schd = 1;
    }
    if (schdap->sda_schdflgs == NEED_START_CPUILDE_SCHED_FLGS)
    {
        schd = 1;
    }
    krlspinunlock_sti(&schdap->sda_lock, &cpuflg);
    if (schd == 1)
    {

        krlschedul();
    }
    return;
}

// 调度算法
thread_t *krlsched_select_thread()
{
    thread_t *retthd = NULL, *tdtmp = NULL, *cur = NULL;
    list_h_t *pos = NULL;
    cpuflg_t cufg;
    uint_t cpuid = hal_retn_cpuid();
    schdata_t *schdap = &osschedcls.scls_schda[cpuid];

    krlspinlock_cli(&schdap->sda_lock, &cufg);

    for (uint_t pity = 0; pity < PRITY_MAX; pity++)
    {
        //从最高优先级开始扫描
        list_for_each(pos, &(schdap->sda_thdlst[pity].tdl_lsth))
        {   //取出当前优先级进程链表下的第一个进程  
            tdtmp = list_entry(pos, thread_t, td_list);
            // 新建进程 或者 运行中进程
            if (tdtmp->td_stus == TDSTUS_RUN || tdtmp->td_stus == TDSTUS_NEW)
            {
                list_del(&tdtmp->td_list);
                cur = schdap->sda_thdlst[pity].tdl_curruntd;                
                if (cur != NULL)
                {   // 直接放到队尾， 和
                    list_add_tail(&cur->td_list, &schdap->sda_thdlst[pity].tdl_lsth);
                }

                schdap->sda_thdlst[pity].tdl_curruntd = tdtmp;
                retthd = tdtmp;
                goto return_step;
            }
        }
        if (schdap->sda_thdlst[pity].tdl_curruntd != NULL)
        {

            retthd = schdap->sda_thdlst[pity].tdl_curruntd;
            goto return_step;
        }
    }
    //如果最后也没有找到进程就返回默认的空转进程
    schdap->sda_prityidx = PRITY_MIN;
    retthd = krlsched_retn_idlethread();

return_step:

    krlspinunlock_sti(&schdap->sda_lock, &cufg);
    return retthd;
}

void krlschedul()
{

    if (krlsched_retn_schedflgs() == NEED_START_CPUILDE_SCHED_FLGS)
    {
        krlsched_set_schedflgs_ex(NOTS_SCHED_FLGS);
        retnfrom_first_sched(krlsched_retn_idlethread());
        return;
    }
    thread_t *prev = krlsched_retn_currthread(),                //返回当前运行进程
             *next = krlsched_select_thread();                  //选择下一个进程
    // kprint("调度器运行 当前进程:%s ID:%d,下一个进程:%s stus:%x ID:%d\n", 
    // prev->td_appfilenm, prev->td_id, next->td_appfilenm, next->td_stus, next->td_id);
    save_to_new_context(next, prev);
    return;
}

void krlschdclass_del_thread_addexit(thread_t *thdp)
{
    uint_t cpuid = hal_retn_cpuid();
    schdata_t *schdap = &osschedcls.scls_schda[cpuid];
    cpuflg_t cufg;

    krlspinlock_cli(&schdap->sda_lock, &cufg);

    schdap->sda_thdlst[thdp->td_priority].tdl_nr--;
    schdap->sda_threadnr--;

    if (schdap->sda_thdlst[thdp->td_priority].tdl_curruntd == thdp)
    {
 
        schdap->sda_thdlst[thdp->td_priority].tdl_curruntd = NULL;
    }

    list_del(&thdp->td_list);
    thdp->td_stus = TDSTUS_EXIT;
    list_add(&thdp->td_list, &schdap->sda_exitlist);

    krlspinunlock_sti(&schdap->sda_lock, &cufg);
    return;
}

void krlsched_exit()
{
    //cpuflg_t cufg, tcufg;
    uint_t cpuid = hal_retn_cpuid();
    schdata_t *schdap = &osschedcls.scls_schda[cpuid];
    thread_t *tdp = krlsched_retn_currthread();

    if (tdp == krlsched_retn_idlethread())
    {
        system_error("krlsched_exit");
        return;
    }

    uint_t pity = tdp->td_priority;

    if (pity >= PRITY_MAX)
    {
        goto err_step;
    }
    if (schdap->sda_thdlst[pity].tdl_nr < 1)
    {
        goto err_step;
    }

    return krlschdclass_del_thread_addexit(tdp);
err_step:
    hal_sysdie("krlsched_wait err");
    return;
}

//TODO 加入进程调度系统
void krlschdclass_add_thread(thread_t *thdp)
{
    uint_t cpuid = hal_retn_cpuid();
    schdata_t *schdap = &osschedcls.scls_schda[cpuid];
    cpuflg_t cufg;

    krlspinlock_cli(&schdap->sda_lock, &cufg);
    list_add(&thdp->td_list, &schdap->sda_thdlst[thdp->td_priority].tdl_lsth);
    schdap->sda_thdlst[thdp->td_priority].tdl_nr++;
    schdap->sda_threadnr++;
    krlspinunlock_sti(&schdap->sda_lock, &cufg);
    krlspinlock_cli(&osschedcls.scls_lock, &cufg);
    osschedcls.scls_threadnr++;
    krlspinunlock_sti(&osschedcls.scls_lock, &cufg);

    return;
}

TNCCALL void __to_new_context(thread_t *next, thread_t *prev)
{
    uint_t cpuid = hal_retn_cpuid();
    schdata_t *schdap = &osschedcls.scls_schda[cpuid];
    //设置当前运行进程为下一个运行的进程
    schdap->sda_currtd = next;
    //设置下一个运行进程的tss为当前CPU的tss
    next->td_context.ctx_nexttss = &x64tss[cpuid];
    //x64tss[cpuid].rsp0 = next->td_krlstktop;
    //设置当前CPU的tss中的R0栈为下一个运行进程的内核栈
    next->td_context.ctx_nexttss->rsp0 = next->td_krlstktop;
    //装载下一个运行进程的MMU页表
    hal_mmu_load(&next->td_mmdsc->msd_mmu);
    if (next->td_stus == TDSTUS_NEW)
    {
        next->td_stus = TDSTUS_RUN;
        retnfrom_first_sched(next);
    }

    return;
}

void save_to_new_context(thread_t *next, thread_t *prev)
{
#ifdef CFG_X86_PLATFORM
    __asm__ __volatile__(
        // 当前寄存器压栈
        "pushfq \n\t"
        "cli \n\t"
        "pushq %%rax\n\t"
        "pushq %%rbx\n\t"
        "pushq %%rcx\n\t"
        "pushq %%rdx\n\t"
        "pushq %%rbp\n\t"
        "pushq %%rsi\n\t"
        "pushq %%rdi\n\t"
        "pushq %%r8\n\t"
        "pushq %%r9\n\t"
        "pushq %%r10\n\t"
        "pushq %%r11\n\t"
        "pushq %%r12\n\t"
        "pushq %%r13\n\t"
        "pushq %%r14\n\t"
        "pushq %%r15\n\t"
        //保存CPU的RSP寄存器到当前进程的机器上下文结构中
        "movq %%rsp,%[PREV_RSP] \n\t"
        //把下一个进程的机器上下文结构中的RSP的值，写入CPU的RSP寄存器中  //事实上这里已经切换到下一个进程了，因为切换进程的内核栈
        "movq %[NEXT_RSP],%%rsp \n\t"
        //调用__to_new_context函数切换MMU页表
        "callq __to_new_context\n\t"
        //恢复下一个进程的通用寄存器
        "popq %%r15\n\t"
        "popq %%r14\n\t"
        "popq %%r13\n\t"
        "popq %%r12\n\t"
        "popq %%r11\n\t"
        "popq %%r10\n\t"
        "popq %%r9\n\t"
        "popq %%r8\n\t"
        "popq %%rdi\n\t"
        "popq %%rsi\n\t"
        "popq %%rbp\n\t"
        "popq %%rdx\n\t"
        "popq %%rcx\n\t"
        "popq %%rbx\n\t"
        "popq %%rax\n\t"
        "popfq \n\t"             //恢复下一个进程的标志寄存器
        : [PREV_RSP] "=m"(prev->td_context.ctx_nextrsp)
        : [NEXT_RSP] "m"(next->td_context.ctx_nextrsp), "D"(next), "S"(prev)
        : "memory");
#endif
    return;
}

void retnfrom_first_sched(thread_t *thrdp)
{

#ifdef CFG_X86_PLATFORM
    __asm__ __volatile__(
        //设置CPU的RSP寄存器为该进程机器上下文结构中的RSP
        "movq %[NEXT_RSP],%%rsp\n\t"    
        //恢复进程保存在内核栈中的段寄存器
        "popq %%r14\n\t"
        "movw %%r14w,%%gs\n\t"
        "popq %%r14\n\t"
        "movw %%r14w,%%fs\n\t"
        "popq %%r14\n\t"
        "movw %%r14w,%%es\n\t"
        "popq %%r14\n\t"
        "movw %%r14w,%%ds\n\t"
        "popq %%r15\n\t"
        //恢复进程保存在内核栈中的通用寄存器
        "popq %%r14\n\t"
        "popq %%r13\n\t"
        "popq %%r12\n\t"
        "popq %%r11\n\t"
        "popq %%r10\n\t"
        "popq %%r9\n\t"
        "popq %%r8\n\t"
        "popq %%rdi\n\t"
        "popq %%rsi\n\t"
        "popq %%rbp\n\t"
        "popq %%rdx\n\t"
        "popq %%rcx\n\t"
        "popq %%rbx\n\t"
        "popq %%rax\n\t"
        //恢复进程保存在内核栈中的RIP、CS、RFLAGS，（有可能需要恢复进程应用程序的RSP、SS）寄存器
        "iretq\n\t"
        :
        : [NEXT_RSP] "m"(thrdp->td_context.ctx_nextrsp)
        : "memory");
#endif
}
