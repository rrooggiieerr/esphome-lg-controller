import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import pins
from esphome.components import climate, uart
from esphome.const import CONF_ID, CONF_RX_PIN, CONF_SENSOR

DEPENDENCIES = ["uart"]
CODEOWNERS = ["@JanM321", "@rrooggiieerr"]
AUTO_LOAD = ["climate"]

TEMPERATURE_SENSOR = 'temperature_sensor'
SLAVE = 'slave'
FAHRENHEIT = 'fahrenheit'

lg_controller_ns = cg.esphome_ns.namespace("lg_controller")
LGControllerComponent = lg_controller_ns.class_("LGControllerComponent", uart.UARTDevice, climate.Climate, cg.Component)

CONFIG_SCHEMA = cv.All(climate.climate_schema(LGControllerComponent).extend(
    {
        cv.GenerateID(): cv.declare_id(LGControllerComponent),
        cv.Required(CONF_RX_PIN): pins.gpio_input_pin_schema,
        cv.Optional(TEMPERATURE_SENSOR): cv.use_id(CONF_SENSOR),
        cv.Optional(SLAVE): cv.boolean,
        cv.Optional(FAHRENHEIT): cv.boolean,
    }
).extend(uart.UART_DEVICE_SCHEMA).extend(cv.COMPONENT_SCHEMA))

async def to_code(config):
    rx_pin = await cg.gpio_pin_expression(config[CONF_RX_PIN])
    var = cg.new_Pvariable(config[CONF_ID], rx_pin)
    
    if TEMPERATURE_SENSOR in config:
        temperature_sensor = await cg.get_variable(config[TEMPERATURE_SENSOR])
        cg.add(var.set_temperature_sensor(temperature_sensor))
    if SLAVE in config:
        cg.add(var.set_slave(config[SLAVE]))
    if FAHRENHEIT in config:
        cg.add(var.set_fahrenheit(config[FAHRENHEIT]))
    
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)
    await climate.register_climate(var, config)

# A schema to use for all LG Controller devices, all LG Controller integrations must extend this!
CONF_LG_CONTROLLER_ID = 'lg_controller_id'
ENTITY_ID = 'entity_id'

LGDevice = lg_controller_ns.class_("LGDevice")

LG_DEVICE_SCHEMA = cv.Schema(
    {
        # cv.GenerateID(CONF_ID): cv.declare_id(LGDevice),
        cv.GenerateID(CONF_LG_CONTROLLER_ID): cv.use_id(LGControllerComponent),
        cv.Required(ENTITY_ID): cv.valid_name,
    }
)

async def register_lg_device(var, config):
    parent = await cg.get_variable(config[CONF_LG_CONTROLLER_ID])
    cg.add(var.set_lg_controller_parent(parent))

    cg.add(var.set_entity_id(config[ENTITY_ID]))
    
    
