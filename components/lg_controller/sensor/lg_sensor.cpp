#include "lg_sensor.h"
#include "esphome/core/log.h"

namespace esphome {
namespace lg_controller {

static const char *const TAG = "lg_controller.sensor";

void LGSensor::set_entity_id(std::string entity_id) {
	LGEntity::set_entity_id(entity_id);

	if(entity_id == LG_ENTITY_ERROR_CODE) {
		this->set_icon("mdi:alert-circle-outline");
	} else if(entity_id == LG_ENTITY_PIPE_TEMPERATURE_IN || entity_id == LG_ENTITY_PIPE_TEMPERATURE_MID || entity_id == LG_ENTITY_PIPE_TEMPERATURE_OUT) {
		this->set_icon("mdi:thermometer");
		this->set_unit_of_measurement("°C");
	}

	this->parent_->add_entity(entity_id, this);
}

}  // namespace lg_controller
}  // namespace esphome
