#include "helpers.h"

namespace esphome {
namespace dexcom_ble_client {

uint16_t crc_xmodem(const uint8_t *data, const uint8_t len) {
  uint16_t crc = 0;
  for (int j = 0; j < len; j++) {
    crc = crc ^ (data[j] << 8);
    for (uint8_t i = 0; i < 8; i++) {
      if (crc & 0x8000)
        crc = (crc << 1) ^ 0x1021;
      else
        crc <<= 1;
    }
  }
  return crc;
}

}  // namespace dexcom_ble_client
}  // namespace esphome