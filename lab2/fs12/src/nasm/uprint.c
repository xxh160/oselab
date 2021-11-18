#include <nasm.h>
#include <common.h>
#include <string.h>

void xprint(char *str, int len, char *color_str, int color_len);

void uprint(char *str, int len, int no_color) {
	char red[12] = { 0 }, no[12] = { 0 };
	red[0] = 0x1B;
	strcat(red, "[31m");
	no[0] = 0x1B;
	strcat(no, "[39m");
	if (no_color == TRUE) xprint(str, len, no, strlen(no));
	else xprint(str, len, red, strlen(red));
}

