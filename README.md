# esphome-dexcom

Dexcom G5/G6 glucose data in ESP Home and Home Assistant.

## Example config

```yaml
substitutions:
  # Set the transmitter id of your Dexcom device here
  transmitter_id: 8YD26N
  use_alternative_bt_channel: "False"

esphome:
  name: "dexcom"

external_components:
  - source: github://Fabian-Schmidt/esphome-dexcom
    # refresh: always

esp32:
  board: esp32dev

deep_sleep:

esp32_ble_tracker:
  scan_parameters:
    interval: 10ms
    window: 10ms
    active: false

dexcom_ble_client:
  id: dexcom_ble_client_id
  transmitter_id: ${transmitter_id}
  use_alternative_bt_channel: ${use_alternative_bt_channel}
  on_disconnect: 
    - deep_sleep.enter:
        # Every 5 minutes Dexcom sensor wakes up.
        # Pre wake up connect to WiFi, API, MQTT etc.
        sleep_duration: 4.5min

sensor:
  - platform: dexcom_ble_client
    type: GLUCOSE_IN_MG_DL
    name: Glucose in mg/dl
    # expire_after: 10.5min # if using mqtt recommendation to set `expire_after` option.
  - platform: dexcom_ble_client
    type: GLUCOSE_IN_MMOL_L
    name: Glucose in mmol/l
  - platform: dexcom_ble_client
    type: GLUCOSE_TREND
    name: Glucose trend
  - platform: dexcom_ble_client
    type: GLUCOSE_PREDICT_IN_MG_DL
    name: Glucose predict in mg/dl
  - platform: dexcom_ble_client
    type: GLUCOSE_PREDICT_IN_MMOL_L
    name: Glucose predict in mmol/l
  - platform: dexcom_ble_client
    type: SENSOR_AGE
    name: Sensor age
  - platform: dexcom_ble_client
    type: SENSOR_SESSION_AGE
    name: Sensor session age
  - platform: dexcom_ble_client
    type: SENSOR_REMAINING_LIFETIME
    name: Sensor remaining lifetime
  - platform: dexcom_ble_client
    type: SENSOR_SESSION_REMAINING_LIFETIME
    name: Sensor session remaining lifetime
```

[Full example configuration](example.yaml)

## Acknowledgement

Thanks to <https://github.com/TheEpicBigBoss/Dexcom-ESP32-Reader> and <https://github.com/NightscoutFoundation/xDrip> for the research.
