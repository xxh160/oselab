#include <engine.h>
#include <fs.h>
#include <common.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <stdlib.h>

// char* readline (const char *prompt);
static char* get_cmd(CDIRINFO_t *pcdi) {
  static char *line_read = NULL;
  if (line_read != NULL) {
    free(line_read);
    line_read = NULL;
  }
	char pwd[80] = {0};
	strcat(pwd, ASNI_FG_GREEN);
	strcat(pwd, "-> ");
	strcat(pwd, pcdi->dir_name);
	strcat(pwd, " ");
	strcat(pwd, ASNI_NONE);
  line_read = readline(pwd);

	if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static struct {
  const char *name;
  int (*handler) (char *, CDIRINFO_t *);
} cmd_table [] = {
	{ "ls",   cmd_ls },
	{ "cat",  cmd_cat },
	{ "exit", cmd_exit },
};

#define N_CMD ARRLEN(cmd_table)

static void main_loop(CDIRINFO_t *pcdi) {
	for (char *str; (str = get_cmd(pcdi)) != NULL; ) {
    char *str_end = str + strlen(str);
    // extract the first token as the command
    char *cmd = strtok(str, " ");
    if (cmd == NULL) continue;
    // treat the remaining string as the arguments
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) args = NULL;
    int i;
    for (i = 0; i < N_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args, pcdi) == QUIT) return;
        break;
      }
    }
    if (i == N_CMD) uerror("unknown command '%s'", cmd);
  }
}

void engine_shut() {
	close_fat12();
	uinfo("%s", "助教学长学姐多少给点分儿叭！！！！！！！");
}

int engine_start(char *img) {
	if (init_fat12(img) == FAILURE) return FAILURE;
	uinfo("%s", "welcome to xsh");
	// 当前打开的文件夹
	CDIRINFO_t cdi;
	init_cdi(&cdi);
	// 打开根目录, 在 engine 运行期间一定会有某个 dir 处于打开状态
	if (open_dir(ROOT_DIR, ROOT_DIR, &cdi) == FAILURE) return FAILURE;
	main_loop(&cdi);
	return SUCCESS;
}

