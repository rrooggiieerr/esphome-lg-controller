#include "lg_number.h"
#include "esphome/core/log.h"

namespace esphome {
namespace lg_controller {

static const char *const TAG = "lg_controller.number";

void LGNumber::set_entity_id(std::string entity_id) {
	LGEntity::set_entity_id(entity_id);

	if(entity_id == LG_ENTITY_FAN_SPEED_SLOW) {
		this->set_icon("mdi:fan-chevron-down");
	} else if(entity_id == LG_ENTITY_FAN_SPEED_LOW) {
		this->set_icon("mdi:fan-speed-1");
	} else if(entity_id == LG_ENTITY_FAN_SPEED_MEDIUM) {
		this->set_icon("mdi:fan-speed-2");
	} else if(entity_id == LG_ENTITY_FAN_SPEED_HIGH) {
		this->set_icon("mdi:fan-speed-3");
	} else if(entity_id == LG_ENTITY_SLEEP_TIMER) {
		this->set_icon("mdi:timer-outline");
		this->publish_state(0);
	}

	this->parent_->add_entity(entity_id, this);
}

void LGNumber::control(float value) {
	LGNumber::control(value);

	if(this->entity_id_ == LG_ENTITY_FAN_SPEED_SLOW) {
		this->parent_->set_fan_speed(0, value);
	} else if(this->entity_id_ == LG_ENTITY_FAN_SPEED_LOW) {
		this->parent_->set_fan_speed(1, value);
	} else if(this->entity_id_ == LG_ENTITY_FAN_SPEED_MEDIUM) {
		this->parent_->set_fan_speed(2, value);
	} else if(this->entity_id_ == LG_ENTITY_FAN_SPEED_HIGH) {
		this->parent_->set_fan_speed(3, value);
	} else if(this->entity_id_ == LG_ENTITY_SLEEP_TIMER) {
		this->parent_->set_sleep_timer(value);
	}
}

}  // namespace lg_controller
}  // namespace esphome
