#pragma once

#include "esphome/core/log.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/dexcom_ble_client/dexcom_ble_client.h"

namespace esphome {
namespace dexcom_ble_client {

#define DEXCOM_SENSOR_LIFETIME (100.0f /*days*/ * 24.0f * 60.0f * 60.0f)
#define DEXCOM_SENSOR_SESSION_LIFETIME (10.0f /*days*/ * 24.0f * 60.0f * 60.0f)

enum class DEXCOM_SENSOR_TYPE {
  UNSET,

  GLUCOSE_IN_MG_DL,
  GLUCOSE_IN_MMOL_L,
  GLUCOSE_TREND,
  GLUCOSE_PREDICT_IN_MG_DL,
  GLUCOSE_PREDICT_IN_MMOL_L,

  SENSOR_AGE,
  SENSOR_SESSION_AGE,
  SENSOR_REMAINING_LIFETIME,
  SENSOR_SESSION_REMAINING_LIFETIME,
};

inline static const char *enum_to_c_str(const DEXCOM_SENSOR_TYPE val) {
  switch (val) {
    case DEXCOM_SENSOR_TYPE::GLUCOSE_IN_MG_DL:
      return "GLUCOSE_IN_MG_DL";
    case DEXCOM_SENSOR_TYPE::GLUCOSE_IN_MMOL_L:
      return "GLUCOSE_IN_MMOL_L";
    case DEXCOM_SENSOR_TYPE::GLUCOSE_TREND:
      return "GLUCOSE_TREND";
    case DEXCOM_SENSOR_TYPE::GLUCOSE_PREDICT_IN_MG_DL:
      return "GLUCOSE_PREDICT_IN_MG_DL";
    case DEXCOM_SENSOR_TYPE::GLUCOSE_PREDICT_IN_MMOL_L:
      return "GLUCOSE_PREDICT_IN_MMOL_L";
    case DEXCOM_SENSOR_TYPE::SENSOR_AGE:
      return "SENSOR_AGE";
    case DEXCOM_SENSOR_TYPE::SENSOR_SESSION_AGE:
      return "SENSOR_SESSION_AGE";
    case DEXCOM_SENSOR_TYPE::SENSOR_REMAINING_LIFETIME:
      return "SENSOR_REMAINING_LIFETIME";
    case DEXCOM_SENSOR_TYPE::SENSOR_SESSION_REMAINING_LIFETIME:
      return "SENSOR_SESSION_REMAINING_LIFETIME";
    default:
      return "UNSET";
  }
}

class DexcomSensor : public Component, public sensor::Sensor, public Parented<DexcomBLEClient> {
 public:
  void dump_config() override;
  void setup() override;

  void set_type(DEXCOM_SENSOR_TYPE val) { this->type_ = val; }

 protected:
  DEXCOM_SENSOR_TYPE type_;
};
}  // namespace dexcom_ble_client
}  // namespace esphome