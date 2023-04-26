#pragma once

#include "esphome/core/log.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/dexcom_ble_client/dexcom_ble_client.h"

namespace esphome {
namespace dexcom_ble_client {

enum class DEXCOM_TEXT_SENSOR_TYPE {
  UNSET,

  STATUS,
  CALIBRATION_STATE,
};

#ifdef ESPHOME_LOG_HAS_CONFIG
static const char *enum_to_c_str(const DEXCOM_TEXT_SENSOR_TYPE val) {
  switch (val) {
    case DEXCOM_TEXT_SENSOR_TYPE::STATUS:
      return "STATUS";
    case DEXCOM_TEXT_SENSOR_TYPE::CALIBRATION_STATE:
      return "CALIBRATION_STATE";
    default:
      return "UNSET";
  }
}
#endif  // ESPHOME_LOG_HAS_CONFIG

class DexcomTextSensor : public Component, public text_sensor::TextSensor, public Parented<DexcomBLEClient> {
 public:
  void dump_config() override;
  void setup() override;

  void set_type(DEXCOM_TEXT_SENSOR_TYPE val) { this->type_ = val; }

 protected:
  DEXCOM_TEXT_SENSOR_TYPE type_;
};

}  // namespace dexcom_ble_client
}  // namespace esphome