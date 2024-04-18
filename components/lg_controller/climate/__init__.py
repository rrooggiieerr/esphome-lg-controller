import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import climate

from .. import CONF_LG_CONTROLLER_ID, LGControllerComponent

DEPENDENCIES = ["lg_controller"]
CODEOWNERS = ["JanM321", "@rrooggiieerr"]

CONFIG_SCHEMA = cv.All(climate.CLIMATE_SCHEMA.extend(
    {
        cv.GenerateID(CONF_LG_CONTROLLER_ID): cv.use_id(LGControllerComponent),
    }
).extend(cv.COMPONENT_SCHEMA))


async def to_code(config):
    var = await cg.get_variable(config[CONF_LG_CONTROLLER_ID])
    await climate.register_climate(var, config)
