#ifndef __ZFS_H
#define __ZFS_H
const uint8_t sig[5] = {0x7F,'z','f','s',0};
struct superblock{
	uint8_t *sig;
	uint32_t root_dirent_lba;
	uint16_t root_dirent_offset;
};
struct dirent{
	uint8_t alloc;
	uint8_t namelen;
	uint8_t name[80];
	uint32_t first_fent_lba;
	uint16_t first_fent_offset;
	uint32_t nxt_dirent_lba;
	uint16_t nxt_dirent_offset;
	uint32_t curr_ent_lba;
	uint16_t curr_ent_offset;
};
struct ent{
	//DIR
	uint8_t alloc;
	uint32_t nxt_ent_lba;
	uint16_t nxt_ent_offset;
	uint32_t nxt_dirent_lba;
	uint16_t nxt_dirent_offset;
	//FILE
};
#ifndef KERNEL
int main(int argc,char *argv[]);
#else
int mkfs();
int is_zfs();
int mkdir(const char *path);
int mkfs_zfs(int lba);
struct superblock *parse_superblk(int lba);
#endif
#endif
