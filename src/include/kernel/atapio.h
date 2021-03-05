#ifndef __ATAPIO_H__
#define __ATAPIO_H__

#include <stdbool.h>
#include <stdint.h>

typedef struct pio_bus {
  uint16_t base_port;
  uint16_t base_control_port;
  uint8_t selected_drive;
} pio_bus_t;

bool has_ata_drive(pio_bus_t *device, bool slave);
int ata_pio_read(uint16_t *target, uint64_t lba, uint16_t sectors,
                 pio_bus_t *device, bool slave);
int ata_pio_write(uint16_t *bytes, uint64_t lba, uint16_t sectors,
                  pio_bus_t *device, bool slave);

#define PIO_PORT_DATA 0
#define PIO_PORT_ERROR 1
#define PIO_PORT_FEATURES 1
#define PIO_PORT_SECTOR_COUNT 2
#define PIO_PORT_LBA_LO 3
#define PIO_PORT_LBA_MID 4
#define PIO_PORT_LBA_HI 5
#define PIO_PORT_DRIVE_HEAD 6
#define PIO_PORT_STATUS 7
#define PIO_PORT_COMMAND 7

#define PIO_CONTROL_STATUS 0
#define PIO_CONTROL_DEVICE_CONTROL 0
#define PIO_CONTROL_DRIVE_ADDRESS 2

#define PIO_STATUS_ERR  1
#define PIO_STATUS_IDX  1 << 1
#define PIO_STATUS_CORR 1 << 2
#define PIO_STATUS_DRQ  1 << 3
#define PIO_STATUS_SRV  1 << 4
#define PIO_STATUS_DF   1 << 5
#define PIO_STATUS_RDY  1 << 6
#define PIO_STATUS_BSY  1 << 7

#define PIO_COMMAND_IDENTIFY 0xEC
#define PIO_COMMAND_READ_SECTORS_EXT 0x24
#define PIO_COMMAND_WRITE_SECTORS_EXT 0x34

#endif // __ATAPIO_H__
