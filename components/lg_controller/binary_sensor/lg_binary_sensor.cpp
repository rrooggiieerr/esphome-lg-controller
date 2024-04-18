#include "lg_binary_sensor.h"
#include "esphome/core/log.h"

namespace esphome {
namespace lg_controller {

static const char *const TAG = "lg_controller.binary_sensor";

void LGBinarySensor::set_entity_id(std::string entity_id) {
	LGEntity::set_entity_id(entity_id);

	if(entity_id == LG_ENTITY_DEFROST) {
		this->set_icon("mdi:snowflake-melt");
	} else if(entity_id == LG_ENTITY_PREHEAT) {
		this->set_icon("mdi:heat-wave");
	} else if(entity_id == LG_ENTITY_OUTDOOR) {
		this->set_icon("mdi:fan");
	}

	this->parent_->add_entity(entity_id, this);
}

}  // namespace lg_controller
}  // namespace esphome
