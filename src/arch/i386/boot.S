// DEFS
#define ASM_FILE 1
#define STACK_SIZE 0x4000
#define EXT_C(sym) sym
#define AOUT_KLUDGE 0

// INCLUDES
#include <multiboot.h>
      
.text
.globl  start, _start
.comm   stack, STACK_SIZE

start:
_start:
  jmp multiboot_entry

.align 8
multiboot_header:
  // header
  .long MULTIBOOT2_HEADER_MAGIC
  .long MULTIBOOT_ARCHITECTURE_I386
  .long multiboot_end - multiboot_header
  .long 0x100000000 - (MULTIBOOT2_HEADER_MAGIC + MULTIBOOT_ARCHITECTURE_I386 + (multiboot_end - multiboot_header))

.align 8
framebuffer_start:
  // framebuffer
  .short MULTIBOOT_HEADER_TAG_FRAMEBUFFER
  .short 0
  .long framebuffer_end - framebuffer_start
  .long 1024
  .long 768
  .long 32
framebuffer_end:

.align 8
multiboot_end_tag:
  // end tag
  .short MULTIBOOT_HEADER_TAG_END
  .short 0
  .long 8
multiboot_end:

multiboot_entry:
  movl    $(stack + STACK_SIZE), %esp
  pushl   $0
  popf

  pushl   %ebx
  pushl   %eax

  xor %ebp, %ebp

  call EXT_C(kernel_main)

  call halt

halt:
  hlt
  jmp halt
