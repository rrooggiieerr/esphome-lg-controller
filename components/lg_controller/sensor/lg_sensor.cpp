#include "lg_sensor.h"
#include "esphome/core/log.h"

namespace esphome {
namespace lg_controller {

static const char *const TAG = "lg_controller.sensor";

void LGSensor::set_entity_id(std::string entity_id) {
	LGEntity::set_entity_id(entity_id);

	this->parent_->add_entity(entity_id, this);
}

}  // namespace lg_controller
}  // namespace esphome
