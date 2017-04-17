#include "lib.h"
#include "mem.h"
#include "zfs.h"
#define ERR -1
const uint8_t sig[5] = {0x7F,'z','f','s',0};
struct dirent *parse_dent(int lba,int offset){
	struct dirent *ret = malloc(1024);
	uint8_t *buf = malloc(1024);
	ata_read_master(buf,lba,0);
	if((offset + sizeof(*ret) + buf[offset + 1]) > 512)
		ata_read_master(&buf[512],lba + 1,0);
	ret->alloc = buf[offset];
	ret->namelen = buf[offset + 1];
	for(int i = 0; i < ret->namelen;i++){
		ret->name[i] = buf[offset + 2 + i];
	}
	for(int i = 24; i >= 0;i-=8)
		ret->first_fent_lba|=buf[offset + ret->namelen + 3 + (24 - i)/8] << i;
	for(int i = 8; i >= 0;i-=8)
		ret->first_fent_offset|=buf[offset + 7 + ret->namelen + (8-i)/8] << i;
	for(int i = 24; i >= 0;i-=8)
		ret->nxt_dirent_lba |= buf[offset + 9 + ret->namelen + (24 - i)/8] << i;
	for(int i = 8; i >= 0;i-=8)
		ret->nxt_dirent_offset |= buf[offset + 13 + ret->namelen + (8 - i)/8] << i;
	for(int i = 24; i >= 0;i-=8)
		ret->curr_ent_lba |= buf[offset + 15 + ret->namelen + (24 - i)/8] << i;
	for(int i = 8; i >= 0;i-=8)
		ret->curr_ent_offset|= buf[offset + 19 + ret->namelen + (8 - i)/8] << i;
	return ret;
}
struct dirent *parse_dirent(const char *str){
	if(strcmp(str,"/") == 0){
		struct dirent *ret = malloc(1024);
		uint8_t *buf = malloc(512);
		struct superblock *sblk = parse_superblk(*(int*)0x501);
		ata_read_master(buf,sblk->root_dirent_lba,0);
		int offset = sblk->root_dirent_offset;
		ret->alloc = buf[offset];
		ret->namelen = buf[offset + 1];
		for(int i = 0; i < ret->namelen;i++)
			ret->name[i] = buf[offset + 2 + i];
		ret->first_fent_lba = buf[offset + 3 + ret->namelen] << 24 | buf[offset + 4 + ret->namelen] << 16 | buf[offset + 5 + ret->namelen] << 8 | buf[offset + 6 + ret->namelen];
		ret->first_fent_offset = buf[offset + 7 + ret->namelen] << 8 | buf[offset + 8 + ret->namelen];
		ret->nxt_dirent_lba = buf[offset + 9 + ret->namelen] << 24 | buf[offset + 10 + ret->namelen] << 16 | buf[offset + 11 + ret->namelen] << 8 | buf[offset + 12 + ret->namelen];
		ret->nxt_dirent_offset = buf[offset + 13 + ret->namelen] << 8 | buf[offset + 14 + ret->namelen];
		ret->curr_ent_lba = buf[offset + 15 + ret->namelen] << 24 | buf[offset + 16 + ret->namelen] << 16 | buf[offset + 17 + ret->namelen] << 8 | buf[offset + 18 + ret->namelen];
		ret->curr_ent_offset = buf[offset + 19 + ret->namelen] << 8 | buf[offset + 20 + ret->namelen];
		return ret;
	}else{
		struct dirent *dent = parse_dirent("/");
		while(dent->nxt_dirent_lba > 0){
			if(strcmp(dent->name,str) == 0)
				return dent;
			uint8_t *buf = malloc(1024);
			ata_read_master(buf,dent->nxt_dirent_lba,0);
			dent = parse_dent(dent->nxt_dirent_lba,dent->nxt_dirent_offset);
		}
		return (struct dirent *)-1;
	}
}
struct free_blk *find_freedent(int size){
	struct dirent *dent = parse_dirent("/");
	uint32_t currlba = 0;
	uint16_t curroffset = 0;
	if(dent->nxt_dirent_lba == 0){
		struct superblock *sblk = parse_superblk(*(int*)0x501);
		struct dirent *root = parse_dirent("/");
		struct free_blk *ret = malloc(sizeof(*ret) * sizeof(ret));
		ret->offset = sizeof(*root);
		ret->lba = sblk->root_dirent_lba;
		return ret;
	}
	struct superblock *sblk = parse_superblk(*(int*)0x501);
	currlba = sblk->root_dirent_lba;
	curroffset = sblk->root_dirent_offset + sizeof(*dent);
	int currsize = sizeof(*dent);
	while(dent->nxt_dirent_lba > 0){
		//kprintf("%d\n",currlba);
		if(dent->alloc == 0 || dent->nxt_dirent_lba == 0){
			if(dent->alloc == 0){
				if(size > (dent->nxt_dirent_offset - curroffset)){
					uint8_t *buf = malloc(1024);
					ata_read_master(buf,dent->nxt_dirent_lba,0);
					currlba = dent->nxt_dirent_lba;
					curroffset = dent->nxt_dirent_offset;
					dent = parse_dent(dent->nxt_dirent_lba,dent->nxt_dirent_offset);
					free(buf);
					continue;
				}else if(((512 - curroffset) + dent->nxt_dirent_offset) < size){
					uint8_t *buf = malloc(1024);
                                        ata_read_master(buf,dent->nxt_dirent_lba,0);
                                        currlba = dent->nxt_dirent_lba;
                                        curroffset = dent->nxt_dirent_offset;
                                        dent = parse_dent(dent->nxt_dirent_lba,dent->nxt_dirent_offset);
					free(buf);
                                        continue;
				}
				struct free_blk *ret = malloc(sizeof(*ret)*sizeof(ret));
        	                if((curroffset + size) > 512)
                        	        ret->lba = currlba + 1;
                	        else
                                	ret->lba = currlba;
                        	ret->offset = curroffset;
                        	return ret;
			}
			struct free_blk *ret = malloc(sizeof(*ret)*sizeof(ret));
			if(currsize + curroffset > 512){
				ret->lba = currlba + 1;
				ret->offset = 0;
			}else{
				ret->lba = currlba;
				ret->offset = currsize + curroffset;
			}

			return ret;
		}
		uint8_t *buf = malloc(1024);
		ata_read_master(buf,dent->nxt_dirent_lba,0);
		currlba = dent->nxt_dirent_lba;
		curroffset = dent->nxt_dirent_offset;
		currsize = sizeof(*dent);
		dent = parse_dent(dent->nxt_dirent_lba,dent->nxt_dirent_offset);
		free(buf);
	}
	struct free_blk *ret = malloc(sizeof(*ret)*sizeof(ret));
        if(currsize + curroffset > 512){
        	ret->lba = currlba + 1;
                ret->offset = 0;
        }else{
            	ret->lba = currlba;
                ret->offset = currsize + curroffset;
        }

	return ret;
}
char *part(const char *str,unsigned int n,unsigned char c){
	char *ret = malloc(1024);
	int j = 0;
	int k = 0;
	for(int i = 0; i < strlen(str);i++){
		if(str[i] == c)
			j++;
		else if(j == n){
			ret[k] = str[i];
			k++;
		}
	}
	return ret;
}
int is_zfs(){
	struct superblock *sblk = parse_superblk(*(int*)0x501);
	if(strcmp(sblk->sig,sig) == 0)
		return 1;
	debug("IS_ZFS","Invalid sig not zfs");
	kprintf("Correct sig:%s\nsig:%s\n",sig,sblk->sig);
	return ERR;
}
int write_dirent(struct dirent *ent,int lba,int offset){
	uint8_t *buf = malloc(1024);
	int err = ata_read_master(buf,lba,0);
	if(err < 0)
		return -1;
	if((offset + sizeof(ent)) > 512){
		err = ata_read_master(&buf[512],lba + 1,0);
		if(err < 0)
			return -1;
	}
	buf[offset] = ent->alloc;
	buf[offset + 1] = ent->namelen;
	for(int i = 0; i < ent->namelen;i++)
		buf[offset + 2] = ent->name[i];
	for(int i = 24; i >= 0;i-=8)
		buf[offset + (24 - i)/8 + ent->namelen + 3] = ent->first_fent_lba >> i;
	for(int i = 8; i >= 0;i-=8)
		buf[offset + (8 - i)/8 + ent->namelen + 7] = ent->first_fent_offset >> i;
	for(int i = 24; i >= 0;i-=8)
		buf[offset + (24 - i)/8 + ent->namelen + 9] = ent->nxt_dirent_lba >> i;
	for(int i = 8; i >= 0;i-=8)
		buf[offset + (8 - i)/8 + ent->namelen + 13] = ent->nxt_dirent_offset >> i;
	for(int i = 24; i >= 0;i-=8)
		buf[offset + (24 - i)/8 + ent->namelen + 15] = ent->curr_ent_lba >> i;
	for(int i = 8; i >= 0;i-=8)
		buf[offset + (24 - i)/8 + ent->namelen + 19] = ent->curr_ent_offset >> i;
	if((offset + sizeof(*ent)) > 512){
		debug("WRITE_DIRENT","Writing 2 blocks");
		err = ata_write_master(buf,lba);
		if(err < 0)
			return -1;
		err = ata_write_master(&buf[512],lba + 1);
		if(err < 0)
			return -1;
	}else{
		err = ata_write_master(buf,lba);
		if(err < 0)
			return -1;
	}
	free(buf);
	return 1;
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
			buf[15 + namelen + i] = dent->curr_ent_lba >> j;
		buf[19 + namelen] = dent->curr_ent_offset >> 8;
		buf[20 + namelen] = dent->curr_ent_offset;
		debug("MKDIR","Writing...");
		ata_write_master(buf,lba);
		debug("MKDIR","Writing ENT");
		struct ent *ent = malloc(512);
		ent->alloc = 1;
		ent->nxt_ent_lba = 0;
		ent->nxt_ent_offset = 0;
		ent->nxt_dirent_lba = 0;
		ent->nxt_dirent_offset = 0;
		debug("MKDIR","Freeing buf");
		free(buf);
		debug("MKDIR","Allocating buf");
		buf = malloc(1024);
		for(i = 0; i < 512;i++)
			buf[i] = 0;
		buf[0] = ent->alloc;
		ata_write_master(buf,dent->curr_ent_lba);
		debug("MKDIR","Done");
		return 1;
	}else{
		struct dirent *dent = parse_dirent("/");
		int lba = sblk->root_dirent_lba;
		int offset = sblk->root_dirent_offset;
		int write_lba = 0;
		int write_offset = 0;
		kprintf("Reading: %d:%d\n",lba,offset);
		while(1 && lba != 0){
			uint8_t *buf = malloc(1024);
			int err = ata_read_master(buf,lba,0);
			if(err < 0){
				debug("MKDIR","failed to read");
				kprintf("%d %d\n",lba,offset);
				panic();
			}
			int _lba = buf[offset + 9 + buf[offset  + 1]] << 24 | buf[offset + 10 + buf[offset + 1]] << 16 | buf[offset + 11 + buf[offset + 1]] << 8 | buf[offset + 12 + buf[offset + 1]];
			int _offset = buf[offset + buf[offset + 1] + 13] << 8 | buf[offset + 14 + buf[offset + 1]];
			if(_lba == 0)
				break;
			lba = _lba;
			offset = _offset;
			free(buf);
		}
		char *name = malloc(80);
		uint8_t *nbuf = malloc(512);
		ata_read_master(nbuf,lba,0);
		int namelen = nbuf[offset + 1];
		for(int i = 0; i < namelen;i++)
			name[i] = nbuf[2 + i];
		struct dirent *prev = parse_dirent(name);
		uint8_t *buf = malloc(1024);
		struct dirent *dent1 = malloc(sizeof(*dent1) * sizeof(dent1) + strlen(path));
		dent1->alloc = 1;
		dent1->namelen = strlen(path);

		strcpy(dent1->name,path);
		dent1->first_fent_lba = 0;
		dent1->first_fent_offset = 0;
		dent1->nxt_dirent_lba = 0;
		dent1->nxt_dirent_offset = 0;
		dent1->curr_ent_lba = 0;
		dent1->curr_ent_offset = 0;
		struct free_blk *blk = find_freedent(sizeof*prev);
		if(blk == (struct free_blk*)-1)
			panic();
		prev->nxt_dirent_lba = blk->lba;
		prev->nxt_dirent_offset = blk->offset;
		kprintf("%d %d\n",prev->nxt_dirent_lba,prev->nxt_dirent_offset);
		/*if((offset + sizeof(*prev)) > 512){
			prev->nxt_dirent_lba = lba + 1;
			prev->nxt_dirent_offset = 0;
		}else{
			prev->nxt_dirent_lba = lba;
			prev->nxt_dirent_offset +=sizeof(*prev);
		}*/
		debug("MKDIR","Writing");
		int err  = write_dirent(prev,lba,offset);
		if(err < 0){
			debug("MKDIR","Failed to write");
			panic();
		}
	}
	return ERR;
}
int mkfs(){
	return mkfs_zfs(*(int*)0x501);
}
int mkfs_zfs(int lba){
	debug("MKFS_ZFS","Writing superblock");
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
	debug("MKFS_ZFS","Writing root directory");
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
DIR *opendir(const char *dirname){
	struct superblock *sblk = parse_superblk(*(int*)0x501);
	if(strcmp(dirname,"/") != 0){
		char *name = part(dirname,1,'/');
		kprintf("%s\n",name);
		return (DIR*)ERR;
	}
	else{
		DIR *ret = malloc(sizeof(*ret) * sizeof(ret));
		ret->dirent = parse_dirent("/");
		if(strcmp(ret->dirent->name,"/") != 0){
			debug("OPENDIR","Invalid FS");
			panic();
		}
	}
}
