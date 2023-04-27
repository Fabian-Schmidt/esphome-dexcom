import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import esp32_ble_tracker, esp32_ble_client
from esphome.const import (
    CONF_ID,
    CONF_MAC_ADDRESS,
    CONF_ON_DISCONNECT,
    CONF_TRIGGER_ID,
)
from esphome import automation

AUTO_LOAD = ["esp32_ble_client"]
CODEOWNERS = ["@Fabian-Schmidt"]
DEPENDENCIES = ["esp32_ble_tracker"]

dexcom_ble_client_ns = cg.esphome_ns.namespace("dexcom_ble_client")
DexcomBLEClient = dexcom_ble_client_ns.class_(
    "DexcomBLEClient", esp32_ble_client.BLEClientBase
)
DexcomBLEClientDisconnectTrigger = dexcom_ble_client_ns.class_(
    "DexcomBLEClientDisconnectTrigger", automation.Trigger.template()
)

CONF_DEXCOM_BLE_ID = "dexcom_ble_id"
CONF_USE_ALTERNATIVE_BT_CHANNEL = "use_alternative_bt_channel"
CONF_TRANSMITTER_ID = "transmitter_id"


def transmitter_id_array(value):
    value = cv.string_strict(value)
    if len(value) != 6:
        raise cv.Invalid("Transmitter Id must be 6 chars long.")
    return value


CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(DexcomBLEClient),
            cv.Required(CONF_TRANSMITTER_ID): transmitter_id_array,
            cv.Optional(CONF_USE_ALTERNATIVE_BT_CHANNEL): cv.boolean,
            cv.Optional(CONF_MAC_ADDRESS): cv.mac_address,
            cv.Optional(CONF_ON_DISCONNECT): automation.validate_automation(
                {
                    cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(
                        DexcomBLEClientDisconnectTrigger
                    ),
                }
            ),
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
    .extend(esp32_ble_tracker.ESP_BLE_DEVICE_SCHEMA)
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await esp32_ble_tracker.register_client(var, config)

    cg.add(var.set_transmitter_id(config[CONF_TRANSMITTER_ID]))
    if CONF_USE_ALTERNATIVE_BT_CHANNEL in config:
        cg.add(
            var.set_use_alternative_bt_channel(config[CONF_USE_ALTERNATIVE_BT_CHANNEL])
        )
    if CONF_MAC_ADDRESS in config:
        cg.add(var.set_address(config[CONF_MAC_ADDRESS].as_hex))
    for conf in config.get(CONF_ON_DISCONNECT, []):
        trigger = cg.new_Pvariable(conf[CONF_TRIGGER_ID], var)
        await automation.build_automation(trigger, [], conf)
