#ifndef __KSHELL_H__
#define __KSHELL_H__

#define check_cmd(c) strcmp((char *)argv[0], (char *)c)

int kshell(void *mbi, unsigned long magic);

#endif // !__KSHELL_H__