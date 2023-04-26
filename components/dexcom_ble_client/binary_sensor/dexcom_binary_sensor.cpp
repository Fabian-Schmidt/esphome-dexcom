#include "dexcom_binary_sensor.h"
#include "esphome/core/log.h"

namespace esphome {
namespace dexcom_ble_client {

static const char *const TAG = "dexcom_ble_client.binary_sensor";

void DexcomBinarySensor::dump_config() {
  LOG_BINARY_SENSOR("", "Dexcom Binary Sensor", this);
  ESP_LOGCONFIG(TAG, "  Type '%s'", enum_to_c_str(this->type_));
}

void DexcomBinarySensor::setup() {
  this->parent_->add_on_message_callback(
      [this](const TIME_RESPONSE_MSG *time_msg, const GLUCOSE_RESPONSE_MSG *glucose_msg) {
        switch (this->type_) {
          case DEXCOM_BINARY_SENSOR_TYPE::SENSOR_LOW_BATTERY:
            this->publish_state(glucose_msg->status == DEXCOM_TRANSMITTER_STATUS::LOW_BATTERY);
            break;
          case DEXCOM_BINARY_SENSOR_TYPE::SENSOR_OKAY:
            this->publish_state(glucose_msg->status == DEXCOM_TRANSMITTER_STATUS::OKAY &&
                                glucose_msg->state == DEXCOM_CALIBRATION_STATE::OKAY);
            break;
          case DEXCOM_BINARY_SENSOR_TYPE::SENSOR_FAILED:
            // https://github.com/NightscoutFoundation/xDrip/blob/f06f3d7a5c334578914dc5623584b17c6ff7fc7b/app/src/main/java/com/eveningoutpost/dexdrip/g5model/CalibrationState.java#L60
            this->publish_state(glucose_msg->state == DEXCOM_CALIBRATION_STATE::SENSOR_FAILED_1 ||
                                glucose_msg->state == DEXCOM_CALIBRATION_STATE::SENSOR_FAILED_2 ||
                                glucose_msg->state == DEXCOM_CALIBRATION_STATE::SENSOR_FAILED_3 ||
                                glucose_msg->state == DEXCOM_CALIBRATION_STATE::SENSOR_FAILED_4 ||
                                glucose_msg->state == DEXCOM_CALIBRATION_STATE::SENSOR_FAILED_5 ||
                                glucose_msg->state == DEXCOM_CALIBRATION_STATE::SENSOR_FAILED_6 ||
                                glucose_msg->state == DEXCOM_CALIBRATION_STATE::SENSOR_FAILED_START_1);
            break;
          case DEXCOM_BINARY_SENSOR_TYPE::SENSOR_STOPPED:
            // https://github.com/NightscoutFoundation/xDrip/blob/f06f3d7a5c334578914dc5623584b17c6ff7fc7b/app/src/main/java/com/eveningoutpost/dexdrip/g5model/CalibrationState.java#L61
            this->publish_state(glucose_msg->state == DEXCOM_CALIBRATION_STATE::STOPPED ||
                                glucose_msg->state == DEXCOM_CALIBRATION_STATE::ENDED ||
                                glucose_msg->state == DEXCOM_CALIBRATION_STATE::SENSOR_FAILED_1 ||
                                glucose_msg->state == DEXCOM_CALIBRATION_STATE::SENSOR_FAILED_2 ||
                                glucose_msg->state == DEXCOM_CALIBRATION_STATE::SENSOR_FAILED_3 ||
                                glucose_msg->state == DEXCOM_CALIBRATION_STATE::SENSOR_FAILED_4 ||
                                glucose_msg->state == DEXCOM_CALIBRATION_STATE::SENSOR_FAILED_5 ||
                                glucose_msg->state == DEXCOM_CALIBRATION_STATE::SENSOR_FAILED_6 ||
                                glucose_msg->state == DEXCOM_CALIBRATION_STATE::SENSOR_FAILED_START_1 ||
                                glucose_msg->state == DEXCOM_CALIBRATION_STATE::SENSOR_STOPPED);
            break;
          case DEXCOM_BINARY_SENSOR_TYPE::SENSOR_TRANSITIONAL:
            // https://github.com/NightscoutFoundation/xDrip/blob/f06f3d7a5c334578914dc5623584b17c6ff7fc7b/app/src/main/java/com/eveningoutpost/dexdrip/g5model/CalibrationState.java#L62
            this->publish_state(glucose_msg->state == DEXCOM_CALIBRATION_STATE::WARMING_UP ||
                                glucose_msg->state == DEXCOM_CALIBRATION_STATE::SENSOR_STARTED ||
                                glucose_msg->state == DEXCOM_CALIBRATION_STATE::SENSOR_STOPPED ||
                                glucose_msg->state == DEXCOM_CALIBRATION_STATE::CALIBRATION_SENT);
            break;
          default:
            break;
        }
      });
}

}  // namespace dexcom_ble_client
}  // namespace esphome