/*
 * mmu_dump.c - Bare metal ARMv7 translation table dumper
 * Copyright 2014 Yifan Lu
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <string.h>
#include <taihen.h>
#include <psp2kern/kernel/modulemgr.h>
#include <psp2kern/kernel/threadmgr.h>
#include <psp2kern/kernel/sysmem.h>
#include <psp2kern/io/fcntl.h>

static int afe;

#define DUMP_PATH "ux0:dump/"
#define LOG_FILE DUMP_PATH "kplugin_log.txt"

static void log_reset();
static void dump_reset();
static void log_write(const char *buffer, size_t length);

#define LOG(...) \
	do { \
		char buffer[256]; \
		snprintf(buffer, sizeof(buffer), ##__VA_ARGS__); \
		log_write(buffer, strlen(buffer)); \
} while (0)

void _start() __attribute__ ((weak, alias ("module_start")));

static unsigned int pa2va(unsigned int pa)
{
    unsigned int va;
    unsigned int vaddr;
    unsigned int paddr;
    unsigned int i;

    va = 0;
    for (i = 0; i < 0x100000; i++)
    {
        vaddr = i << 12;
        __asm__("mcr p15,0,%1,c7,c8,0\n\t"
                "mrc p15,0,%0,c7,c4,0\n\t" : "=r" (paddr) : "r" (vaddr));
        if ((pa & 0xFFFFF000) == (paddr & 0xFFFFF000))
        {
            va = vaddr + (pa & 0xFFF);
            break;
        }
    }
    return va;
}

static void mmu_get_perms(int ap2, int ap1, int *ur, int *uw, int *pr, int *pw)
{
    /* AFE enabled, use simple permissions */
    if (afe)
    {
        *ur = ap1 > 1;
        *uw = !ap2 && ap1 > 1;
        *pr = 1;
        *pw = !ap2 && ap1 < 2;
    }
    else
    {
        *pw = (!ap2 && ap1);
        *pr = *pw || ap1;
        *ur = ap1 > 1;
        *uw = !ap2 && ap1 == 3;
    }
}

static void mmu_dump_pages(unsigned int vaddr, unsigned int entry)
{
    int xn;
    int ng;
    int s;
    int ap2;
    int ap1;
    int pr;
    int pw;
    int ur;
    int uw;
    unsigned int paddr;
    SceUID fd;

    if ((entry & 0x3) == 0x1) /* large page */
    {
        xn = entry & 0x8000;
        ng = entry & 0x800;
        s = entry & 0x400;
        ap2 = entry & 0x200;
        ap1 = (entry >> 4) & 3;
        mmu_get_perms(ap2, ap1, &ur, &uw, &pr, &pw);
        paddr = entry & 0xFFFF0000;
	if( paddr >= 0x40201000 && paddr < 0x5FD00000 && paddr != 0x44C20000 && paddr != 0x44C30000 && paddr != 0x443C0000){
		
		if( (paddr-0x44300000) <= (0x44400000-0x44300000) ){
				LOG("-[0x%08X] %s Not Dumpable PA:0x%08X NG:%d SH:%d UR:%d UW:%d PR:%d PW:%d XN:%d\n", vaddr, "Sm Page  ", paddr, !!ng, !!s, !!ur, !!uw, !!pr, !!pw, !!xn);
			}else{
				LOG("-[0x%08X] %s Dumpable PA:0x%08X NG:%d SH:%d UR:%d UW:%d PR:%d PW:%d XN:%d\n", vaddr, "Lg Page  ", paddr, !!ng, !!s, !!ur, !!uw, !!pr, !!pw, !!xn);
				
				fd = ksceIoOpen("ux0:dump/memory.bin",SCE_O_WRONLY | SCE_O_CREAT | SCE_O_APPEND, 6);
				ksceIoWrite(fd, (void*) vaddr, 0x1000);
				ksceIoClose(fd);
			}
            
			
        }
        else{
            LOG("-[0x%08X] %s Not Dumpable PA:0x%08X NG:%d SH:%d UR:%d UW:%d PR:%d PW:%d XN:%d\n", vaddr, "Lg Page  ", paddr, !!ng, !!s, !!ur, !!uw, !!pr, !!pw, !!xn);
        }
    }
    else if ((entry & 0x2)) /* small page */
    {
        xn = entry & 1;
        ng = entry & 0x800;
        s = entry & 0x400;
        ap2 = entry & 0x200;
        ap1 = (entry >> 4) & 3;
        mmu_get_perms(ap2, ap1, &ur, &uw, &pr, &pw);
        paddr = entry & 0xFFFFF000;
		//0x4434C000
        if( paddr >= 0x40201000 && paddr < 0x5FD00000){
			if( (paddr-0x47D80000) <= (0x47D90000-0x47D80000) ){
				LOG("-[0x%08X] %s Not Dumpable PA:0x%08X NG:%d SH:%d UR:%d UW:%d PR:%d PW:%d XN:%d\n", vaddr, "Sm Page  ", paddr, !!ng, !!s, !!ur, !!uw, !!pr, !!pw, !!xn);
			}else if ( (paddr-0x44C09000) <= (0x44C1A000-0x44C09000) ){
				LOG("-[0x%08X] %s Not Dumpable PA:0x%08X NG:%d SH:%d UR:%d UW:%d PR:%d PW:%d XN:%d\n", vaddr, "Sm Page  ", paddr, !!ng, !!s, !!ur, !!uw, !!pr, !!pw, !!xn);
			}else if ( (paddr-0x44300000) <= (0x44400000-0x44300000) ){
				LOG("-[0x%08X] %s Not Dumpable PA:0x%08X NG:%d SH:%d UR:%d UW:%d PR:%d PW:%d XN:%d\n", vaddr, "Sm Page  ", paddr, !!ng, !!s, !!ur, !!uw, !!pr, !!pw, !!xn);
			}else{
				LOG("-[0x%08X] %s Dumpable PA:0x%08X NG:%d SH:%d UR:%d UW:%d PR:%d PW:%d XN:%d\n", vaddr, "Sm Page  ", paddr, !!ng, !!s, !!ur, !!uw, !!pr, !!pw, !!xn);
				
				fd = ksceIoOpen("ux0:dump/memory.bin",SCE_O_WRONLY | SCE_O_CREAT | SCE_O_APPEND, 6);
				ksceIoWrite(fd, (void*) vaddr, 0x1000);
				ksceIoClose(fd);
				
            }
        }
        else{
            LOG("-[0x%08X] %s Not Dumpable PA:0x%08X NG:%d SH:%d UR:%d UW:%d PR:%d PW:%d XN:%d\n", vaddr, "Sm Page  ", paddr, !!ng, !!s, !!ur, !!uw, !!pr, !!pw, !!xn);
        }
    }
    else
    {
        LOG("-[0x%08X] %s\n", vaddr, "Unmapped ");
    }
}

static void mmu_dump_sections(unsigned int vaddr, unsigned int entry)
{
    int ns;
    int ss;
    int ng;
    int s;
    int ap1;
    int ap2;
    int domain;
    int xn;
    int pr;
    int pw;
    int ur;
    int uw;
    unsigned int paddr;
    unsigned int i;
    unsigned int *tbl;
    

    if ((entry & 0x3) == 2) /* section or supersection */
    {
        ns = entry & 0x80000;
        ss = entry & 0x40000;
        ng = entry & 0x20000;
        s = entry & 0x10000;
        ap2 = entry & 0x8000;
        ap1 = (entry >> 10) & 3;
        domain = (entry >> 5) & 15;
        xn = entry & 0x10;
        mmu_get_perms(ap2, ap1, &ur, &uw, &pr, &pw);
        paddr = ss ? entry & 0xFF000000 : entry & 0xFFF00000;

        LOG("[0x%08X] %s PA:0x%08X NG:%d SH:%d UR:%d UW:%d PR:%d PW:%d XN:%d NS:%d DOM:%02X\n", vaddr, ss ? "S-Section " : "Section   ", paddr, !!ng, !!s, !!ur, !!uw, !!pr, !!pw, !!xn, !!ns, domain);
    }
    else if ((entry & 0x3) == 1) /* page table */
    {
        domain = (entry >> 5) & 15;
        ns = entry & 8;
        paddr = entry & 0xFFFFFC00;
        tbl = (unsigned int *)pa2va(paddr);
        LOG("[0x%08X] %s PA:0x%08X VA:0x%08X NS:%d DOM:%02X\n", vaddr, "Page TBL  ", paddr, tbl, !!ns, domain);
        for (i = 0; i < 0x100; i++)
        {
            mmu_dump_pages(vaddr+(i<<12), tbl[i]);
        }
    }
    else if ((entry & 0x3) == 0) /* not mapped */
    {
        LOG("[0x%08X] %s\n", vaddr, "Unmapped  ");
    }
    else
    {
        LOG("[0x%08X] %s\n", vaddr, "Invalid   ");
    }
}

int mmu_dump(void)
{
    unsigned int ttbr[2];
    int ttbcr;
    int n;
    unsigned int i = 0;
    
    unsigned int *ttb_vaddr[2];
    unsigned int entry;

    __asm__("mrc p15,0,%0,c2,c0,0" : "=r" (ttbr[0]));
    __asm__("mrc p15,0,%0,c2,c0,1" : "=r" (ttbr[1]));
    __asm__("mrc p15,0,%0,c2,c0,2" : "=r" (ttbcr));
    LOG("TTBR0: 0x%08X, TTBR1: 0x%08X, TTBCR: 0x%08X\n", ttbr[0], ttbr[1], ttbcr);

    n = ttbcr & 0x7;
    ttbr[0] &= (unsigned int)((int)0x80000000 >> (31 - 14 + 1 - n));
    ttbr[1] &= 0xFFFFC000;

    ttb_vaddr[0] = (unsigned int *)pa2va(ttbr[0]);
    ttb_vaddr[1] = (unsigned int *)pa2va(ttbr[1]);
    LOG("TBBR0 (physical): 0x%08X, (virtual): 0x%08X\n", ttbr[0], i<<12);
    LOG("TBBR1 (physical): 0x%08X, (virtual): 0x%08X\n", ttbr[1], i<<12);

    LOG("Dumping TTBR0...\n");
    for (i = 0; i < (1 << 12 - n); i++)
    {
        entry = ttb_vaddr[0][i];
        mmu_dump_sections(i<<20, entry);
    }

    if (n)
    {
        LOG("Dumping TTBR1...\n");
        for (i = ((~0xEFFF & 0xFFFF) >> n); i < 0x1000; i++)
        {
            entry = ttb_vaddr[1][i];
            mmu_dump_sections(i<<20, entry);
        }
    }
    return 0;
}

int module_start(SceSize argc, const void *args)
{
    ksceIoMkdir("ux0:dump", 6);
    log_reset();
    dump_reset();
    unsigned int sctlr;

    __asm__("mrc p15,0,%0,c1,c0,0" : "=r" (sctlr));
    afe = sctlr & 0x20000000;
    mmu_dump();

    return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize argc, const void *args)
{
	return SCE_KERNEL_STOP_SUCCESS;
}

void log_reset()
{
	SceUID fd = ksceIoOpen(LOG_FILE,
		SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC, 6);
	if (fd < 0)
		return;

	ksceIoClose(fd);
}

void dump_reset()
{
	SceUID fd = ksceIoOpen("ux0:dump/memory.bin",
		SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC, 6);
	if (fd < 0)
		return;

	ksceIoClose(fd);
}

void log_write(const char *buffer, size_t length)
{
	extern int ksceIoMkdir(const char *, int);
	ksceIoMkdir(DUMP_PATH, 6);

	SceUID fd = ksceIoOpen(LOG_FILE,
		SCE_O_WRONLY | SCE_O_CREAT | SCE_O_APPEND, 6);
	if (fd < 0)
		return;

	ksceIoWrite(fd, buffer, length);
	ksceIoClose(fd);
}
