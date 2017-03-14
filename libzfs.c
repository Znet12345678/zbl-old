#include "lib.h"
#include "mem.h"
#include "zfs.h"
int is_zfs(){
	struct superblock *sblk = parse_superblk(*(int*)0x501);
	if(strcmp(sblk->sig,sig) == 0)
		return 1;
	kprintf("Invalid sig not zfs\n");
	kprintf("Correct sig:%s\nsig:%s\n",sig,sblk->sig);
	return -1;
}
int mkdir(const char *path){
	struct superblock *sblk = parse_superblk(*(int*)0x501);
	if(strcmp(path,"/") == 0){
		uint32_t lba = sblk->root_dirent_lba;
		kprintf("[MKDIR]Writing root directory to lba %d\n",lba);
		struct dirent *dent = malloc(512);
		dent->alloc = 1;
		dent->namelen = strlen(path);
		strcpy(dent->name,path);
		dent->first_fent_lba = 0;
		dent->first_fent_offset = 0;
		dent->nxt_dirent_lba = 0;
		dent->nxt_dirent_offset = 0;
		dent->curr_ent_lba = lba + 1;
		dent->curr_ent_offset = 0;
		uint8_t *buf = malloc(512);
		buf[0] = dent->alloc;
		buf[1] = dent->namelen;
		int i = 0;
		kprintf("[MKDIR]Writing %s to %d\n",dent->name,lba);
		int namelen = dent->namelen;
		while(i < dent->namelen){
			buf[2 + i] = dent->name[i];
			i++;
		}
		i = 0;
		for(int j = 24; j >= 0;j-=8,i++)
			buf[3 + namelen + i] = dent->first_fent_lba >> j;
		buf[7 + namelen] = dent->first_fent_offset >> 8;
		buf[8 + namelen] = dent->first_fent_offset;
		i = 0;
		for(int j = 24; j >= 0;j-=8,i++)
			buf[9 + namelen + i] = dent->nxt_dirent_lba >> j;
		buf[13 + namelen] = dent->nxt_dirent_offset >> 8;
		buf[14 + namelen] = dent->nxt_dirent_offset;
		i = 0;
		for(int j = 24; j >= 0;j-=8,i++)
			buf[15 + namelen] = dent->curr_ent_lba >> j;
		buf[19 + namelen] = dent->curr_ent_offset >> 8;
		buf[20 + namelen] = dent->curr_ent_offset;
		kprintf("[MKDIR]Writing...\n");
		ata_write_master(buf,lba);
		kprintf("[MKDIR]Writing ENT\n");
		struct ent *ent = malloc(512);
		ent->alloc = 1;
		ent->nxt_ent_lba = 0;
		ent->nxt_ent_offset = 0;
		ent->nxt_dirent_lba = 0;
		ent->nxt_dirent_offset = 0;
		kprintf("[MKDIR DEBUG]Freeing buf\n");
		free(buf);
		kprintf("[MKDIR DEBUG]Allocating buf\n");
		buf = malloc(1024);
		for(i = 0; i < 512;i++)
			buf[i] = 0;
		buf[0] = ent->alloc;
		ata_write_master(buf,dent->curr_ent_lba);
		kprintf("[MKDIR]Done\n");
		return 1;
	}
	return -1;
}
int mkfs(){
	return mkfs_zfs(*(int*)0x501);
}
int mkfs_zfs(int lba){
	kprintf("[MKFS_ZFS]Writing superblock\n");
	struct superblock *sblk = malloc(512);
	memcpy(sblk->sig,sig,16);
	sblk->root_dirent_lba = lba + 1;
	sblk->root_dirent_offset = 0;
	uint8_t *buf = malloc(512);
	for(int i = 0; i < 512;i++)
		buf[i] = 0;
	for(int i = 0; i <= 4;i++)
		buf[i] = sblk->sig[i];
	int j = 4;
	for(int i = 24; i >= 0;i-=8,j++)
		buf[j] = sblk->root_dirent_lba >> i;
	buf[j + 5] = sblk->root_dirent_offset >> 8;
	buf[j + 6] = sblk->root_dirent_offset;
	ata_write_master(buf,lba);
	kprintf("[MKFS_ZFS]Writing root directory\n");
	int err = mkdir("/");
	if(err < 0)
		panic();
}
struct superblock *parse_superblk(int lba){
	struct superblock * ret = malloc(512);
	uint8_t *buf = malloc(512);
	ata_read_master(buf,lba,0);
	ret->sig = malloc(1024);
	memcpy(ret->sig,buf,4);
	ret->root_dirent_lba = buf[4] << 24 | buf[5] << 16 | buf[6] << 8 | buf[7];
	ret->root_dirent_offset = buf[8] << 8 | buf[9];
	return ret;
}
