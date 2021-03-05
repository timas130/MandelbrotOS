#ifndef __KSHELL_H__
#define __KSHELL_H__

#define check_cmd(c) strcmp((char *)argv[0], (char *)c)

typedef struct shell_command {
  char *name;
  char *description;
  void (*func)(int argc, const char *argv[]);
} shell_command_t;

int kshell(void *mbi, unsigned long magic);

#endif // !__KSHELL_H__
