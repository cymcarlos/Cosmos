/**********************************************************
        物理内存空间数组文件msadsc_t.h
***********************************************************
                彭东
**********************************************************/
#ifndef _MSADSC_T_H
#define _MSADSC_T_H

#define PAGPHYADR_SZLSHBIT (12)
#define MSAD_PAGE_MAX (8)
#define MSA_SIZE (1 << PAGPHYADR_SZLSHBIT)

// 挂入链表类型
#define MF_OLKTY_INIT (0)				// 初始化标识
#define MF_OLKTY_ODER (1)
#define MF_OLKTY_BAFH (2)
#define MF_OLKTY_TOBJ (3)

#define MF_LSTTY_LIST (0)

// 分配类型
#define MF_MOCTY_FREE (0)				// 分配类型空闲
#define MF_MOCTY_KRNL (1)				// 分配类型内核
#define MF_MOCTY_USER (2)				// 分配类型用户

#define MF_MRV1_VAL (0)
#define MF_UINDX_INIT (0)
#define MF_UINDX_MAX (0xffffff)

//内存区类型 
#define MF_MARTY_INIT (0)				 // 初始化用
#define MF_MARTY_HWD (1)
#define MF_MARTY_KRL (2)
#define MF_MARTY_PRC (3)
#define MF_MARTY_SHD (4)

//内存空间地址描述符标志
typedef struct s_MSADFLGS{    
	u32_t mf_olkty:2;    //挂入链表的类型   
	u32_t mf_lstty:1;    //是否挂入链表       
	u32_t mf_mocty:2;    //分配类型，被谁占用了，内核还是应用或者空闲     
	u32_t mf_marty:3;    //属于哪个区     
	u32_t mf_uindx:24;   //分配计数 
}__attribute__((packed)) msadflgs_t; 

// 分配位
#define  PAF_NO_ALLOC (0)
#define  PAF_ALLOC (1)

#define  PAF_NO_SHARED (0)				//共享位-没有共享
#define  PAF_NO_SWAP (0)				//交换位 没有交换		
#define  PAF_NO_CACHE (0)
#define  PAF_NO_KMAP (0)
#define  PAF_NO_LOCK (0)
#define  PAF_NO_DIRTY (0)
#define  PAF_NO_BUSY (0)
#define  PAF_RV2_VAL (0)
#define  PAF_INIT_PADRS (0)
//物理地址和标志  
typedef struct s_PHYADRFLGS
{
    u64_t paf_alloc:1;     //分配位   实际只是占用1
    u64_t paf_shared:1;    //共享位
    u64_t paf_swap:1;      //交换位
    u64_t paf_cache:1;     //缓存位
    u64_t paf_kmap:1;      //映射位
    u64_t paf_lock:1;      //锁定位
    u64_t paf_dirty:1;     //脏位
    u64_t paf_busy:1;      //忙位
    u64_t paf_rv2:4;       //保留位
    u64_t paf_padrs:52;    //页物理地址位
}__attribute__((packed)) phyadrflgs_t;





//内存空间地址描述符
typedef struct s_MSADSC{    
	list_h_t md_list;           //链表    
	spinlock_t md_lock;         //保护自身的自旋锁    
	msadflgs_t md_indxflgs;     //内存空间地址描述符标志    
	phyadrflgs_t md_phyadrs;    //物理地址和标志    
	void* md_odlink;            //相邻且相同大小msadsc的指针
}__attribute__((packed)) msadsc_t;




#endif