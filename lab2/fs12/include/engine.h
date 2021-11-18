#ifndef __ENGINE_H_
#define __ENGINE_H_

#include "fs.h"
#define QUIT 1
#define CONTINUE 0

int cmd_ls(char *args, CDIRINFO_t *pcdi);

int cmd_cat(char *args, CDIRINFO_t *pcdi);

int cmd_exit(char *args, CDIRINFO_t *pcdi);

int engine_start(char *img);

void engine_shut();

#endif

