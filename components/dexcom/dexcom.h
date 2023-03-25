#pragma once

#include "esphome/core/component.h"
#include "esphome/components/ble_client/ble_client.h"
#include "esphome/components/esp32_ble_tracker/esp32_ble_tracker.h"
#include "esphome/components/sensor/sensor.h"

#ifdef USE_ESP32

#include <aes/esp_aes.h>

namespace esphome {
namespace dexcom {

enum class BT_NOTIFICATION_TYPE {
  UNSET,
  OFF,
  NOTIFICATION,
  INDICATION,
  NOTIFICATION_INDICATION,
};

enum class DEXCOM_OPCODE : u_int8_t {
  UNKNOWN = 0x00,
  AUTH_INIT = 0x01,
  AUTH_CHALLENGE = 0x03,
  AUTH_CHALLENGE_RESPONSE = 0x04,
  AUTH_FINISH = 0x05,
  KEEP_ALIVE = 0x06,
  BOND_REQUEST = 0x07,
  DISCONNECT = 0x09,
  TIME = 0x24,
  TIME_RESPONSE = 0x25,
};

enum class DEXCOM_BT_CHANNEL : u_int8_t {
  UNSET = 0x00,
  ALT_CHANNEL = 0x01,
  NORMAL_CHANNEL = 0x02,
};

enum class DEXCOM_AUTH_RESULT : u_int8_t {
  UNSET = 0x00,
  AUTHENTICATED = 0x01,
  NOT_AUTHENTICATED = 0x02,
};

static const char *enum_to_c_str(const DEXCOM_AUTH_RESULT val) {
  switch (val) {
    case DEXCOM_AUTH_RESULT::AUTHENTICATED:
      return "AUTHENTICATED";
    case DEXCOM_AUTH_RESULT::NOT_AUTHENTICATED:
      return "NOT_AUTHENTICATED";
    default:
      return "";
  }
}

enum class DEXCOM_BOND_REQUEST : u_int8_t {
  UNSET = 0x00,
  NO_BONDING = 0x01,
  BONDING = 0x02,
};

static const char *enum_to_c_str(const DEXCOM_BOND_REQUEST val) {
  switch (val) {
    case DEXCOM_BOND_REQUEST::NO_BONDING:
      return "NO_BONDING";
    case DEXCOM_BOND_REQUEST::BONDING:
      return "BONDING";
    default:
      return "";
  }
}

struct AUTH_INIT_MSG {  // NOLINT(readability-identifier-naming,altera-struct-pack-align)
  std::array<u_int8_t, 8> token;
  DEXCOM_BT_CHANNEL channel;
} __attribute__((packed));

struct AUTH_CHALLENGE_MSG {  // NOLINT(readability-identifier-naming,altera-struct-pack-align)
  std::array<u_int8_t, 8> tokenHash;
  std::array<u_int8_t, 8> challenge;
} __attribute__((packed));

struct AUTH_CHALLENGE_RESPONSE_MSG {  // NOLINT(readability-identifier-naming,altera-struct-pack-align)
  std::array<u_int8_t, 8> challenge_response;
} __attribute__((packed));

struct AUTH_FINISH_MSG {  // NOLINT(readability-identifier-naming,altera-struct-pack-align)
  DEXCOM_AUTH_RESULT auth;
  DEXCOM_BOND_REQUEST bond;
} __attribute__((packed));

struct KEEP_ALIVE {  // NOLINT(readability-identifier-naming,altera-struct-pack-align)
  u_int8_t unknown;  // 0x19
} __attribute__((packed));

struct DEXCOM_MSG {  // NOLINT(readability-identifier-naming,altera-struct-pack-align)
  DEXCOM_OPCODE opcode;
  union {
    AUTH_INIT_MSG init_msg;
    AUTH_CHALLENGE_MSG challenge_msg;
    AUTH_CHALLENGE_RESPONSE_MSG challenge_response_msg;
    AUTH_FINISH_MSG auth_finish_msg;
    KEEP_ALIVE keep_alive;
  };
} __attribute__((packed));

static const esp32_ble_tracker::ESPBTUUID SERVICE_UUID =
    esp32_ble_tracker::ESPBTUUID::from_raw("f8083532-849e-531c-c594-30f1f86a4ea5");

// static const esp32_ble_tracker::ESPBTUUID  esp32_ble::ESPBTUUID::from_uint16(0x2902);

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

class Dexcom : public Component, public ble_client::BLEClientNode {
 public:
  void dump_config() override;
  // void update() override;

  void gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if,
                           esp_ble_gattc_cb_param_t *param) override;
  std::string get_name() {
    if (this->name_.empty()) {
      return this->parent_->address_str();
    } else {
      return this->name_;
    }
  }
  void set_transmitter_id(const char *val) { this->transmitter_id_ = val; }
  void set_use_alternative_bt_channel(bool val) { this->use_alternative_bt_channel_ = val; }

 protected:
  std::string name_;

  const char *transmitter_id_;
  bool use_alternative_bt_channel_ = false;

  void read_incomming_msg_(const u_int16_t handle, uint8_t *value, const u_int16_t value_len);
  u_int16_t find_handle_(const esp32_ble_tracker::ESPBTUUID *characteristic);
  u_int16_t find_descriptor(u_int16_t handle);
  bool register_notify_(const u_int16_t handle, const u_int16_t handle_desc, BT_NOTIFICATION_TYPE type);
  bool write_handle_(const u_int16_t handle, uint8_t *value, const u_int16_t value_len);
  bool read_handle_(const u_int16_t handle);
  std::array<u_int8_t, 8> encrypt_(std::array<u_int8_t, 8> data);

  u_int16_t handle_communication_ = 0;
  u_int16_t handle_control_ = 0;
  u_int16_t handle_authentication_ = 0;
  u_int16_t handle_authentication_desc_ = 0;
  u_int16_t handle_backfill_ = 0;

  void reset_state(){};
};

}  // namespace dexcom
}  // namespace esphome

#endif
