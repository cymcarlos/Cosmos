/**********************************************************
        平台相关的宏定义文件platforms3c2440_t.h
***********************************************************
                彭东
**********************************************************/
#ifndef _PLATFORM_T_H
#define _PLATFORM_T_H
#include "bdvideo_t.h"


#ifdef CFG_X86_PLATFORM
#define CPUCORE_MAX 1
#define SDRAM_MAPVECTPHY_ADDR 0x30000000

#define KRNL_INRAM_START 0x30000000
#define LINK_VIRT_ADDR 0x30008000
#define LINK_LINE_ADDR 0x30008000
#define KERNEL_VIRT_ADDR 0x30008000
#define KERNEL_PHYI_ADDR 0x30008000
#define PAGE_TLB_DIR 0x30004000
#define PAGE_TLB_SIZE 4096
#define INIT_HEAD_STACK_ADDR 0x34000000

#define CPU_VECTOR_PHYADR 0x30000000
#define CPU_VECTOR_VIRADR 0


#define PTE_SECT_AP (3<<10)
#define PTE_SECT_DOMAIN (0<<5)
#define PTE_SECT_NOCW (0<<2)
#define PTE_SECT_BIT (2)

#define PLFM_ADRSPCE_NR 29

#define INTSRC_MAX 32

#define KRNL_MAP_VIRTADDRESS_SIZE 0x400000000
#define KRNL_VIRTUAL_ADDRESS_START 0xffff800000000000                   // 虚拟地址开始地址
#define KRNL_VIRTUAL_ADDRESS_END 0xffffffffffffffff
#define USER_VIRTUAL_ADDRESS_START 0                                    // 用户的虚拟空间开始地址
#define USER_VIRTUAL_ADDRESS_END 0x00007fffffffffff                     // 用户的虚拟空间结束地址
#define KRNL_MAP_PHYADDRESS_START 0
#define KRNL_MAP_PHYADDRESS_END 0x400000000                             // 物理地址结束地址
#define KRNL_MAP_PHYADDRESS_SIZE 0x400000000
#define KRNL_MAP_VIRTADDRESS_START KRNL_VIRTUAL_ADDRESS_START           // 虚拟地址开始地址
#define KRNL_MAP_VIRTADDRESS_END (KRNL_MAP_VIRTADDRESS_START+KRNL_MAP_VIRTADDRESS_SIZE)
#define KRNL_ADDR_ERROR 0xf800000000000


#define MBS_MIGC (u64_t)((((u64_t)'L')<<56)|(((u64_t)'M')<<48)|(((u64_t)'O')<<40)|(((u64_t)'S')<<32)|(((u64_t)'M')<<24)|(((u64_t)'B')<<16)|(((u64_t)'S')<<8)|((u64_t)'P'))

typedef struct s_MRSDP
{
    u64_t rp_sign;
    u8_t rp_chksum;
    u8_t rp_oemid[6];
    u8_t rp_revn;
    u32_t rp_rsdtphyadr;
    u32_t rp_len;
    u64_t rp_xsdtphyadr;
    u8_t rp_echksum;
    u8_t rp_resv[3];
}__attribute__((packed)) mrsdp_t;


// 二级引导收集信息的信息
typedef struct s_MACHBSTART
{
    u64_t   mb_migc;                        //LMOSMBSP//0
    u64_t   mb_chksum;
    u64_t   mb_krlinitstack;                // 内核栈地址
    u64_t   mb_krlitstacksz;                // 内核栈大小
    u64_t   mb_imgpadr;                     // 操作系统映像地址
    u64_t   mb_imgsz;                       // 操作系统映像大小
    u64_t   mb_krlimgpadr;                  // 内核映像地址
    u64_t   mb_krlsz;                       // 内核映像大小
    u64_t   mb_krlvec;
    u64_t   mb_krlrunmode;
    u64_t   mb_kalldendpadr;
    u64_t   mb_ksepadrs;
    u64_t   mb_ksepadre;
    u64_t   mb_kservadrs;
    u64_t   mb_kservadre;
    u64_t   mb_nextwtpadr;                      // 下一个4kb对齐的地址            
    u64_t   mb_bfontpadr;                       //操作系统字体地址
    u64_t   mb_bfontsz;                         //操作系统字体大小
    u64_t   mb_fvrmphyadr;                      // 机器显存地址
    u64_t   mb_fvrmsz;                          // 机器显存地址大小
    u64_t   mb_cpumode;                         // cpu工作模式
    u64_t   mb_memsz;                           // 机器内存大小
    u64_t   mb_e820padr;                        // 机器e820地址
    u64_t   mb_e820nr;                          // 数组元素个数
    u64_t   mb_e820sz;                          // 机器e820数组大小
    u64_t   mb_e820expadr;                      // 机器e820扩展地址
    u64_t   mb_e820exnr;                        // 扩展数组元素个数
    u64_t   mb_e820exsz;                        // 扩展机器e820数组大小
    u64_t   mb_memznpadr;                       // 内存分区信息开始地址 memarea_t数组的开始地址（物理地址）
    u64_t   mb_memznnr;                         // 内存分区数量    
    u64_t   mb_memznsz;                         // 信息所占大小
    u64_t   mb_memznchksum;
    u64_t   mb_memmappadr;                      // 物理内存页开始地址物理地址
    u64_t   mb_memmapnr;                        // 物理内存页页数
    u64_t   mb_memmapsz;                        // 内存页大小
    u64_t   mb_memmapchksum;
    u64_t   mb_pml4padr;                         //机器页表数据地址
    u64_t   mb_subpageslen;                      //机器页表个数
    u64_t   mb_kpmapphymemsz;                    // 操作系统映射空间
    u64_t   mb_ebdaphyadr;
    mrsdp_t mb_mrsdp;
    graph_t mb_ghparm;                            // 图形信息        
}__attribute__((packed)) machbstart_t;


#define MBSPADR ((machbstart_t*)(0x100000))       // 信息结构体开始地址


#define BFH_RW_R 1
#define BFH_RW_W 2

#define BFH_BUF_SZ 0x1000
#define BFH_ONERW_SZ 0x1000
#define BFH_RWONE_OK 1
#define BFH_RWONE_ER 2
#define BFH_RWALL_OK 3

#define FHDSC_NMAX 192                     // 文件名最大长度
#define FHDSC_SZMAX 256
#define MDC_ENDGIC 0xaaffaaffaaffaaff
#define MDC_RVGIC 0xffaaffaaffaaffaa

#define MLOSDSC_OFF (0x1000)
#define RAM_USABLE 1
#define RAM_RESERV 2
#define RAM_ACPIREC 3
#define RAM_ACPINVS 4
#define RAM_AREACON 5
typedef struct s_e820{
    u64_t saddr;    /* start of memory segment8  地址开始地址*/             
    u64_t lsize;    /* size of memory segment8   地址长度*/
    u32_t type;    /* type of memory segment     地址类型4*/
}__attribute__((packed)) e820map_t;

typedef struct s_fhdsc{
    u64_t fhd_type;                         //文件类型    
    u64_t fhd_subtype;                      //文件子类型    
    u64_t fhd_stuts;                        //文件状态    
    u64_t fhd_id;                           //文件id    
    u64_t fhd_intsfsoff;                    //文件在映像文件位置开始偏移    
    u64_t fhd_intsfend;                     //文件在映像文件的结束偏移    
    u64_t fhd_frealsz;                      //文件实际大小    
    u64_t fhd_fsum;                         //文件校验和    
    char   fhd_name[FHDSC_NMAX];            //文件名
    }fhdsc_t;


//映像文件头描述符
typedef struct s_mlosrddsc
{
    u64_t mdc_mgic;                         //映像文件标识
    u64_t mdc_sfsum;                        //未使用
    u64_t mdc_sfsoff;                       //未使用
    u64_t mdc_sfeoff;                       //未使用
    u64_t mdc_sfrlsz;                       //未使用
    u64_t mdc_ldrbk_s;                      //映像文件中二级引导器的开始偏移地址  
    u64_t mdc_ldrbk_e;                      //映像文件中二级引导器的结束偏移地址
    u64_t mdc_ldrbk_rsz;                    //映像文件中二级引导器的实际大小
    u64_t mdc_ldrbk_sum;                    //映像文件中二级引导器的校验和------类似奇偶校验
    u64_t mdc_fhdbk_s;                      //映像文件中文件头描述的开始偏移----
    u64_t mdc_fhdbk_e;                      //映像文件中文件头描述的结束偏移
    u64_t mdc_fhdbk_rsz;                    //映像文件中文件头描述的实际大小
    u64_t mdc_fhdbk_sum;                    //映像文件中文件头描述的校验和
    u64_t mdc_filbk_s;                      //映像文件中文件数据的开始偏移
    u64_t mdc_filbk_e;                      //映像文件中文件数据的结束偏移
    u64_t mdc_filbk_rsz;                    //映像文件中文件数据的实际大小
    u64_t mdc_filbk_sum;                    //映像文件中文件数据的校验和
    u64_t mdc_ldrcodenr;                    //映像文件中二级引导器的文件头描述符的索引号
    u64_t mdc_fhdnr;                        //映像文件中文件头描述符有多少个
    u64_t mdc_filnr;                        //映像文件中文件头有多少个
    u64_t mdc_endgic;                       //映像文件结束标识
    u64_t mdc_rv;                           //映像文件版本
}mlosrddsc_t;



#endif



#endif // PLATFORM_S3C2440_T_H
