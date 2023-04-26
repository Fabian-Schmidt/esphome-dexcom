#pragma once

#include "dexcom_msg.h"
#include "helpers.h"
#include "esphome/components/esp32_ble_client/ble_client_base.h"
#include "esphome/components/esp32_ble_tracker/esp32_ble_tracker.h"
#include "esphome/core/automation.h"
#include "esphome/core/component.h"
#include "esphome/core/helpers.h"

#ifdef USE_ESP32

#include <aes/esp_aes.h>
#include <esp_bt_defs.h>
#include <esp_gap_ble_api.h>
#include <esp_gatt_common_api.h>
#include <esp_gattc_api.h>
// #include <array>
#include <string>

namespace esphome {
namespace dexcom_ble_client {

enum class BT_NOTIFICATION_TYPE {
  UNSET,
  OFF,
  NOTIFICATION,
  INDICATION,
  NOTIFICATION_INDICATION,
};

static const esp32_ble_tracker::ESPBTUUID SERVICE_UUID =
    esp32_ble_tracker::ESPBTUUID::from_raw("f8083532-849e-531c-c594-30f1f86a4ea5");

// NOTIFY, READ
static const esp32_ble_tracker::ESPBTUUID CHARACTERISTIC_UUID_COMMUNICATION =
    esp32_ble_tracker::ESPBTUUID::from_raw("F8083533-849E-531C-C594-30F1F86A4EA5");
// INDICATE, WRITE
static const esp32_ble_tracker::ESPBTUUID CHARACTERISTIC_UUID_CONTROL =
    esp32_ble_tracker::ESPBTUUID::from_raw("F8083534-849E-531C-C594-30F1F86A4EA5");
// INDICATE, READ, WRITE (G6 Plus INDICATE / WRITE)
static const esp32_ble_tracker::ESPBTUUID CHARACTERISTIC_UUID_AUTHENTICATION =
    esp32_ble_tracker::ESPBTUUID::from_raw("F8083535-849E-531C-C594-30F1F86A4EA5");
// NOTIFY, READ, WRITE (G6 Plus NOTIFY)
static const esp32_ble_tracker::ESPBTUUID CHARACTERISTIC_UUID_BACKFILL =
    esp32_ble_tracker::ESPBTUUID::from_raw("F8083536-849E-531C-C594-30F1F86A4EA5");

class DexcomBLEClient : public esp32_ble_client::BLEClientBase {
 public:
  void setup() override;
  void dump_config() override;
  void loop() override;

  bool gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if,
                           esp_ble_gattc_cb_param_t *param) override;

  void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param) override;
  bool parse_device(const esp32_ble_tracker::ESPBTDevice &device) override;
  void set_enabled(bool enabled);

  bool enabled;

  void set_state(esp32_ble_tracker::ClientState state) override;

  void set_transmitter_id(const char *val) { this->transmitter_id_ = val; }
  void set_use_alternative_bt_channel(bool val) { this->use_alternative_bt_channel_ = val; }

  void add_on_message_callback(std::function<void(const TIME_RESPONSE_MSG *, const GLUCOSE_RESPONSE_MSG *)> callback) {
    this->on_message_callback_.add(std::move(callback));
  }

 protected:
  void read_incomming_msg_(const uint16_t handle, uint8_t *value, const uint16_t value_len);
  void submit_value_to_sensors_();
  uint16_t find_handle_(const esp32_ble_tracker::ESPBTUUID *characteristic);
  uint16_t find_descriptor(uint16_t handle);
  bool register_notify_(const uint16_t handle, const uint16_t handle_desc, BT_NOTIFICATION_TYPE type);
  bool write_handle_(const uint16_t handle, uint8_t *value, const uint16_t value_len);
  bool read_handle_(const uint16_t handle);
  std::array<uint8_t, 8> encrypt_(std::array<uint8_t, 8> data);
  bool node_established_();
  const char *get_name_c_str_();

  const char *transmitter_id_;
  bool use_alternative_bt_channel_ = false;
  esp32_ble_tracker::ClientState node_state;
  CallbackManager<void(const TIME_RESPONSE_MSG *, const GLUCOSE_RESPONSE_MSG *)> on_message_callback_{};

  uint8_t register_notify_counter_ = 0;
  uint16_t handle_communication_ = 0;
  uint16_t handle_control_ = 0;
  uint16_t handle_control_desc_ = 0;
  uint16_t handle_authentication_ = 0;
  uint16_t handle_authentication_desc_ = 0;
  uint16_t handle_backfill_ = 0;

  bool got_valid_msg_ = false;
  TIME_RESPONSE_MSG time_msg_{};
  GLUCOSE_RESPONSE_MSG glucose_msg_{};

  inline void reset_state_() {
    this->got_valid_msg_ = false;
    this->time_msg_ = {};
    this->glucose_msg_ = {};
  }
};

}  // namespace dexcom_ble_client
}  // namespace esphome

#endif  // USE_ESP32
