#pragma once

#include "esphome/core/datatypes.h"
#include "dexcom_msg.h"

namespace esphome {
namespace dexcom_ble_client {

/// @brief See bottom of this page: http://www.nongnu.org/avr-libc/user-manual/group__util__crc.html
/// Polynomial: x^16 + x^12 + x^5 + 1 (0x1021)
uint16_t crc_xmodem(const uint8_t *data, const uint8_t len);

inline uint16_t crc_xmodem(const DEXCOM_MSG *dexcom_msg, const uint8_t len) {
  return crc_xmodem((uint8_t *) dexcom_msg, len - 2);
}

}  // namespace dexcom_ble_client
}  // namespace esphome