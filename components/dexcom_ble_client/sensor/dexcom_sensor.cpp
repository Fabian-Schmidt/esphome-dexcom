#include "dexcom_sensor.h"
#include "esphome/core/log.h"

namespace esphome {
namespace dexcom_ble_client {

static const char *const TAG = "dexcom_ble_client.sensor";

void DexcomSensor::dump_config() {
  LOG_SENSOR("", "Dexcom Sensor", this);
  ESP_LOGCONFIG(TAG, "  Type '%s'", enum_to_c_str(this->type_));
}

void DexcomSensor::setup() {
  this->parent_->add_on_message_callback(
      [this](const TIME_RESPONSE_MSG *time_msg, const GLUCOSE_RESPONSE_MSG *glucose_msg) {
        switch (this->type_) {
          case DEXCOM_SENSOR_TYPE::GLUCOSE_IN_MG_DL:
            this->publish_state(glucose_msg->glucose);
            break;
          case DEXCOM_SENSOR_TYPE::GLUCOSE_IN_MMOL_L:
            this->publish_state(((float) glucose_msg->glucose) / 18.0f);
            break;
          case DEXCOM_SENSOR_TYPE::GLUCOSE_TREND:
            this->publish_state(glucose_msg->trend);
            break;
          case DEXCOM_SENSOR_TYPE::GLUCOSE_PREDICT_IN_MG_DL:
            this->publish_state(glucose_msg->predicted_glucose);
            break;
          case DEXCOM_SENSOR_TYPE::GLUCOSE_PREDICT_IN_MMOL_L:
            this->publish_state(((float) glucose_msg->predicted_glucose) / 18.0f);
            break;
          case DEXCOM_SENSOR_TYPE::SENSOR_AGE:
            this->publish_state(time_msg->currentTime);
            break;
          case DEXCOM_SENSOR_TYPE::SENSOR_SESSION_AGE:
            this->publish_state(time_msg->currentTime - time_msg->sessionStartTime);
            break;
          case DEXCOM_SENSOR_TYPE::SENSOR_REMAINING_LIFETIME:
            this->publish_state(DEXCOM_SENSOR_LIFETIME - time_msg->currentTime);
            break;
          case DEXCOM_SENSOR_TYPE::SENSOR_SESSION_REMAINING_LIFETIME:
            this->publish_state(DEXCOM_SENSOR_SESSION_LIFETIME - (time_msg->currentTime - time_msg->sessionStartTime));
            break;
          default:
            break;
        }
      });
}

}  // namespace dexcom_ble_client
}  // namespace esphome