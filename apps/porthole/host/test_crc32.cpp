// os::crc32 guards every save on every board in the field: the table version must give the very sums the bit-by-bit
// one gave, or every stored save would read as damaged after the update.
#include "host/check.h"
#include <stdio.h>
#include <string.h>
#include "crc32.h"

static int checks = 0;
// The sum as it shipped (os/crc32.h before the table), kept here as the reference.
static uint32_t bitwise(const void* d, size_t n) {
  const uint8_t* p = (const uint8_t*)d; uint32_t c = 0xFFFFFFFFu;
  for (size_t i = 0; i < n; i++) { c ^= p[i]; for (int k = 0; k < 8; k++) c = (c >> 1) ^ (0xEDB88320u & (0u - (c & 1))); }
  return ~c;
}

int main() {
  CHECK(os::crc32("123456789", 9) == 0xCBF43926u);   // the standard check value
  CHECK(os::crc32(nullptr, 0) == 0);
  uint8_t buf[2048]; uint32_t r = 2463534242u;
  for (int round = 0; round < 8; round++) {
    for (uint8_t& b : buf) { r ^= r << 13; r ^= r >> 17; r ^= r << 5; b = round == 0 ? 0 : round == 1 ? 0xFF : (uint8_t)r; }
    for (size_t n = 0; n <= sizeof buf; n += n < 64 ? 1 : 61) CHECK(os::crc32(buf, n) == bitwise(buf, n));   // every short length, then strides
    for (size_t at = 1; at < 9; at++) CHECK(os::crc32(buf + at, 935) == bitwise(buf + at, 935));          // any alignment
  }
  for (int v = 0; v < 256; v++) { const uint8_t b = (uint8_t)v; CHECK(os::crc32(&b, 1) == bitwise(&b, 1)); }
  printf("test_crc32: %d checks passed\n", checks);
  return 0;
}
