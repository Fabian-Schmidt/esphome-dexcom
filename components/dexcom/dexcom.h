#pragma once

#include "dexcom_msg.h"
#include "esphome/core/component.h"
#include "esphome/components/ble_client/ble_client.h"
#include "esphome/components/esp32_ble_tracker/esp32_ble_tracker.h"
#include "esphome/components/sensor/sensor.h"

#ifdef USE_ESP32

#include <aes/esp_aes.h>

namespace esphome {
namespace dexcom {

#define DEXCOM_SENSOR_LIFETIME (100.0f /*days*/ * 24.0f * 60.0f * 60.0f)
#define DEXCOM_SENSOR_SESSION_LIFETIME (10.0f /*days*/ * 24.0f * 60.0f * 60.0f)

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

class Dexcom : public Component, public ble_client::BLEClientNode {
 public:
  void dump_config() override;
  // void update() override;

  void gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if,
                           esp_ble_gattc_cb_param_t *param) override;
  void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param) override;
  std::string get_name() {
    if (this->name_.empty()) {
      return this->parent_->address_str();
    } else {
      return this->name_;
    }
  }
  void set_transmitter_id(const char *val) { this->transmitter_id_ = val; }
  void set_use_alternative_bt_channel(bool val) { this->use_alternative_bt_channel_ = val; }
  void set_glucose_in_mg_dl(sensor::Sensor *val) { this->glucose_in_mg_dl_ = val; };
  void set_glucose_in_mmol_l(sensor::Sensor *val) { this->glucose_in_mmol_l_ = val; };
  void set_trend(sensor::Sensor *val) { this->trend_ = val; };
  void set_glucose_predict_in_mg_dl(sensor::Sensor *val) { this->glucose_predict_in_mg_dl_ = val; };
  void set_glucose_predict_in_mmol_l(sensor::Sensor *val) { this->glucose_predict_in_mmol_l_ = val; };
  void set_sensor_age(sensor::Sensor *val) { this->sensor_age_ = val; };
  void set_sensor_session_age(sensor::Sensor *val) { this->sensor_session_age_ = val; };
  void set_sensor_remaining_lifetime(sensor::Sensor *val) { this->sensor_remaining_lifetime_ = val; };
  void set_sensor_session_remaining_lifetime(sensor::Sensor *val) { this->sensor_session_remaining_lifetime_ = val; };

 protected:
  std::string name_;

  const char *transmitter_id_;
  bool use_alternative_bt_channel_ = false;

  void read_incomming_msg_(const uint16_t handle, uint8_t *value, const uint16_t value_len);
  void submit_value_to_sensors_();
  uint16_t find_handle_(const esp32_ble_tracker::ESPBTUUID *characteristic);
  uint16_t find_descriptor(uint16_t handle);
  bool register_notify_(const uint16_t handle, const uint16_t handle_desc, BT_NOTIFICATION_TYPE type);
  bool write_handle_(const uint16_t handle, uint8_t *value, const uint16_t value_len);
  bool read_handle_(const uint16_t handle);
  std::array<uint8_t, 8> encrypt_(std::array<uint8_t, 8> data);

  uint8_t register_notify_counter_ = 0;
  uint16_t handle_communication_ = 0;
  uint16_t handle_control_ = 0;
  uint16_t handle_control_desc_ = 0;
  uint16_t handle_authentication_ = 0;
  uint16_t handle_authentication_desc_ = 0;
  uint16_t handle_backfill_ = 0;

  sensor::Sensor *glucose_in_mg_dl_{nullptr};
  sensor::Sensor *glucose_in_mmol_l_{nullptr};
  sensor::Sensor *trend_{nullptr};
  sensor::Sensor *glucose_predict_in_mg_dl_{nullptr};
  sensor::Sensor *glucose_predict_in_mmol_l_{nullptr};
  sensor::Sensor *sensor_age_{nullptr};
  sensor::Sensor *sensor_session_age_{nullptr};
  sensor::Sensor *sensor_remaining_lifetime_{nullptr};
  sensor::Sensor *sensor_session_remaining_lifetime_{nullptr};

  bool got_valid_msg_ = false;
  TIME_RESPONSE_MSG time_msg_{};
  GLUCOSE_RESPONSE_MSG glucose_msg_{};

  inline void reset_state() {
    this->got_valid_msg_ = false;
    this->time_msg_ = {};
    this->glucose_msg_ = {};
  }
};

}  // namespace dexcom
}  // namespace esphome

#endif
