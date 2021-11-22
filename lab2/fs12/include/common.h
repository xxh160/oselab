#ifndef __COMMON_H_
#define __COMMON_H_

#define ROOT_ENTRY_SIZE 32
#define SECTOR_SIZE 512
#define ROOT_DIR "/" 

#define DELETE_NAME 0xe5
#define KANJI_NAME 0x05
#define END_NAME 0x00

#define FILE_ATTR 0x20
#define DIR_ATTR 0x10
#define FILE_ATTR_WIN 0x00

#define FINAL_CLUS 0xFFF

#define SUCCESS 0
#define FAILURE -1
#define TRUE 1
#define FALSE 0

#define ASNI_FG_GREEN "\33[1;32m"
#define ASNI_FG_RED "\33[1;31m"
#define ASNI_FG_BLUE "\33[1;34m"
#define ASNI_NONE "\33[0m"

#include <stdio.h>

#define uerror(format, ...) \
	printf(ASNI_FG_BLUE "[Error]: " ASNI_FG_RED format ".\n" ASNI_NONE, __VA_ARGS__)

#define uinfo(format, ...) \
	printf(ASNI_FG_BLUE "[Info ]: " ASNI_FG_GREEN format ".\n" ASNI_NONE, __VA_ARGS__)

#define ARRLEN(arr) (int)(sizeof(arr) / sizeof(arr[0]))

#endif
