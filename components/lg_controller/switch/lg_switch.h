#pragma once

#include "esphome/components/switch/switch.h"
#include "esphome/components/lg_controller/lg_controller.h"

namespace esphome {
namespace lg_controller {

class LGSwitch: public LGEntity, public switch_::Switch, public Component {
public:
	void setup() override;
	void set_entity_id(std::string entity_id);

	void write_state(bool state) {
		publish_state(state);
		if(parent_ != nullptr)
			this->parent_->set_changed();
	}
	void dump_config() override;
	void set_state(bool state);

	void restore_and_set_mode(switch_::SwitchRestoreMode mode) {
		this->set_restore_mode(mode);
		if (auto state = get_initial_state_with_restore_mode()) {
			this->write_state(*state);
		}
	}
};

}  // namespace lg_controller
}  // namespace esphome
