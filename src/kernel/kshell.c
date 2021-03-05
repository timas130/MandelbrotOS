#include <kernel/colors.h>
#include <kernel/cpuid.h>
#include <kernel/kbd.h>
#include <kernel/kpanic.h>
#include <kernel/kshell.h>
#include <kernel/pit.h>
#include <kernel/power.h>
#include <kernel/text.h>
#include <kernel/vbe.h>
#include <kernelinf.h>
#include <stdbool.h>
#include <string.h>

void cmd_help(int argc, const char *argv[]);
void cmd_hello() { printf("Hello, world!\r\n"); }
void cmd_mandelbrot(int argc, const char *argv[]) {
  if (argc == 5) {
    mandelbrot(atof(argv[1]), atof(argv[2]), atof(argv[3]), atof(argv[4]),
               0x000000);
  } else if (argc == 1) {
    mandelbrot(-1.95, -1.2, 2.5, 2.5, 0x000000);
  } else {
    printf("Usage: %s [<left> <top> <xside> <yside>]\r\n", argv[0]);
  }
}
void cmd_bsod() {
  printf("WARNING: this command will cause a BSoD, it's "
         "used just for testing.\r\nContinue: [y/N] ");
  char *confirm = gets();
  for (int i = 0; confirm[i]; i++) {
    confirm[i] = tolower(confirm[i]);
  }

  if (strcmp(confirm, "yes") != 0 || strcmp(confirm, "y") != 0) {
    kpanic("User induced BSOD", 20);
  } else {
    printf("Aborted!\r\n");
  }
}
void cmd_cls() { cls(); }
void cmd_sysfetch() {
  printf("%s\r\n", KERNEL_ARTS);

  // Value chosen to be 1 hour, 1 minute, 1 second, and 1 millisecond
  long milli = timer_ticks;
  // 3600000 milliseconds in an hour
  long hr = milli / 3600000;
  milli = milli - 3600000 * hr;
  // 60000 milliseconds in a minute
  long min = milli / 60000;
  milli = milli - 60000 * min;

  // 1000 milliseconds in a second
  long sec = milli / 1000;
  milli = milli - 1000 * sec;

  printf("\r\nKernel name: %s\r\nKernel version: %s\r\nKernel build date: %s "
         "%s\r\nUptime: %u hours %u minutes %u seconds\r\nCPU brand: %s\r\nCPU "
         "model number: "
         "%u\r\n",
         KERNEL_NAME, KERNEL_VERS, KENREL_DATE, KERNEL_TIME, hr, min, sec,
         get_vendor(), get_model());

#ifdef __clang__
  printf("Compiler version: clang %d.%d.%d\r\n", __clang_major__,
         __clang_minor__, __clang_patchlevel__);
#else
  printf("Compiler version: gcc %d.%d.%d\r\n", __GNUC__, __GNUC_MINOR__,
         __GNUC_PATCHLEVEL__);
#endif

  //#region color test
  bg_color = RED;
  printf(" ");
  bg_color = GREEN;
  printf(" ");
  bg_color = YELLOW;
  printf(" ");
  bg_color = BLUE;
  printf(" ");
  bg_color = MAGENTA;
  printf(" ");
  bg_color = CYAN;
  printf(" ");
  bg_color = WHITE;
  printf(" ");
  bg_color = GRAY;
  printf(" ");
  bg_color = FG;
  printf(" ");

  printf("\r\n");

  bg_color = DARK_RED;
  printf(" ");
  bg_color = DARK_GREEN;
  printf(" ");
  bg_color = DARK_YELLOW;
  printf(" ");
  bg_color = DARK_BLUE;
  printf(" ");
  bg_color = DARK_MAGENTA;
  printf(" ");
  bg_color = DARK_CYAN;
  printf(" ");
  bg_color = BLACK;
  printf(" ");
  bg_color = DARK_GRAY;
  printf(" ");
  bg_color = BG;
  printf(" ");

  bg_color = BG;
  printf("\r\n\n");
  //#endregion color test
}
void cmd_reboot() { reboot(); }
void cmd_echo(int argc, const char *argv[]) {
  for (int i = 1; i < argc; i++) {
    printf(argv[i]);
    printf(" ");
  }
  printf("\r\n");
}
void cmd_ctheme(int argc, const char *argv[]) {
  if (strcmp((char *)argv[1], "help") != 0 || argc != 2) {
    printf("Usage: ctheme <theme>\r\n"
           "Available themes:\r\n");
    for (size_t i = 0; i < sizeof(themes) / sizeof(char *); ++i) {
      printf("- %s\r\n", themes[i]);
    }
    return;
  }
  for (unsigned long i = 0; i != strlen(*themes); i++) {
    if (strcmp(themes[i], (char *)argv[1]) != 0) {
      printf("The theme is now %s\r\n", themes[i]);
      // TODO: make a struct array for all themes, this is ugly
      init_color(theme_red[i], theme_dred[i], theme_green[i], theme_dgreen[i],
                 theme_yellow[i], theme_dyellow[i], theme_blue[i],
                 theme_dblue[i], theme_magenta[i], theme_dmagenta[i],
                 theme_cyan[i], theme_dcyan[i], theme_white[i], theme_black[i],
                 theme_gray[i], theme_dgray[i], theme_bg[i], theme_fg[i]);
      init_text(5);
      return;
    }
  }
  fg_color = RED;
  printf("error: ");
  fg_color = FG;
  printf("The selected theme doesn't exist\r\n");
}

static shell_command_t commands[] = {
    {"help", "print the available commands", cmd_help},
    {"hello", "print \"Hello, world!\"", cmd_hello},
    {"mandelbrot", "draw a mandelbrot", cmd_mandelbrot},
    {"bsod", "show a blue screen of death", cmd_bsod},
    {"cls", "clear the screen", cmd_cls},
    {"clear", "alias for cls (clear the screen)", cmd_cls},
    {"sysfetch", "get information about the system", cmd_sysfetch},
    {"reboot", "reboot the system", cmd_reboot},
    {"echo", "print the string passed as the argument", cmd_echo},
    {"ctheme", "change the current theme", cmd_ctheme}};
static int commands_amount = sizeof(commands) / sizeof(shell_command_t);

void cmd_help(int argc, const char *argv[]) {
  int page = 0;
  if (argc > 1) {
    page = atoi((char *)argv[1]) - 1;
  }

#define CMD_HELP_PAGE_SIZE 10

  int total_pages = commands_amount / CMD_HELP_PAGE_SIZE +
                    (commands_amount % CMD_HELP_PAGE_SIZE != 0);
  if (page >= total_pages) {
    printf("%s: invalid page number\r\n", argv[0]);
    return;
  }

  printf("# help page %d\r\n", page + 1);
  for (int i = 10 * page; i < 10 * page + 10; ++i) {
    if (i >= commands_amount)
      break;
    printf("%s - %s\r\n", commands[i].name, commands[i].description);
  }
  if (page + 1 >= total_pages)
    printf("# to open the next page, do \"help %d\"\r\n", page + 2);
}

int kshell(void *mbi, unsigned long magic) {
  while (1) {
    printf("PROMPT> ");

    char *args = gets();
    char line[strlen(args) + 1];
    const char *argv[wspaceamount(args) + 1];
    int argc = 0;

    for (unsigned int i = 0; i < strlen(args) + 1; i++) {
      line[i] = args[i];
    }

    argv[argc] = strtok(line, " ");
    while (argv[argc]) {
      argc++;
      argv[argc] = strtok(0, " ");
    }
    if (argc == 0) {
      continue;
    }

    bool command_found = false;
    for (int n = 0; n < commands_amount; ++n) {
      if (strcmp((char *)argv[0], commands[n].name) != 0) {
        commands[n].func(argc, argv);
        command_found = true;
        break;
      }
    }
    if (!command_found)
      printf("Command not found: %s\r\n", argv[0]);
  }
}
