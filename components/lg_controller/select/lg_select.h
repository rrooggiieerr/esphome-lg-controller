#pragma once

#include "esphome/components/select/select.h"
#include "esphome/components/lg_controller/lg_controller.h"

namespace esphome {
namespace lg_controller {

class LGSelect : public LGEntity, public select::Select, public Component {
public:
	void setup() {
//		this->set_internal(true);
	}
	void set_entity_id(std::string entity_id);
protected:
	void control(const std::string &value) override;
};

}  // namespace lg_controller
}  // namespace esphome
