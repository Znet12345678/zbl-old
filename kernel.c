#include <stdint.h>
#include "libk/fs.h"
void print(const char *str){
	kprintf("%s",str);
}
#ifdef STANDALONE
void *memmove(void *dstpntr,const void *srcptr,unsigned long size){
        unsigned char * dst = (unsigned char *)dstpntr;
        const unsigned char *src = (const unsigned char *)srcptr;
        if(dst < src)
                for(unsigned long i = 0; i < size;i++)
                        dst[i] = src[i];
        else
                for(unsigned long i = size; i != 0;i--)
                        dst[i - 1] = src[i - 1];
        return dstpntr;
}
#endif
void *memcpy(void *dstptr,const void *srcptr,unsigned long size){
	unsigned char *dst = (unsigned char *)dstptr;
	const unsigned char * src = (const unsigned char *)srcptr;
	for(int i = 0; i < size;i++)
		dst[i] = src[i];
	return dstptr;
}
void panic(){
	kprintf("panic()");
	while(1)
		;
}
unsigned long strlen(const char *str){
	int ret = 0;
	while(str[ret] != 0)
		ret++;
	return ret;
}
char *strcpy(char *dest,const char *src){
	int i = strlen(dest);
	int j = 0;
	while(j < strlen(src)){
		dest[i] = src[j];
		i++;
		j++;
	}
	return dest;
}
char *strcat(char *dest,const char *src){
	return strcpy(&dest[strlen(dest)],src);
}
int main(){
	mem_init();
	t_readvals();
	kprintf("[KERNEL]Kernel successfully loaded!\n");
	int l = __prim_getlba() + 1;
	kprintf("[DEBUG]Starting LBA:%d\n",l);
	*(int*)0x501 = l;
	if(is_zfs() < 0){
		kprintf("No file system detected\n");
		mkfs();
	}
	kprintf("[KERNEL]Done\n");
	while(1){
		gets();
		kprintf("\n");
	}
	/*char *malloctest1 = malloc(1024);
	strcpy(malloctest1,"Malloc works!\n");
	char *malloctest2 = malloc(1024);
	strcpy(malloctest2,"Malloc doesnt work!\n");
	kprintf("%s",malloctest1);
	kprintf("Loading...\n");
	free(malloctest1);
	free(malloctest2);
//	if(!has_sysinf()){
//		kprintf("Generating system information...\n");
//		gen_sysinf();
//	}
        struct fs_info *fsinfo = malloc(512);
        fsinfo->alloc = 1;
        uint32_t lba = end_of_prefs();
        fsinfo->start_lba = lba;
        fsinfo->mount_path_len = 1;
        fsinfo->mount_path = malloc(8);
        fsinfo->mount_path[0] = '/';
	mount(fsinfo);
	isfs();
	while(1)
		;
	if(!isfs()){
		kprintf("Making filesystem!\n");
		__mkfs("/");
	}
	mkdir("/test");
	kprintf("Done\n");*/
	while(1)
		;
}
