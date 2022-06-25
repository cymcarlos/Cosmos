/**********************************************************
        物理内存区间文件memarea.c
***********************************************************
                彭东
**********************************************************/
#include "cosmostypes.h"
#include "cosmosmctrl.h"

void arclst_t_init(arclst_t *initp)
{
	list_init(&initp->al_lru1);
	list_init(&initp->al_lru2);
	initp->al_lru1nr = 0;
	initp->al_lru2nr = 0;
	return;
}

mmstus_t mafo_deft_init(struct s_MEMAREA *memarea, void *valp, uint_t val)
{
	return MMSTUS_ERR;
}

mmstus_t mafo_deft_exit(struct s_MEMAREA *memarea)
{
	return MMSTUS_ERR;
}

mmstus_t mafo_deft_afry(struct s_MEMAREA *memarea, mmafrets_t *mafrspack, void *valp, uint_t val)
{
	return MMSTUS_ERR;
}

void mafuncobjs_t_init(mafuncobjs_t *initp)
{
	initp->mafo_init = mafo_deft_init;
	initp->mafo_exit = mafo_deft_exit;
	initp->mafo_aloc = mafo_deft_afry;
	initp->mafo_free = mafo_deft_afry;
	initp->mafo_recy = mafo_deft_afry;
	return;
}

void bafhlst_t_init(bafhlst_t *initp, u32_t stus, uint_t oder, uint_t oderpnr)
{
	knl_spinlock_init(&initp->af_lock);
	initp->af_stus = stus;
	initp->af_oder = oder;				
	initp->af_oderpnr = oderpnr;		//TODO   数组长度？
	initp->af_fobjnr = 0;
	//initp->af_aobjnr=0;
	initp->af_mobjnr = 0;
	initp->af_alcindx = 0;
	initp->af_freindx = 0;
	list_init(&initp->af_frelst);
	list_init(&initp->af_alclst);
	list_init(&initp->af_ovelst);
	return;
}

void memdivmer_t_init(memdivmer_t *initp)
{
	knl_spinlock_init(&initp->dm_lock);			// 自旋锁
	initp->dm_stus = 0;							// 状态位
	initp->dm_dmmaxindx = 0;
	initp->dm_phydmindx = 0;
	initp->dm_predmindx = 0;
	initp->dm_divnr = 0;
	initp->dm_mernr = 0;
	//循环初始化memdivmer_t结构体中dm_mdmlielst数组中的每个bafhlst_t结构的基本数据
	for (uint_t li = 0; li < MDIVMER_ARR_LMAX; li++)
	{
		bafhlst_t_init(&initp->dm_mdmlielst[li], BAFH_STUS_DIVM, li, (1UL << li));

	}
	bafhlst_t_init(&initp->dm_onemsalst, BAFH_STUS_ONEM, 0, 1UL);		//单个数据
	return;
}

void memarea_t_init(memarea_t *initp)
{
	// 初始化结构体的基础数据
	list_init(&initp->ma_list);				// 指向自己
	knl_spinlock_init(&initp->ma_lock);		// 自旋锁初始化
	initp->ma_stus = 0;						// 状态位
	initp->ma_flgs = 0;						// 标志位
	initp->ma_type = MA_TYPE_INIT;			// 内存类型
	//knl_sem_init(&initp->ma_sem,SEM_MUTEX_ONE_LOCK,SEM_FLG_MUTEX);
	//init_wait_l_head(&initp->ma_waitlst,general_wait_wake_up);
	initp->ma_maxpages = 0;
	initp->ma_allocpages = 0;
	initp->ma_freepages = 0;
	initp->ma_resvpages = 0;
	initp->ma_horizline = 0;
	initp->ma_logicstart = 0;		
	initp->ma_logicend = 0;
	initp->ma_logicsz = 0;
	initp->ma_effectstart = 0;
	initp->ma_effectend = 0;
	initp->ma_effectsz = 0;
	list_init(&initp->ma_allmsadsclst);
	initp->ma_allmsadscnr = 0;
	arclst_t_init(&initp->ma_arcpglst);
	mafuncobjs_t_init(&initp->ma_funcobj);
	// 初始化分割合并结构体
	memdivmer_t_init(&initp->ma_mdmdata);
	initp->ma_privp = NULL;
	return;
}

bool_t init_memarea_core(machbstart_t *mbsp)
{
	// 获取内存分区的memarea_t 开始地址， ===》mb_nextwtpadr
	u64_t phymarea = mbsp->mb_nextwtpadr;
	if (initchkadr_is_ok(mbsp, phymarea, (sizeof(memarea_t) * MEMAREA_MAX)) != 0)
	{
		return FALSE;
	}
	// 物理地址转位虚拟地址
	memarea_t *virmarea = (memarea_t *)phyadr_to_viradr((adr_t)phymarea);
	for (uint_t mai = 0; mai < MEMAREA_MAX; mai++)
	{
		memarea_t_init(&virmarea[mai]);
	}
	virmarea[0].ma_type = MA_TYPE_HWAD;
	virmarea[0].ma_logicstart = MA_HWAD_LSTART;
	virmarea[0].ma_logicend = MA_HWAD_LEND;
	virmarea[0].ma_logicsz = MA_HWAD_LSZ;
	virmarea[1].ma_type = MA_TYPE_KRNL;
	virmarea[1].ma_logicstart = MA_KRNL_LSTART;
	virmarea[1].ma_logicend = MA_KRNL_LEND;
	virmarea[1].ma_logicsz = MA_KRNL_LSZ;
	virmarea[2].ma_type = MA_TYPE_PROC;
	virmarea[2].ma_logicstart = MA_PROC_LSTART;
	virmarea[2].ma_logicend = MA_PROC_LEND;
	virmarea[2].ma_logicsz = MA_PROC_LSZ;
	virmarea[3].ma_type = MA_TYPE_SHAR;
	mbsp->mb_memznpadr = phymarea;
	mbsp->mb_memznnr = MEMAREA_MAX;
	mbsp->mb_memznsz = sizeof(memarea_t) * MEMAREA_MAX;
	mbsp->mb_nextwtpadr = PAGE_ALIGN(phymarea + sizeof(memarea_t) * MEMAREA_MAX);
	//.......
	return TRUE;
}

// 初始化内存分区
LKINIT void init_memarea()
{
	//kprint("memarea_t is sz[%x]\n",sizeof(memarea_t));
	if (init_memarea_core(&kmachbsp) == FALSE)
	{
		system_error("init_memarea_core fail");
	}
	//kprint("memareaphy:%x,nr:%x,sz:%x,np:%x\n",kmachbsp.mb_memznpadr,kmachbsp.mb_memznnr,kmachbsp.mb_memznsz,kmachbsp.mb_nextwtpadr
	//	);
	//disp_memarea(&kmachbsp);
	//die(0);
	return;
}

bool_t find_inmarea_msadscsegmant(memarea_t *mareap, msadsc_t *fmstat, uint_t fmsanr, msadsc_t **retmsastatp,
										 msadsc_t **retmsaendp, uint_t *retfmnr)
{
	if (NULL == mareap || NULL == fmstat || 0 == fmsanr || NULL == retmsastatp ||
		NULL == retmsaendp || NULL == retfmnr)
	{
		return FALSE;
	}

	return TRUE;
}

uint_t continumsadsc_is_ok(msadsc_t *prevmsa, msadsc_t *nextmsa, msadflgs_t *cmpmdfp)
{
	if (NULL == prevmsa || NULL == cmpmdfp)
	{
		return (~0UL);
	}

	if (NULL != prevmsa && NULL != nextmsa)
	{
		// 属于这个内存分区  &&  当前物理页没有分配（分配计数为0） && 物理页分配类型为空闲 &&  物理地址标记位为未分配
		if (prevmsa->md_indxflgs.mf_marty == cmpmdfp->mf_marty &&
			0 == prevmsa->md_indxflgs.mf_uindx &&
			MF_MOCTY_FREE == prevmsa->md_indxflgs.mf_mocty &&
			PAF_NO_ALLOC == prevmsa->md_phyadrs.paf_alloc)
		{
			// 类似
			if (nextmsa->md_indxflgs.mf_marty == cmpmdfp->mf_marty &&
				0 == nextmsa->md_indxflgs.mf_uindx &&
				MF_MOCTY_FREE == nextmsa->md_indxflgs.mf_mocty &&
				PAF_NO_ALLOC == nextmsa->md_phyadrs.paf_alloc)
			{
				// 判断是否连续
				if ((nextmsa->md_phyadrs.paf_padrs << PSHRSIZE) - (prevmsa->md_phyadrs.paf_padrs << PSHRSIZE) == PAGESIZE)
				{
					return 2;
				}
				return 1;
			}
			return 1;
		}
		return 0;
	}

	return (~0UL);
}

// cmpmdfp 	内存空间地址描述符标志   明显也是要属于这个分区才能算 
// fmsanr 	扫描总页数， 一般为物理页数
// retmnr 	返回的数据结构  多少个连续的， 0：一个连续（单独一个，其实就是不连续）  1：两个连续
bool_t scan_len_msadsc(msadsc_t *mstat, msadflgs_t *cmpmdfp, uint_t mnr, uint_t *retmnr)
{
	uint_t retclok = 0;
	uint_t retnr = 0;
	if (NULL == mstat || NULL == cmpmdfp || 0 == mnr || NULL == retmnr)
	{
		return FALSE;
	}
	for (uint_t tmdx = 0; tmdx < mnr - 1; tmdx++)
	{
		retclok = continumsadsc_is_ok(&mstat[tmdx], &mstat[tmdx + 1], cmpmdfp);
		if ((~0UL) == retclok)	// 数据有问题， 为空等
		{
			*retmnr = 0;
			return FALSE;
		}
		if (0 == retclok)		// 数据有问题， 前一个就不是空闲，或者连续
		{
			*retmnr = 0;
			return FALSE;
		}
		if (1 == retclok)		// 刚好下一个就不是空闲，或者连续
		{
			*retmnr = retnr;
			return TRUE;
		}						// 连续且空闲
		retnr++;
	}
	*retmnr = retnr;
	return TRUE;
}

uint_t check_continumsadsc(memarea_t *mareap, msadsc_t *stat, msadsc_t *end, uint_t fmnr)
{
	msadsc_t *ms = stat, *me = end;
	u32_t muindx = 0;
	msadflgs_t *mdfp = NULL;
	if (NULL == ms || NULL == me || 0 == fmnr || ms > me)
	{
		return 0;
	}
	switch (mareap->ma_type)
	{
	case MA_TYPE_HWAD:
	{
		muindx = MF_MARTY_HWD << 5;
		mdfp = (msadflgs_t *)(&muindx);
		break;
	}
	case MA_TYPE_KRNL:
	{
		muindx = MF_MARTY_KRL << 5;
		mdfp = (msadflgs_t *)(&muindx);
		break;
	}
	case MA_TYPE_PROC:
	{
		muindx = MF_MARTY_PRC << 5;
		mdfp = (msadflgs_t *)(&muindx);
		break;
	}
	default:
	{
		muindx = 0;
		mdfp = NULL;
		break;
	}
	}
	if (0 == muindx || NULL == mdfp)
	{
		return 0;
	}
	uint_t ok = 0;
	if (ms == me)
	{
		if (0 != ms->md_indxflgs.mf_uindx)
		{
			return 0;
		}
		if (ms->md_indxflgs.mf_marty != mdfp->mf_marty)
		{
			return 0;
		}
		if (MF_MOCTY_FREE != ms->md_indxflgs.mf_mocty)
		{
			return 0;
		}
		if (PAF_NO_ALLOC != ms->md_phyadrs.paf_alloc)
		{
			return 0;
		}

		if ((ok + 1) != fmnr)
		{
			return 0;
		}
		return ok + 1;
	}
	for (; ms < me; ms++)
	{
		if (ms->md_indxflgs.mf_marty != mdfp->mf_marty ||
			(ms + 1)->md_indxflgs.mf_marty != mdfp->mf_marty)
		{
			return 0;
		}
		if (MF_MOCTY_FREE != ms->md_indxflgs.mf_mocty ||
			MF_MOCTY_FREE != (ms + 1)->md_indxflgs.mf_mocty)
		{
			return 0;
		}
		if (ms->md_indxflgs.mf_uindx != 0 ||
			(ms + 1)->md_indxflgs.mf_uindx != 0)
		{
			return 0;
		}
		if (PAF_NO_ALLOC != ms->md_phyadrs.paf_alloc ||
			PAF_NO_ALLOC != (ms + 1)->md_phyadrs.paf_alloc)
		{
			return 0;
		}
		if (PAGESIZE != (((ms + 1)->md_phyadrs.paf_padrs << PSHRSIZE) - (ms->md_phyadrs.paf_padrs << PSHRSIZE)))

		{
			return 0;
		}
		ok++;
	}
	if (0 == ok)
	{
		return 0;
	}
	if ((ok + 1) != fmnr)
	{
		return 0;
	}
	return ok;
}

// mareap 		内存分区地址
// fmstat 		物理页开始地址
// fntmsanr 	扫描开始物理页下标
// fmsanr		扫描总页数
bool_t merlove_scan_continumsadsc(memarea_t *mareap, msadsc_t *fmstat, uint_t *fntmsanr, uint_t fmsanr,
										 msadsc_t **retmsastatp, msadsc_t **retmsaendp, uint_t *retfmnr)
{
	if (NULL == mareap || NULL == fmstat || NULL == fntmsanr ||
		0 == fmsanr || NULL == retmsastatp || NULL == retmsaendp || NULL == retfmnr)
	{
		return FALSE;
	}
	if (*fntmsanr >= fmsanr)
	{
		return FALSE;
	}
	u32_t muindx = 0;
	msadflgs_t *mdfp = NULL;
	switch (mareap->ma_type)
	{
	case MA_TYPE_HWAD:
	{
		muindx = MF_MARTY_HWD << 5;
		mdfp = (msadflgs_t *)(&muindx);
		break;
	}
	case MA_TYPE_KRNL:
	{
		muindx = MF_MARTY_KRL << 5;
		mdfp = (msadflgs_t *)(&muindx);
		break;
	}
	case MA_TYPE_PROC:
	{
		muindx = MF_MARTY_PRC << 5;
		mdfp = (msadflgs_t *)(&muindx);
		break;
	}
	default:
	{
		muindx = 0;
		mdfp = NULL;
		break;
	}
	}
	if (0 == muindx || NULL == mdfp)
	{
		return FALSE;
	}

	msadsc_t *msastat = fmstat;
	uint_t retfindmnr = 0;
	bool_t rets = FALSE;
	uint_t tmidx = *fntmsanr; 			// 开始扫描的物理页下标
	for (; tmidx < fmsanr; tmidx++)
	{
		// 属于这个内存分区  &&  当前物理页没有分配（分配计数为0） && 物理页分配类型为空闲 &&  物理地址标记位为未分配
		if (msastat[tmidx].md_indxflgs.mf_marty == mdfp->mf_marty &&
			0 == msastat[tmidx].md_indxflgs.mf_uindx &&
			MF_MOCTY_FREE == msastat[tmidx].md_indxflgs.mf_mocty &&
			PAF_NO_ALLOC == msastat[tmidx].md_phyadrs.paf_alloc)
		{
			//返回从这个msadsc_t结构开始到下一个非空闲 或者地址非连续的msadsc_t结构对应的msadsc_t结构索引号到retfindmnr变量中
			rets = scan_len_msadsc(&msastat[tmidx], mdfp, fmsanr, &retfindmnr);
			if (FALSE == rets)
			{
				system_error("scan_len_msadsc err\n");
			}
			*fntmsanr = tmidx + retfindmnr + 1;				// 更新开始扫描的物理页下标  retfindmnr = 0 则一个连续都没有
			*retmsastatp = &msastat[tmidx];					// 连续的开始地址
			*retmsaendp = &msastat[tmidx + retfindmnr];		// 连续的结束地址， 闭区间， 结束地址就包括自己的
			*retfmnr = retfindmnr + 1;						// 连续的个数
			return TRUE;
		}
	}
	if (tmidx >= fmsanr)			// 超过了物理页的页数， 明显有问题，容错处理
	{
		*fntmsanr = fmsanr;
		*retmsastatp = NULL;
		*retmsaendp = NULL;
		*retfmnr = 0;
		return TRUE;
	}
	return FALSE;
}
//给msadsc_t结构打上标签
// mareap 内存分区
// msadsc_t 开始地址 
// msanr msadsc_t 长度 
uint_t merlove_setallmarflgs_onmemarea(memarea_t *mareap, msadsc_t *mstat, uint_t msanr)
{
	if (NULL == mareap || NULL == mstat || 0 == msanr)
	{
		return ~0UL;
	}
	u32_t muindx = 0;
	msadflgs_t *mdfp = NULL;
	//获取内存区类型
	switch (mareap->ma_type)
	{
	case MA_TYPE_HWAD:
	{
		muindx = MF_MARTY_HWD << 5;					//硬件区标签		这里左移5位是因为mf_marty字段前面还有5位
		mdfp = (msadflgs_t *)(&muindx);
		break;
	}
	case MA_TYPE_KRNL:
	{
		muindx = MF_MARTY_KRL << 5;					//内核区标签
		mdfp = (msadflgs_t *)(&muindx);
		break;
	}
	case MA_TYPE_PROC:
	{
		muindx = MF_MARTY_PRC << 5;					//应用区标签
		mdfp = (msadflgs_t *)(&muindx);
		break;
	}
	case MA_TYPE_SHAR:
	{
		return 0;
	}
	default:
	{
		muindx = 0;
		mdfp = NULL;
		break;
	}
	}
	if (0 == muindx || NULL == mdfp)
	{
		return ~0UL;
	}
	u64_t phyadr = 0;
	uint_t retnr = 0;
	//扫描所有的msadsc_t结构
	for (uint_t mix = 0; mix < msanr; mix++)
	{
		if (MF_MARTY_INIT == mstat[mix].md_indxflgs.mf_marty)			//刚初始化
		{
			//获取msadsc_t结构对应的地址
			phyadr = mstat[mix].md_phyadrs.paf_padrs << PSHRSIZE;		// 实际的物理地址
			//和内存区的地址区间比较
			if (phyadr >= mareap->ma_logicstart && ((phyadr + PAGESIZE) - 1) <= mareap->ma_logicend)
			{
				//设置msadsc_t结构的标签 
				mstat[mix].md_indxflgs.mf_marty = mdfp->mf_marty;		// 
				retnr++;
			}
		}
	}
	return retnr;
}

uint_t test_setflgs(memarea_t *mareap, msadsc_t *mstat, uint_t msanr)
{
	u32_t muindx = 0;
	msadflgs_t *mdfp = NULL;
	if (NULL == mareap || NULL == mstat || 0 == msanr)
	{
		return ~0UL;
	}
	switch (mareap->ma_type)
	{
	case MA_TYPE_HWAD:
	{
		muindx = MF_MARTY_HWD << 5;
		mdfp = (msadflgs_t *)(&muindx);
		break;
	}
	case MA_TYPE_KRNL:
	{
		muindx = MF_MARTY_KRL << 5;
		mdfp = (msadflgs_t *)(&muindx);
		break;
	}
	case MA_TYPE_PROC:
	{
		muindx = MF_MARTY_PRC << 5;
		mdfp = (msadflgs_t *)(&muindx);
		break;
	}
	case MA_TYPE_SHAR:
	{
		return 0;
	}
	default:
	{
		muindx = 0;
		mdfp = NULL;
		break;
	}
	}
	if (0 == muindx || NULL == mdfp)
	{
		return ~0UL;
	}
	u64_t phyadr = 0;
	uint_t retnr = 0;
	for (uint_t mix = 0; mix < msanr; mix++)
	{
		/*if(MF_MOCTY_FREE==mstat[mix].md_indxflgs.mf_mocty&&
			PAF_NO_ALLOC==mstat[mix].md_phyadrs.paf_alloc&&
			0==mstat[mix].md_indxflgs.mf_uindx)*/
		{
			phyadr = mstat[mix].md_phyadrs.paf_padrs << PSHRSIZE;
			if (phyadr >= mareap->ma_logicstart && ((phyadr + PAGESIZE) - 1) <= mareap->ma_logicend)
			{
				if (mstat[mix].md_indxflgs.mf_marty == mdfp->mf_marty)
					retnr++;
			}
		}
	}
	return retnr;
}

//根据地址连续的msadsc_t结构的数量查找合适bafhlst_t结构
bafhlst_t *find_continumsa_inbafhlst(memarea_t *mareap, uint_t fmnr)
{
	bafhlst_t *retbafhp = NULL;
	uint_t in = 0;
	if (NULL == mareap || 0 == fmnr)			// 检查判断
	{
		return NULL;
	}

	if (MA_TYPE_PROC == mareap->ma_type)		//TODO 如果是用户区， 直接返回第一个？， 这里不太懂
	{
		return &mareap->ma_mdmdata.dm_onemsalst;
	}
	if (MA_TYPE_SHAR == mareap->ma_type)		// 如果是共享区, 直接返回null
	{
		return NULL;
	}

	in = 0;
	retbafhp = NULL;
	// 选择一个 刚好的bafhlst_t 例如   fmnr = 10 则选择af_oderpnr = 8(li = 3) 的
	// 选择一个 刚好的bafhlst_t 例如   fmnr = 16 则选择af_oderpnr = 16(li = 4) 的

	for (uint_t li = 0; li < MDIVMER_ARR_LMAX; li++)
	{
		if ((mareap->ma_mdmdata.dm_mdmlielst[li].af_oderpnr) <= fmnr)
		{
			retbafhp = &mareap->ma_mdmdata.dm_mdmlielst[li];
			in++;
		}
	}
	// 容错处理
	if (MDIVMER_ARR_LMAX <= in || NULL == retbafhp)
	{
		return NULL;
	}
	return retbafhp;
}

bool_t continumsadsc_add_procmareabafh(memarea_t *mareap, bafhlst_t *bafhp, msadsc_t *fstat, msadsc_t *fend, uint_t fmnr)
{
	if (NULL == mareap || NULL == bafhp || NULL == fstat || NULL == fend || 0 == fmnr)
	{
		return FALSE;
	}
	if (BAFH_STUS_ONEM != bafhp->af_stus || MA_TYPE_PROC != mareap->ma_type)
	{
		return FALSE;
	}
	if (bafhp->af_oderpnr != 1)
	{
		return FALSE;
	}
	if ((&fstat[fmnr - 1]) != fend)
	{
		return FALSE;
	}
	for (uint_t tmpnr = 0; tmpnr < fmnr; tmpnr++)
	{
		fstat[tmpnr].md_indxflgs.mf_olkty = MF_OLKTY_BAFH;
		fstat[tmpnr].md_odlink = bafhp;
		list_add(&fstat[tmpnr].md_list, &bafhp->af_frelst);
		bafhp->af_fobjnr++;
		bafhp->af_mobjnr++;
		mareap->ma_maxpages++;
		mareap->ma_freepages++;
		mareap->ma_allmsadscnr++;
	}
	return TRUE;
}
// mareap 内存分区
// bafhp bafhlst_t结构
// fstat bafhlst_t指针  开始的那个
// fend  bafhlst_t指针  结束那个
// bafhlst_t结构 fmnr的挂的物理页长度
bool_t continumsadsc_add_bafhlst(memarea_t *mareap, bafhlst_t *bafhp, msadsc_t *fstat, msadsc_t *fend, uint_t fmnr)
{
	// 一堆容错检查
	if (NULL == mareap || NULL == bafhp || NULL == fstat || NULL == fend || 0 == fmnr)
	{
		return FALSE;
	}
	if (bafhp->af_oderpnr != fmnr)
	{
		return FALSE;
	}
	if ((&fstat[fmnr - 1]) != fend)
	{
		return FALSE;
	}
	fstat->md_indxflgs.mf_olkty = MF_OLKTY_ODER;		// TODO	标记为开始？
	//开始的msadsc_t结构的md_odlink指向最后的msadsc_t结构的首地址
	fstat->md_odlink = fend;
	// fstat==fend
	fend->md_indxflgs.mf_olkty = MF_OLKTY_BAFH;
	//最后的msadsc_t结构指向它属于的bafhlst_t结构
	fend->md_odlink = bafhp;
	//把多个地址连续的msadsc_t结构的的开始的那个msadsc_t结构挂载到bafhlst_t结构的af_frelst(空闲中)中
	list_add(&fstat->md_list, &bafhp->af_frelst);		// af_frelst <===> fstat->md_list  <===>  af_frelst->next(旧的)
	bafhp->af_fobjnr++;									// TODO 为啥是 +1 空闲页面， 难道是 多少个是连续段的意思？
	bafhp->af_mobjnr++;									// 挂载的总页面
	mareap->ma_maxpages += fmnr;						
	mareap->ma_freepages += fmnr;
	mareap->ma_allmsadscnr += fmnr;
	return TRUE;
}

// rfstat 			开始地址
// rfend 			结束地址
// rfmnr			长度
bool_t continumsadsc_mareabafh_core(memarea_t *mareap, msadsc_t **rfstat, msadsc_t **rfend, uint_t *rfmnr)
{

	if (NULL == mareap || NULL == rfstat || NULL == rfend || NULL == rfmnr)
	{
		return FALSE;
	}
	uint_t retval = *rfmnr, tmpmnr = 0;
	msadsc_t *mstat = *rfstat, *mend = *rfend;
	// mstat 开始的msadsc_t指针
	// mend  结束的msadsc_t指针
	if (1 > (retval))		// 容错处理
	{
		return FALSE;
	}
	//根据地址连续的msadsc_t结构的数量查找合适bafhlst_t结构
	bafhlst_t *bafhp = find_continumsa_inbafhlst(mareap, retval);
	if (NULL == bafhp)			// 为空异常
	{
		return FALSE;
	}
	if (retval < bafhp->af_oderpnr)		// 页数大于连续页数，异常
	{
		return FALSE;
	}
	// 不是用户区
	if ((BAFH_STUS_DIVP == bafhp->af_stus || BAFH_STUS_DIVM == bafhp->af_stus) && MA_TYPE_PROC != mareap->ma_type)
	{
		// 剩余的长度
		// 如果是 retval = 9，  不是2的倍数， 应该是 9 - 8 = 1 页，  8 页挂在3下标上
		// 1 页就直接挂0下标的上
		tmpmnr = retval - bafhp->af_oderpnr;	
		if (continumsadsc_add_bafhlst(mareap, bafhp, mstat, &mstat[bafhp->af_oderpnr - 1], bafhp->af_oderpnr) == FALSE)
		{
			return FALSE;
		}
		if (tmpmnr == 0)
		{
			*rfmnr = tmpmnr;
			*rfend = NULL;
			return TRUE;
		}
		*rfstat = &mstat[bafhp->af_oderpnr];	// 被截断的剩余连续物理页的指针的地址
		*rfmnr = tmpmnr;						// 剩余的长度

		return TRUE;
	}
	// bafhp->af_stus 如果是单个的 且是用户区的
	if (BAFH_STUS_ONEM == bafhp->af_stus && MA_TYPE_PROC == mareap->ma_type)
	{
		if (continumsadsc_add_procmareabafh(mareap, bafhp, mstat, mend, *rfmnr) == FALSE)
		{
			return FALSE;
		}
		*rfmnr = 0;
		*rfend = NULL;
		return TRUE;
	}

	return FALSE;
}

// 把物理页挂到
bool_t merlove_continumsadsc_mareabafh(memarea_t *mareap, msadsc_t *mstat, msadsc_t *mend, uint_t mnr)
{
	if (NULL == mareap || NULL == mstat || NULL == mend || 0 == mnr)
	{
		return FALSE;
	}
	uint_t mnridx = mnr;
	msadsc_t *fstat = mstat, *fend = mend;
	//如果mnridx > 0并且NULL != fend就循环调用continumsadsc_mareabafh_core函数，而mnridx和fend由这个函数控制
	for (; (mnridx > 0 && NULL != fend);)
	{
		//为一段地址连续的msadsc_t结构寻找合适m_mdmlielst数组中的bafhlst_t结构
		if (continumsadsc_mareabafh_core(mareap, &fstat, &fend, &mnridx) == FALSE)
		{
			system_error("continumsadsc_mareabafh_core fail\n");
		}
	}
	return TRUE;
}

// msanr 物理页页数
bool_t merlove_mem_onmemarea(memarea_t *mareap, msadsc_t *mstat, uint_t msanr)
{
	// 检查， 去掉为空的情况， 错误的情况
	if (NULL == mareap || NULL == mstat || 0 == msanr)
	{
		return FALSE;
	}
	if (MA_TYPE_SHAR == mareap->ma_type)			
	{
		return TRUE;
	}
	if (MA_TYPE_INIT == mareap->ma_type)
	{
		return FALSE;
	}
	msadsc_t *retstatmsap = NULL, *retendmsap = NULL, *fntmsap = mstat;
	uint_t retfindmnr = 0;
	uint_t fntmnr = 0;
	bool_t retscan = FALSE;


	for (; fntmnr < msanr;)
	{
		// 扫描， 会更新fntmnr
		retscan = merlove_scan_continumsadsc(mareap, fntmsap, &fntmnr, msanr, &retstatmsap, &retendmsap, &retfindmnr);
		if (FALSE == retscan)					// 容错处理
		{
			system_error("merlove_scan_continumsadsc fail\n");
		}
		if (NULL != retstatmsap && NULL != retendmsap)
		{
			// 再次检查
			if (check_continumsadsc(mareap, retstatmsap, retendmsap, retfindmnr) == 0)
			{
				system_error("check_continumsadsc fail\n");
			}
			//把一组连续的msadsc_t结构体挂载到合适的m_mdmlielst数组中的bafhlst_t结构中
			if (merlove_continumsadsc_mareabafh(mareap, retstatmsap, retendmsap, retfindmnr) == FALSE)
			{
				system_error("merlove_continumsadsc_mareabafh fail\n");
			}
		}
	}
	return TRUE;
}

bool_t merlove_mem_core(machbstart_t *mbsp)
{
	//获取msadsc_t结构的首地址
	msadsc_t *mstatp = (msadsc_t *)phyadr_to_viradr((adr_t)mbsp->mb_memmappadr);
	//获取msadsc_t结构的个数
	uint_t msanr = (uint_t)mbsp->mb_memmapnr, maxp = 0;
	//获取memarea_t结构的首地址
	memarea_t *marea = (memarea_t *)phyadr_to_viradr((adr_t)mbsp->mb_memznpadr);
	uint_t sretf = ~0UL, tretf = ~0UL;
	//遍历每个memarea_t结构
	for (uint_t mi = 0; mi < (uint_t)mbsp->mb_memznnr; mi++)
	{
		sretf = merlove_setallmarflgs_onmemarea(&marea[mi], mstatp, msanr);
		if ((~0UL) == sretf)
		{
			return FALSE;
		}
		// TODO 这里应该是测试
		tretf = test_setflgs(&marea[mi], mstatp, msanr);
		if ((~0UL) == tretf)
		{
			return FALSE;
		}
		if (sretf != tretf)
		{
			return FALSE;
		}
	}
	//遍历每个memarea_t结构
	for (uint_t maidx = 0; maidx < (uint_t)mbsp->mb_memznnr; maidx++)
	{
		//针对其中一个memarea_t结构对msadsc_t结构进行合并
		if (merlove_mem_onmemarea(&marea[maidx], mstatp, msanr) == FALSE)
		{
			return FALSE;
		}
		maxp += marea[maidx].ma_maxpages;
	}

	return TRUE;
}

uint_t check_multi_msadsc(msadsc_t *mstat, bafhlst_t *bafhp, memarea_t *mareap)
{
	if (NULL == mstat || NULL == bafhp || NULL == mareap)
	{
		return 0;
	}
	if (MF_OLKTY_ODER != mstat->md_indxflgs.mf_olkty &&
		MF_OLKTY_BAFH != mstat->md_indxflgs.mf_olkty)
	{
		return 0;
	}
	if (NULL == mstat->md_odlink)
	{
		return 0;
	}

	msadsc_t *mend = NULL; //(msadsc_t*)mstat->md_odlink;
	if (MF_OLKTY_ODER == mstat->md_indxflgs.mf_olkty)
	{
		mend = (msadsc_t *)mstat->md_odlink;
	}
	if (MF_OLKTY_BAFH == mstat->md_indxflgs.mf_olkty)
	{
		mend = mstat;
	}
	if (NULL == mend)
	{
		return 0;
	}
	uint_t mnr = (mend - mstat) + 1;
	if (mnr != bafhp->af_oderpnr)
	{
		return 0;
	}
	if (MF_OLKTY_BAFH != mend->md_indxflgs.mf_olkty)
	{
		return 0;
	}
	if ((bafhlst_t *)(mend->md_odlink) != bafhp)
	{
		return 0;
	}

	u64_t phyadr = (~0UL);
	if (mnr == 1)
	{
		if (mstat->md_indxflgs.mf_marty != (u32_t)mareap->ma_type)
		{
			return 0;
		}
		if (PAF_NO_ALLOC != mstat->md_phyadrs.paf_alloc ||
			0 != mstat->md_indxflgs.mf_uindx)
		{
			return 0;
		}
		phyadr = mstat->md_phyadrs.paf_padrs << PSHRSIZE;
		if (phyadr < mareap->ma_logicstart || (phyadr + PAGESIZE - 1) > mareap->ma_logicend)
		{
			return 0;
		}
		return 1;
	}
	uint_t idx = 0;
	for (uint_t mi = 0; mi < mnr - 1; mi++)
	{
		if (mstat[mi].md_indxflgs.mf_marty != (u32_t)mareap->ma_type)
		{
			return 0;
		}
		if (PAF_NO_ALLOC != mstat[mi].md_phyadrs.paf_alloc ||
			0 != mstat[mi].md_indxflgs.mf_uindx)
		{
			return 0;
		}
		if (PAF_NO_ALLOC != mstat[mi + 1].md_phyadrs.paf_alloc ||
			0 != mstat[mi + 1].md_indxflgs.mf_uindx)
		{
			return 0;
		}
		if (((mstat[mi].md_phyadrs.paf_padrs << PSHRSIZE) + PAGESIZE) != (mstat[mi + 1].md_phyadrs.paf_padrs << PSHRSIZE))
		{
			return 0;
		}
		if ((mstat[mi].md_phyadrs.paf_padrs << PSHRSIZE) < mareap->ma_logicstart ||
			(((mstat[mi + 1].md_phyadrs.paf_padrs << PSHRSIZE) + PAGESIZE) - 1) > mareap->ma_logicend)
		{
			return 0;
		}
		idx++;
	}
	return idx + 1;
}

bool_t check_one_bafhlst(bafhlst_t *bafhp, memarea_t *mareap)
{
	if (NULL == bafhp || NULL == mareap)
	{
		return FALSE;
	}
	if (1 > bafhp->af_mobjnr && 1 > bafhp->af_fobjnr)
	{
		return TRUE;
	}
	uint_t lindx = 0;
	list_h_t *tmplst = NULL;
	msadsc_t *msap = NULL;
	list_for_each(tmplst, &bafhp->af_frelst)
	{
		msap = list_entry(tmplst, msadsc_t, md_list);
		if (bafhp->af_oderpnr != check_multi_msadsc(msap, bafhp, mareap))
		{
			return FALSE;
		}
		lindx++;
	}
	if (lindx != bafhp->af_fobjnr || lindx != bafhp->af_mobjnr)
	{
		return FALSE;
	}
	return TRUE;
}

bool_t check_one_memarea(memarea_t *mareap)
{
	if (NULL == mareap)
	{
		return FALSE;
	}
	if (1 > mareap->ma_maxpages)
	{
		return TRUE;
	}

	for (uint_t li = 0; li < MDIVMER_ARR_LMAX; li++)
	{
		if (check_one_bafhlst(&mareap->ma_mdmdata.dm_mdmlielst[li], mareap) == FALSE)
		{
			return FALSE;
		}
	}

	if (check_one_bafhlst(&mareap->ma_mdmdata.dm_onemsalst, mareap) == FALSE)
	{
		return FALSE;
	}
	return TRUE;
}
void mem_check_mareadata(machbstart_t *mbsp)
{
	memarea_t *marea = (memarea_t *)phyadr_to_viradr((adr_t)mbsp->mb_memznpadr);
	for (uint_t maidx = 0; maidx < mbsp->mb_memznnr; maidx++)
	{
		if (check_one_memarea(&marea[maidx]) == FALSE)
		{
			system_error("check_one_memarea fail\n");
		}
	}
	return;
}

//初始化页面合并 
void init_merlove_mem()
{
	if (merlove_mem_core(&kmachbsp) == FALSE)
	{
		system_error("merlove_mem_core fail\n");
	}
	mem_check_mareadata(&kmachbsp);
	//kprint("init_merlove_mem OK\n");
	//disp_memarea(&kmachbsp);
	//disp_memarea(&kmachbsp);
	//die(0);
	return;
}

void disp_bafhlst(bafhlst_t *bafhp)
{
	if (bafhp->af_mobjnr > 0)
	{
		kprint("bafhlst_t.af_stus:%x,af_indx:%x,af_onebnr:%x,af_fobjnr:%x\n",
			   bafhp->af_stus, bafhp->af_oder, bafhp->af_oderpnr, bafhp->af_fobjnr);
	}
	return;
}

void disp_memarea(machbstart_t *mbsp)
{
	memarea_t *marea = (memarea_t *)phyadr_to_viradr((adr_t)mbsp->mb_memznpadr);
	for (uint_t i = 0; i < mbsp->mb_memznnr; i++)
	{
		kprint("memarea.ma_type:%x,ma_maxpages:%x,ma_freepages:%x,ma_allmsadscnr:%x\n",
			   marea[i].ma_type, marea[i].ma_maxpages, marea[i].ma_freepages, marea[i].ma_allmsadscnr);
		
		for (uint_t li = 0; li < MDIVMER_ARR_LMAX; li++)
		{
			disp_bafhlst(&marea[i].ma_mdmdata.dm_mdmlielst[li]);
		}
		disp_bafhlst(&marea[i].ma_mdmdata.dm_onemsalst);
	}
	return;
}