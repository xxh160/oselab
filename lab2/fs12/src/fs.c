#include <fs.h>
#include <stdint.h>
#include <stdio.h>
#include <common.h>
#include <assert.h>
#include <string.h>

static uint8_t boot_buffer[SECTOR_SIZE];
// img pointer
static FILE *p_img = NULL;
// boot info
static BPB_t *bi;
// volume info
static VINFO_t vi;

// 从第 sec 个扇区开始读, 读 count 个扇区, 读到 buffer 里去
static int read_sec(int sec, int count, uint8_t *buffer) {
	// find correct offset
	if (fseek(p_img, sec * SECTOR_SIZE, SEEK_SET)) return FAILURE;
	fread(buffer, SECTOR_SIZE, count, p_img);
	return SUCCESS;
}

static int check_analyse_bi() {
	if (bi->sig_55 != 0x55) return FAILURE;
	if (bi->sig_aa != 0xaa) return FAILURE;
	// success
	vi.bytes_sec = bi->byte_sec_h << 8 | bi->byte_sec_l;
	vi.sec_clus = bi->sec_clus;
	vi.root_entry_cnt = bi->root_entry_cnt_h << 8 | bi->root_entry_cnt_l;
	vi.boot_part_sec = 0;
	vi.boot_part_size = bi->rsvd_sec_cnt_h << 8 | bi->rsvd_sec_cnt_l; 
	vi.fat1_sec = vi.boot_part_sec + vi.boot_part_size;
	vi.fat_num = bi->n_fats;
	vi.fat_size = bi->fat_sec_h << 8 | bi->fat_sec_l;
	vi.root_entry_sec = vi.fat1_sec + vi.fat_num * vi.fat_size;
	vi.data_sec = vi.root_entry_sec + 
								(vi.root_entry_cnt * ROOT_ENTRY_SIZE + SECTOR_SIZE - 1) / SECTOR_SIZE;
	return SUCCESS;
}

// init file pointer
// read boot partition
int init_fat12(char *img) {
	// open fat12 img
	p_img = fopen(img, "r");		
	if (p_img == NULL) return FAILURE;
	// record boot info
	if (read_sec(0, 1, boot_buffer) == FAILURE) return FAILURE;
	bi = (BPB_t*) boot_buffer;
	if (check_analyse_bi() == FAILURE) return FAILURE;
	return SUCCESS;
}

void close_fat12() {
	fclose(p_img);
}

void get_dir_name(FINFO_t *pfi, char *real_name) {
	int i;
	for (i = 0; i < 8; ++i) {
		if (pfi->dir_name[i] == ' ') break;
		real_name[i] = pfi->dir_name[i];
	}
	if (pfi->dir_name_ext[0] != ' ') real_name[i++] = '.';
	for (int j = 0; j < 3; ++j) {
		if (pfi->dir_name_ext[j] == ' ') break;
		real_name[i++] = pfi->dir_name_ext[j];
	}
	real_name[i] = 0;
}

void init_cdi(CDIRINFO_t *cdi) {
	memset(cdi->dir_name, 0, CDI_NAME_SIZE);
	memset(cdi->buffer, 0, SECTOR_SIZE);
	cdi->cur_entry = 0;
	cdi->cur_sec = 0;
}

void cdicpy(CDIRINFO_t *dest, CDIRINFO_t *src) {
	strcpy(dest->dir_name, src->dir_name);
	memcpy(dest->buffer, src->buffer, SECTOR_SIZE);
	dest->cur_sec = src->cur_sec;
	dest->cur_entry = src->cur_entry;
}

static void open_root_dir(CDIRINFO_t *pcdi) {
	read_sec(vi.root_entry_sec, 1, pcdi->buffer);
	pcdi->dir_name[0] = '/';
	pcdi->dir_name[1] = 0;
	pcdi->cur_sec = vi.root_entry_sec;
	pcdi->cur_entry = 0;
}

int is_relative_dir(FINFO_t *pfi) {
	if (pfi->dir_attr != DIR_ATTR) return FALSE;
	if (pfi->dir_name[0] == '.' && pfi->dir_name[1] == '.' && pfi->dir_name[2] == ' ') return TRUE;
	if (pfi->dir_name[0] == '.' && pfi->dir_name[1] == ' ') return TRUE;
	return FALSE;
}

int next_entry(CDIRINFO_t *pcdi, FINFO_t *pfi) {
	memcpy(pfi, &((FINFO_t *) pcdi->buffer)[pcdi->cur_entry++], sizeof(FINFO_t));	
	// 已经删除的 entry
	// 不考虑长名字
	while (pfi->dir_name[0] == DELETE_NAME || pfi->dir_name[0] == KANJI_NAME) {
		memcpy(pfi, &((FINFO_t *) pcdi->buffer)[pcdi->cur_entry++], sizeof(FINFO_t));	
	}
	// 最后一个 entry
	// 重置 entry
	if (pfi->dir_name[0] == END_NAME) {
		pcdi->cur_entry = 0;
		return FAILURE;
	}
	return SUCCESS;
}

// return next clus number
// if null, return 0xFFF
static uint32_t read_fat(int clus) {	
	clus -= (vi.data_sec - 2);
	uint8_t fat_buffer[SECTOR_SIZE] = { 0 };
	uint32_t start_sec = vi.fat1_sec + (clus + clus / 2) / SECTOR_SIZE;
	read_sec(start_sec, 1, fat_buffer);
	// 0 字节为第一个
	// 不考虑 fat 表项的起始地址在边界的情况, 即在 511 字节的情况
	uint32_t offset = (clus + clus / 2) % SECTOR_SIZE;
	uint32_t result = (uint32_t) fat_buffer[offset] |
						        (uint32_t) fat_buffer[offset + 1] << 8;
	// odd : ****....********
	// even: ********....****
	// low bit - high bit
	// * is valid
	if (clus & 1) result = result >> 4;
	else result = result & 0xFFF;
	return result;	
}

int next_file_clus(CFILEINFO_t *pcfi) {
	if (pcfi->next_clus == FINAL_CLUS) return FAILURE;	
	pcfi->cur_clus = pcfi->next_clus + vi.data_sec - 2;
	pcfi->next_clus = read_fat(pcfi->cur_clus);
	memset(pcfi->buffer, 0, SECTOR_SIZE);
	read_sec(pcfi->cur_clus, 1, pcfi->buffer);
	return SUCCESS;
}

void init_cfi(CFILEINFO_t *cfi) {
	cfi->cur_clus = 0;
	cfi->first_clus = 0;
	cfi->next_clus = 0;
	memset(cfi->buffer, 0, SECTOR_SIZE);
}

int read_file(char *file_path, char *file_name, CFILEINFO_t *pcfi, CDIRINFO_t *pcdi) {
	CDIRINFO_t cdi_back;
	init_cdi(&cdi_back);
	cdicpy(&cdi_back, pcdi);
	if (open_dir(file_path, file_path, pcdi) == FAILURE) {
		cdicpy(pcdi, &cdi_back);
		uerror("can not open dir '%s'", file_path);	
		return FAILURE;
	}
	pcdi->cur_entry = 0;
	FINFO_t fi;
	while(next_entry(pcdi, &fi) == SUCCESS) {
		char real_name[12] = { 0 };
		get_dir_name(&fi, real_name);
		if (strcmp(file_name, real_name) != 0) continue;
		if (fi.dir_attr != FILE_ATTR && fi.dir_attr != FILE_ATTR_WIN) {
			uerror("'%s' is not a file", file_name);
			cdicpy(pcdi, &cdi_back);
			return FAILURE;
		}
		if (fi.file_size == 0) {
			cdicpy(pcdi, &cdi_back);
			return FAILURE;
		}
		// find the file entry			
		pcfi->first_clus = (fi.fst_clus_h << 8 | fi.fst_clus_l) +
											 vi.data_sec - 2;
		pcfi->cur_clus = pcfi->first_clus;
		pcfi->next_clus = read_fat(pcfi->first_clus);
		read_sec(pcfi->first_clus, 1, pcfi->buffer);
		cdicpy(pcdi, &cdi_back);
		return SUCCESS;
	}
	cdicpy(pcdi, &cdi_back);
	uerror("no such file '%s'", file_name);
	return FAILURE;
}

// 打开文件夹, 将一系列必要信息存入 cdi 中
// 读取其第一个扇区, 也就是簇, 存入 buffer
// dir entry 从 buffer 中直接提取
// dir_name: 当前 对 full_path 切片出的文件路径
// full_path: 传入时的文件路径, 可能自己也是个相对路径
int open_dir(char *dir_name, char *full_path, CDIRINFO_t *pcdi) {
	// 组合成绝对路径
	char full[64] = { 0 };
	// 从最开始开始寻址
	pcdi->cur_entry = 0;
	if (strcmp(full_path, ROOT_DIR) == 0) {
		// 根目录直接打开
		open_root_dir(pcdi);
		return SUCCESS;
	}
	// 不是打开根目录
	CDIRINFO_t cdi_back;
	// backup
	cdicpy(&cdi_back, pcdi);
	assert(strlen(dir_name) > 0);
	char *tar_dir = dir_name;	
	char *str_end = tar_dir + strlen(tar_dir); 
	// 如果第一个字符是 /, 则为绝对寻址
	// 直接打开根目录
	if (tar_dir[0] == '/') {
		open_root_dir(pcdi);
		++tar_dir;
	}
	// 从当前目录开始寻址
	FINFO_t fi;
	char *cur_path = strtok(tar_dir, "/");
	tar_dir = cur_path + strlen(cur_path) + 1;
	// 相对寻址, 需要更新 full
	strcat(full, pcdi->dir_name);
	strcat(full, ((pcdi->dir_name[strlen(pcdi->dir_name) - 1] == '/') ? "" : "/"));
	strcat(full, cur_path);
	char real_name[12] = { 0 };
	while (next_entry(pcdi, &fi) == SUCCESS) {
		get_dir_name(&fi, real_name);	
		if (strcmp(cur_path, real_name) != 0) continue;
		// find
		// if it is file
		if (fi.dir_attr == FILE_ATTR || fi.dir_attr == FILE_ATTR_WIN) {
			uerror("can not open file '%s' as directory", full);
			return FAILURE;
		}
		// success! find the entry!
		// 覆盖 pcdi 为当前文件夹
		memset(pcdi->dir_name, 0, CDI_NAME_SIZE);
		strcat(pcdi->dir_name, full);
		strcat(pcdi->dir_name, "/");
		pcdi->cur_sec = (fi.fst_clus_h << 8 | fi.fst_clus_l) +
										vi.data_sec - 2;
		if ((fi.fst_clus_h << 8 | fi.fst_clus_l) == 0) pcdi->cur_sec = vi.root_entry_sec;
		pcdi->cur_entry = 0;
		memset(pcdi->buffer, 0, SECTOR_SIZE);
		read_sec(pcdi->cur_sec, 1, pcdi->buffer);
		if (tar_dir < str_end) {
			if (open_dir(tar_dir, full, pcdi) == FAILURE) {
				cdicpy(pcdi, &cdi_back);
				return FAILURE;
			}
		}
		return SUCCESS;
	}	
	uerror("no such file/dir '%s'", full);
	return FAILURE;	
}

