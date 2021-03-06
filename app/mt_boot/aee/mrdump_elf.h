/* Copyright Statement:
*
* This software/firmware and related documentation ("MediaTek Software") are
* protected under relevant copyright laws. The information contained herein
* is confidential and proprietary to MediaTek Inc. and/or its licensors.
* Without the prior written permission of MediaTek inc. and/or its licensors,
* any reproduction, modification, use or disclosure of MediaTek Software,
* and information contained herein, in whole or in part, shall be strictly prohibited.
*/
/* MediaTek Inc. (C) 2016. All rights reserved.
*
* BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
* THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
* RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
* AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
* NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
* SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
* SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
* THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
* THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
* CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
* SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
* STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
* CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
* AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
* OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
* MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*/

#if !defined(__MRDUMP_ELF_H__)
#define __MRDUMP_ELF_H__

#include <stdint.h>

#define ELFMAG      "\177ELF"
#define SELFMAG     4

#define CORE_STR "CORE"

#ifndef ELF_CORE_EFLAGS
#define ELF_CORE_EFLAGS 0
#endif

#define EI_NIDENT   16

#define EI_CLASS    4
#define EI_DATA     5
#define EI_VERSION  6
#define EI_OSABI    7
#define EI_PAD      8

#define EM_ARM 40
#define EM_AARCH64 183

#define ET_CORE 4

#define PT_LOAD 1
#define PT_NOTE 4

#define ELFCLASS32 1
#define ELFCLASS64 2

#define NT_PRSTATUS 1
#define NT_PRFPREG 2
#define NT_PRPSINFO 3
/* MRDUMP support note type */
#define NT_MRDUMP_MACHDESC 0xAEE0
#define NT_MRDUMP_CBLOCK 0xBEEF


#define PF_R 0x4
#define PF_W 0x2
#define PF_X 0x1

#define ELFOSABI_NONE 0

#define EV_CURRENT 1

#define ELFDATA2LSB 1

#define ELF_PRARGSZ 80

#define MRDUMP_TYPE_FULL_MEMORY 0
#define MRDUMP_TYPE_KERNEL_1 1
#define MRDUMP_TYPE_KERNEL_2 2

#define MRDUMP_TYPE_MASK 0x3

typedef uint32_t Elf32_Addr;
typedef uint16_t Elf32_Half;
typedef uint32_t Elf32_Off;
typedef int32_t Elf32_Sword;
typedef uint32_t Elf32_Word;

typedef uint64_t Elf64_Addr;
typedef uint16_t Elf64_Half;
typedef uint64_t Elf64_Off;
typedef uint32_t Elf64_Word;
typedef uint64_t Elf64_Xword;

struct elf_siginfo {
	int si_signo;
	int si_code;
	int si_errno;
};

struct __attribute__((__packed__)) elf_mrdump_machdesc {
	uint32_t flags;
	uint32_t nr_cpus;

	uint64_t phys_offset;
	uint64_t total_memory;

	uint64_t page_offset;
	uint64_t high_memory;

	uint64_t kimage_vaddr;

	uint64_t modules_start;
	uint64_t modules_end;

	uint64_t vmalloc_start;
	uint64_t vmalloc_end;

	uint64_t master_page_table;

	uint64_t dfdmem_pa;
};

#define ELF_ARM_NGREGS 18
typedef uint32_t elf_arm_gregset_t[ELF_ARM_NGREGS];

typedef struct elf32_hdr {
	unsigned char e_ident[EI_NIDENT];
	Elf32_Half e_type;
	Elf32_Half e_machine;
	Elf32_Word e_version;
	Elf32_Addr e_entry;
	Elf32_Off e_phoff;
	Elf32_Off e_shoff;
	Elf32_Word e_flags;
	Elf32_Half e_ehsize;
	Elf32_Half e_phentsize;
	Elf32_Half e_phnum;
	Elf32_Half e_shentsize;
	Elf32_Half e_shnum;
	Elf32_Half e_shstrndx;
} Elf64_Ehdr;

typedef struct elf32_phdr {
	Elf32_Word p_type;
	Elf32_Off p_offset;
	Elf32_Addr p_vaddr;
	Elf32_Addr p_paddr;
	Elf32_Word p_filesz;
	Elf32_Word p_memsz;
	Elf32_Word p_flags;
	Elf32_Word p_align;
} Elf32_Phdr;

typedef struct elf32_note {
	Elf32_Word   n_namesz;       /* Name size */
	Elf32_Word   n_descsz;       /* Content size */
	Elf32_Word   n_type;         /* Content type */
} Elf_Nhdr;

struct elf32_timeval {
	int32_t tv_sec;
	int32_t tv_usec;
};

struct elf32_arm_prstatus {
	struct elf_siginfo pr_info;
	short pr_cursig;
	unsigned long pr_sigpend;
	unsigned long pr_sighold;

	int32_t pr_pid;
	int32_t pr_ppid;
	int32_t pr_pgrp;

	int32_t pr_sid;
	struct elf32_timeval pr_utime;
	struct elf32_timeval pr_stime;
	struct elf32_timeval pr_cutime;
	struct elf32_timeval pr_cstime;

	elf_arm_gregset_t pr_reg;

	int pr_fpvalid;
};

struct elf32_prpsinfo {
	char pr_state;
	char pr_sname;
	char pr_zomb;
	char pr_nice;
	unsigned long pr_flag;

	uint16_t pr_uid;
	uint16_t pr_gid;

	int32_t pr_pid;
	int32_t pr_ppid;
	int32_t pr_pgrp;
	int32_t pr_sid;

	char pr_fname[16];
	char pr_psargs[ELF_PRARGSZ];
};

#define ELF_ARM64_NGREGS 34
typedef uint64_t elf_arm64_gregset_t[ELF_ARM64_NGREGS];

typedef struct elf64_hdr {
	unsigned char e_ident[EI_NIDENT];
	Elf64_Half e_type;
	Elf64_Half e_machine;
	Elf64_Word e_version;
	Elf64_Addr e_entry;
	Elf64_Off e_phoff;
	Elf64_Off e_shoff;
	Elf64_Word e_flags;
	Elf64_Half e_ehsize;
	Elf64_Half e_phentsize;
	Elf64_Half e_phnum;
	Elf64_Half e_shentsize;
	Elf64_Half e_shnum;
	Elf64_Half e_shstrndx;
} Elf64_ehdr;

typedef struct elf64_phdr {
	Elf64_Word p_type;
	Elf64_Word p_flags;
	Elf64_Off p_offset;
	Elf64_Addr p_vaddr;
	Elf64_Addr p_paddr;
	Elf64_Xword p_filesz;
	Elf64_Xword p_memsz;
	Elf64_Xword p_align;
} Elf64_phdr;

typedef struct elf64_note {
	Elf64_Word    n_namesz;   /* Name size */
	Elf64_Word    n_descsz;   /* Content size */
	Elf64_Word    n_type;     /* Content type */
} Elf64_nhdr;

struct elf_timeval64 {
	int64_t tv_sec;
	int64_t tv_usec;
};

struct elf_arm64_prstatus64 {
	struct elf_siginfo pr_info;
	short pr_cursig;
	uint64_t pr_sigpend;
	uint64_t pr_sighold;

	int32_t pr_pid;
	int32_t pr_ppid;
	int32_t pr_pgrp;

	int32_t pr_sid;
	struct elf_timeval64 pr_utime;
	struct elf_timeval64 pr_stime;
	struct elf_timeval64 pr_cutime;
	struct elf_timeval64 pr_cstime;

	elf_arm64_gregset_t pr_reg;

	int pr_fpvalid;
};

struct elf_prpsinfo64 {
	char pr_state;
	char pr_sname;
	char pr_zomb;
	char pr_nice;
	uint64_t pr_flag;

	uint32_t pr_uid;
	uint32_t pr_gid;

	int32_t pr_pid;
	int32_t pr_ppid;
	int32_t pr_pgrp;
	int32_t pr_sid;

	char pr_fname[16];
	char pr_psargs[ELF_PRARGSZ];
};

#endif /* __MRDUMP_ELF_H__ */
