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

bool has_ata_device(struct pio_bus *device) {
  // FIXME: doesn't work for primary bus on qemu :(
  outb(device->base_port + PIO_PORT_SECTOR_COUNT, 0);
  outb(device->base_port + PIO_PORT_LBA_LO, 0);
  outb(device->base_port + PIO_PORT_LBA_MID, 0);
  outb(device->base_port + PIO_PORT_LBA_HI, 0);
  outb(device->base_port + PIO_PORT_COMMAND, PIO_COMMAND_IDENTIFY);

  uint8_t error = inb(device->base_port + PIO_PORT_ERROR);
  uint8_t status = inb(device->base_port + PIO_PORT_STATUS);
  return ! (status == 0 || error != 0);
}

int ata_pio_read(uint16_t *target, uint32_t lba, uint8_t sectors,
                 struct pio_bus *device, bool slave) {
  uint8_t drive = 0xE0 | (slave << 4) | ((lba >> 24) & 0x0F);
  printf("drive: %x\r\n", drive);
  if (device->selected_drive != drive) {
    outb(device->base_port + PIO_PORT_DRIVE_HEAD, drive);
    device->selected_drive = drive;
    for (int i = 0; i < 4; ++i) {
      inb(device->base_control_port + PIO_CONTROL_STATUS);
    }
  }

  ata_wait_bsy(device);

  outb(device->base_port + PIO_PORT_SECTOR_COUNT, sectors);
  outb(device->base_port + PIO_PORT_LBA_LO,
       (uint8_t) lba);
  outb(device->base_port + PIO_PORT_LBA_MID,
       (uint8_t) (lba >> 8));
  outb(device->base_port + PIO_PORT_LBA_HI,
       (uint8_t) (lba >> 16));

  outb(device->base_port + PIO_PORT_COMMAND, PIO_COMMAND_READ_SECTORS);

  for (int i = 0; i < sectors; ++i) {
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

int ata_pio_write(uint16_t *bytes, uint32_t lba, uint8_t sectors,
                  struct pio_bus *device, bool slave) {
  uint8_t drive = 0xE0 | (slave << 4) | ((lba >> 24) & 0x0F);
  if (device->selected_drive != drive) {
    outb(device->base_port + PIO_PORT_DRIVE_HEAD, drive);
    device->selected_drive = drive;
    for (int i = 0; i < 4; ++i) {
      inb(device->base_control_port + PIO_CONTROL_STATUS);
    }
  }

  ata_wait_bsy(device);

  outb(device->base_port + PIO_PORT_SECTOR_COUNT, sectors);
  outb(device->base_port + PIO_PORT_LBA_LO,
       (uint8_t) lba);
  outb(device->base_port + PIO_PORT_LBA_MID,
       (uint8_t) (lba >> 8));
  outb(device->base_port + PIO_PORT_LBA_HI,
       (uint8_t) (lba >> 16));

  outb(device->base_port + PIO_PORT_COMMAND, PIO_COMMAND_WRITE_SECTORS);

  for (int i = 0; i < sectors; ++i) {
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
