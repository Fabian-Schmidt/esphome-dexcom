
#include "esphome/core/helpers.h"

namespace esphome {
namespace dexcom {

enum class DEXCOM_OPCODE : uint8_t {
  UNKNOWN = 0x00,
  AUTH_INIT = 0x01,
  AUTH_CHALLENGE = 0x03,
  AUTH_CHALLENGE_RESPONSE = 0x04,
  AUTH_FINISH = 0x05,
  KEEP_ALIVE = 0x06,
  BOND_REQUEST = 0x07,
  BOND_REQUEST_RESPONSE = 0x08,
  DISCONNECT = 0x09,
  TIME = 0x24,
  TIME_RESPONSE = 0x25,
  G5_GLUCOSE_MSG = 0x30,
  G5_GLUCOSE_RESPONSE_MSG = 0x31,
  G6_GLUCOSE_MSG = 0x4E,
  G6_GLUCOSE_RESPONSE_MSG = 0x4F,
  KEEP_ALIVE_RESPONSE = 0xFF,
};

enum class DEXCOM_BT_CHANNEL : uint8_t {
  UNSET = 0x00,
  NORMAL_CHANNEL = 0x01,
  ALT_CHANNEL = 0x02,
};

enum class DEXCOM_AUTH_RESULT : uint8_t {
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

enum class DEXCOM_BOND_REQUEST : uint8_t {
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

enum class DEXCOM_TRANSMITTER_STATUS : uint8_t {
  OKAY = 0x00,
  LOW_BATTERY = 0x81,
  EXPIRED = 0x83,
  // BRICKED
};

static const bool enum_value_okay(const DEXCOM_TRANSMITTER_STATUS val) {
  switch (val) {
    case DEXCOM_TRANSMITTER_STATUS::OKAY:
    case DEXCOM_TRANSMITTER_STATUS::LOW_BATTERY:
      return true;
    default:
      return false;
  }
}

static const char *enum_to_c_str(const DEXCOM_TRANSMITTER_STATUS val) {
  switch (val) {
    case DEXCOM_TRANSMITTER_STATUS::OKAY:
      return "Okay";
    case DEXCOM_TRANSMITTER_STATUS::LOW_BATTERY:
      return "Low Battery";
    case DEXCOM_TRANSMITTER_STATUS::EXPIRED:
      return "Expired";
    default:
      if (val > DEXCOM_TRANSMITTER_STATUS::LOW_BATTERY) {
        return "Bricked";
      } else {
        return "Unknown";
      }
  }
}

// Source:
// https://github.com/NightscoutFoundation/xDrip/blob/38fee03be56da6ce9cbe90e08dbe8e4b651c6edc/app/src/main/java/com/eveningoutpost/dexdrip/G5Model/CalibrationState.java
enum class DEXCOM_CALIBRATION_STATE : uint8_t {
  UNKNOWN = 0x00,
  STOPPED = 0x01,
  WARMING_UP = 0x02,
  EXCESS_NOISE = 0x03,
  NEEDS_INITIAL_CALIBRATION = 0x04,
  NEEDS_SECOND_CALIBRATION = 0x05,
  OKAY = 0x06,
  NEEDS_CALIBRATION = 0x07,
  CONFUSED_CALIBRATION_1 = 0x08,
  CONFUSED_CALIBRATION_2 = 0x09,
  NEEDS_MORE_CALIBRATION = 0x0A,
  SENSOR_FAILED_1 = 0x0B,
  SENSOR_FAILED_2 = 0x0C,
  UNUSUAL_CALIBRATION = 0x0D,
  INSUFFICIENT_CALIBRATION = 0x0E,
  ENDED = 0x0F,
  SENSOR_FAILED_3 = 0x10,
  TRANSMITTER_PROBLEM = 0x11,
  SENSOR_ERRORS = 0x12,
  SENSOR_FAILED_4 = 0x13,
  SENSOR_FAILED_5 = 0x14,
  SENSOR_FAILED_6 = 0x15,
  SENSOR_FAILED_START_1 = 0x16,
  SENSOR_FAILED_START_2 = 0x17,
  SENSOR_EXPIRED = 0x18,
  SENSOR_FAILED_7 = 0x19,
  SENSOR_STOPPED_2 = 0x1A,
  SENSOR_FAILED_8 = 0x1B,
  SENSOR_FAILED_9 = 0x1C,
  SENSOR_FAILED_10 = 0x1D,
  SENSOR_FAILED_11 = 0x1E,
  SENSOR_STARTED = 0xC1,
  SENSOR_STOPPED = 0xC2,
  CALIBRATION_SENT = 0xC3,
};

static const bool enum_value_okay(const DEXCOM_CALIBRATION_STATE val) {
  switch (val) {
    case DEXCOM_CALIBRATION_STATE::OKAY:
    case DEXCOM_CALIBRATION_STATE::NEEDS_CALIBRATION:
      return true;
    default:
      return false;
  }
}

static const char *enum_to_c_str(const DEXCOM_CALIBRATION_STATE val) {
  switch (val) {
    case DEXCOM_CALIBRATION_STATE::STOPPED:
      return "Stopped";
    case DEXCOM_CALIBRATION_STATE::WARMING_UP:
      return "Warming Up";
    case DEXCOM_CALIBRATION_STATE::EXCESS_NOISE:
      return "Excess Noise";
    case DEXCOM_CALIBRATION_STATE::NEEDS_INITIAL_CALIBRATION:
      return "Needs Initial Calibration";
    case DEXCOM_CALIBRATION_STATE::NEEDS_SECOND_CALIBRATION:
      return "Needs Second Calibration";
    case DEXCOM_CALIBRATION_STATE::OKAY:
      return "Okay";
    case DEXCOM_CALIBRATION_STATE::NEEDS_CALIBRATION:
      return "Needs Calibration";
    case DEXCOM_CALIBRATION_STATE::CONFUSED_CALIBRATION_1:
      return "Confused Calibration 1";
    case DEXCOM_CALIBRATION_STATE::CONFUSED_CALIBRATION_2:
      return "Confused Calibration 2";
    case DEXCOM_CALIBRATION_STATE::NEEDS_MORE_CALIBRATION:
      return "Needs More Calibration";
    case DEXCOM_CALIBRATION_STATE::SENSOR_FAILED_1:
      return "Sensor Failed 1";
    case DEXCOM_CALIBRATION_STATE::SENSOR_FAILED_2:
      return "Sensor Failed 2";
    case DEXCOM_CALIBRATION_STATE::UNUSUAL_CALIBRATION:
      return "Unusual Calibration";
    case DEXCOM_CALIBRATION_STATE::INSUFFICIENT_CALIBRATION:
      return "Insufficient Calibration";
    case DEXCOM_CALIBRATION_STATE::ENDED:
      return "Ended";
    case DEXCOM_CALIBRATION_STATE::SENSOR_FAILED_3:
      return "Sensor Failed 3";
    case DEXCOM_CALIBRATION_STATE::TRANSMITTER_PROBLEM:
      return "Transmitter Problem";
    case DEXCOM_CALIBRATION_STATE::SENSOR_ERRORS:
      return "Sensor Errors";
    case DEXCOM_CALIBRATION_STATE::SENSOR_FAILED_4:
      return "Sensor Failed 4";
    case DEXCOM_CALIBRATION_STATE::SENSOR_FAILED_5:
      return "Sensor Failed 5";
    case DEXCOM_CALIBRATION_STATE::SENSOR_FAILED_6:
      return "Sensor Failed 6";
    case DEXCOM_CALIBRATION_STATE::SENSOR_FAILED_START_1:
      return "Sensor Failed Start 1";
    case DEXCOM_CALIBRATION_STATE::SENSOR_FAILED_START_2:
      return "Sensor Failed Start 2";
    case DEXCOM_CALIBRATION_STATE::SENSOR_EXPIRED:
      return "Sensor Expired";
    case DEXCOM_CALIBRATION_STATE::SENSOR_FAILED_7:
      return "Sensor Failed 7";
    case DEXCOM_CALIBRATION_STATE::SENSOR_STOPPED_2:
      return "Sensor Stopped 2";
    case DEXCOM_CALIBRATION_STATE::SENSOR_FAILED_8:
      return "Sensor Failed 8";
    case DEXCOM_CALIBRATION_STATE::SENSOR_FAILED_9:
      return "Sensor Failed 9";
    case DEXCOM_CALIBRATION_STATE::SENSOR_FAILED_10:
      return "Sensor Failed 10";
    case DEXCOM_CALIBRATION_STATE::SENSOR_FAILED_11:
      return "Sensor Failed 11";
    case DEXCOM_CALIBRATION_STATE::SENSOR_STARTED:
      return "Sensor Started";
    case DEXCOM_CALIBRATION_STATE::SENSOR_STOPPED:
      return "Sensor Stopped";
    case DEXCOM_CALIBRATION_STATE::CALIBRATION_SENT:
      return "Calibration Sent";
    default:
      return "Unknown";
  }
}

struct AUTH_INIT_MSG {  // NOLINT(readability-identifier-naming,altera-struct-pack-align)
  std::array<uint8_t, 8> token;
  DEXCOM_BT_CHANNEL channel;
} __attribute__((packed));

struct AUTH_CHALLENGE_MSG {  // NOLINT(readability-identifier-naming,altera-struct-pack-align)
  std::array<uint8_t, 8> tokenHash;
  std::array<uint8_t, 8> challenge;
} __attribute__((packed));

struct AUTH_CHALLENGE_RESPONSE_MSG {  // NOLINT(readability-identifier-naming,altera-struct-pack-align)
  std::array<uint8_t, 8> challenge_response;
} __attribute__((packed));

struct AUTH_FINISH_MSG {  // NOLINT(readability-identifier-naming,altera-struct-pack-align)
  DEXCOM_AUTH_RESULT auth;
  DEXCOM_BOND_REQUEST bond;
} __attribute__((packed));

struct KEEP_ALIVE_MSG {  // NOLINT(readability-identifier-naming,altera-struct-pack-align)
  uint8_t unknown;      // 0x19
} __attribute__((packed));

struct BOND_REQUEST_RESPONSE_MSG {  // NOLINT(readability-identifier-naming,altera-struct-pack-align)
  uint8_t unknown;                 // 0x01
} __attribute__((packed));

struct TIME_MSG {  // NOLINT(readability-identifier-naming,altera-struct-pack-align)
  uint8_t unknown_E6;
  uint8_t unknown_64;
} __attribute__((packed));

struct TIME_RESPONSE_MSG {  // NOLINT(readability-identifier-naming,altera-struct-pack-align)
  DEXCOM_TRANSMITTER_STATUS status;
  uint32_t currentTime;
  // Session start = Activation date + sessionStartTime * 1000
  uint32_t sessionStartTime;
  uint32_t unknow;
  uint16_t crc;
} __attribute__((packed));

struct GLUCOSE_MSG {  // NOLINT(readability-identifier-naming,altera-struct-pack-align)
                      // G5 - 0x53
                      // G6 - 0x0A
  uint8_t unknown_A;

  // G5 - 0x36
  // G6 - 0xA9
  uint8_t unknown_B;
} __attribute__((packed));

struct GLUCOSE_RESPONSE_MSG {  // NOLINT(readability-identifier-naming,altera-struct-pack-align)
  DEXCOM_TRANSMITTER_STATUS status;
  uint32_t sequence;
  /// Seconds since transmitter activation
  uint32_t timestamp;
  uint16_t glucose : 12;
  uint8_t glucoseIsDisplayOnly : 4;
  DEXCOM_CALIBRATION_STATE state;
  int8_t trend;
  // Mask necessary? 0x03ff?
  uint16_t predicted_glucose;
  uint16_t crc;
} __attribute__((packed));

struct KEEP_ALIVE_RESPONSE_MSG {  // NOLINT(readability-identifier-naming,altera-struct-pack-align)
  // 0x06
  uint8_t a;
  // 0x01
  uint8_t b;
} __attribute__((packed));

struct DEXCOM_MSG {  // NOLINT(readability-identifier-naming,altera-struct-pack-align)
  DEXCOM_OPCODE opcode;
  union {
    AUTH_INIT_MSG init_msg;
    AUTH_CHALLENGE_MSG challenge_msg;
    AUTH_CHALLENGE_RESPONSE_MSG challenge_response_msg;
    AUTH_FINISH_MSG auth_finish_msg;
    KEEP_ALIVE_MSG keep_alive;
    BOND_REQUEST_RESPONSE_MSG bond_request_response_msg;
    TIME_MSG time;
    TIME_RESPONSE_MSG time_response;
    GLUCOSE_MSG glucose_msg;
    GLUCOSE_RESPONSE_MSG glucose_response_msg;
    KEEP_ALIVE_RESPONSE_MSG keep_alive_response;
  };
} __attribute__((packed));

}  // namespace dexcom
}  // namespace esphome