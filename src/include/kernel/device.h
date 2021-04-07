#ifndef __DEVICE_H__
#define __DEVICE_H__

#include <stdint.h>

typedef struct device_t device_t;

struct device_t {
  void *device;

  int (*read)(void *device, void *data, uint64_t offset, uint64_t size);
  int (*write)(void *device, void *data, uint64_t offset, uint64_t size);
  uint64_t (*get_size)(void *device);
};

#endif // !__DEVICE_H__
