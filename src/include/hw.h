#ifndef __HW_H__
#define __HW_H__

#include <stdint.h>

uint8_t inb(uint16_t _port);
void outb(uint16_t _port, uint8_t _data);

uint16_t inw(uint16_t _port);
void outw(uint16_t _port, uint16_t _data);

uint32_t inl(uint16_t _port);
void outl(uint16_t _port, uint32_t _data);

void io_wait(void);
void insl(unsigned reg, unsigned int *buffer, int quads);

unsigned char port_byte_in(unsigned short port);
void port_byte_out(unsigned short port, unsigned char data);
uint16_t port_word_in(uint16_t port);
void port_word_out(unsigned short port, unsigned short data);
void port_long_out(uint32_t port, uint32_t value);
uint32_t port_long_in(uint32_t port);

#endif // !__HW_H__
