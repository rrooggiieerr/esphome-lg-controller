import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import select, lg_controller
from esphome.const import CONF_ID

from .. import lg_controller_ns, LG_DEVICE_SCHEMA, ENTITY_ID

DEPENDENCIES = ["lg_controller"]
CODEOWNERS = ["JanM321", "@rrooggiieerr"]

# Can we get these from lg_controller.h?
LG_ENTITY_VANE_POSITION_1 = "vane_position_1"
LG_ENTITY_VANE_POSITION_2 = "vane_position_2"
LG_ENTITY_VANE_POSITION_3 = "vane_position_3"
LG_ENTITY_VANE_POSITION_4 = "vane_position_4"
LG_ENTITY_OVERHEATING = "overheating"

LGSelect = lg_controller_ns.class_("LGSelect", select.Select, cg.Component)

CONFIG_SCHEMA = cv.All(select.SELECT_SCHEMA.extend(LG_DEVICE_SCHEMA).extend(
    {
        cv.GenerateID(CONF_ID): cv.declare_id(LGSelect),
        cv.Required(ENTITY_ID): cv.one_of(LG_ENTITY_VANE_POSITION_1, LG_ENTITY_VANE_POSITION_2, LG_ENTITY_VANE_POSITION_3, LG_ENTITY_VANE_POSITION_4, LG_ENTITY_OVERHEATING),
    }
).extend(cv.COMPONENT_SCHEMA))

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    options = ["0 (Default)", "1 (Up)", "2", "3", "4", "5", "6 (Down)" ]
    if config[ENTITY_ID] == "overheating":
        options = ["0 (Default)", "1 (+4C/+6C)", "2 (+2C/+4C)", "3 (-1C/+1C)", "4 (-0.5C/+0.5C)"]

    await select.register_select(var, config, options=options)
    await lg_controller.register_lg_device(var, config)
