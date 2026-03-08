import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor, lg_controller
from esphome.const import CONF_ID

from .. import lg_controller_ns, LG_DEVICE_SCHEMA, ENTITY_ID

DEPENDENCIES = ["lg_controller"]
CODEOWNERS = ["@JanM321", "@rrooggiieerr"]

# Can we get these from lg_controller.h?
LG_ENTITY_ERROR_CODE = "error_code"
LG_ENTITY_PIPE_TEMPERATURE_IN = "pipe_temperature_in"
LG_ENTITY_PIPE_TEMPERATURE_MID = "pipe_temperature_mid"
LG_ENTITY_PIPE_TEMPERATURE_OUT = "pipe_temperature_out"

LG_ENTITY_ICONS = {
    LG_ENTITY_ERROR_CODE: "mdi:alert-circle-outline",
    LG_ENTITY_PIPE_TEMPERATURE_IN: "mdi:thermometer",
    LG_ENTITY_PIPE_TEMPERATURE_MID: "mdi:thermometer",
    LG_ENTITY_PIPE_TEMPERATURE_OUT: "mdi:thermometer",    
}

LGSensor = lg_controller_ns.class_("LGSensor", sensor.Sensor, cg.Component)

CONFIG_SCHEMA = cv.All(sensor.sensor_schema(LGSensor).extend(LG_DEVICE_SCHEMA).extend(
    {
        cv.GenerateID(CONF_ID): cv.declare_id(LGSensor),
        cv.Required(ENTITY_ID): cv.one_of(LG_ENTITY_ERROR_CODE, LG_ENTITY_PIPE_TEMPERATURE_IN, LG_ENTITY_PIPE_TEMPERATURE_MID, LG_ENTITY_PIPE_TEMPERATURE_OUT),
    }
).extend(cv.COMPONENT_SCHEMA))

async def to_code(config):
    entity_id = config[ENTITY_ID]
    config["icon"] = LG_ENTITY_ICONS[entity_id]

    if entity_id in [LG_ENTITY_PIPE_TEMPERATURE_IN, LG_ENTITY_PIPE_TEMPERATURE_MID, LG_ENTITY_PIPE_TEMPERATURE_OUT]:
        config["unit_of_measurement"] = "°C"

    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await sensor.register_sensor(var, config)
    await lg_controller.register_lg_device(var, config)
