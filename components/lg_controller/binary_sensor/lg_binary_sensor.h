#pragma once

#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/lg_controller/lg_controller.h"

namespace esphome {
namespace lg_controller {

class LGBinarySensor : public LGEntity, public binary_sensor::BinarySensor, public Component {
public:
	void set_entity_id(std::string entity);
};

}  // namespace lg_controller
}  // namespace esphome
