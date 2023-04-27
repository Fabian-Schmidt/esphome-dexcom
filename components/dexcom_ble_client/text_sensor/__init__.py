import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import text_sensor
from esphome.const import (
    CONF_TYPE,
)
from .. import CONF_DEXCOM_BLE_ID, DexcomBLEClient, dexcom_ble_client_ns

DEPENDENCIES = ["dexcom_ble_client"]
CODEOWNERS = ["@Fabian-Schmidt"]

DexcomTextSensor = dexcom_ble_client_ns.class_(
    "DexcomTextSensor", text_sensor.TextSensor, cg.Component
)

DEXCOM_TEXT_SENSOR_TYPE = dexcom_ble_client_ns.namespace("DEXCOM_TEXT_SENSOR_TYPE")

CONF_SUPPORTED_TYPE = {
    "STATUS": {
        CONF_TYPE: DEXCOM_TEXT_SENSOR_TYPE.STATUS,
    },
    "CALIBRATION_STATE": {
        CONF_TYPE: DEXCOM_TEXT_SENSOR_TYPE.CALIBRATION_STATE,
    },
}

CONFIG_SCHEMA = (
    text_sensor.text_sensor_schema(DexcomTextSensor)
    .extend(
        {
            cv.GenerateID(CONF_DEXCOM_BLE_ID): cv.use_id(DexcomBLEClient),
            cv.Required(CONF_TYPE): cv.enum(CONF_SUPPORTED_TYPE, upper=True),
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
)


async def to_code(config):
    paren = await cg.get_variable(config[CONF_DEXCOM_BLE_ID])
    var = await text_sensor.new_text_sensor(config, paren)
    await cg.register_component(var, config)

    cg.add(var.set_type(CONF_SUPPORTED_TYPE[config[CONF_TYPE]][CONF_TYPE]))
