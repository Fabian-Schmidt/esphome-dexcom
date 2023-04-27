#pragma once

#include "dexcom_ble_client.h"

#ifdef USE_ESP32

namespace esphome {
namespace dexcom_ble_client {

class DexcomBLEClientDisconnectTrigger : public Trigger<> {
 public:
  explicit DexcomBLEClientDisconnectTrigger(DexcomBLEClient *parent) {
    parent->add_on_disconnect_callback([this]() { this->trigger(); });
  }
};

}  // namespace dexcom_ble_client
}  // namespace esphome
#endif  // USE_ESP32