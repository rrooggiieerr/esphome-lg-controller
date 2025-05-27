import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import switch, lg_controller
from esphome.const import CONF_ID

from .. import lg_controller_ns, LG_DEVICE_SCHEMA, ENTITY_ID

DEPENDENCIES = ["lg_controller"]
CODEOWNERS = ["@JanM321", "@rrooggiieerr"]

# Can we get these from lg_controller.h?
LG_ENTITY_AIR_PURIFIER = "air_purifier"
LG_ENTITY_INTERNAL_THERMISTOR = "internal_thermistor"
LG_ENTITY_AUTO_DRY = "auto_dry"

LGSwitch = lg_controller_ns.class_("LGSwitch", cg.Component, switch.Switch)

CONFIG_SCHEMA = cv.All(switch.switch_schema(LGSwitch).extend(LG_DEVICE_SCHEMA).extend(
    {
        cv.GenerateID(CONF_ID): cv.declare_id(LGSwitch),
        cv.Required(ENTITY_ID): cv.one_of(LG_ENTITY_AIR_PURIFIER, LG_ENTITY_INTERNAL_THERMISTOR, LG_ENTITY_AUTO_DRY),
    }
).extend(cv.COMPONENT_SCHEMA))

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await switch.register_switch(var, config)
    await lg_controller.register_lg_device(var, config)
