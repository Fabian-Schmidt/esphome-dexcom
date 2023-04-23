#pragma once

#include "esphome/core/datatypes.h"

namespace esphome {
namespace dexcom {

/// @brief See bottom of this page: http://www.nongnu.org/avr-libc/user-manual/group__util__crc.html
/// Polynomial: x^16 + x^12 + x^5 + 1 (0x1021)
const u_int16_t crc_xmodem(const u_int8_t *data, const u_int8_t len);

}  // namespace dexcom
}  // namespace esphome