#include <cpuid.h>
#include <kernel/alloc.h>
#include <kernel/cpuid.h>
#include <kernel/text.h>
#include <stdint.h>

int cpuid_string(int code, uint32_t where[4]) {
  __cpuid(code, *where, *(where + 1), *(where + 3), *(where + 2));
  return (int)where[0];
}

int get_model(void) {
  int ebx, unused;
  __cpuid(0, unused, ebx, unused, unused);
  return ebx;
}

char *get_vendor() {
  // FIXME: not using regular array because it would allocate
  //        it on the stack
  uint32_t *string = malloc(5 * sizeof(uint32_t));
  cpuid_string(0, string);

  return (char *)(string + 1);
}
