/**********************************************************
        物理内存管理器初始化文件memmgrinit_t.h
***********************************************************
                彭东 
**********************************************************/
#ifndef _MEMMGRINIT_T_H
#define _MEMMGRINIT_T_H

typedef struct s_MEMMGROB
{
	list_h_t mo_list;
	spinlock_t mo_lock;					//保护自身自旋锁
	uint_t mo_stus;						//状态
	uint_t mo_flgs;						//标志
	sem_t mo_sem;						// CYM_TODO 信号量？
	u64_t mo_memsz;						//内存大小
	u64_t mo_maxpages;					//内存最大页面数
	u64_t mo_freepages;					//内存最大空闲页面数
	u64_t mo_alocpages;					//已经分配的页数
	u64_t mo_resvpages;					//内存最大分配页面数
	u64_t mo_horizline;					//内存分配水位线
	phymmarge_t* mo_pmagestat;			// 机器e820扩展地址 转为虚拟地址
	u64_t mo_pmagenr;					// 机器e820扩展长度
	msadsc_t* mo_msadscstat;			// 物理内存页结构开始地址（虚拟地址） 
	u64_t mo_msanr;						// 物理内存页页数
	memarea_t* mo_mareastat;			// 内存分区信息结构体开始地址 memarea_t数组的开始地址(虚拟地址)
	u64_t mo_mareanr;					//  内存分区数量    
	kmsobmgrhed_t mo_kmsobmgr;
	void* mo_privp;
	void* mo_extp;
}memmgrob_t;
#endif


