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
#include <kernel/power.h>
#include <kernel/serial.h>
#include <kernel/text.h>
#include <kernel/vbe.h>
#include <macros.h>
#include <multiboot.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <kernel/atapio.h>

int do_checks(unsigned long magic, void *mbi) {
  if (magic != MULTIBOOT2_BOOTLOADER_MAGIC) {
    serial_writestring("Invalid multiboot2 magic\r\n");
    return 1; // return to boot.s and halt
  }
  if (init_vbe(mbi) != 0) {
    serial_writestring("Failed to find framebuffer tag");
    return 1;
  }
  return 0;
}

int kernel_main(unsigned long magic, unsigned long addr) {

  // Variable definitions
  inited_funs_no = 0;
  void *mbi = (void *)addr;

  // Init functions
  init_serial();
  do_checks(magic, mbi);
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
  set_kbd();
  
  printf("\r\n");
  printf("The keyboard layout is: %s\r\n", kbd);

  serial_writestring("Serial inited!\r\n");

  struct pio_bus bus;
  bus.base_port = 0x1F0;
  bus.base_control_port = 0x3F6;
  bus.selected_drive = 0;
  if (has_ata_device(&bus)) {
    serial_writestring("!! >>> ATA DEVICE EXISTS <<< !!\r\n");
    printf("!! >>> ATA DEVICE EXISTS <<< !!\r\n");
  } else {
    serial_writestring("ata device doesn't exist :(\r\n");
    printf("ata device doesn't exist :(\r\n");
  }

  struct pio_bus bus2;
  bus.base_port = 0x170;
  bus.base_control_port = 0x376;
  bus.selected_drive = 0;
  if (has_ata_device(&bus)) {
    serial_writestring("!! >>> ATA DEVICE 2 EXISTS <<< !!\r\n");
    printf("!! >>> ATA DEVICE 2 EXISTS <<< !!\r\n");
  } else {
    serial_writestring("ata device 2 doesn't exist :(\r\n");
    printf("ata device 2 doesn't exist :(\r\n");
  }

  uint16_t target[256];
  ata_pio_read(target, 0, 1, &bus, false);

  kshell(mbi, magic);

  return 0;
}
