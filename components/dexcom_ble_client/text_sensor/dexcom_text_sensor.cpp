#include "dexcom_text_sensor.h"
#include "esphome/core/log.h"

namespace esphome {
namespace dexcom_ble_client {

static const char *const TAG = "dexcom_ble_client.text_sensor";

void DexcomTextSensor::dump_config() {
  LOG_TEXT_SENSOR("", "Dexcom Text Sensor", this);
  ESP_LOGCONFIG(TAG, "  Type '%s'", enum_to_c_str(this->type_));
}

DexcomTextSensor::DexcomTextSensor(DexcomBLEClient *parent) {
  parent->add_on_message_callback([this](const TIME_RESPONSE_MSG *time_msg, const GLUCOSE_RESPONSE_MSG *glucose_msg) {
    switch (this->type_) {
      case DEXCOM_TEXT_SENSOR_TYPE::STATUS:
        this->publish_state(enum_to_c_str(glucose_msg->status));
        break;
      case DEXCOM_TEXT_SENSOR_TYPE::CALIBRATION_STATE:
        this->publish_state(enum_to_c_str(glucose_msg->state));
        break;
      default:
        break;
    }
  });
}

}  // namespace dexcom_ble_client
}  // namespace esphome