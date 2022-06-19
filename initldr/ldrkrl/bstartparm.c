#include "cmctl.h"
void testm2m()
{
    u8_t *s = (u8_t *)0x400000;
    u8_t *d = (u8_t *)(0x400000 - 1);
    for (u8_t i = 0; i < 10; i++)
    {
        s[i] = i;
    }
    for (u8_t j = 0; j < 10; j++)
    {
        kprint("s[%d]:%d\n", j, s[j]);
    }
    m2mcopy(s, d, 10);
    for (u8_t k = 0; k < 10; k++)
    {
        kprint("d[%d]:%d\n", k, d[k]);
    }
    die(0);
    return;
}

void disp_mbsp(machbstart_t *mbsp)
{
    kprint("mbsp->mb_imgpadr:%x\n", mbsp->mb_imgpadr);
    kprint("mbsp->mb_imgsz:%x\n", mbsp->mb_imgsz);
    kprint("mbsp->mb_krlimgpadr:%x\n", mbsp->mb_krlimgpadr);
    kprint("mbsp->mb_krlimgsz:%x\n", mbsp->mb_krlsz);
    kprint("mbsp->mb_e820padr:%x\n", mbsp->mb_e820padr);
    kprint("mbsp->mb_e820nr:%x\n", mbsp->mb_e820nr);
    kprint("mbsp->mb_e820sz:%x\n", mbsp->mb_e820sz);
    kprint("mbsp->mb_nextwtpadr:%x\n", mbsp->mb_nextwtpadr);
    kprint("mbsp->mb_kalldendpadr:%x\n", mbsp->mb_kalldendpadr);
    kprint("mbsp->mb_pml4padr:%x\n", mbsp->mb_pml4padr);
    kprint("mbsp->mb_subpageslen:%x\n", mbsp->mb_subpageslen);
    kprint("mbsp->mb_kpmapphymemsz:%x\n", mbsp->mb_kpmapphymemsz);
    kprint("mbsp->mb_cpumode:%x\n", mbsp->mb_cpumode);
    kprint("mbsp->mb_memsz:%x\n", mbsp->mb_memsz);
    kprint("mbsp->mb_krlinitstack:%x\n", mbsp->mb_krlinitstack);
    kprint("mbsp->mb_krlitstacksz:%x\n", mbsp->mb_krlitstacksz);
    kprint("mbsp->mb_bfontpadr:%x\n", mbsp->mb_bfontpadr);
    kprint("mbsp->mb_bfontsz:%x\n", mbsp->mb_bfontsz);
    die(0);
    return;
}

void init_bstartparm()
{
    machbstart_t *mbsp = MBSPADR;    // 给一个地址
    machbstart_t_init(mbsp);         // 初始化
    init_chkcpu(mbsp);               // 初始化，检查cpu
    init_mem(mbsp);                  // 内存布局信息  先放在 ETYBAK_ADR这里
    if (0 == get_wt_imgfilesz(mbsp))
    {
        kerror("imgfilesz 0");
    }
    init_krlinitstack(mbsp);          // 初始化内核栈
    init_krlfile(mbsp);               // 放置内核文件, 放置好了，返回就可以执行init_entry.asm的start
    init_defutfont(mbsp);             // 放字体文件
    init_meme820(mbsp);               // // 干的是移动内存数组的事， 应该是低位地址后续会覆盖的问题
    init_bstartpages(mbsp);           // mmu分页初始化
    init_graph(mbsp);
    die(0x400);
    return;
}

 // 初始化machbstart_t结构体
void machbstart_t_init(machbstart_t *initp)
{
    memset(initp, 0, sizeof(machbstart_t));     // 设置为0
    initp->mb_migc = MBS_MIGC;                  // 设置一个字符串LMOSMBSP 属于老师的浪漫吧
    return;
}

int adrzone_is_ok(u64_t sadr, u64_t slen, u64_t kadr, u64_t klen)
{
    if (kadr >= sadr && kadr <= (sadr + slen))
    {
        return -1;
    }
    /*if(kadr<=sadr&&((kadr+klen)>=(sadr+slen)))
    {
        return -2;
    }*/
    if (kadr <= sadr && ((kadr + klen) >= sadr))
    {
        return -2;
    }

    return 0;
}

//  检查地址是否ok
int chkadr_is_ok(machbstart_t *mbsp, u64_t chkadr, u64_t cksz)
{
    //u64_t len=chkadr+cksz;
    if (adrzone_is_ok((mbsp->mb_krlinitstack - mbsp->mb_krlitstacksz), mbsp->mb_krlitstacksz, chkadr, cksz) != 0)
    {
        return -1;
    }
    if (adrzone_is_ok(mbsp->mb_imgpadr, mbsp->mb_imgsz, chkadr, cksz) != 0)
    {
        return -2;
    }
    if (adrzone_is_ok(mbsp->mb_krlimgpadr, mbsp->mb_krlsz, chkadr, cksz) != 0)
    {
        return -3;
    }
    if (adrzone_is_ok(mbsp->mb_bfontpadr, mbsp->mb_bfontsz, chkadr, cksz) != 0)
    {
        return -4;
    }
    if (adrzone_is_ok(mbsp->mb_e820padr, mbsp->mb_e820sz, chkadr, cksz) != 0)
    {
        return -5;
    }
    if (adrzone_is_ok(mbsp->mb_memznpadr, mbsp->mb_memznsz, chkadr, cksz) != 0)
    {
        return -6;
    }
    if (adrzone_is_ok(mbsp->mb_memmappadr, mbsp->mb_memmapsz, chkadr, cksz) != 0)
    {
        return -7;
    }
    return 0;
}
