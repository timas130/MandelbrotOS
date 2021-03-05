#include <drivers/serial.h>
#include <font.h>
#include <kernel/init.h>
#include <kernel/kbd.h>
#include <kernel/power.h>
#include <kernel/text.h>
#include <kernel/vbe.h>
#include <macros.h>

struct stackframe {
  struct stackframe *ebp;
  uint32_t eip;
};

void trace(unsigned int MaxFrames) {
  struct stackframe *stk;
  __asm__("movl %%ebp, %0;" : "=r"(stk));

  printf("System traceback:\n\rEBP                 EIP\r\n");
  for (unsigned int frame = 0; stk && frame < MaxFrames; ++frame) {
    // Unwind to previous stack frame
    printf("0x%X            0x%X\n\r", stk->ebp, stk->eip);
    stk = stk->ebp;
  }
}

void kpanic(char *message, int exno) {
#ifndef __clang__
  register int eax __asm__("eax");
  register int ebx __asm__("ebx");
  register int ecx __asm__("ecx");
  register int edx __asm__("edx");
  register int esi __asm__("esi");
  register int edi __asm__("edi");
  register int esp __asm__("esp");
  register int ax __asm__("ax");
  register int bx __asm__("bx");
  register int cx __asm__("cx");
  register int dx __asm__("dx");

  int eax_state = eax;
  int ebx_state = ebx;
  int ecx_state = ecx;
  int edx_state = edx;
  int esi_state = esi;
  int edi_state = edi;
  int esp_state = esp;
  int ax_state = ax;
  int bx_state = bx;
  int cx_state = cx;
  int dx_state = dx;
#endif

  serial_writestring("Kernel panic: ");
  serial_writestring(message);
  serial_writestring("\r\n");

  fg_color = RED;
  bg_color = BLUE;

  cls();

  for (unsigned long j = 0; j != (fb_width / GLYPH_WIDTH + 1) / 2 - 12; j++) {
    printf(" ");
  }

  printf("KERNEL PANIC\r\n");

  fg_color = FG;

  printf("%s: System halted!\r\nError code: %u\r\n\n", message, exno);

  printf("Your system has been halted due to: %s Error code: %u. The system "
         "cannot be recoverd from here. Please press any key to reboot your "
         "computer. Some debug "
         "information has been included bellow.\r\n\r\n",
         message, exno);

  printf("Amount of successfully inited funcs: %u\r\n", inited_funs_no);
  printf("All successfully inited funcs: ");

  for (int i = 0; i != inited_funs_no; i++) {
    printf("%s, ", inited_funcs[i]);
  }

  printf("\b\b\r\n\r\n");

#ifndef __clang__ // gcc only, doesn't work on clang properly
  printf("ASM registers:\r\n");
  printf("eax=0x%X ebx=0x%X ecx=0x%X edx=0x%X esi=0x%X edi=0x%X esp=0x%X "
         "ax=0x%X bx=0x%X cx=0x%X dx=%X\r\n\r\n",
         eax_state, ebx_state, ecx_state, edx_state, esi_state, edi_state,
         esp_state, ax_state, bx_state, cx_state, dx_state);
#endif

  trace(20);

  char prev = currkey;

  while (1) {
    char now = currkey;

    if (now != prev) {
      reboot();
    }

    prev = now;
  }
}
