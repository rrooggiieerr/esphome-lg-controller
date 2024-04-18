#include "lg_select.h"
#include "esphome/core/log.h"

namespace esphome {
namespace lg_controller {

static const char *const TAG = "lg_controller.select";

void LGSelect::set_entity_id(std::string entity_id) {
	LGEntity::set_entity_id(entity_id);

	this->parent_->add_entity(entity_id, this);
}

void LGSelect::control(const std::string &value) {
	auto idx = this->index_of(value);
	if (idx.has_value()) {
		if(this->entity_id_ == LG_ENTITY_VANE_POSITION_1) {
			this->parent_->set_vane_position(1, *idx);
		} else if(this->entity_id_ == LG_ENTITY_VANE_POSITION_2) {
			this->parent_->set_vane_position(2, *idx);
		} else if(this->entity_id_ == LG_ENTITY_VANE_POSITION_3) {
			this->parent_->set_vane_position(3, *idx);
		} else if(this->entity_id_ == LG_ENTITY_VANE_POSITION_4) {
			this->parent_->set_vane_position(4, *idx);
		} else if(this->entity_id_ == LG_ENTITY_OVERHEATING) {
			this->parent_->set_overheating(*idx);
		}
		return;
	}

	ESP_LOGW(TAG, "Invalid value %s", value.c_str());
}

}  // namespace lg_controller
}  // namespace esphome
