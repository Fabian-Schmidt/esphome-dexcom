#include "dexcom.h"
#include "esphome/core/log.h"

#ifdef USE_ESP32

namespace esphome {
namespace dexcom {

static const char *const TAG = "dexcom";

/**
 * Expected steps:
 * TODO
 * 1. ESP_GATTC_OPEN_EVT
 * 2. ESP_GATTC_SEARCH_CMPL_EVT -> `esp_ble_gattc_read_char` - with ESP_GATT_AUTH_REQ_SIGNED_MITM
 * - Auth happens if necessary
 * 3. ESP_GATTC_READ_CHAR_EVT -> first value & (if notify) `esp_ble_gattc_register_for_notify`
 * 4. ESP_GATTC_REG_FOR_NOTIFY_EVT
 * 5. ESP_GATTC_NOTIFY_EVT -> continous values
 * if notify: every 20 seconds (polling frequency) a 60 second keep alive is send.
 */

void Dexcom::dump_config() { ESP_LOGCONFIG(TAG, "Dexcom"); }

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
      break;

    case ESP_GATTC_SEARCH_CMPL_EVT:
      ESP_LOGD(TAG, "[%s] Search complete", this->get_name().c_str());
      this->node_state = esp32_ble_tracker::ClientState::ESTABLISHED;
      this->handle_communication_ = this->find_handle_(&CHARACTERISTIC_UUID_COMMUNICATION);
      this->handle_control_ = this->find_handle_(&CHARACTERISTIC_UUID_CONTROL);
      this->handle_authentication_ = this->find_handle_(&CHARACTERISTIC_UUID_AUTHENTICATION);
      this->handle_authentication_desc_ = this->find_descriptor(handle_authentication_);
      this->handle_backfill_ = this->find_handle_(&CHARACTERISTIC_UUID_BACKFILL);

      this->read_handle_(this->handle_authentication_);

      this->register_notify_(this->handle_authentication_, this->handle_authentication_desc_,
                             BT_NOTIFICATION_TYPE::NOTIFICATION_INDICATION);

      {
        DEXCOM_MSG response;
        response.opcode = DEXCOM_OPCODE::AUTH_INIT;
        response.init_msg.token = {0x19, 0xF3, 0x89, 0xF8, 0xB7, 0x58, 0x41, 0x33};
        response.init_msg.channel =
            this->use_alternative_bt_channel_ ? DEXCOM_BT_CHANNEL::ALT_CHANNEL : DEXCOM_BT_CHANNEL::NORMAL_CHANNEL;
        this->write_handle_(this->handle_authentication_, (u_int8_t *) &response, 1 + sizeof(AUTH_INIT_MSG));
      }
      break;

    case ESP_GATTC_READ_CHAR_EVT:
      if (param->read.conn_id != this->parent_->get_conn_id()) {
        break;
      }
      if (param->read.status == ESP_GATT_OK) {
        this->node_state = esp32_ble_tracker::ClientState::ESTABLISHED;
        ESP_LOGV(TAG, "[%s] Reading char at handle 0x%04x, status=%d, data=%s", this->get_name().c_str(),
                 param->read.handle, param->read.status,
                 format_hex_pretty(param->read.value, param->read.value_len).c_str());
      } else {
        ESP_LOGW(TAG, "[%s] Error reading char at handle 0x%04x, status=%d", this->get_name().c_str(),
                 param->read.handle, param->read.status);
        break;
      }
      this->read_incomming_msg_(param->read.handle, param->read.value, param->read.value_len);
      break;

    case ESP_GATTC_REG_FOR_NOTIFY_EVT:
      if (param->reg_for_notify.status == ESP_GATT_OK) {
        this->node_state = esp32_ble_tracker::ClientState::ESTABLISHED;
        ESP_LOGV(TAG, "[%s] Register notify 0x%04x status=%d", this->get_name().c_str(), param->reg_for_notify.handle,
                 param->reg_for_notify.status);
      } else {
        // param->reg_for_notify.status == ESP_GATT_NO_RESOURCES
        ESP_LOGW(TAG, "[%s] Register notify 0x%04x status=%d", this->get_name().c_str(), param->reg_for_notify.handle,
                 param->reg_for_notify.status);
      }
      break;

    case ESP_GATTC_NOTIFY_EVT:
      ESP_LOGV(TAG, "[%s] Notify to handle 0x%04x is notify=%s, data=%s", this->get_name().c_str(),
               param->notify.handle, param->notify.is_notify ? "true" : "false",
               format_hex_pretty(param->notify.value, param->notify.value_len).c_str());
      this->read_incomming_msg_(param->notify.handle, param->notify.value, param->notify.value_len);
      break;

    case ESP_GATTC_WRITE_CHAR_EVT:
      if (param->write.status == ESP_GATT_OK) {
        ESP_LOGV(TAG, "[%s] Write to handle 0x%04x status=%d", this->get_name().c_str(), param->write.handle,
                 param->write.status);
        // if (param->write.handle == this->handle_authentication_) {
        this->read_handle_(param->write.handle);
        //}
      } else {
        // status 0x05 - ESP_GATT_INSUF_AUTHENTICATION
        ESP_LOGW(TAG, "[%s] Write to handle 0x%04x status=%d", this->get_name().c_str(), param->write.handle,
                 param->write.status);
      }
      break;
    case ESP_GATTC_WRITE_DESCR_EVT:
      break;

    default:
      break;
  }
}
void Dexcom::read_incomming_msg_(const u_int16_t handle, uint8_t *value, const u_int16_t value_len) {
  auto dexcom_msg = (const DEXCOM_MSG *) value;
  DEXCOM_MSG response;

  if (handle == this->handle_authentication_) {
    switch (dexcom_msg->opcode) {
      case DEXCOM_OPCODE::AUTH_CHALLENGE:
        if (value_len == (1 + sizeof(AUTH_CHALLENGE_MSG))) {
          // Here we could check if the tokenHash is the encrypted 8 bytes from the authRequestTxMessage ([1] to
          // [8]); To check if the Transmitter is a valid dexcom transmitter (because only the correct one should
          // know the ID).

          response.opcode = DEXCOM_OPCODE::AUTH_CHALLENGE_RESPONSE;
          response.challenge_response_msg.challenge_response = this->encrypt_(dexcom_msg->challenge_msg.challenge);
          this->write_handle_(handle, (u_int8_t *) &response, 1 + sizeof(AUTH_CHALLENGE_RESPONSE_MSG));
        }
        break;
      case DEXCOM_OPCODE::AUTH_FINISH:
        if (value_len == (1 + sizeof(AUTH_FINISH_MSG))) {
          ESP_LOGD(TAG, "[%s] Auth result %s %s", this->get_name().c_str(),
                   enum_to_c_str(dexcom_msg->auth_finish_msg.auth), enum_to_c_str(dexcom_msg->auth_finish_msg.bond));
          if (dexcom_msg->auth_finish_msg.auth == DEXCOM_AUTH_RESULT::AUTHENTICATED) {
            if (dexcom_msg->auth_finish_msg.bond != DEXCOM_BOND_REQUEST::NO_BONDING) {
              // Request bonding
              response.opcode = DEXCOM_OPCODE::KEEP_ALIVE;
              response.keep_alive.unknown = 0x19;
              this->write_handle_(handle, (u_int8_t *) &response, 1 + sizeof(KEEP_ALIVE_MSG));

              response.opcode = DEXCOM_OPCODE::BOND_REQUEST;
              this->write_handle_(handle, (u_int8_t *) &response, 1);
            } else {
              response.opcode = DEXCOM_OPCODE::AUTH_INIT;
              response.init_msg.token = {0x19, 0xF3, 0x89, 0xF8, 0xB7, 0x58, 0x41, 0x33};
              response.init_msg.channel = this->use_alternative_bt_channel_ ? DEXCOM_BT_CHANNEL::ALT_CHANNEL
                                                                            : DEXCOM_BT_CHANNEL::NORMAL_CHANNEL;
              this->write_handle_(this->handle_authentication_, (u_int8_t *) &response, 1 + sizeof(AUTH_INIT_MSG));
            }

            response.opcode = DEXCOM_OPCODE::TIME;
            response.time.unknown_E6 = 0xE6;
            response.time.unknown_64 = 0x64;
            this->write_handle_(this->handle_control_, (u_int8_t *) &response, 1 + sizeof(TIME_MSG));
          }
        }
        break;

      default:
        break;
    }
  } else if (handle == this->handle_control_) {
    switch (dexcom_msg->opcode) {
      case DEXCOM_OPCODE::TIME_RESPONSE:
        if (value_len >= (1 + sizeof(TIME_RESPONSE_MSG))) {
          ESP_LOGI(TAG, "Time - Status:              %d", dexcom_msg->time_response.status);

          ESP_LOGI(TAG, "Time - since activation:    %d (%d days, %d hours)\n",
                   dexcom_msg->time_response.currentTime,  // Activation date is now() - currentTime * 1000
                   dexcom_msg->time_response.currentTime / (60 * 60 * 24),     // Days round down
                   (dexcom_msg->time_response.currentTime / (60 * 60)) % 24);  // Remaining hours
          ESP_LOGI(TAG, "Time - since session start: %d", dexcom_msg->time_response.sessionStartTime);

          if (dexcom_msg->time_response.status == 0x81)
            ESP_LOGI(TAG, "WARNING - Low Battery");
          if (dexcom_msg->time_response.status == 0x83)
            ESP_LOGI(TAG, "WARNING - Transmitter Expired");
        }
        break;
      default:
        break;
    }
  }
}

u_int16_t Dexcom::find_handle_(const esp32_ble_tracker::ESPBTUUID *characteristic) {
  auto *chr = this->parent_->get_characteristic(SERVICE_UUID, *characteristic);
  if (chr == nullptr) {
    ESP_LOGW(TAG, "[%s] No characteristic found at service %s char %s", this->get_name().c_str(),
             SERVICE_UUID.to_string().c_str(), (*characteristic).to_string().c_str());
    return 0;
  }
  return chr->handle;
}

u_int16_t Dexcom::find_descriptor(u_int16_t handle) {
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

bool Dexcom::register_notify_(const u_int16_t handle, const u_int16_t handle_desc, BT_NOTIFICATION_TYPE type) {
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

  u_int16_t notify_en;
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
  }
  ESP_LOGD(TAG, "[%s] wrote notify=%u to status config 0x%04x, for conn %d", this->get_name().c_str(), notify_en,
           handle, this->parent_->get_conn_id());
  return true;
}

bool Dexcom::write_handle_(const u_int16_t handle, uint8_t *value, const u_int16_t value_len) {
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

bool Dexcom::read_handle_(const u_int16_t handle) {
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

std::array<u_int8_t, 8> Dexcom::encrypt_(const std::array<u_int8_t, 8> data) {
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

  u_int8_t input[16];
  std::memcpy(input, data.data(), 8);
  std::memcpy(input + 8, data.data(), 8);
  u_int8_t output[16];

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
