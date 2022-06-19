;*****************************************************************************
;*		内核初始化入口文件init_entry.asm				    *
;*				彭东	     
;*****************************************************************************
%define MBSP_ADR 0x100000										; 信息结构体的开始地址
%define IA32_EFER 0C0000080H									;msr 的其中一个 寄存器地址    这些地址都是手册里有的
%define MSR_IA32_MISC_ENABLE 0x000001a0							;msr 的其中另外一个 寄存器地址 这些地址都是手册里有的
%define PML4T_BADR 0x1000000      ;0x20000;0x5000				KINITPAGE_PHYADR 顶级页目录开始地址 
%define KRLVIRADR 0xffff800000000000							; 虚拟地址开始地址
%define KINITSTACK_OFF 16
global _start
global x64_GDT													; global 对其他东西可见
global kernel_pml4
extern hal_start

[section .start.text]
[BITS 32]
_start:
	cli
	mov ax,0x10
	mov ds,ax
	mov es,ax
	mov ss,ax
	mov fs,ax
	mov gs,ax
    lgdt [eGdtPtr]      			;加载eGdtPtr  64位的GDTR寄存器
;开启 PAE
    mov eax, cr4
    bts eax, 5                      ; CR4.PAE = 1
    mov cr4, eax
    mov eax, PML4T_BADR				; 加载MMU , 
    mov cr3, eax					; 将顶级页目录开始地址 放入cr3
;开启SSE
	mov eax, cr4
    bts eax, 9                      ; CR4.OSFXSR = 1
    bts eax, 10                      ; CR4.OSXMMEXCPT = 1
    mov cr4, eax	
;开启 64bits long-mode
    mov ecx, IA32_EFER
    rdmsr							; 将IA32_EFER地址的msr寄存器读取到 EDX:EAX 中 			
    bts eax, 8                      ; IA32_EFER.LME =1
    wrmsr							; 回写IA32_EFER地址的msr寄存器
	
	mov ecx, MSR_IA32_MISC_ENABLE   ; 另外一个msr寄存器
	rdmsr							; 读
	btr eax, 6                      ; L3Cache =1
    wrmsr							; 回写
	
;开启 PE 和 paging
    mov eax, cr0
    bts eax, 0                      ; CR0.PE =1
    bts eax, 31

;开启 CACHE       
    btr eax,29		;CR0.NW=0
    btr eax,30		;CR0.CD=0  CACHE
        
    mov cr0, eax                    ; IA32_EFER.LMA = 1
    jmp 08:entry64			 		; todo  这里为啥是08
[BITS 64]
entry64:
	mov ax,0x10						; todo 这里为啥是弄个0x10
	mov ds,ax
	mov es,ax
	mov ss,ax
	mov fs,ax
	mov gs,ax
	xor rax,rax
	xor rbx,rbx
	xor rbp,rbp
	xor rcx,rcx
	xor rdx,rdx
	xor rdi,rdi
	xor rsi,rsi
	xor r8,r8
	xor r9,r9
	xor r10,r10
	xor r11,r11
	xor r12,r12
	xor r13,r13
	xor r14,r14
	xor r15,r15
    mov rbx,MBSP_ADR				;			// 初步信息结构体开始地址
    mov rax,KRLVIRADR				; 			// 虚拟地址开始地址
    mov rcx,[rbx+KINITSTACK_OFF]	;           16 * 4  = 64bit， MBSP_ADR跳开魔法数字后的开始地址
    add rax,rcx
    xor rcx,rcx
	xor rbx,rbx
	mov rsp,rax
	push 0
	push 0x8
    mov rax,hal_start   			;调用内核主函数
	push rax
    dw 0xcb48
    jmp $		 		;跳转到当前地址， 就是死循环

		
[section .start.data]
[BITS 32]
ex64_GDT:
enull_x64_dsc:	dq 0	
ekrnl_c64_dsc:  dq 0x0020980000000000           ; 64-bit 内核代码段
ekrnl_d64_dsc:  dq 0x0000920000000000           ; 64-bit 内核数据段   
euser_c64_dsc:  dq 0x0020f80000000000           ; 64-bit 用户代码段
euser_d64_dsc:  dq 0x0000f20000000000           ; 64-bit 用户数据段
eGdtLen			equ	$ - enull_x64_dsc			; GDT长度
eGdtPtr:		dw eGdtLen - 1					; GDT界限
				dq ex64_GDT

[section .start.data.pml4]

stack:
	times 1024 dq 0

kernel_pml4:	
	times 512*10 dq 0
