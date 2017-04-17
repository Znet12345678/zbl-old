#include <stdint.h>
#include "zfs.h"
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
	dest[strlen(dest)] = 0;
	return dest;
}
char *strcat(char *dest,const char *src){
	return strcpy(&dest[strlen(dest)],src);
}
int main(){
	mem_init();
	t_readvals();
	debug("KERNEL","Kernel successfully loaded!");
	int l = __prim_getlba() + 1;
	kprintf("[KERNEL]Starting LBA:%d\n",l);
	*(int*)0x501 = l;
	if(is_zfs() < 0){
		kprintf("No file system detected\n");
		mkfs();
	}
	struct dirent *ent = parse_dirent("/");
	kprintf("[alloc] %d [namelen] %d [name] %s [first_fent_lba] %d [first_fent_offset] %d [nxt_dirent_lba] %d [nxt_dirent_offset] %d [curr_ent_lba] %d [curr_ent_offset] %d\n",ent->alloc,ent->namelen,ent->name,ent->first_fent_lba,ent->first_fent_offset,ent->nxt_dirent_lba,ent->nxt_dirent_offset,ent->curr_ent_lba,ent->curr_ent_offset);
	DIR *d = opendir("/");
	mkdir("/test");
	debug("KERNEL","Done");
	while(1){
		gets(0);
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
}
