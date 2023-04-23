#include "helpers.h"

namespace esphome {
namespace dexcom {

const u_int16_t crc_xmodem(const u_int8_t *data, const u_int8_t len) {
  u_int16_t crc;
  for (int j = 0; j < len; j++) {
    crc = crc ^ (data[j] << 8);
    for (u_int8_t i = 0; i < 8; i++) {
      if (crc & 0x8000)
        crc = (crc << 1) ^ 0x1021;
      else
        crc <<= 1;
    }
  }
  return crc;
}

}  // namespace dexcom
}  // namespace esphome