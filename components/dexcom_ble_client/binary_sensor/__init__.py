import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor
from esphome.const import (
    CONF_TYPE,
)
from .. import CONF_DEXCOM_BLE_ID, DexcomBLEClient, dexcom_ble_client_ns

DEPENDENCIES = ["dexcom_ble_client"]
CODEOWNERS = ["@Fabian-Schmidt"]

DexcomBinarySensor = dexcom_ble_client_ns.class_(
    "DexcomBinarySensor", binary_sensor.BinarySensor, cg.Component
)

DEXCOM_BINARY_SENSOR_TYPE = dexcom_ble_client_ns.namespace("DEXCOM_BINARY_SENSOR_TYPE")

CONF_SUPPORTED_TYPE = {
    "SENSOR_LOW_BATTERY": {
        CONF_TYPE: DEXCOM_BINARY_SENSOR_TYPE.SENSOR_LOW_BATTERY,
    },
    "SENSOR_OKAY": {
        CONF_TYPE: DEXCOM_BINARY_SENSOR_TYPE.SENSOR_OKAY,
    },
    "SENSOR_FAILED": {
        CONF_TYPE: DEXCOM_BINARY_SENSOR_TYPE.SENSOR_FAILED,
    },
    "SENSOR_STOPPED": {
        CONF_TYPE: DEXCOM_BINARY_SENSOR_TYPE.SENSOR_STOPPED,
    },
    "SENSOR_TRANSITIONAL": {
        CONF_TYPE: DEXCOM_BINARY_SENSOR_TYPE.SENSOR_TRANSITIONAL,
    },
}


CONFIG_SCHEMA = (
    binary_sensor.binary_sensor_schema(DexcomBinarySensor)
    .extend(
        {
            cv.GenerateID(CONF_DEXCOM_BLE_ID): cv.use_id(DexcomBLEClient),
            cv.Required(CONF_TYPE): cv.enum(CONF_SUPPORTED_TYPE, upper=True),
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
)


async def to_code(config):
    var = await binary_sensor.new_binary_sensor(config)
    await cg.register_component(var, config)
    await cg.register_parented(var, config[CONF_DEXCOM_BLE_ID])

    cg.add(var.set_type(CONF_SUPPORTED_TYPE[config[CONF_TYPE]][CONF_TYPE]))
