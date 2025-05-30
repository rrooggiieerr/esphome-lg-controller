import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor, lg_controller
from esphome.const import CONF_ID

from .. import lg_controller_ns, LG_DEVICE_SCHEMA, ENTITY_ID

DEPENDENCIES = ["lg_controller"]
CODEOWNERS = ["@JanM321", "@rrooggiieerr"]

# Can we get these from lg_controller.h?
LG_ENTITY_DEFROST = "defrost"
LG_ENTITY_PREHEAT = "preheat"
LG_ENTITY_OUTDOOR = "outdoor"
LG_ENTITY_AUTO_DRY_ACTIVE = "auto_dry_active"

LGBinarySensor = lg_controller_ns.class_("LGBinarySensor", binary_sensor.BinarySensor, cg.Component)

CONFIG_SCHEMA = cv.All(binary_sensor.binary_sensor_schema(LGBinarySensor).extend(LG_DEVICE_SCHEMA).extend(
    {
        cv.GenerateID(CONF_ID): cv.declare_id(LGBinarySensor),
        cv.Required(ENTITY_ID): cv.one_of(LG_ENTITY_DEFROST, LG_ENTITY_PREHEAT, LG_ENTITY_OUTDOOR, LG_ENTITY_AUTO_DRY_ACTIVE),
    }
).extend(cv.COMPONENT_SCHEMA))

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await binary_sensor.register_binary_sensor(var, config)
    await lg_controller.register_lg_device(var, config)
