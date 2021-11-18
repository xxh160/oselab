#ifndef __FS_H_
#define __FS_H_ 

#include <stdint.h>
#include <common.h>

#define CDI_NAME_SIZE 128

// BPB structure common
// locate in the first sector of the colume in the Reserved Region
typedef struct bpb {
	uint8_t jmp_boot[3]; // jmp instr to boot code
	uint8_t oem_name[8]; // OEM name identifier
	// little endian! tmd
	uint8_t byte_sec_l; // count of bytes per sector, low bit
	uint8_t byte_sec_h; // count of bytes per sector, high bit
	uint8_t sec_clus; // number of sectors per allocation unit(cluster)
	uint8_t	rsvd_sec_cnt_l;
	uint8_t rsvd_sec_cnt_h;
	uint8_t n_fats; // the count of FATs: 2
	uint8_t root_entry_cnt_l;
	uint8_t root_entry_cnt_h;
	uint8_t total_sec16_l; // total count of sectors on the volume, 16 bit: 0xe0
	uint8_t total_sec16_h;
	uint8_t media;
	uint8_t fat_sec_l; // count of sectors occupied by one FAT: 9
	uint8_t fat_sec_h;
	uint8_t sec_track_l; // sectors per track for interrupt 0x13? 0x12
	uint8_t sec_track_h; 
	uint8_t n_heads_l; // number of heads for interrupt 0x13: 2
	uint8_t n_heads_h; 
	uint8_t hid_sec_cnt[4]; // count of hidden sectors preceding the partition the contians this FAT volume?   
	uint8_t total_sec32[4]; // total count of sectors on the volume, 32 bit
	// extended BPB structure
	uint8_t drive_num;
	uint8_t reserved;
	uint8_t boot_sig;
	uint8_t vol_id[4];
	uint8_t vol_label[11];
	uint8_t file_sys_type[8];
	uint8_t code[448];
	uint8_t sig_55; // 0x55
	uint8_t sig_aa; // 0xaa
} BPB_t;

typedef struct file_info {
	uint8_t dir_name[8]; // 空位填 space
	uint8_t dir_name_ext[3];
	uint8_t dir_attr;
	uint8_t reserved[10];
	uint8_t wrt_time_l;
	uint8_t wrt_time_h;
	uint8_t wrt_date_l;
	uint8_t wrt_date_h;
	uint8_t fst_clus_l;
	uint8_t fst_clus_h;
	uint32_t file_size;
} FINFO_t;

typedef struct volume_info {
	// 每扇区多少字节
	uint32_t bytes_sec;
	// 每簇多少扇区
	uint32_t sec_clus;
	// 根目录条目数量
	uint32_t root_entry_cnt;
	// 引导扇区起始扇区号
	uint32_t boot_part_sec;
	// 引导扇区大小, 以扇区为单位
	uint32_t boot_part_size;
	// 第一个 FAT 表的起始扇区号
	uint32_t fat1_sec;
	// FAT 表的数量
	uint32_t fat_num;
	// FAT 表的大小, 以扇区为单位
	uint32_t fat_size;
	// 根目录区的起始扇区号
	uint32_t root_entry_sec;
	// 数据区的起始扇区号
	uint32_t data_sec;
} VINFO_t;

// 当前打开的文件夹
typedef struct cur_dir_info {
	char dir_name[CDI_NAME_SIZE];
	// fat12 一个簇里只有一个扇区
	uint32_t cur_sec;
	uint32_t cur_entry;
	uint8_t buffer[SECTOR_SIZE];
} CDIRINFO_t; 

typedef struct cur_file_info {
	uint32_t first_clus;
	uint32_t cur_clus;
	uint8_t buffer[SECTOR_SIZE];
	uint32_t next_clus;
} CFILEINFO_t;

int init_fat12(char *img);

void close_fat12();

void get_dir_name(FINFO_t *pfi, char *real_name);

void init_cdi(CDIRINFO_t *cdi);

void cdicpy(CDIRINFO_t *dest, CDIRINFO_t *src);

int is_relative_dir(FINFO_t *pfi);

int next_entry(CDIRINFO_t *pcdi, FINFO_t *pfi); 

int next_file_clus(CFILEINFO_t *pcfi); 

void init_cfi(CFILEINFO_t *cfi);

int read_file(char *file_path, char *file_name, CFILEINFO_t *pcfi, CDIRINFO_t *pcdi);

int open_dir(char *dir_name, char *full_path, CDIRINFO_t *pcdi);

#endif
