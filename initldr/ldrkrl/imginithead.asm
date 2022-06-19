MBT_HDR_FLAGS	EQU 0x00010003			     ; 魔法数字
MBT_HDR_MAGIC	EQU 0x1BADB002				 ; 魔法数字
MBT2_MAGIC		EQU 0xe85250d6				 ; 魔法数字
global _start
extern inithead_entry
[section .text]								 ; 段text 一般是代码，			
[bits 32]									 ;  32位
_start:
	jmp _entry								 ; 入口		
align 4										 ; 对其4 地址要给4整除， 因为 dd是双字， 刚好4个字节
mbt_hdr:								     ; grub1
	dd MBT_HDR_MAGIC				
	dd MBT_HDR_FLAGS
	dd -(MBT_HDR_MAGIC+MBT_HDR_FLAGS)			
	dd mbt_hdr
	dd _start
	dd 0
	dd 0
	dd _entry
	;
	; multiboot header
	;
ALIGN 8
mbhdr:										 ; grub2两种格式
	DD	0xE85250D6
	DD	0
	DD	mhdrend - mbhdr
	DD	-(0xE85250D6 + 0 + (mhdrend - mbhdr))
	DW	2, 0
	DD	24
	DD	mbhdr
	DD	_start
	DD	0
	DD	0
	DW	3, 0
	DD	12
	DD	_entry 
	DD      0  
	DW	0, 0
	DD	8
mhdrend:

_entry:
	cli								    ;关闭中断
	in al, 0x70
	or al, 0x80	
	out 0x70,al							;关掉不可屏蔽中断

	lgdt [GDT_PTR]						;加载GDT地址到GDTR寄存器
	jmp dword 0x8 :_32bits_mode			;长跳转到_32bits_mode

_32bits_mode:
	mov ax, 0x10
	mov ds, ax
	mov ss, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	xor eax,eax
	xor ebx,ebx
	xor ecx,ecx
	xor edx,edx
	xor edi,edi
	xor esi,esi
	xor ebp,ebp
	xor esp,esp
	mov esp,0x7c00						;0x7c00代表什么地址？栈顶
	call inithead_entry
	jmp 0x200000						;initldrkrl.bin 放的地址 也是一个魔法数字了



GDT_START:
knull_dsc: dq 0
kcode_dsc: dq 0x00cf9e000000ffff
kdata_dsc: dq 0x00cf92000000ffff
k16cd_dsc: dq 0x00009e000000ffff			 ; 代码段描述符
k16da_dsc: dq 0x000092000000ffff		     ; 数据段描述符
GDT_END:
GDT_PTR:
GDTLEN	dw GDT_END-GDT_START-1	;GDT界限
GDTBASE	dd GDT_START
