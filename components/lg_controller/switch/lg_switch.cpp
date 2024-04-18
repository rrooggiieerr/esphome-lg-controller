#include "lg_switch.h"
#include "esphome/core/log.h"

namespace esphome {
namespace lg_controller {

static const char *const TAG = "lg_controller.switch";

void LGSwitch::setup() {
	ESP_LOGCONFIG(TAG, "Setting up LG Controller switch...");

	//	this->set_internal(true);
	optional<bool> initial_state =
			Switch::get_initial_state_with_restore_mode();
	if (initial_state.has_value()) {
		// if it has a value, restore_mode is not "DISABLED", therefore act on the switch:
		if (initial_state.value()) {
			this->turn_on();
		} else {
			this->turn_off();
		}
	}
}

void LGSwitch::dump_config() {
	LOG_SWITCH(TAG, "LG Controller switch", this);
	ESP_LOGCONFIG(TAG, "  Entity: %s" , this->entity_id_.c_str());
}

void LGSwitch::set_entity_id(std::string entity_id) {
	LGEntity::set_entity_id(entity_id);

	if(entity_id == LG_ENTITY_AIR_PURIFIER) {
		this->set_icon("mdi:pine-tree");
	}
	else if(entity_id == LG_ENTITY_INTERNAL_THERMISTOR) {
		this->set_icon("mdi:thermometer");
		this->restore_and_set_mode(switch_::SWITCH_RESTORE_DEFAULT_OFF);
	}

	this->parent_->add_entity(entity_id, this);
}

void LGSwitch::set_state(bool state) {
	if(this->entity_id_ == LG_ENTITY_AIR_PURIFIER) {
	}
	else if(this->entity_id_ == LG_ENTITY_INTERNAL_THERMISTOR) {
//		this->parent_->set_
	}
}

}  // namespace lg_controller
}  // namespace esphome
