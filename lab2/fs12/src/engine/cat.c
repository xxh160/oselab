#include <fs.h>
#include <engine.h>
#include <string.h>
#include <common.h>
#include <nasm.h>

// 把最后一个 / 变成 \0 以区分路径和文件名
static int last_tok(char *file_path) {
	int len = strlen(file_path);
	for (int i = len - 1; i >= 0; --i) {
		if (file_path[i] == '/') {
			file_path[i] = 0;
			return SUCCESS;
		}
	}
	return FAILURE;
}

static void print_file(CFILEINFO_t *pcfi) {
	uprint((char *) pcfi->buffer, strlen((char *) pcfi->buffer), 1);
	while (next_file_clus(pcfi) == SUCCESS) {
		uprint((char *) pcfi->buffer, strlen((char *) pcfi->buffer), 1);
	}
	uprint("\n", 1, 1);
}

int cmd_cat(char *args, CDIRINFO_t *pcdi) {
	if (args == NULL) {
		uerror("%s", "file name required");
		return CONTINUE;
	}
	char *str_end = args + strlen(args);
	char *file_path = strtok(args, " ");
	char *next = file_path + strlen(file_path) + 1;	
	if (next < str_end) {
		uerror("invalid argument: '%s'", next);
		return CONTINUE;
	}
	char *file_name;
	// 有路径
	if (last_tok(file_path) == SUCCESS) {
		file_name = file_path + strlen(file_path) + 1;
	} else {
		file_name = file_path;
		file_path = NULL;
	}
	if (file_path == NULL || strlen(file_path) == 0) file_path = pcdi->dir_name;
	// start reading	
	CFILEINFO_t cfi;
	init_cfi(&cfi);
	if (read_file(file_path, file_name, &cfi, pcdi) == FAILURE) return CONTINUE;
	print_file(&cfi);
	return CONTINUE;	
}

