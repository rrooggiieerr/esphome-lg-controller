#pragma once

#include "esphome/components/number/number.h"
#include "esphome/components/lg_controller/lg_controller.h"

namespace esphome {
namespace lg_controller {

class LGNumber: public LGEntity, public number::Number, public Component {
public:
	void setup() {
//		this->set_internal(true);
	}
	void set_entity_id(std::string entity_id);
protected:
	void control(float value) override;
};

}  // namespace lg_controller
}  // namespace esphome
