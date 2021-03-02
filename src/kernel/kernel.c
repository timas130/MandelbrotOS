#include <font.h>
#include <kernel/alloc.h>
#include <kernel/gdt.h>
#include <kernel/idt.h>
#include <kernel/init.h>
#include <kernel/irq.h>
#include <kernel/isr.h>
#include <kernel/kbd.h>
#include <kernel/kshell.h>
#include <kernel/pit.h>
#include <kernel/text.h>
#include <kernel/vbe.h>
#include <kernel/power.h>
#include <kernel/serial.h>
#include <macros.h>
#include <multiboot.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

void kernel_main(unsigned long magic, unsigned long addr) {
  init_serial();

  if (magic != MULTIBOOT2_BOOTLOADER_MAGIC) {
    serial_writestring("invalid multiboot2 magic\r\n");
    return; // return to boot.s and halt
  }

  inited_funs_no = 0;
  void *mbi = (void *) addr;

  if (init_vbe(mbi) != 0) {
    serial_writestring("failed to find framebuffer tag");
    return;
  }
  init_color(0xff0000, 0x990000, 0x00ff00, 0x009900, 0xffff00, 0x999900,
             0x0000ff, 0x000099, 0xff00ff, 0x990099, 0x00ffff, 0x009999,
             0xffffff, 0x000000, 0x999999, 0x444444, 0x161925, 0xdedede);
  init_text(5);

  init_check(init_gdt(), "GDT", true);
  init_check(init_idt(), "IDT", true);
  init_check(init_isr(), "ISR", true);
  init_check(init_irq(), "IRQ", true);
  init_check(enable_interrupts(), "Interrupts", true);
  init_check(init_timer(), "Programmable interrupt timer", true);
  init_check(init_heap(0x00f00000), "Memory allocator", true);
  init_check(kbd_init(), "Keyboard", true);

  printf("\r\n");

  serial_writestring("hello from serial!\r\n");

  kshell(mbi, magic);

  return;
}
