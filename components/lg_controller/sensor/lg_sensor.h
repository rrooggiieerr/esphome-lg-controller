#pragma once

#include "esphome/components/sensor/sensor.h"
#include "esphome/components/lg_controller/lg_controller.h"

namespace esphome {
namespace lg_controller {

class LGSensor : public LGEntity, public sensor::Sensor, public Component {
public:
	void setup() {
//		this->set_internal(true);
	}
	void set_entity_id(std::string entity_id);
};

}  // namespace lg_controller
}  // namespace esphome
