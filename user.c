#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <taihen.h>
#include <psp2kern/kernel/modulemgr.h>
#include <psp2kern/kernel/threadmgr.h>
#include <psp2kern/kernel/sysmem.h>
#include <psp2kern/io/fcntl.h>

void _start() __attribute__ ((weak, alias ("module_start")));

char magic[] = {
	0x9B, 0x9E, 0x51, 0xD2
};

typedef struct{
	uint64_t unk0; //0
	uint32_t unk1; //8
	uint32_t va;   //c
	uint32_t size; //10
	uint32_t unk3; //14
	uint32_t pa;   //18
	uint32_t magic; //1c
} KASLR_TABLE;

int module_start(SceSize argc, const void *args)
{
	uint8_t memory[0x1000];
	uint8_t small[0x1000];
	uint8_t data[0x20];
	
	//kaslr shitty table 0x20000 - 0xb0000 -> ends with 9b 9e 51 d2
	//found one at 0x30000 so let's stick with that
	//size is 0x10000
	//possible pa area is 0x40200000 - 0x60000000
	/*
	0000C000 20 00 01 00 FF FF FF FF 01 00 00 00 00 00 00 00  ...............
	0000C010 00 10 00 00 00 00 10 00 00 00 30 40 9B 9E 51 D2 ..........0@..Q.
	*/
	int i=0;
	int j=0x10;
	int temp1=0;
	int temp2=0x10;
	sceIoRemove("ux0:dump/memory_success.bin");
	sceIoRemove("ux0:dump/table.bin");
	
start:
	while(i<j){
		taiMemcpyKernelToUser(memory, (void*) 0x20000 + (0x1000 * i), 0x1000);
		if(sceClibMemcmp(memory + 0x1C,magic,4) == 0){
			SceUID fd = sceIoOpen("ux0:dump/memory_success.bin",
			SCE_O_WRONLY | SCE_O_CREAT | SCE_O_APPEND, 6);
			
			sceIoWrite(fd, memory, 0x1000);
			
			sceIoClose(fd);
			temp1=i;
			temp2=j;
			sceIoRemove("ux0:dump/memory_success.bin");
			goto finish;
		}else{
			j=j+0x10;
			i=i+0x10;
			goto start;
		}
	}
	
finish:
	for(i=temp1;i<temp2;i++) {
		taiMemcpyKernelToUser(memory, (void*) 0x20000 + (0x1000 * i), 0x1000);
		SceUID fd = sceIoOpen("ux0:dump/table.bin",
			SCE_O_WRONLY | SCE_O_CREAT | SCE_O_APPEND, 6);
			
			sceIoWrite(fd, memory, 0x1000);
			
			sceIoClose(fd);
	}
	
	i=0;
	int lol=0;
	SceUID fp = sceIoOpen("ux0:dump/table.bin",
			SCE_O_RDONLY, 0777);
	sceIoRemove("ux0:dump/out_memory.bin");
	sceIoRemove("ux0:dump/out_logs.bin");
	while(i != 0x800){
		sceIoLseek(fp, 0 + (0x20*i), SCE_SEEK_SET);
		sceIoRead(fp,data,0x20);
		
		KASLR_TABLE* kaslr_table = (KASLR_TABLE*)data;
		//0x40200000 - 0x60000000
		if(kaslr_table->pa == 0xE8100000 && kaslr_table->magic == 0xD2519E9B){
			int multiplier = (kaslr_table->size)/0x1000;
			for(lol=0;lol < multiplier; lol++) {
				taiMemcpyKernelToUser(small, (void*) kaslr_table->va + (0x1000 * lol), 0x1000);
				SceUID wfd = sceIoOpen("ux0:dump/out_memory.bin", SCE_O_WRONLY | SCE_O_CREAT | SCE_O_APPEND, 6);
				
				sceIoWrite(wfd, small, 0x1000);
				
				sceIoClose(wfd);
			}
		}
		
		i++;
	}
	sceIoClose(fp);

	
    return SCE_KERNEL_START_SUCCESS;

	
}

int module_stop(SceSize argc, const void *args)
{
	return SCE_KERNEL_STOP_SUCCESS;
}