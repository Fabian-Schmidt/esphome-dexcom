import esphome.codegen as cg
import esphome.config_validation as cv
import esphome.core as CORE
from esphome.components import ble_client, sensor
from esphome.const import (
    CONF_ID,
)

DEPENDENCIES = ["esp32_ble_tracker"]

CONF_USE_ALTERNATIVE_BT_CHANNEL = "use_alternative_bt_channel"
CONF_TRANSMITTER_ID = "transmitter_id"

dexcom_ns = cg.esphome_ns.namespace("dexcom")
Dexcom = dexcom_ns.class_(
    "Dexcom", ble_client.BLEClientNode, cg.Component
)


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
            cv.Required(CONF_TRANSMITTER_ID): transmitter_id_array
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
        cg.add(var.set_use_alternative_bt_channel(config[CONF_USE_ALTERNATIVE_BT_CHANNEL]))
    if config[CONF_TRANSMITTER_ID]:
        cg.add(var.set_transmitter_id(config[CONF_TRANSMITTER_ID]))

