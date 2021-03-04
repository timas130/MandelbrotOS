#include <hw.h>

// From Bran's OSDEV series because I can't be bothered to write my own.

inline uint8_t inb(uint16_t _port) {
  unsigned char rv;
  __asm__ __volatile__("inb %1, %0" : "=a"(rv) : "dN"(_port));
  return rv;
}
inline void outb(uint16_t _port, uint8_t _data) {
  __asm__ __volatile__("outb %1, %0" : : "dN"(_port), "a"(_data));
}

inline uint16_t inw(uint16_t _port) {
  uint16_t rv;
  __asm__ __volatile__("inw %1, %0" : "=a"(rv) : "dN"(_port));
  return rv;
}
inline void outw(unsigned short _port, uint16_t _data) {
  __asm__ __volatile__("outw %1, %0" : : "dN"(_port), "a"(_data));
}

inline uint32_t inl(uint16_t _port) {
  uint32_t rv;
  __asm__ __volatile__("inl %1, %0" : "=a"(rv) : "dN"(_port));
  return rv;
}
inline void outl(unsigned short _port, uint32_t _data) {
  __asm__ __volatile__("outl %1, %0" : : "dN"(_port), "a"(_data));
}

inline void io_wait(void) {
    __asm__ __volatile__("jmp 1f\n\t"
                         "1:jmp 2f\n\t"
                         "2:");
}

void insl(unsigned reg, unsigned int *buffer, int quads) {
  int index;
  for (index = 0; index < quads; index++) {
    buffer[index] = inb(reg);
  }
}
