import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import number, lg_controller
from esphome.const import CONF_ID

from .. import lg_controller_ns, LG_DEVICE_SCHEMA, ENTITY_ID

DEPENDENCIES = ["lg_controller"]
CODEOWNERS = ["@JanM321", "@rrooggiieerr"]

# Can we get these from lg_controller.h?
LG_ENTITY_FAN_SPEED_SLOW = "fan_speed_slow"
LG_ENTITY_FAN_SPEED_LOW = "fan_speed_low"
LG_ENTITY_FAN_SPEED_MEDIUM = "fan_speed_medium"
LG_ENTITY_FAN_SPEED_HIGH = "fan_speed_high"
LG_ENTITY_SLEEP_TIMER = "sleep_timer"

LGNumber = lg_controller_ns.class_("LGNumber", number.Number, cg.Component)

CONFIG_SCHEMA = cv.All(number.NUMBER_SCHEMA.extend(LG_DEVICE_SCHEMA).extend(
    {
        cv.GenerateID(CONF_ID): cv.declare_id(LGNumber),
        cv.Required(ENTITY_ID): cv.one_of(LG_ENTITY_FAN_SPEED_SLOW, LG_ENTITY_FAN_SPEED_LOW, LG_ENTITY_FAN_SPEED_MEDIUM, LG_ENTITY_FAN_SPEED_HIGH, LG_ENTITY_SLEEP_TIMER),
    }
).extend(cv.COMPONENT_SCHEMA))

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    max_value = 255
    if config[ENTITY_ID] == "sleep_timer":
        max_value = 420
    await number.register_number(var, config, min_value=0, max_value=max_value, step=1)
    await lg_controller.register_lg_device(var, config)
