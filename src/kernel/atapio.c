#include <hw.h>
#include <kernel/atapio.h>
#include <kernel/pit.h>
#include <kernel/text.h>

void ata_wait_bsy(struct pio_bus *device) {
  while (inb(device->base_port + PIO_PORT_STATUS) & PIO_STATUS_BSY);
}
void ata_wait_drq(struct pio_bus *device) {
  while (true) {
    uint8_t status = inb(device->base_port + PIO_PORT_STATUS);
    if (status & PIO_STATUS_DRQ ||
        status & PIO_STATUS_ERR ||
        status & PIO_STATUS_DF) return;
  }
}

void ata_pio_select_drive(struct pio_bus *device, bool slave) {
  uint8_t drive = 0x40 | (slave << 4);
  if (device->selected_drive != drive) {
    outb(device->base_port + PIO_PORT_DRIVE_HEAD, drive);
    device->selected_drive = drive;
    for (int i = 0; i < 4; ++i) {
      inb(device->base_control_port + PIO_CONTROL_STATUS);
    }
  }
}
void ata_pio_send_lba48(struct pio_bus *device, uint64_t lba,
                        uint16_t sectors) {
  outb(device->base_port + PIO_PORT_SECTOR_COUNT, sectors >> 8);
  outb(device->base_port + PIO_PORT_LBA_LO, lba >> 24);
  outb(device->base_port + PIO_PORT_LBA_MID, lba >> 32);
  outb(device->base_port + PIO_PORT_LBA_HI, lba >> 40);
  outb(device->base_port + PIO_PORT_SECTOR_COUNT, sectors);
  outb(device->base_port + PIO_PORT_LBA_LO, lba);
  outb(device->base_port + PIO_PORT_LBA_MID, lba >> 8);
  outb(device->base_port + PIO_PORT_LBA_HI, lba >> 16);
}

// FIXME: this is completely broken now
bool has_ata_drive(struct pio_bus *device, bool slave) {
  if (! slave) outb(device->base_port + PIO_PORT_DRIVE_HEAD, 0xA0);
  else outb(device->base_port + PIO_PORT_DRIVE_HEAD, 0xB0);

  for (int i = 0; i < 4; ++i) {
    inb(device->base_control_port + PIO_CONTROL_STATUS);
  }

  outb(device->base_port + PIO_PORT_SECTOR_COUNT, 0);
  outb(device->base_port + PIO_PORT_LBA_LO, 0);
  outb(device->base_port + PIO_PORT_LBA_MID, 0);
  outb(device->base_port + PIO_PORT_LBA_HI, 0);
  outb(device->base_port + PIO_PORT_COMMAND, PIO_COMMAND_IDENTIFY);

  uint8_t status = inb(device->base_port + PIO_PORT_STATUS);
  return status != 0;
}

int ata_pio_read(uint16_t *target, uint64_t lba, uint16_t sectors,
                 struct pio_bus *device, bool slave) {
  ata_pio_select_drive(device, slave);
  ata_wait_bsy(device);
  ata_pio_send_lba48(device, lba, sectors);

  outb(device->base_port + PIO_PORT_COMMAND,
       PIO_COMMAND_READ_SECTORS_EXT);

  for (unsigned int i = 0; i < sectors; ++i) {
    ata_wait_bsy(device);
    ata_wait_drq(device);
    uint8_t status = inb(device->base_port + PIO_PORT_STATUS);
    if (status & PIO_STATUS_ERR) {
      printf("ata error: status: %x error: %x\r\n",
             status, inb(device->base_port + PIO_PORT_ERROR));
      return 1;
    }
    if (status & PIO_STATUS_DF) {
      printf("ata drive fault: status: %x", status);
      return 2;
    }
    for (int j = 0; j < 256; ++j) {
      target[j] = inw(device->base_port + PIO_PORT_DATA);
    }
    target += 256;
  }
  return 0;
}

int ata_pio_write(uint16_t *bytes, uint64_t lba, uint16_t sectors,
                  struct pio_bus *device, bool slave) {
  ata_pio_select_drive(device, slave);
  ata_wait_bsy(device);
  ata_pio_send_lba48(device, lba, sectors);

  outb(device->base_port + PIO_PORT_COMMAND,
       PIO_COMMAND_WRITE_SECTORS_EXT);

  for (unsigned int i = 0; i < sectors; ++i) {
    ata_wait_bsy(device);
    ata_wait_drq(device);
    uint8_t status = inb(device->base_port + PIO_PORT_STATUS);
    if (status & PIO_STATUS_ERR) {
      printf("ata error: status: %x error: %x\r\n",
             status, inb(device->base_port + PIO_PORT_ERROR));
      return 1;
    }
    if (status & PIO_STATUS_DF) {
      printf("ata drive fault: status: %x", status);
      return 2;
    }
    for (int j = 0; j < 256; ++j) {
      outw(device->base_port + PIO_PORT_DATA, bytes[j]);
    }
    bytes += 256;
  }
  return 0;
}
