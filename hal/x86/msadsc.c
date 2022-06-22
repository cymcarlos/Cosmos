/**********************************************************
        物理内存空间数组文件msadsc.c
***********************************************************
                彭东
**********************************************************/
#include "cosmostypes.h"
#include "cosmosmctrl.h"

// 物理内存也得初始化
void msadsc_t_init(msadsc_t *initp)
{
	list_init(&initp->md_list);							// 前和后都指向自己
	knl_spinlock_init(&initp->md_lock);					// 初始化自旋锁
	initp->md_indxflgs.mf_olkty = MF_OLKTY_INIT;
	initp->md_indxflgs.mf_lstty = MF_LSTTY_LIST;			// 是否
	initp->md_indxflgs.mf_mocty = MF_MOCTY_FREE;			// 分配类型
	initp->md_indxflgs.mf_marty = MF_MARTY_INIT;			// 内存分区
	initp->md_indxflgs.mf_uindx = MF_UINDX_INIT;			// 分配计数
	initp->md_phyadrs.paf_alloc = PAF_NO_ALLOC;				// 分配位
	initp->md_phyadrs.paf_shared = PAF_NO_SHARED;			// 共享位
	initp->md_phyadrs.paf_swap = PAF_NO_SWAP;				//交换位		
	initp->md_phyadrs.paf_cache = PAF_NO_CACHE;				//缓存位
	initp->md_phyadrs.paf_kmap = PAF_NO_KMAP;				//映射位
	initp->md_phyadrs.paf_lock = PAF_NO_LOCK;				//锁定位
	initp->md_phyadrs.paf_dirty = PAF_NO_DIRTY;				//脏位
	initp->md_phyadrs.paf_busy = PAF_NO_BUSY;				//忙位
	initp->md_phyadrs.paf_rv2 = PAF_RV2_VAL;				//保留位
	initp->md_phyadrs.paf_padrs = PAF_INIT_PADRS;			//页物理地址位	
	initp->md_odlink = NULL;								//相邻且相同大小msadsc的指针
	return;
}

void disp_one_msadsc(msadsc_t *mp)
{
	kprint("msadsc_t.md_f:_ux[%x],_my[%x],md_phyadrs:_alc[%x],_shd[%x],_swp[%x],_che[%x],_kmp[%x],_lck[%x],_dty[%x],_bsy[%x],_padrs[%x]\n",
		   (uint_t)mp->md_indxflgs.mf_uindx, (uint_t)mp->md_indxflgs.mf_mocty, (uint_t)mp->md_phyadrs.paf_alloc, (uint_t)mp->md_phyadrs.paf_shared, (uint_t)mp->md_phyadrs.paf_swap, (uint_t)mp->md_phyadrs.paf_cache, (uint_t)mp->md_phyadrs.paf_kmap, (uint_t)mp->md_phyadrs.paf_lock,
		   (uint_t)mp->md_phyadrs.paf_dirty, (uint_t)mp->md_phyadrs.paf_busy, (uint_t)(mp->md_phyadrs.paf_padrs << 12));
	return;
}

//计算msadsc_t物理内存也结构数组的开始地址和数组元素个数
// 机器信息中的mb_nextwtpadr就是开始地址， 长度直接算
// 返回的是虚拟地址
bool_t ret_msadsc_vadrandsz(machbstart_t *mbsp, msadsc_t **retmasvp, u64_t *retmasnr)
{
	if (NULL == mbsp || NULL == retmasvp || NULL == retmasnr)
	{
		return FALSE;
	}
	if (mbsp->mb_e820exnr < 1 || NULL == mbsp->mb_e820expadr || (mbsp->mb_e820exnr * sizeof(phymmarge_t)) != mbsp->mb_e820exsz)
	{
		*retmasvp = NULL;
		*retmasnr = 0;
		return FALSE;
	}
	phymmarge_t *pmagep = (phymmarge_t *)phyadr_to_viradr((adr_t)mbsp->mb_e820expadr);
	u64_t usrmemsz = 0, msadnr = 0;
	for (u64_t i = 0; i < mbsp->mb_e820exnr; i++)
	{
		if (PMR_T_OSAPUSERRAM == pmagep[i].pmr_type)
		{
			usrmemsz += pmagep[i].pmr_lsize;
			msadnr += (pmagep[i].pmr_lsize >> 12);
		}
	}
	if (0 == usrmemsz || (usrmemsz >> 12) < 1 || msadnr < 1)
	{
		*retmasvp = NULL;
		*retmasnr = 0;
		return FALSE;
	}
	//msadnr=usrmemsz>>12;
	if (0 != initchkadr_is_ok(mbsp, mbsp->mb_nextwtpadr, (msadnr * sizeof(msadsc_t))))
	{
		system_error("ret_msadsc_vadrandsz initchkadr_is_ok err\n");
	}

	*retmasvp = (msadsc_t *)phyadr_to_viradr((adr_t)mbsp->mb_nextwtpadr);
	*retmasnr = msadnr;
	return TRUE;
}

// msadsc_t 结构体开始虚拟地址
// 对应物理内存的开始地址
void write_one_msadsc(msadsc_t *msap, u64_t phyadr)
{
	msadsc_t_init(msap);
	phyadrflgs_t *tmp = (phyadrflgs_t *)(&phyadr);
	// 物理页地址
	msap->md_phyadrs.paf_padrs = tmp->paf_padrs;		
	return;
}


u64_t init_msadsc_core(machbstart_t *mbsp, msadsc_t *msavstart, u64_t msanr)
{
	//获取phymmarge_t结构数组开始地址， 虚拟地址
	phymmarge_t *pmagep = (phymmarge_t *)phyadr_to_viradr((adr_t)mbsp->mb_e820expadr);
	u64_t mdindx = 0;
	//扫描phymmarge_t结构数组
	for (u64_t i = 0; i < mbsp->mb_e820exnr; i++)
	{
		//判断phymmarge_t结构的类型是不是可用内存
		if (PMR_T_OSAPUSERRAM == pmagep[i].pmr_type)
		{
			//每次加上4KB-1比较是否小于等于phymmarge_t结构的结束地址	
			// phymmarge_t的开始结束地址是二级引导收集的信息
			for (u64_t start = pmagep[i].pmr_saddr; start < pmagep[i].pmr_end; start += 4096)
			{
				if ((start + 4096 - 1) <= pmagep[i].pmr_end)
				{
					//与当前地址为参数写入第mdindx个msadsc结构
					write_one_msadsc(&msavstart[mdindx], start);
					mdindx++;
				}
			}
		}
	}

	return mdindx;
}

// 初始化内存页
void init_msadsc()
{
	u64_t  coremdnr = 0, 
		   msadscnr = 0;				//应该是内存分页数组的长度
	msadsc_t *msadscvp = NULL;			//内存分页数的数组， 随便指向一个null
	machbstart_t *mbsp = &kmachbsp;     //mbsp指向kmachbsp的地址， 
	//计算msadsc_t物理内存也结构数组的开始地址和数组元素个数
	if (ret_msadsc_vadrandsz(mbsp, &msadscvp, &msadscnr) == FALSE)
	{
		system_error("init_msadsc ret_msadsc_vadrandsz err\n");
	}
	//开始真正初始化msadsc_t结构数组
	coremdnr = init_msadsc_core(mbsp, msadscvp, msadscnr);
	if (coremdnr != msadscnr)
	{
		system_error("init_msadsc init_msadsc_core err\n");
	}
	mbsp->mb_memmappadr = viradr_to_phyadr((adr_t)msadscvp); // 虚拟地址转为物理地址
	mbsp->mb_memmapnr = coremdnr;
	mbsp->mb_memmapsz = coremdnr * sizeof(msadsc_t);
	// 更新mb_nextwtpadr
	mbsp->mb_nextwtpadr = PAGE_ALIGN(mbsp->mb_memmappadr + mbsp->mb_memmapsz);
	return;
}

void disp_phymsadsc()
{
	u64_t coremdnr = 0;
	msadsc_t *msadscvp = NULL;
	machbstart_t *mbsp = &kmachbsp;

	msadscvp = (msadsc_t *)phyadr_to_viradr((adr_t)mbsp->mb_memmappadr);
	coremdnr = mbsp->mb_memmapnr;

	for (int i = 0; i < 10; ++i)
	{
		disp_one_msadsc(&msadscvp[i]);
	}

	for (u64_t i = coremdnr / 2; i < coremdnr / 2 + 10; ++i)
	{
		disp_one_msadsc(&msadscvp[i]);
	}

	for (u64_t i = coremdnr - 10; i < coremdnr; ++i)
	{
		disp_one_msadsc(&msadscvp[i]);
	}
	return;
}

//搜索一段内存地址空间所对应的msadsc_t结构
// msastart 	物理内存页开始地址
// msanr 		物理内存页数
// ocpystat 	开始地址
// ocpyend 		结束地址
u64_t search_segment_occupymsadsc(msadsc_t *msastart, u64_t msanr, u64_t ocpystat, u64_t ocpyend)
{
	u64_t mphyadr = 0, fsmsnr = 0;
	msadsc_t *fstatmp = NULL;
	for (u64_t mnr = 0; mnr < msanr; mnr++)
	{
		// 这里为啥要移动 左移12位， 
		// 因为这里实际只是52位（4k对齐， 低12位无效所以不存）
		// 左移12位才是真实地址
		if ((msastart[mnr].md_phyadrs.paf_padrs << PSHRSIZE) == ocpystat)	
		{
			//找出开始地址对应的第一个msadsc_t结构，就跳转到step1
			fstatmp = &msastart[mnr];
			goto step1;
		}
	}
step1:
	fsmsnr = 0;
	if (NULL == fstatmp)
	{
		return 0;
	}
	for (u64_t tmpadr = ocpystat; tmpadr < ocpyend; tmpadr += PAGESIZE, fsmsnr++)
	{
		//从开始地址对应的第一个msadsc_t结构开始设置，直到结束地址对应的最后一个masdsc_t结构
		mphyadr = fstatmp[fsmsnr].md_phyadrs.paf_padrs << PSHRSIZE;
		if (mphyadr != tmpadr)
		{
			return 0;
		}
		if (MF_MOCTY_FREE != fstatmp[fsmsnr].md_indxflgs.mf_mocty ||
			0 != fstatmp[fsmsnr].md_indxflgs.mf_uindx ||
			PAF_NO_ALLOC != fstatmp[fsmsnr].md_phyadrs.paf_alloc)
		{
			return 0;
		}
		fstatmp[fsmsnr].md_indxflgs.mf_mocty = MF_MOCTY_KRNL;		// 设置分配给了内核
		fstatmp[fsmsnr].md_indxflgs.mf_uindx++;						// 分配计数+1
		fstatmp[fsmsnr].md_phyadrs.paf_alloc = PAF_ALLOC;			// 设置分配位为1
	}
	u64_t ocpysz = ocpyend - ocpystat;
	//进行一些数据的正确性检查
	if ((ocpysz & 0xfff) != 0)		// 是否4k对齐
	{
		if (((ocpysz >> PSHRSIZE) + 1) != fsmsnr)
		{
			return 0;
		}
		return fsmsnr;
	}
	if ((ocpysz >> PSHRSIZE) != fsmsnr)
	{
		return 0;
	}
	return fsmsnr;
}

void test_schkrloccuymm(machbstart_t *mbsp, u64_t ocpystat, u64_t sz)
{
	msadsc_t *msadstat = (msadsc_t *)phyadr_to_viradr((adr_t)mbsp->mb_memmappadr);
	u64_t msanr = mbsp->mb_memmapnr;
	u64_t chkmnr = 0;
	u64_t chkadr = ocpystat;
	if ((sz & 0xfff) != 0)
	{
		chkmnr = (sz >> PSHRSIZE) + 1;
	}
	else
	{
		chkmnr = sz >> PSHRSIZE;
	}
	msadsc_t *fstatmp = NULL;
	for (u64_t mnr = 0; mnr < msanr; mnr++)
	{
		if ((msadstat[mnr].md_phyadrs.paf_padrs << PSHRSIZE) == ocpystat)
		{
			fstatmp = &msadstat[mnr];
			goto step1;
		}
	}
step1:
	if (fstatmp == NULL)
	{
		system_error("fstatmp NULL\n");
	}

	for (u64_t i = 0; i < chkmnr; i++, chkadr += PAGESIZE)
	{
		if (chkadr != fstatmp[i].md_phyadrs.paf_padrs << PSHRSIZE)
		{
			system_error("chkadr != err\n");
		}
		if (PAF_ALLOC != fstatmp[i].md_phyadrs.paf_alloc)
		{
			system_error("PAF_ALLOC err\n");
		}
		if (1 != fstatmp[i].md_indxflgs.mf_uindx)
		{
			system_error("mf_uindx err\n");
		}
		if (MF_MOCTY_KRNL != fstatmp[i].md_indxflgs.mf_mocty)
		{
			system_error("mf_olkty err\n");
		}
	}
	if (chkadr != (ocpystat + (chkmnr * PAGESIZE)))
	{
		system_error("test_schkrloccuymm err\n");
	}
	return;
}

bool_t search_krloccupymsadsc_core(machbstart_t *mbsp)
{
	u64_t retschmnr = 0;
	// 物理页开始地址转化为虚拟地址
	msadsc_t *msadstat = (msadsc_t *)phyadr_to_viradr((adr_t)mbsp->mb_memmappadr);
	u64_t msanr = mbsp->mb_memmapnr;
	retschmnr = search_segment_occupymsadsc(msadstat, msanr, 0, 0x1000);  // 0x1000 = 2^12 = 4k ,刚好一个物理页 
	if (0 == retschmnr)
	{
		return FALSE;
	}
	// mbsp->mb_krlinitstack & (~(0xfffUL)) 取得是高 52位，低12位全部之置为0，
	// TODO mbsp->mb_krlinitstack 作为结束地址的
	retschmnr = search_segment_occupymsadsc(msadstat, msanr, mbsp->mb_krlinitstack & (~(0xfffUL)), mbsp->mb_krlinitstack);
	if (0 == retschmnr)
	{
		return FALSE;
	}
	//搜索内核占用的内存页所对应msadsc_t结构
	// 内核映像地址， 直接到现在的mbsp->mb_nextwtpadr
	retschmnr = search_segment_occupymsadsc(msadstat, msanr, mbsp->mb_krlimgpadr, mbsp->mb_nextwtpadr);
	if (0 == retschmnr)
	{
		return FALSE;
	}
	//搜索映像文件占用的内存页所对应msadsc_t结构
	retschmnr = search_segment_occupymsadsc(msadstat, msanr, mbsp->mb_imgpadr, mbsp->mb_imgpadr + mbsp->mb_imgsz);
	if (0 == retschmnr)
	{
		return FALSE;
	}
	return TRUE;
}

//初始化搜索内核占用的内存页面
void init_search_krloccupymm(machbstart_t *mbsp)
{
	//实际初始化搜索内核占用的内存页面
	if (search_krloccupymsadsc_core(mbsp) == FALSE)
	{
		system_error("search_krloccupymsadsc_core fail\n");
	}
	return;
}
