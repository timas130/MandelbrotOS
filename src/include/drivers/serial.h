#ifndef __SERIAL_H
#define __SERIAL_H

#define PORT 0x3f8 // COM1

int init_serial();
int serial_received();
int is_transmit_empty();
char read_serial();
void write_serial(char);
unsigned long serial_writestring(char *);
#endif
