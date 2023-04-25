import esphome.codegen as cg
import esphome.config_validation as cv
import esphome.core as CORE
from esphome.components import ble_client, sensor
from esphome.const import (
    CONF_ID,
    DEVICE_CLASS_DURATION,
    ICON_EMPTY,
    UNIT_EMPTY,
    UNIT_SECOND,
    STATE_CLASS_MEASUREMENT,
    STATE_CLASS_TOTAL_INCREASING,
)

DEPENDENCIES = ["esp32_ble_tracker"]

CONF_USE_ALTERNATIVE_BT_CHANNEL = "use_alternative_bt_channel"
CONF_TRANSMITTER_ID = "transmitter_id"
CONF_GLUCOSE_IN_MG_DL = "glucose_in_mg_dl"
CONF_GLUCOSE_IN_MMOL_L = "glucose_in_mmol_l"
CONF_TREND = "trend"
CONF_GLUCOSE_PREDICT_IN_MG_DL = "glucose_predict_in_mg_dl"
CONF_GLUCOSE_PREDICT_IN_MMOL_L = "glucose_predict_in_mmol_l"
CONF_SENSOR_AGE = "sensor_age"
CONF_SENSOR_SESSION_AGE = "sensor_session_age"
CONF_SENSOR_REMAINING_LIFETIME = "sensor_remaining_lifetime"
CONF_SENSOR_SESSION_REMAINING_LIFETIME = "sensor_session_remaining_lifetime"

UNIT_MG_DL = "mg/dl"
UNIT_MMOL_L = "mmol/l"

dexcom_ns = cg.esphome_ns.namespace("dexcom")
Dexcom = dexcom_ns.class_("Dexcom", ble_client.BLEClientNode, cg.Component)


def transmitter_id_array(value):
    value = cv.string_strict(value)
    if len(value) != 6:
        raise cv.Invalid("Transmitter Id must be 6 chars long.")
    return value


CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(Dexcom),
            cv.Optional(CONF_USE_ALTERNATIVE_BT_CHANNEL): cv.boolean,
            cv.Required(CONF_TRANSMITTER_ID): transmitter_id_array,
            cv.Optional(CONF_GLUCOSE_IN_MG_DL): sensor.sensor_schema(
                unit_of_measurement=UNIT_MG_DL,
                icon=ICON_EMPTY,
                accuracy_decimals=0,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_GLUCOSE_IN_MMOL_L): sensor.sensor_schema(
                unit_of_measurement=UNIT_MMOL_L,
                icon=ICON_EMPTY,
                accuracy_decimals=1,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_TREND): sensor.sensor_schema(
                unit_of_measurement=UNIT_EMPTY,
                icon=ICON_EMPTY,
                accuracy_decimals=0,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_GLUCOSE_PREDICT_IN_MG_DL): sensor.sensor_schema(
                unit_of_measurement=UNIT_MG_DL,
                icon=ICON_EMPTY,
                accuracy_decimals=0,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_GLUCOSE_PREDICT_IN_MMOL_L): sensor.sensor_schema(
                unit_of_measurement=UNIT_MMOL_L,
                icon=ICON_EMPTY,
                accuracy_decimals=1,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_SENSOR_AGE): sensor.sensor_schema(
                unit_of_measurement=UNIT_SECOND,
                icon=ICON_EMPTY,
                accuracy_decimals=0,
                device_class=DEVICE_CLASS_DURATION,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_SENSOR_SESSION_AGE): sensor.sensor_schema(
                unit_of_measurement=UNIT_SECOND,
                icon=ICON_EMPTY,
                accuracy_decimals=0,
                device_class=DEVICE_CLASS_DURATION,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_SENSOR_REMAINING_LIFETIME): sensor.sensor_schema(
                unit_of_measurement=UNIT_SECOND,
                icon=ICON_EMPTY,
                accuracy_decimals=0,
                device_class=DEVICE_CLASS_DURATION,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_SENSOR_SESSION_REMAINING_LIFETIME): sensor.sensor_schema(
                unit_of_measurement=UNIT_SECOND,
                icon=ICON_EMPTY,
                accuracy_decimals=0,
                device_class=DEVICE_CLASS_DURATION,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
        }
    )
    .extend(ble_client.BLE_CLIENT_SCHEMA)
    .extend(cv.COMPONENT_SCHEMA),
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await ble_client.register_ble_node(var, config)

    if config[CONF_USE_ALTERNATIVE_BT_CHANNEL]:
        cg.add(
            var.set_use_alternative_bt_channel(config[CONF_USE_ALTERNATIVE_BT_CHANNEL])
        )
    if config[CONF_TRANSMITTER_ID]:
        cg.add(var.set_transmitter_id(config[CONF_TRANSMITTER_ID]))

    if CONF_GLUCOSE_IN_MG_DL in config:
        sens = await sensor.new_sensor(config[CONF_GLUCOSE_IN_MG_DL])
        cg.add(var.set_glucose_in_mg_dl(sens))
    if CONF_GLUCOSE_IN_MMOL_L in config:
        sens = await sensor.new_sensor(config[CONF_GLUCOSE_IN_MMOL_L])
        cg.add(var.set_glucose_in_mmol_l(sens))
    if CONF_TREND in config:
        sens = await sensor.new_sensor(config[CONF_TREND])
        cg.add(var.set_trend(sens))
    if CONF_GLUCOSE_PREDICT_IN_MG_DL in config:
        sens = await sensor.new_sensor(config[CONF_GLUCOSE_PREDICT_IN_MG_DL])
        cg.add(var.set_glucose_predict_in_mg_dl(sens))
    if CONF_GLUCOSE_PREDICT_IN_MMOL_L in config:
        sens = await sensor.new_sensor(config[CONF_GLUCOSE_PREDICT_IN_MMOL_L])
        cg.add(var.set_glucose_predict_in_mmol_l(sens))
    if CONF_SENSOR_AGE in config:
        sens = await sensor.new_sensor(config[CONF_SENSOR_AGE])
        cg.add(var.set_sensor_age(sens))
    if CONF_SENSOR_SESSION_AGE in config:
        sens = await sensor.new_sensor(config[CONF_SENSOR_SESSION_AGE])
        cg.add(var.set_sensor_session_age(sens))
    if CONF_SENSOR_REMAINING_LIFETIME in config:
        sens = await sensor.new_sensor(config[CONF_SENSOR_REMAINING_LIFETIME])
        cg.add(var.set_sensor_remaining_lifetime(sens))
    if CONF_SENSOR_SESSION_REMAINING_LIFETIME in config:
        sens = await sensor.new_sensor(config[CONF_SENSOR_SESSION_REMAINING_LIFETIME])
        cg.add(var.set_sensor_session_remaining_lifetime(sens))
