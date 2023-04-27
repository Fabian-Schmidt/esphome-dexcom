#pragma once

#include "esphome/core/log.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/dexcom_ble_client/dexcom_ble_client.h"

namespace esphome {
namespace dexcom_ble_client {

enum class DEXCOM_BINARY_SENSOR_TYPE {
  UNSET,

  SENSOR_LOW_BATTERY,
  SENSOR_OKAY,
  SENSOR_FAILED,
  SENSOR_STOPPED,
  SENSOR_TRANSITIONAL,
};

inline static const char *enum_to_c_str(const DEXCOM_BINARY_SENSOR_TYPE val) {
  switch (val) {
    case DEXCOM_BINARY_SENSOR_TYPE::SENSOR_LOW_BATTERY:
      return "SENSOR_LOW_BATTERY";
    case DEXCOM_BINARY_SENSOR_TYPE::SENSOR_OKAY:
      return "SENSOR_OKAY";
    case DEXCOM_BINARY_SENSOR_TYPE::SENSOR_FAILED:
      return "SENSOR_FAILED";
    case DEXCOM_BINARY_SENSOR_TYPE::SENSOR_STOPPED:
      return "SENSOR_STOPPED";
    case DEXCOM_BINARY_SENSOR_TYPE::SENSOR_TRANSITIONAL:
      return "SENSOR_TRANSITIONAL";
    default:
      return "UNSET";
  }
}

class DexcomBinarySensor : public Component, public binary_sensor::BinarySensor {
 public:
  explicit DexcomBinarySensor(DexcomBLEClient *parent);
  void dump_config() override;

  void set_type(DEXCOM_BINARY_SENSOR_TYPE val) { this->type_ = val; }

 protected:
  DEXCOM_BINARY_SENSOR_TYPE type_;
};
}  // namespace dexcom_ble_client
}  // namespace esphome