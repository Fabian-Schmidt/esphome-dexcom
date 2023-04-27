import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import (
    CONF_ACCURACY_DECIMALS,
    CONF_DEVICE_CLASS,
    CONF_ICON,
    CONF_ID,
    CONF_STATE_CLASS,
    CONF_TYPE,
    CONF_UNIT_OF_MEASUREMENT,
    DEVICE_CLASS_DURATION,
    ICON_EMPTY,
    STATE_CLASS_MEASUREMENT,
    UNIT_EMPTY,
    UNIT_SECOND,
)
from .. import CONF_DEXCOM_BLE_ID, DexcomBLEClient, dexcom_ble_client_ns

DEPENDENCIES = ["dexcom_ble_client"]
CODEOWNERS = ["@Fabian-Schmidt"]

UNIT_MG_DL = "mg/dl"
UNIT_MMOL_L = "mmol/l"

DexcomSensor = dexcom_ble_client_ns.class_("DexcomSensor", sensor.Sensor, cg.Component)

DEXCOM_SENSOR_TYPE = dexcom_ble_client_ns.namespace("DEXCOM_SENSOR_TYPE")

CONF_SUPPORTED_TYPE = {
    "GLUCOSE_IN_MG_DL": {
        CONF_TYPE: DEXCOM_SENSOR_TYPE.GLUCOSE_IN_MG_DL,
        CONF_UNIT_OF_MEASUREMENT: UNIT_MG_DL,
        CONF_ACCURACY_DECIMALS: 0,
    },
    "GLUCOSE_IN_MMOL_L": {
        CONF_TYPE: DEXCOM_SENSOR_TYPE.GLUCOSE_IN_MMOL_L,
        CONF_UNIT_OF_MEASUREMENT: UNIT_MMOL_L,
        CONF_ACCURACY_DECIMALS: 1,
    },
    "GLUCOSE_TREND": {
        CONF_TYPE: DEXCOM_SENSOR_TYPE.GLUCOSE_TREND,
        CONF_UNIT_OF_MEASUREMENT: UNIT_EMPTY,
        CONF_ACCURACY_DECIMALS: 0,
    },
    "GLUCOSE_PREDICT_IN_MG_DL": {
        CONF_TYPE: DEXCOM_SENSOR_TYPE.GLUCOSE_PREDICT_IN_MG_DL,
        CONF_UNIT_OF_MEASUREMENT: UNIT_MG_DL,
        CONF_ACCURACY_DECIMALS: 0,
    },
    "GLUCOSE_PREDICT_IN_MMOL_L": {
        CONF_TYPE: DEXCOM_SENSOR_TYPE.GLUCOSE_PREDICT_IN_MMOL_L,
        CONF_UNIT_OF_MEASUREMENT: UNIT_MMOL_L,
        CONF_ACCURACY_DECIMALS: 1,
    },
    "SENSOR_AGE": {
        CONF_TYPE: DEXCOM_SENSOR_TYPE.SENSOR_AGE,
        CONF_UNIT_OF_MEASUREMENT: UNIT_SECOND,
        CONF_ACCURACY_DECIMALS: 0,
        CONF_DEVICE_CLASS: DEVICE_CLASS_DURATION,
    },
    "SENSOR_SESSION_AGE": {
        CONF_TYPE: DEXCOM_SENSOR_TYPE.SENSOR_SESSION_AGE,
        CONF_UNIT_OF_MEASUREMENT: UNIT_SECOND,
        CONF_ACCURACY_DECIMALS: 0,
        CONF_DEVICE_CLASS: DEVICE_CLASS_DURATION,
    },
    "SENSOR_REMAINING_LIFETIME": {
        CONF_TYPE: DEXCOM_SENSOR_TYPE.SENSOR_REMAINING_LIFETIME,
        CONF_UNIT_OF_MEASUREMENT: UNIT_SECOND,
        CONF_ACCURACY_DECIMALS: 0,
        CONF_DEVICE_CLASS: DEVICE_CLASS_DURATION,
    },
    "SENSOR_SESSION_REMAINING_LIFETIME": {
        CONF_TYPE: DEXCOM_SENSOR_TYPE.SENSOR_SESSION_REMAINING_LIFETIME,
        CONF_UNIT_OF_MEASUREMENT: UNIT_SECOND,
        CONF_ACCURACY_DECIMALS: 0,
        CONF_DEVICE_CLASS: DEVICE_CLASS_DURATION,
    },
}


def set_default_based_on_type():
    def set_defaults_(config):
        type = config[CONF_TYPE]
        # set defaults based on sensor type:
        if CONF_STATE_CLASS not in config:
            config[CONF_STATE_CLASS] = sensor.validate_state_class(
                STATE_CLASS_MEASUREMENT
            )

        if (
            CONF_UNIT_OF_MEASUREMENT not in config
            and CONF_UNIT_OF_MEASUREMENT in CONF_SUPPORTED_TYPE[type]
        ):
            config[CONF_UNIT_OF_MEASUREMENT] = CONF_SUPPORTED_TYPE[type][
                CONF_UNIT_OF_MEASUREMENT
            ]

        if CONF_ICON not in config and CONF_ICON in CONF_SUPPORTED_TYPE[type]:
            config[CONF_ICON] = CONF_SUPPORTED_TYPE[type][CONF_ICON]

        if (
            CONF_ACCURACY_DECIMALS not in config
            and CONF_ACCURACY_DECIMALS in CONF_SUPPORTED_TYPE[type]
        ):
            config[CONF_ACCURACY_DECIMALS] = CONF_SUPPORTED_TYPE[type][
                CONF_ACCURACY_DECIMALS
            ]

        if (
            CONF_DEVICE_CLASS not in config
            and CONF_DEVICE_CLASS in CONF_SUPPORTED_TYPE[type]
        ):
            config[CONF_DEVICE_CLASS] = CONF_SUPPORTED_TYPE[type][CONF_DEVICE_CLASS]

        return config

    return set_defaults_


CONFIG_SCHEMA = (
    sensor.sensor_schema()
    .extend(
        {
            cv.GenerateID(): cv.declare_id(DexcomSensor),
            cv.GenerateID(CONF_DEXCOM_BLE_ID): cv.use_id(DexcomBLEClient),
            cv.Required(CONF_TYPE): cv.enum(CONF_SUPPORTED_TYPE, upper=True),
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
)
FINAL_VALIDATE_SCHEMA = set_default_based_on_type()


async def to_code(config):
    paren = await cg.get_variable(config[CONF_DEXCOM_BLE_ID])
    var = cg.new_Pvariable(config[CONF_ID], paren)
    await cg.register_component(var, config)
    await sensor.register_sensor(var, config)

    cg.add(var.set_type(CONF_SUPPORTED_TYPE[config[CONF_TYPE]][CONF_TYPE]))
