#include <engine.h>
#include <fs.h>
#include <common.h>
#include <string.h>
#include <assert.h>
#include <nasm.h>

static char *legal_arg[] = { "-l", "-ll" };

#define NO_ARG -1
#define N_ARG ARRLEN(legal_arg)

static int parse(char *arg, char **dir, int *argu) {
	for (int i = 0; i < N_ARG; ++i) {
		if (strcmp(legal_arg[i], arg) == 0) {
			if (*argu != -1) return SUCCESS;
			*argu = i;
			return SUCCESS;
		}
	}
	// invalid argument
	if (arg[0] == '-') return FAILURE;	
	// dir, can only be set once
	if (*dir != NULL) return FAILURE;
	*dir = arg;
	return SUCCESS;
}

static int parse_args(char *args, char **dir, int *argu) {
	if (args == NULL) {
		return SUCCESS;
	}
	// args end addr
	char *str_end = args + strlen(args);
	char *args_back = args, *cur_arg = NULL;
	do {
		cur_arg = strtok(args_back, " ");
		args_back = cur_arg + strlen(cur_arg) + 1;
		if (parse(cur_arg, dir, argu) == FAILURE) {
			uerror("invalid argument: '%s'", cur_arg);
			uinfo("%s", "tips: dirname can only be set once");
			uinfo("%s", "tips: valid argument: -l, -ll");
			return FAILURE;
		}
	} while (args_back < str_end);
	return SUCCESS;
}

static void print_num(int num) {
	char size[10] = {0};
	sprintf(size, "%d", num);
	uprint(size, strlen(size), 1);
}

static void cal_file_dir_num(FINFO_t *pfi, CDIRINFO_t *pcdi, int *file_n, int *dir_n) {
	pcdi->cur_entry = 0;
	*file_n = 0;
	*dir_n = 0;
	while (next_entry(pcdi, pfi) == SUCCESS) {
		if (pfi->dir_attr == FILE_ATTR || pfi->dir_attr == FILE_ATTR_WIN) ++(*file_n);
		else if (pfi->dir_attr == DIR_ATTR) {
			// . .. 不计
			if (is_relative_dir(pfi)) continue;
			++(*dir_n);
		}
	}
	assert(pcdi->cur_entry == 0);
}

// 这里最好用函数表, 但是我懒了
static void print_dir(CDIRINFO_t *pcdi, FINFO_t *pfi, int arg) {
	char real_name[12] = { 0 };
	get_dir_name(pfi, real_name);
	uprint(real_name, strlen(real_name), 0);		
	if (arg == NO_ARG) {
		uprint("  ", 2, 1);
		return;
	}
	// 有参数
	uprint(" ", 1, 1);
	CDIRINFO_t cdi_back;
	FINFO_t fi;
	init_cdi(&cdi_back);
	cdicpy(&cdi_back, pcdi);
	// 会改变 pcdi
	if (is_relative_dir(pfi)) {
		uprint("\n", 1, 1);
		return;
	}
	open_dir(real_name, real_name, pcdi);
	int file_n, dir_n;	
	cal_file_dir_num(&fi, pcdi, &file_n, &dir_n);
	cdicpy(pcdi, &cdi_back);
	print_num(dir_n);
	uprint(" ", 1, 1);
	print_num(file_n);
	uprint("\n", 1, 1);
}

static void print_file(FINFO_t *pfi, int arg) {
	char real_name[12] = { 0 };
	get_dir_name(pfi, real_name);
	uprint(real_name, strlen(real_name), 1);	
	if (arg != NO_ARG) {
		uprint(" ", 1, 1);
		print_num(pfi->file_size);
		uprint("\n", 1, 1);
		return;
	}	
	uprint("  ", 2, 1);
}

static void print_content(CDIRINFO_t *pcdi, int arg) {
	// 从第一个 entry 开始
	pcdi->cur_entry = 0;
	// 计算文件数和文件夹数
	int file_n = 0, dir_n = 0;	
	FINFO_t fi;
	cal_file_dir_num(&fi, pcdi, &file_n, &dir_n);
	// 输出当前路径名和冒号
	uprint(pcdi->dir_name, strlen(pcdi->dir_name), 1);
	// 输出文件数和文件夹数
	if (arg != NO_ARG) {
		uprint(" ", 1, 1);
		print_num(dir_n);
		uprint(" ", 1, 1);
		print_num(file_n);
	}
	uprint(":\n", 2, 1);
	CDIRINFO_t cdi_back;
	init_cdi(&cdi_back);
	assert(pcdi->cur_entry == 0);
	while (next_entry(pcdi, &fi) == SUCCESS) {
		if (fi.dir_attr == FILE_ATTR || fi.dir_attr == FILE_ATTR_WIN) print_file(&fi, arg);
		else if (fi.dir_attr == DIR_ATTR) print_dir(pcdi, &fi, arg);
	}
	assert(pcdi->cur_entry == 0);
	while (next_entry(pcdi, &fi) == SUCCESS) {
		if (fi.dir_attr == FILE_ATTR || fi.dir_attr == FILE_ATTR_WIN) continue;
		if (is_relative_dir(&fi)) continue;
		// 打印换行符, 开始递归
		uprint("\n", 1, 1);
		// 当前文件夹名
		char real_name[12] = { 0 };
		get_dir_name(&fi, real_name);
		// 打开文件夹, 进入, print_content
		// pcdi 的 cur_entry 不一定为 0
		cdicpy(&cdi_back, pcdi);
		open_dir(real_name, real_name, pcdi);
		print_content(pcdi, arg);
		// 恢复 pcdi
		cdicpy(pcdi, &cdi_back);
	}
}

int cmd_ls(char *args, CDIRINFO_t *pcdi) {
	char *dir = NULL;
	int argu = NO_ARG;
	if (parse_args(args, &dir, &argu) == FAILURE) return CONTINUE;
	if (dir == NULL) dir = pcdi->dir_name;
	CDIRINFO_t cdi_back;
	init_cdi(&cdi_back);
	cdicpy(&cdi_back, pcdi);
	// read fat12
	if (open_dir(dir, dir, pcdi) == FAILURE) {
		uerror("can not open dir '%s'", dir);
		return CONTINUE;
	}
	// open successfully, now we are in `dir`
	print_content(pcdi, argu);
	uprint("\n", 1, 1);
	cdicpy(pcdi, &cdi_back);
	return CONTINUE;
}
