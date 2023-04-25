#include "dexcom.h"
#include "helpers.h"
#include "esphome/core/log.h"

#ifdef USE_ESP32

namespace esphome {
namespace dexcom {

static const char *const TAG = "dexcom";

void Dexcom::dump_config() {
  ESP_LOGCONFIG(TAG, "Dexcom");
  ESP_LOGCONFIG(TAG, "  Transmitter id: %s", this->transmitter_id_);
  ESP_LOGCONFIG(TAG, "  Use Alternative BT Channel: %s", YESNO(this->use_alternative_bt_channel_));
  LOG_SENSOR("  ", "Glucose in mg/dl", this->glucose_in_mg_dl_);
  LOG_SENSOR("  ", "Glucose in mmol/l", this->glucose_in_mmol_l_);
  LOG_SENSOR("  ", "Trend", this->trend_);
  LOG_SENSOR("  ", "Glucose predict in mg/dl", this->glucose_predict_in_mg_dl_);
  LOG_SENSOR("  ", "Glucose predict in mmol/l", this->glucose_predict_in_mmol_l_);
  LOG_SENSOR("  ", "Sensor age", this->sensor_age_);
  LOG_SENSOR("  ", "Sensor session age", this->sensor_session_age_);
  LOG_SENSOR("  ", "Sensor remaining lifetime", this->sensor_remaining_lifetime_);
  LOG_SENSOR("  ", "Sensor session remaining lifetime", this->sensor_session_remaining_lifetime_);
}

void Dexcom::gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param) {
  switch (event) {
    case ESP_GATTC_OPEN_EVT:
      if (param->open.status == ESP_GATT_OK) {
        ESP_LOGI(TAG, "[%s] Connected successfully", this->get_name().c_str());
        this->reset_state();
      }
      break;

    case ESP_GATTC_DISCONNECT_EVT:
      ESP_LOGI(TAG, "[%s] Disconnected", this->get_name().c_str());
      this->submit_value_to_sensors_();
      break;

    case ESP_GATTC_SEARCH_CMPL_EVT:
      ESP_LOGD(TAG, "[%s] Search complete", this->get_name().c_str());
      this->node_state = esp32_ble_tracker::ClientState::ESTABLISHED;
      this->handle_communication_ = this->find_handle_(&CHARACTERISTIC_UUID_COMMUNICATION);
      ESP_LOGVV(TAG, "[%s] handle_communication 0x%04x", this->get_name().c_str(), this->handle_communication_);
      this->handle_control_ = this->find_handle_(&CHARACTERISTIC_UUID_CONTROL);
      ESP_LOGVV(TAG, "[%s] handle_control 0x%04x", this->get_name().c_str(), this->handle_control_);
      this->handle_control_desc_ = this->find_descriptor(handle_control_);
      ESP_LOGVV(TAG, "[%s] handle_control_desc 0x%04x", this->get_name().c_str(), this->handle_control_desc_);
      this->handle_authentication_ = this->find_handle_(&CHARACTERISTIC_UUID_AUTHENTICATION);
      ESP_LOGVV(TAG, "[%s] handle_authentication 0x%04x", this->get_name().c_str(), this->handle_authentication_);
      this->handle_authentication_desc_ = this->find_descriptor(handle_authentication_);
      ESP_LOGVV(TAG, "[%s] handle_authentication_desc 0x%04x", this->get_name().c_str(),
                this->handle_authentication_desc_);
      this->handle_backfill_ = this->find_handle_(&CHARACTERISTIC_UUID_BACKFILL);
      ESP_LOGVV(TAG, "[%s] handle_backfill 0x%04x", this->get_name().c_str(), this->handle_backfill_);

      this->register_notify_(this->handle_authentication_, this->handle_authentication_desc_,
                             BT_NOTIFICATION_TYPE::INDICATION);

    case ESP_GATTC_NOTIFY_EVT:
      ESP_LOGD(TAG, "[%s] Notify to handle 0x%04x is %s, data=%s", this->get_name().c_str(), param->notify.handle,
               param->notify.is_notify ? "notify" : "indicate",
               format_hex_pretty(param->notify.value, param->notify.value_len).c_str());
      this->read_incomming_msg_(param->notify.handle, param->notify.value, param->notify.value_len);
      break;

    case ESP_GATTC_WRITE_DESCR_EVT:
      if (param->write.status == ESP_GATT_OK) {
        ESP_LOGV(TAG, "[%s] Write to descr handle 0x%04x status=%d", this->get_name().c_str(), param->write.handle,
                 param->write.status);
        this->register_notify_counter_++;
        if (this->register_notify_counter_ == 2) {
          if (param->write.handle == this->handle_authentication_desc_) {
            DEXCOM_MSG response;
            response.opcode = DEXCOM_OPCODE::AUTH_INIT;
            response.init_msg.token = {0x19, 0xF3, 0x89, 0xF8, 0xB7, 0x58, 0x41, 0x33};
            response.init_msg.channel =
                this->use_alternative_bt_channel_ ? DEXCOM_BT_CHANNEL::ALT_CHANNEL : DEXCOM_BT_CHANNEL::NORMAL_CHANNEL;
            this->write_handle_(this->handle_authentication_, (uint8_t *) &response, 1 + sizeof(AUTH_INIT_MSG));
          } else if (param->write.handle == this->handle_control_desc_) {
            DEXCOM_MSG response;
            response.opcode = DEXCOM_OPCODE::TIME;
            response.time.crc = crc_xmodem(&response, 1 + sizeof(TIME_MSG));
            this->write_handle_(this->handle_control_, (uint8_t *) &response, 1 + sizeof(TIME_MSG));
          }
        }
      } else {
        ESP_LOGW(TAG, "[%s] Write to descr handle 0x%04x status=%d", this->get_name().c_str(), param->write.handle,
                 param->write.status);
      }
      break;

    default:
      break;
  }
}

void Dexcom::gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param) {
  switch (event) {
    case ESP_GAP_BLE_AUTH_CMPL_EVT:
      if (param->ble_security.auth_cmpl.success) {
        this->register_notify_(this->handle_control_, this->handle_control_desc_, BT_NOTIFICATION_TYPE::INDICATION);
      }
      break;
    default:
      break;
  }
}

void Dexcom::read_incomming_msg_(const uint16_t handle, uint8_t *value, const uint16_t value_len) {
  auto dexcom_msg = (const DEXCOM_MSG *) value;
  DEXCOM_MSG response;

  if (handle == this->handle_authentication_) {
    switch (dexcom_msg->opcode) {
      case DEXCOM_OPCODE::AUTH_CHALLENGE:
        if (value_len == (1 + sizeof(AUTH_CHALLENGE_MSG))) {
          // Here we could check if the tokenHash is the encrypted 8 bytes from the
          // authRequestTxMessage ([1] to [8]); To check if the Transmitter is a valid dexcom
          // transmitter (because only the correct one should know the ID).

          response.opcode = DEXCOM_OPCODE::AUTH_CHALLENGE_RESPONSE;
          response.challenge_response_msg.challenge_response = this->encrypt_(dexcom_msg->challenge_msg.challenge);
          this->write_handle_(handle, (uint8_t *) &response, 1 + sizeof(AUTH_CHALLENGE_RESPONSE_MSG));
        }
        break;
      case DEXCOM_OPCODE::AUTH_FINISH:
        if (value_len == (1 + sizeof(AUTH_FINISH_MSG))) {
          ESP_LOGD(TAG, "[%s] Auth result %s %s", this->get_name().c_str(),
                   enum_to_c_str(dexcom_msg->auth_finish_msg.auth), enum_to_c_str(dexcom_msg->auth_finish_msg.bond));

          if (dexcom_msg->auth_finish_msg.auth == DEXCOM_AUTH_RESULT::AUTHENTICATED) {
            if (dexcom_msg->auth_finish_msg.bond == DEXCOM_BOND_REQUEST::BONDING) {
              // Request bonding
              // response.opcode = DEXCOM_OPCODE::KEEP_ALIVE;
              // response.keep_alive.time = 10;
              // this->write_handle_(handle, (uint8_t *) &response, 1 + sizeof(KEEP_ALIVE_MSG));

              response.opcode = DEXCOM_OPCODE::BOND_REQUEST;
              this->write_handle_(handle, (uint8_t *) &response, 1);
            } else {
              // What TODO here?
            }
          } else {
            // What TODO here?
          }
        }
        break;
      case DEXCOM_OPCODE::BOND_REQUEST_RESPONSE:
        if (value_len == (1) + sizeof(BOND_REQUEST_RESPONSE_MSG)) {
          ESP_LOGD(TAG, "[%s] BOND_REQUEST_RESPONSE %u", this->get_name().c_str(),
                   dexcom_msg->bond_request_response_msg.unknown);
        }
        break;
      case DEXCOM_OPCODE::INVALID_RESPONSE:
        if (value_len == (1 + sizeof(INVALID_RESPONSE_MSG))) {
          ESP_LOGW(TAG, "[%s] Invalid message %u len: %u", this->get_name().c_str(),
                   dexcom_msg->invalid_response.opcode, dexcom_msg->invalid_response.msg_length);
        }
        break;
      default:
        break;
    }
  } else if (handle == this->handle_control_) {
    switch (dexcom_msg->opcode) {
      case DEXCOM_OPCODE::TIME_RESPONSE:
        if (value_len >= (1 + sizeof(TIME_RESPONSE_MSG))) {
          const uint16_t crc = crc_xmodem(value, (1 + sizeof(TIME_RESPONSE_MSG)) - 2);
          if (dexcom_msg->time_response.crc != crc) {
            ESP_LOGW(TAG, "Time - CRC error");
          } else if (!enum_value_okay(dexcom_msg->time_response.status)) {
            ESP_LOGW(TAG, "Time - Status: %s", enum_to_c_str(dexcom_msg->time_response.status));
          } else {
            this->time_msg_ = dexcom_msg->time_response;

            ESP_LOGI(TAG, "Time - Status:           %s (%u)", enum_to_c_str(dexcom_msg->time_response.status),
                     dexcom_msg->time_response.status);

            ESP_LOGI(TAG, "Time - Since activation: %d (%d days, %d hours)", dexcom_msg->time_response.currentTime,
                     dexcom_msg->time_response.currentTime / (60 * 60 * 24),     // Days round down
                     (dexcom_msg->time_response.currentTime / (60 * 60)) % 24);  // Remaining hours
            ESP_LOGD(TAG, "Time - Since start:      %d", dexcom_msg->time_response.sessionStartTime);
            const auto session_runtime =
                dexcom_msg->time_response.currentTime - dexcom_msg->time_response.sessionStartTime;
            ESP_LOGI(TAG, "Time - Session runtime:  %d (%d days, %d hours)", session_runtime,
                     session_runtime / (60 * 60 * 24),     // Days round down
                     (session_runtime / (60 * 60)) % 24);  // Remaining hours

            if (dexcom_msg->time_response.status == DEXCOM_TRANSMITTER_STATUS::LOW_BATTERY) {
              ESP_LOGW(TAG, "Time - Low Battery");
            }

            response.opcode = DEXCOM_OPCODE::G6_GLUCOSE_MSG;
            response.glucose_msg.crc = crc_xmodem(&response, 1 + sizeof(GLUCOSE_MSG));
            this->write_handle_(handle, (uint8_t *) &response, 1 + sizeof(GLUCOSE_MSG));
          }
        }
        break;
      case DEXCOM_OPCODE::G6_GLUCOSE_RESPONSE_MSG:
        if (value_len >= (1 + sizeof(GLUCOSE_RESPONSE_MSG))) {
          const uint16_t crc = crc_xmodem(value, (1 + sizeof(GLUCOSE_RESPONSE_MSG)) - 2);
          if (dexcom_msg->glucose_response_msg.crc != crc) {
            ESP_LOGW(TAG, "Glucose - CRC error");
          } else if (!enum_value_okay(dexcom_msg->glucose_response_msg.status)) {
            ESP_LOGW(TAG, "Glucose - Status: %s", enum_to_c_str(dexcom_msg->glucose_response_msg.status));
          } else if (!enum_value_okay(dexcom_msg->glucose_response_msg.state)) {
            ESP_LOGW(TAG, "Glucose - State: %s", enum_to_c_str(dexcom_msg->glucose_response_msg.state));
          } else {
            this->glucose_msg_ = dexcom_msg->glucose_response_msg;
            this->got_valid_msg_ = true;

            ESP_LOGI(TAG, "Glucose - Status:          %s (%u)", enum_to_c_str(dexcom_msg->glucose_response_msg.status),
                     dexcom_msg->glucose_response_msg.status);
            ESP_LOGD(TAG, "Glucose - Sequence:        %u", dexcom_msg->glucose_response_msg.sequence);
            ESP_LOGI(TAG, "Glucose - Timestamp:       %u", dexcom_msg->glucose_response_msg.timestamp);
            ESP_LOGI(TAG, "Glucose - Glucose:         %u", dexcom_msg->glucose_response_msg.glucose);
            ESP_LOGI(TAG, "Glucose - DisplayOnly:     %s",
                     YESNO(dexcom_msg->glucose_response_msg.glucoseIsDisplayOnly));
            ESP_LOGI(TAG, "Glucose - State:           %s (%u)", enum_to_c_str(dexcom_msg->glucose_response_msg.state),
                     dexcom_msg->glucose_response_msg.state);
            ESP_LOGI(TAG, "Glucose - Trend:           %i", dexcom_msg->glucose_response_msg.trend);
            ESP_LOGI(TAG, "Glucose - Glucose predict: %u", dexcom_msg->glucose_response_msg.predicted_glucose);
          }

          response.opcode = DEXCOM_OPCODE::DISCONNECT;
          this->write_handle_(handle, (uint8_t *) &response, 1);
        }

        break;
      default:
        break;
    }
  }
}

void Dexcom::submit_value_to_sensors_() {
  if (this->got_valid_msg_) {
    if (this->glucose_in_mg_dl_ != nullptr) {
      this->glucose_in_mg_dl_->publish_state(this->glucose_msg_.glucose);
    }
    if (this->glucose_in_mmol_l_ != nullptr) {
      this->glucose_in_mmol_l_->publish_state(((float) this->glucose_msg_.glucose) / 18.0f);
    }
    if (this->trend_ != nullptr) {
      this->trend_->publish_state(this->glucose_msg_.trend);
    }
    if (this->glucose_predict_in_mg_dl_ != nullptr) {
      this->glucose_predict_in_mg_dl_->publish_state(this->glucose_msg_.predicted_glucose);
    }
    if (this->glucose_predict_in_mmol_l_ != nullptr) {
      this->glucose_predict_in_mmol_l_->publish_state(((float) this->glucose_msg_.predicted_glucose) / 18.0f);
    }
    if (this->sensor_age_ != nullptr) {
      this->sensor_age_->publish_state(this->time_msg_.currentTime);
    }
    if (this->sensor_session_age_ != nullptr) {
      this->sensor_session_age_->publish_state(this->time_msg_.currentTime - this->time_msg_.sessionStartTime);
    }
    if (this->sensor_remaining_lifetime_ != nullptr) {
      this->sensor_remaining_lifetime_->publish_state(DEXCOM_SENSOR_LIFETIME - this->time_msg_.currentTime);
    }
    if (this->sensor_session_remaining_lifetime_ != nullptr) {
      this->sensor_session_remaining_lifetime_->publish_state(
          DEXCOM_SENSOR_SESSION_LIFETIME - (this->time_msg_.currentTime - this->time_msg_.sessionStartTime));
    }

    this->reset_state();
  }
}

uint16_t Dexcom::find_handle_(const esp32_ble_tracker::ESPBTUUID *characteristic) {
  auto *chr = this->parent_->get_characteristic(SERVICE_UUID, *characteristic);
  if (chr == nullptr) {
    ESP_LOGW(TAG, "[%s] No characteristic found at service %s char %s", this->get_name().c_str(),
             SERVICE_UUID.to_string().c_str(), (*characteristic).to_string().c_str());
    return 0;
  }
  return chr->handle;
}

uint16_t Dexcom::find_descriptor(uint16_t handle) {
  if (handle == 0) {
    return 0;
  }
  auto *descr = this->parent_->get_config_descriptor(handle);
  if (descr == nullptr) {
    ESP_LOGW(TAG, "[%s] No Descriptor for status handle 0x%x.", this->get_name().c_str(), handle);
    return 0;
  } else if (descr->uuid.get_uuid().len != ESP_UUID_LEN_16 ||
             descr->uuid.get_uuid().uuid.uuid16 != ESP_GATT_UUID_CHAR_CLIENT_CONFIG) {
    ESP_LOGW(TAG, "[%s] Descriptor 0x%x (uuid %s) is not a client config char uuid", this->get_name().c_str(), handle,
             descr->uuid.to_string().c_str());
    return 0;
  } else {
    return descr->handle;
  }
}

bool Dexcom::register_notify_(const uint16_t handle, const uint16_t handle_desc, BT_NOTIFICATION_TYPE type) {
  this->register_notify_counter_ = 0;

  auto status =
      esp_ble_gattc_register_for_notify(this->parent_->get_gattc_if(), this->parent_->get_remote_bda(), handle);

  if (status) {
    ESP_LOGW(TAG, "[%s] Error sending notify request for service %s handle 0x%04x, status=%d", this->get_name().c_str(),
             SERVICE_UUID.to_string().c_str(), handle, status);
    return false;
  }

  if (handle_desc == 0) {
    return false;
  }

  uint16_t notify_en;
  switch (type) {
    case BT_NOTIFICATION_TYPE::OFF:
      notify_en = 0;
      break;
    case BT_NOTIFICATION_TYPE::NOTIFICATION:
      notify_en = 1;
      break;
    case BT_NOTIFICATION_TYPE::INDICATION:
      notify_en = 2;
      break;
    case BT_NOTIFICATION_TYPE::NOTIFICATION_INDICATION:
      notify_en = 3;
      break;
    default:
      return true;
      break;
  }
  status = esp_ble_gattc_write_char_descr(this->parent_->get_gattc_if(), this->parent_->get_conn_id(), handle_desc,
                                          sizeof(notify_en), (uint8_t *) &notify_en, ESP_GATT_WRITE_TYPE_RSP,
                                          ESP_GATT_AUTH_REQ_NONE);
  if (status) {
    ESP_LOGW(TAG, "esp_ble_gattc_write_char_descr error, status=%d", status);
    return false;
  } else {
    ESP_LOGD(TAG, "[%s] Wrote notify=%u to handle 0x%04x, desc 0x%04x, for conn %d", this->get_name().c_str(),
             notify_en, handle, handle_desc, this->parent_->get_conn_id());
    return true;
  }
}

bool Dexcom::write_handle_(const uint16_t handle, uint8_t *value, const uint16_t value_len) {
  auto status = esp_ble_gattc_write_char(this->parent_->get_gattc_if(), this->parent_->get_conn_id(), handle, value_len,
                                         value, ESP_GATT_WRITE_TYPE_RSP, ESP_GATT_AUTH_REQ_NONE);

  if (status) {
    ESP_LOGW(TAG, "[%s] Error sending write request for service %s handle 0x%04x, status=%d, data=%s",
             this->get_name().c_str(), SERVICE_UUID.to_string().c_str(), handle, status,
             format_hex_pretty(value, value_len).c_str());
    return false;
  } else {
    ESP_LOGD(TAG, "[%s] Sending write request for service %s handle 0x%04x, data=%s", this->get_name().c_str(),
             SERVICE_UUID.to_string().c_str(), handle, format_hex_pretty(value, value_len).c_str());
    return true;
  }
}

bool Dexcom::read_handle_(const uint16_t handle) {
  auto status = esp_ble_gattc_read_char(this->parent_->get_gattc_if(), this->parent_->get_conn_id(), handle,
                                        ESP_GATT_AUTH_REQ_NONE);

  if (status) {
    ESP_LOGW(TAG, "[%s] Error sending read request for service %s handle 0x%04x, status=%d", this->get_name().c_str(),
             SERVICE_UUID.to_string().c_str(), handle, status);
    return false;
  } else {
    ESP_LOGD(TAG, "[%s] Sending read request for service %s handle 0x%04x", this->get_name().c_str(),
             SERVICE_UUID.to_string().c_str(), handle);
    return true;
  }
}

std::array<uint8_t, 8> Dexcom::encrypt_(const std::array<uint8_t, 8> data) {
  std::array<uint8_t, 8> ret;

  esp_aes_context ctx;
  esp_aes_init(&ctx);

  const std::array<uint8_t, 16> key{'0',
                                    '0',
                                    this->transmitter_id_[0],
                                    this->transmitter_id_[1],
                                    this->transmitter_id_[2],
                                    this->transmitter_id_[3],
                                    this->transmitter_id_[4],
                                    this->transmitter_id_[5],
                                    '0',
                                    '0',
                                    this->transmitter_id_[0],
                                    this->transmitter_id_[1],
                                    this->transmitter_id_[2],
                                    this->transmitter_id_[3],
                                    this->transmitter_id_[4],
                                    this->transmitter_id_[5]};

  auto status = esp_aes_setkey(&ctx, key.data(), key.size() * 8);
  if (status != 0) {
    ESP_LOGE(TAG, "[%s] Error during esp_aes_setkey operation (%i).", this->get_name().c_str(), status);
    esp_aes_free(&ctx);
    return ret;
  }

  uint8_t input[16];
  std::memcpy(input, data.data(), 8);
  std::memcpy(input + 8, data.data(), 8);
  uint8_t output[16];

  status = esp_aes_crypt_ecb(&ctx, ESP_AES_ENCRYPT, input, output);
  if (status != 0) {
    ESP_LOGE(TAG, "[%s] Error during esp_aes_crypt_ecb operation (%i).", this->get_name().c_str(), status);
    esp_aes_free(&ctx);
    return ret;
  }

  esp_aes_free(&ctx);

  std::memcpy(ret.data(), output, 8);
  ESP_LOGV(TAG, "[%s] Enrypted message: %s", this->get_name().c_str(),
           format_hex_pretty(ret.data(), ret.size()).c_str());
  return ret;
}

}  // namespace dexcom
}  // namespace esphome

#endif
