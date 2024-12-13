#include "temp_conversion.h"

#include <set>

namespace esphome {
namespace lg_controller {

static const char *TAG = "lg_controller";

constexpr int8_t TempConversion::FahToLGCel[];
constexpr int8_t TempConversion::LGCelToCelAdjustment[];

bool LGControllerComponent::parse_capability(LgCapability capability) {
	switch (capability) {
		case LgCapability::PURIFIER:
			return (this->nvs_storage_.capabilities_message[2] & 0x02) != 0;
		case LgCapability::FAN_AUTO:
			return (this->nvs_storage_.capabilities_message[3] & 0x01) != 0;
		case LgCapability::FAN_SLOW:
			return (this->nvs_storage_.capabilities_message[3] & 0x20) != 0;
		case LgCapability::FAN_LOW:
			return (this->nvs_storage_.capabilities_message[3] & 0x10) != 0;
		case LgCapability::FAN_LOW_MEDIUM:
			return (this->nvs_storage_.capabilities_message[6] & 0x08) != 0;
		case LgCapability::FAN_MEDIUM:
			return (this->nvs_storage_.capabilities_message[3] & 0x08) != 0;
		case LgCapability::FAN_MEDIUM_HIGH:
			return (this->nvs_storage_.capabilities_message[6] & 0x10) != 0;
		case LgCapability::FAN_HIGH:
			return true;
		case LgCapability::MODE_HEATING:
			return (this->nvs_storage_.capabilities_message[2] & 0x40) != 0;
		case LgCapability::MODE_FAN:
			return (this->nvs_storage_.capabilities_message[2] & 0x80) != 0;
		case LgCapability::MODE_AUTO:
			return (this->nvs_storage_.capabilities_message[2] & 0x08) != 0;
		case LgCapability::MODE_DEHUMIDIFY:
			return (this->nvs_storage_.capabilities_message[2] & 0x80) != 0;
		case LgCapability::HAS_ONE_VANE:
			return (this->nvs_storage_.capabilities_message[5] & 0x40) != 0;
		case LgCapability::HAS_TWO_VANES:
			return (this->nvs_storage_.capabilities_message[5] & 0x80) != 0;
		case LgCapability::HAS_FOUR_VANES:
			// Actual flag is unknown, assume 4 vanes if neither 1 nor 2 vanes are supported
			// and the vane control bit is set.
			return (this->nvs_storage_.capabilities_message[5] & 0x40) == 0 &&
				(this->nvs_storage_.capabilities_message[5] & 0x80) == 0 &&
				(this->nvs_storage_.capabilities_message[4] & 0x01) != 0;
		case LgCapability::VERTICAL_SWING:
			return (this->nvs_storage_.capabilities_message[1] & 0x80) != 0;
		case LgCapability::HORIZONTAL_SWING:
			return (this->nvs_storage_.capabilities_message[1] & 0x40) != 0;
		case LgCapability::HAS_ESP_VALUE_SETTING:
			return (this->nvs_storage_.capabilities_message[4] & 0x02) != 0;
		case LgCapability::OVERHEATING_SETTING:
			return (this->nvs_storage_.capabilities_message[7] & 0x80) != 0;
		case LgCapability::AUTO_DRY:
			return (this->nvs_storage_.capabilities_message[4] & 0x80) != 0;
	}
	return false;
}

void LGControllerComponent::configure_capabilities() {
	// Default traits
	this->supported_traits_.set_supported_modes({
		climate::CLIMATE_MODE_OFF,
		climate::CLIMATE_MODE_COOL,
		climate::CLIMATE_MODE_HEAT,
		climate::CLIMATE_MODE_DRY,
		climate::CLIMATE_MODE_FAN_ONLY,
		climate::CLIMATE_MODE_HEAT_COOL,
	});
	this->supported_traits_.set_supported_fan_modes({
		climate::CLIMATE_FAN_LOW,
		climate::CLIMATE_FAN_MEDIUM,
		climate::CLIMATE_FAN_HIGH,
		climate::CLIMATE_FAN_AUTO,
	});
	this->supported_traits_.set_supported_swing_modes({
		climate::CLIMATE_SWING_OFF,
		climate::CLIMATE_SWING_BOTH,
		climate::CLIMATE_SWING_VERTICAL,
		climate::CLIMATE_SWING_HORIZONTAL,
	});
	this->supported_traits_.set_supports_current_temperature(true);
	this->supported_traits_.set_supports_two_point_target_temperature(false);
	this->supported_traits_.set_supports_action(false);
	this->supported_traits_.set_visual_min_temperature(MIN_TEMP_SETPOINT);
	this->supported_traits_.set_visual_max_temperature(MAX_TEMP_SETPOINT);
	this->supported_traits_.set_visual_current_temperature_step(this->fahrenheit_ ? 1 : 0.5);
	this->supported_traits_.set_visual_target_temperature_step(this->fahrenheit_ ? 1 : 0.5);

	// Only override defaults if the capabilities are known
	if (this->nvs_storage_.capabilities_message[0] != 0) {
		// Configure the climate traits
		std::set<climate::ClimateMode> device_modes;
		device_modes.insert(climate::CLIMATE_MODE_OFF);
		device_modes.insert(climate::CLIMATE_MODE_COOL);
		if (this->parse_capability(LgCapability::MODE_HEATING))
			device_modes.insert(climate::CLIMATE_MODE_HEAT);
		if (this->parse_capability(LgCapability::MODE_FAN))
			device_modes.insert(climate::CLIMATE_MODE_FAN_ONLY);
		if (this->parse_capability(LgCapability::MODE_AUTO))
			device_modes.insert(climate::CLIMATE_MODE_HEAT_COOL);
		if (this->parse_capability(LgCapability::MODE_DEHUMIDIFY))
			device_modes.insert(climate::CLIMATE_MODE_DRY);
		this->supported_traits_.set_supported_modes(device_modes);

		std::set<climate::ClimateFanMode> fan_modes;
		if (this->parse_capability(LgCapability::FAN_AUTO))
			fan_modes.insert(climate::CLIMATE_FAN_AUTO);
		if (this->parse_capability(LgCapability::FAN_SLOW))
			fan_modes.insert(climate::CLIMATE_FAN_QUIET);
		if (this->parse_capability(LgCapability::FAN_LOW))
			fan_modes.insert(climate::CLIMATE_FAN_LOW);
		if (this->parse_capability(LgCapability::FAN_MEDIUM))
			fan_modes.insert(climate::CLIMATE_FAN_MEDIUM);
		if (this->parse_capability(LgCapability::FAN_HIGH))
			fan_modes.insert(climate::CLIMATE_FAN_HIGH);
		this->supported_traits_.set_supported_fan_modes(fan_modes);

		std::set<climate::ClimateSwingMode> swing_modes;
		swing_modes.insert(climate::CLIMATE_SWING_OFF);
		if (this->parse_capability(LgCapability::VERTICAL_SWING) && parse_capability(LgCapability::HORIZONTAL_SWING))
			swing_modes.insert(climate::CLIMATE_SWING_BOTH);
		if (this->parse_capability(LgCapability::VERTICAL_SWING))
			swing_modes.insert(climate::CLIMATE_SWING_VERTICAL);
		if (this->parse_capability(LgCapability::HORIZONTAL_SWING))
			swing_modes.insert(climate::CLIMATE_SWING_HORIZONTAL);
		this->supported_traits_.set_supported_swing_modes(swing_modes);

		// Disable unsupported entities
		if (this->vane_select_1_ != nullptr && !this->parse_capability(LgCapability::HAS_ONE_VANE))
			this->vane_select_1_->set_internal(true);
		if (this->vane_select_2_ != nullptr && !this->parse_capability(LgCapability::HAS_TWO_VANES))
			this->vane_select_2_->set_internal(true);
		if (this->vane_select_3_ != nullptr && !this->parse_capability(LgCapability::HAS_FOUR_VANES))
			this->vane_select_3_->set_internal(true);
		if (this->vane_select_4_ != nullptr && !this->parse_capability(LgCapability::HAS_FOUR_VANES))
			this->vane_select_4_->set_internal(true);

		if (this->fan_speed_slow_ != nullptr && (this->slave_ || !this->parse_capability(LgCapability::HAS_ESP_VALUE_SETTING) || !this->parse_capability(LgCapability::FAN_SLOW)))
			this->fan_speed_slow_->set_internal(true);
		if (this->fan_speed_low_ != nullptr && (this->slave_ || !this->parse_capability(LgCapability::HAS_ESP_VALUE_SETTING) || !this->parse_capability(LgCapability::FAN_LOW)))
			this->fan_speed_low_->set_internal(true);
		if (this->fan_speed_medium_ != nullptr && (this->slave_ || !this->parse_capability(LgCapability::HAS_ESP_VALUE_SETTING) || !this->parse_capability(LgCapability::FAN_MEDIUM)))
			this->fan_speed_medium_->set_internal(true);
		if (this->fan_speed_high_ != nullptr && (this->slave_ || !this->parse_capability(LgCapability::HAS_ESP_VALUE_SETTING) || !this->parse_capability(LgCapability::FAN_HIGH)))
			this->fan_speed_high_->set_internal(true);
		if (this->overheating_select_ != nullptr && (this->slave_ || !this->parse_capability(LgCapability::OVERHEATING_SETTING)))
			this->overheating_select_->set_internal(true);

		if (this->purifier_entity_ != nullptr && !this->parse_capability(LgCapability::PURIFIER))
			this->purifier_entity_->set_internal(true);
		if (this->auto_dry_entity_ != nullptr && !this->parse_capability(LgCapability::AUTO_DRY))
			this->auto_dry_entity_->set_internal(true);
		if (this->auto_dry_active_entity_ != nullptr && !this->parse_capability(LgCapability::AUTO_DRY))
			this->auto_dry_active_entity_->set_internal(true);
	}

	if(this->internal_thermistor_entity_ != nullptr && this->slave_)
		this->internal_thermistor_entity_->set_internal(true);
	if(this->sleep_timer_ != nullptr && this->slave_)
		this->sleep_timer_->set_internal(true);
}

void LGControllerComponent::setup() {
	// Load our custom NVS storage to get the capabilities message
	ESPPreferenceObject pref = global_preferences->make_preference<NVSStorage>(this->get_object_id_hash() ^ NVS_STORAGE_VERSION);
	pref.load(&this->nvs_storage_);

	auto restore = this->restore_state_();
	if (restore.has_value()) {
		restore->apply(this);
	} else {
		this->mode = climate::CLIMATE_MODE_OFF;
		this->target_temperature = 20;
		this->fan_mode = climate::CLIMATE_FAN_MEDIUM;
		this->swing_mode = climate::CLIMATE_SWING_OFF;
		this->publish_state();
	}

	// Configure climate traits and entities based on the capabilities message (if available)
	this->configure_capabilities();


	while (this->available() > 0) {
		uint8_t b;
		this->read_byte(&b);
	}
	this->set_changed();

	// Call `update` every 6 seconds, but first wait 10 seconds.
	this->set_timeout("initial_send", 10000, [this]() {
		this->set_interval("update", 6000, [this]() { update(); });
	});
}

void LGControllerComponent::dump_config() {
	ESP_LOGCONFIG(TAG, "LG Controller component:");
}

// Process changes from HA.
void LGControllerComponent::control(const climate::ClimateCall &call) {
	if (call.get_mode().has_value()) {
		this->mode = *call.get_mode();
	}
	if (call.get_target_temperature().has_value()) {
		this->target_temperature = *call.get_target_temperature();
	}
	if (call.get_fan_mode().has_value()) {
		this->fan_mode = *call.get_fan_mode();
	}
	if (call.get_swing_mode().has_value()) {
		set_swing_mode(*call.get_swing_mode());
	}
	this->set_changed();
	this->publish_state();
}

// Sets position of vane index (1-4) to position (0-6).
void LGControllerComponent::set_vane_position(int index, int position) {
	if (index < 1 || index > 4) {
		ESP_LOGE(TAG, "Unexpected vane index: %d", index);
		return;
	}
	if (position < 0 || position > 6) {
		ESP_LOGE(TAG, "Unexpected vane position: %d", position);
		return;
	}
	if (this->vane_position_[index-1] == position) {
		return;
	}
	ESP_LOGD(TAG, "Setting vane %d position: %d", index, position);
	this->vane_position_[index-1] = position;
	if (!this->is_initializing_) {
		this->pending_type_a_settings_change_ = true;
	}
}

// Sets installer setting fan speed index (0-3 for slow-high) to value (0-255), with "0" being the factory default
void LGControllerComponent::set_fan_speed(int index, int value) {
	if (index < 0 || index > 3) {
		ESP_LOGE(TAG, "Unexpected fan speed index: %d", index);
		return;
	}
	if (value<0 || value>255) {
		ESP_LOGE(TAG, "Unexpected fan speed: %d", value);
		return;
	}

	if (this->fan_speed_[index] == value) {
		return;
	}

	this->fan_speed_[index] = value;
	if (!this->is_initializing_) {
		this->pending_type_a_settings_change_ = true;
	}
}

// Set overheating installer setting 15 to 0-4.
void LGControllerComponent::set_overheating(int value) {
	if (value < 0 || value > 4) {
		ESP_LOGE(TAG, "Unexpected overheating value: %d", value);
		return;
	}

	if (this->overheating_ == value) {
		return;
	}

	ESP_LOGD(TAG, "Setting overheating installer setting: %d", value);

	this->overheating_ = value;
	if (!this->is_initializing_) {
		this->pending_type_b_settings_change_ = true;
	}
}

void LGControllerComponent::set_sleep_timer(int minutes) {
	if (this->ignore_sleep_timer_callback_) {
		return;
	}
	// 0 clears the timer. Accept max 7 hours.
	if (minutes < 0 || minutes > 7 * 60) {
		ESP_LOGE(TAG, "Ignoring invalid sleep timer value: %d minutes", minutes);
		return;
	}
	if (this->slave_) {
		ESP_LOGE(TAG, "Ignoring sleep timer for slave controller");
		return;
	}
	ESP_LOGD(TAG, "Setting sleep timer: %d minutes", minutes);
	if (minutes > 0) {
		this->sleep_timer_target_millis_ = millis() + unsigned(minutes) * 60 * 1000;
		this->active_reservation_ = true;
	} else {
		this->sleep_timer_target_millis_.reset();
		this->active_reservation_ = false;
	}
	this->set_changed();
}

optional<float> LGControllerComponent::get_room_temp() {
	if(this->temperature_sensor_ != nullptr) {
		float temp = this->temperature_sensor_->get_state();
		if (std::isnan(temp) || temp == 0) {
			return {};
		}
		if (this->fahrenheit_) {
			temp = TempConversion::fahrenheit_to_lgcelsius(temp);
		}
		if (temp < 11) {
			return 11;
		}
		if (temp > 35) {
			return 35;
		}
		// Round to nearest 0.5 degrees.
		return round(temp * 2) / 2;
	} else {
		//ToDo: Read sensor from HVAC
	}
	return {};
}

optional<uint32_t> LGControllerComponent::get_sleep_timer_minutes() {
	if (!this->sleep_timer_target_millis_.has_value()) {
		return {};
	}
	int32_t diff = int32_t(this->sleep_timer_target_millis_.value() - millis());
	if (diff <= 0) {
		return {};
	}
	uint32_t minutes = uint32_t(diff) / 1000 / 60 + 1;
	return minutes;
}

uint8_t LGControllerComponent::calc_checksum(const uint8_t* buffer) {
	size_t result = 0;
	for (size_t i = 0; i < 12; i++) {
		result += buffer[i];
	}
	return (result & 0xff) ^ 0x55;
}

void LGControllerComponent::set_swing_mode(climate::ClimateSwingMode mode) {
	if (this->swing_mode != mode) {
		// If vertical swing is off, send a 0xAA message to restore the vane position.
		if (mode == climate::CLIMATE_SWING_OFF || mode == climate::CLIMATE_SWING_HORIZONTAL) {
			this->pending_type_a_settings_change_ = true;
		}
	}
	this->swing_mode = mode;
}

void LGControllerComponent::add_entity(std::string entity_id, binary_sensor::BinarySensor *entity) {
	ESP_LOGD(TAG, "Adding binary sensor entity for id %s", entity_id.c_str());

	if(entity_id == LG_ENTITY_DEFROST) {
		this->defrost_entity_ = entity;
//		entity->publish_state(this->defrost_);
	} else if(entity_id == LG_ENTITY_PREHEAT) {
		this->preheat_entity_ = entity;
//		entity->publish_state(this->defrost_);
	} else if(entity_id == LG_ENTITY_OUTDOOR) {
		this->outdoor_entity_ = entity;
//		entity->publish_state(this->defrost_);
	} else if(entity_id == LG_ENTITY_AUTO_DRY_ACTIVE) {
		this->auto_dry_active_entity_ = entity;
	}
}

void LGControllerComponent::add_entity(std::string entity_id, number::Number *entity) {
	ESP_LOGD(TAG, "Adding number entity for id %s", entity_id.c_str());

	if(entity_id == LG_ENTITY_FAN_SPEED_SLOW) {
		this->fan_speed_slow_ = entity;
	} else if(entity_id == LG_ENTITY_FAN_SPEED_LOW) {
		this->fan_speed_low_ = entity;
	} else if(entity_id == LG_ENTITY_FAN_SPEED_MEDIUM) {
		this->fan_speed_medium_ = entity;
	} else if(entity_id == LG_ENTITY_FAN_SPEED_HIGH) {
		this->fan_speed_high_ = entity;
	} else if(entity_id == LG_ENTITY_SLEEP_TIMER) {
		this->sleep_timer_ = entity;
	}
}

void LGControllerComponent::add_entity(std::string entity_id, select::Select *entity) {
	ESP_LOGD(TAG, "Adding select entity for id %s", entity_id.c_str());

	if(entity_id == LG_ENTITY_VANE_POSITION_1) {
		this->vane_select_1_ = entity;
	} else if(entity_id == LG_ENTITY_VANE_POSITION_2) {
		this->vane_select_2_ = entity;
	} else if(entity_id == LG_ENTITY_VANE_POSITION_3) {
		this->vane_select_3_ = entity;
	} else if(entity_id == LG_ENTITY_VANE_POSITION_4) {
		this->vane_select_4_ = entity;
	} else if(entity_id == LG_ENTITY_OVERHEATING) {
		this->overheating_select_ = entity;
	}
}

void LGControllerComponent::add_entity(std::string entity_id, sensor::Sensor *entity) {
	ESP_LOGD(TAG, "Adding sensor entity for id %s", entity_id.c_str());

	if(entity_id == LG_ENTITY_ERROR_CODE) {
		this->error_code_ = entity;
	} else if(entity_id == LG_ENTITY_PIPE_TEMPERATURE_IN) {
		this->pipe_temp_in_ = entity;
	} else if(entity_id == LG_ENTITY_PIPE_TEMPERATURE_MID) {
		this->pipe_temp_mid_ = entity;
	} else if(entity_id == LG_ENTITY_PIPE_TEMPERATURE_OUT) {
		this->pipe_temp_out_ = entity;
	}
}

void LGControllerComponent::add_entity(std::string entity_id, switch_::Switch *entity) {
	ESP_LOGD(TAG, "Adding switch entity for id %s", entity_id.c_str());

	if(entity_id == LG_ENTITY_AIR_PURIFIER) {
		this->purifier_entity_ = entity;
		this->purifier_ = this->purifier_entity_->state;
	} else if(entity_id == LG_ENTITY_INTERNAL_THERMISTOR) {
		this->internal_thermistor_entity_ = entity;
		this->internal_thermistor_ = this->internal_thermistor_entity_->state;
	} else if(entity_id == LG_ENTITY_AUTO_DRY) {
		this->auto_dry_entity_ = entity;
		this->auto_dry_ = this->auto_dry_entity_->state;
	}
}

void LGControllerComponent::send_status_message() {
	// Byte 0: message type.
	this->send_buf_[0] = this->slave_ ? 0x28 : 0xA8;

	// Byte 1: changed flag (0x1), power on (0x2), mode (0x1C), fan speed (0x70).
	uint8_t b = 0;
	if (this->pending_status_change_) {
		b |= 0x1;
	}
	switch (this->mode) {
		case climate::CLIMATE_MODE_COOL:
			b |= (0 << 2) | 0x2;
			break;
		case climate::CLIMATE_MODE_DRY:
			b |= (1 << 2) | 0x2;
			break;
		case climate::CLIMATE_MODE_FAN_ONLY:
			b |= (2 << 2) | 0x2;
			break;
		case climate::CLIMATE_MODE_HEAT_COOL:
			b |= (3 << 2) | 0x2;
			break;
		case climate::CLIMATE_MODE_HEAT:
			b |= (4 << 2) | 0x2;
			break;
		case climate::CLIMATE_MODE_OFF:
			// Don't set power-on flag, but preserve previous operation mode.
			b |= (this->last_recv_status_[1] & 0x1C);
			break;
		default:
			ESP_LOGE(TAG, "unknown operation mode, turning off");
			b |= (2 << 2);
			break;
	}
	switch (*this->fan_mode) {
		case climate::CLIMATE_FAN_LOW:
			b |= 0 << 5;
			break;
		case climate::CLIMATE_FAN_MEDIUM:
			b |= 1 << 5;
			break;
		case climate::CLIMATE_FAN_HIGH:
			b |= 2 << 5;
			break;
		case climate::CLIMATE_FAN_AUTO:
			b |= 3 << 5;
			break;
		case climate::CLIMATE_FAN_QUIET:
			b |= 4 << 5;
			break;
		default:
			ESP_LOGE(TAG, "unknown fan mode, using Medium");
			b |= 1 << 5;
			break;
	}
	this->send_buf_[1] = b;

	// Byte 2: swing mode and purifier/plasma setting. Preserve the other bits.
	b = this->last_recv_status_[2] & ~(0x4|0x40|0x80);
	if (this->purifier_) {
		b |= 0x4;
	}
	switch (this->swing_mode) {
		case climate::CLIMATE_SWING_OFF:
			break;
		case climate::CLIMATE_SWING_HORIZONTAL:
			b |= 0x40;
			break;
		case climate::CLIMATE_SWING_VERTICAL:
			b |= 0x80;
			break;
		case climate::CLIMATE_SWING_BOTH:
			b |= 0x40 | 0x80;
			break;
		default:
			ESP_LOGE(TAG, "unknown swing mode");
			break;
	}
	this->send_buf_[2] = b;

	// Byte 3.
	this->send_buf_[3] = this->last_recv_status_[3];
	if (this->active_reservation_) {
		this->send_buf_[3] |= 0x10;
	} else {
		this->send_buf_[3] &= ~0x10;
	}

	// Byte 4.
	this->send_buf_[4] = this->last_recv_status_[4];

	float target = this->target_temperature;
	if (this->fahrenheit_) {
		target = TempConversion::celsius_to_lgcelsius(target);
	}
	if (target < MIN_TEMP_SETPOINT) {
		target = MIN_TEMP_SETPOINT;
	} else if (target > MAX_TEMP_SETPOINT) {
		target = MAX_TEMP_SETPOINT;
	}

	// Byte 5. Unchanged except for the low bit which indicates the target temperature has a
	// 0.5 fractional part.
	this->send_buf_[5] = this->last_recv_status_[5] & ~0x1;
	if (target - uint8_t(target) == 0.5) {
		this->send_buf_[5] |= 0x1;
	}

	// Byte 6: thermistor setting and target temperature (fractional part in byte 5).
	// Byte 7: room temperature. Preserve the (unknown) upper two bits.
	enum ThermistorSetting { Unit = 0, Controller = 1, TwoTH = 2 };
	ThermistorSetting thermistor =
			this->internal_thermistor_ ? ThermistorSetting::Unit : ThermistorSetting::Controller;
	float temp;
	if (auto maybe_temp = get_room_temp()) {
		temp = *maybe_temp;
	} else {
		// Room temperature isn't available. Use the unit's thermistor and send something
		// reasonable.
		thermistor = ThermistorSetting::Unit;
		temp = 20;
	}
	this->send_buf_[6] = (thermistor << 4) | ((uint8_t(target) - 15) & 0xf);
	this->send_buf_[7] = (this->last_recv_status_[7] & 0xC0) | uint8_t((temp - 10) * 2);

	// Bytes 8-10. Initialize bytes 8-9 to 0 to not echo back timer settings set by the AC.
	this->send_buf_[8] = 0;
	this->send_buf_[9] = 0;
	this->send_buf_[10] = this->last_recv_status_[10];

	if (this->is_initializing_) {
		// Request settings when controller turns on.
		this->send_buf_[8] |= 0x40;
		// Set bit 0x80 of byte 10 to use byte 9 for the Fahrenheit setting flag (0x40).
		if (this->fahrenheit_) {
			this->send_buf_[9] |= 0x40;
		}
		this->send_buf_[10] = 0x80;
	} else if (optional<uint32_t> minutes = this->get_sleep_timer_minutes()) {
		// Set sleep timer.
		// Byte 8 stores the kind (0x38) and high bits of number of minutes (0x7).
		// Byte 9 stores the low bits of the number of minutes.
		constexpr uint8_t timer_kind_sleep = 3;
		this->send_buf_[8] = timer_kind_sleep << 3;
		this->send_buf_[8] |= (*minutes >> 8) & 0b111;
		this->send_buf_[9] = *minutes & 0xff;
	}

	// Byte 11.
	this->send_buf_[11] = this->last_recv_status_[11];

	// Byte 12.
	this->send_buf_[12] = this->calc_checksum(this->send_buf_);

	ESP_LOGD(TAG, "sending %s", format_hex_pretty(this->send_buf_, MsgLen).c_str());
	this->write_array(this->send_buf_, MsgLen);

	this->pending_status_change_ = false;
	this->pending_send_ = PendingSendKind::Status;
	this->last_sent_status_millis_ = millis();

	// If we sent an updated temperature to the AC, update temperature in HA too.
	// Slave controller temperature sensor is ignored.
	if (!this->slave_ && thermistor == ThermistorSetting::Controller) {
		float ha_temp = temp;
		if (this->fahrenheit_) {
			ha_temp = TempConversion::lgcelsius_to_celsius(ha_temp);
		}
		if (this->current_temperature != ha_temp) {
			this->current_temperature = ha_temp;
			this->publish_state();
		}
	}
}

void LGControllerComponent::send_type_a_settings_message() {
	if (this->last_recv_type_a_settings_[0] != 0xCA && this->last_recv_type_a_settings_[0] != 0xAA) {
		ESP_LOGE(TAG, "Unexpected missing previous CA/AA message");
		this->pending_type_a_settings_change_ = false;
		return;
	}

	// Copy settings from the CA/AA message we received.
	memcpy(this->send_buf_, this->last_recv_type_a_settings_, MsgLen);
	this->send_buf_[0] = this->slave_ ? 0x2A : 0xAA;

	// Bytes 2-6 store the installer fan speeds
	this->send_buf_[2] = this->fan_speed_[0];
	this->send_buf_[3] = this->fan_speed_[1];
	this->send_buf_[4] = this->fan_speed_[2];
	this->send_buf_[5] = this->fan_speed_[3];

	// Bytes 7-8 store vane positions.
	this->send_buf_[7] = (this->send_buf_[7] & 0xf0) | (this->vane_position_[0] & 0x0f); // Set vane 1
	this->send_buf_[7] = (this->send_buf_[7] & 0x0f) | ((this->vane_position_[1] & 0x0f) << 4); // Set vane 2
	this->send_buf_[8] = (this->send_buf_[8] & 0xf0) | (this->vane_position_[2] & 0x0f); // Set vane 3
	this->send_buf_[8] = (this->send_buf_[8] & 0x0f) | ((this->vane_position_[3] & 0x0f) << 4); // Set vane 4

	// Set auto dry setting.
	uint8_t b = this->send_buf_[11] & ~0x8;
	if (this->auto_dry_) {
		b |= 0x8;
	}
	this->send_buf_[11] = b;

	this->send_buf_[12] = calc_checksum(this->send_buf_);

	ESP_LOGD(TAG, "sending %s", format_hex_pretty(this->send_buf_, MsgLen).c_str());
	this->write_array(this->send_buf_, MsgLen);

	this->pending_type_a_settings_change_ = false;
	this->pending_send_ = PendingSendKind::TypeA;
}

void LGControllerComponent::send_type_b_settings_message(bool timed) {
	if (timed) {
		ESP_LOGD(TAG, "sending timed AB message");
	}
	if (this->last_recv_type_b_settings_[0] != 0xCB && this->last_recv_type_b_settings_[0] != 0xAB) {
		ESP_LOGE(TAG, "Unexpected missing previous CB/AB message");
		this->pending_type_b_settings_change_ = false;
		// Don't try to send another message immediately after.
		this->last_sent_recv_type_b_millis_ = millis();
		return;
	}

	// Copy settings from the CB/AB message we received.
	memcpy(this->send_buf_, this->last_recv_type_b_settings_, MsgLen);
	this->send_buf_[0] = this->slave_ ? 0x2B : 0xAB;

	// Set the high bit of the second byte to request a CB message from the unit.
	if (timed) {
		this->send_buf_[1] |= 0x80;
	} else {
		this->send_buf_[1] &= ~0x80;
	}

	// Byte 2 stores installer setting 15.
	this->send_buf_[2] = (this->send_buf_[2] & 0xC7) | (this->overheating_ << 3);

	this->send_buf_[12] = calc_checksum(this->send_buf_);

	ESP_LOGD(TAG, "sending %s", format_hex_pretty(this->send_buf_, MsgLen).c_str());
	this->write_array(this->send_buf_, MsgLen);

	this->pending_type_b_settings_change_ = false;
	this->pending_send_ = PendingSendKind::TypeB;
	this->last_sent_recv_type_b_millis_ = millis();
}

void LGControllerComponent::process_message(const uint8_t* buffer, bool* had_error) {
	ESP_LOGD(TAG, "received %s", format_hex_pretty(buffer, MsgLen).c_str());

	if (calc_checksum(buffer) != buffer[12]) {
		// When initializing, the unit sends an all-zeroes message as padding between
		// messages. Ignore those false checksum failures.
		auto is_zero = [](uint8_t b) { return b == 0; };
		if (std::all_of(buffer, buffer + MsgLen, is_zero)) {
			ESP_LOGD(TAG, "Ignoring padding message sent by unit");
			return;
		}
		ESP_LOGE(TAG, "invalid checksum %s", format_hex_pretty(buffer, MsgLen).c_str());
		*had_error = true;
		return;
	}

	if (this->pending_send_ != PendingSendKind::None && memcmp(this->send_buf_, buffer, MsgLen) == 0) {
		ESP_LOGD(TAG, "verified send");
		this->pending_send_ = PendingSendKind::None;
		return;
	}

	// Determine message type.
	optional<MessageSender> sender;
	switch (buffer[0] & 0xf8) {
		case 0xC8:
			sender = MessageSender::Unit;
			break;
		case 0xA8:
			if (!this->slave_) {
				// Ignore (our own?) master controller messages.
				return;
			}
			sender = MessageSender::Master;
			break;
		case 0x28:
			if (this->slave_) {
				// Ignore (our own?) slave controller messages.
				return;
			}
			sender = MessageSender::Slave;
			break;
		default:
			return; // Unknown message sender. Ignore.
	}

	switch (buffer[0] & 0b111) {
		case 0: // 0xC8/A8/28
			this->process_status_message(*sender, buffer, had_error);
			break;
		case 1: // 0xC9
			this->process_capabilities_message(*sender, buffer);
			break;
		case 2: // 0xCA/AA/2A
			this->process_type_a_settings_message(*sender, buffer);
			break;
		case 3: // 0xCB/AB/2B
			this->process_type_b_settings_message(*sender, buffer);
			break;
		default:
			return;
	}
}

void LGControllerComponent::process_status_message(MessageSender sender, const uint8_t* buffer, bool* had_error) {
	// If we just had a failure, ignore this messsage because it might be invalid too.
	if (*had_error) {
		ESP_LOGE(TAG, "ignoring due to previous error %s",
				 format_hex_pretty(buffer, MsgLen).c_str());
		return;
	}

	// Consider slave controller initialized if we received a status message from the other
	// controller or the unit.
	if (this->slave_) {
		this->is_initializing_ = false;
	}

	// Handle simple input sensors first. These are safe to update even if we have a pending
	// change.

	this->defrost_ = buffer[3] & 0x4;
	if(this->defrost_entity_ != nullptr)
		this->defrost_entity_->publish_state(this->defrost_);
	this->preheat_ = buffer[3] & 0x8;
	if(this->preheat_entity_ != nullptr)
		this->preheat_entity_->publish_state(this->preheat_);

	if (sender == MessageSender::Unit && this->error_code_ != nullptr) {
		this->error_code_->publish_state(buffer[11]);
	}

	// When turning on the outdoor unit, the AC sometimes reports ON => OFF => ON within a
	// few seconds. No big deal but it causes noisy state changes in HA. Only report OFF if
	// the last state change was at least 8 seconds ago.
	bool outdoor_on = buffer[5] & 0x4;
	bool outdoor_changed = this->outdoor_ != outdoor_on;
	if (outdoor_on) {
		this->outdoor_ = true;
	} else if (millis() - this->last_outdoor_change_millis_ > 8000) {
		this->outdoor_ = false;
	}
	if(this->outdoor_entity_ != nullptr)
		this->outdoor_entity_->publish_state(this->outdoor_);
	if (outdoor_changed) {
		this->last_outdoor_change_millis_ = millis();
	}

	if (sender == MessageSender::Unit && this->auto_dry_entity_ != nullptr) {
		bool unit_off = (buffer[1] & 0x2) == 0;
		bool drying = (buffer[10] & 0x10) && unit_off;
		if(this->auto_dry_active_entity_ != nullptr)
			this->auto_dry_active_entity_->publish_state(drying);
	}

	bool read_temp = false;
	if (this->slave_) {
		// Let the slave controller report the temperature from the master.
		read_temp = (sender == MessageSender::Master);
	} else {
		// Report the unit's room temperature only if we're using the internal thermistor.
		// With an external temperature sensor, some units report the temperature we sent and
		// others always send the internal temperature.
		read_temp = (sender == MessageSender::Unit && this->internal_thermistor_);
	}
	if (read_temp) {
		float room_temp = float(buffer[7] & 0x3F) / 2 + 10;
		if (this->fahrenheit_) {
			room_temp = TempConversion::lgcelsius_to_celsius(room_temp);
		}
		if (this->current_temperature != room_temp) {
			this->current_temperature = room_temp;
			this->publish_state();
		}
	}

	// Don't update our settings if we have a pending change/send, because else we overwrite
	// changes we still have to send (or are sending) to the AC.
	if (this->pending_status_change_) {
		ESP_LOGD(TAG, "ignoring because pending change");
		return;
	}
	if (this->pending_send_ == PendingSendKind::Status) {
		ESP_LOGD(TAG, "ignoring because pending send");
		return;
	}

	if (sender != MessageSender::Slave) {
		memcpy(this->last_recv_status_, buffer, MsgLen);
	}

	uint8_t b = buffer[1];
	if ((b & 0x2) == 0) {
		this->mode = climate::CLIMATE_MODE_OFF;
	} else {
		uint8_t mode_val = (b >> 2) & 0b111;
		switch (mode_val) {
			case 0:
				this->mode = climate::CLIMATE_MODE_COOL;
				break;
			case 1:
				this->mode = climate::CLIMATE_MODE_DRY;
				break;
			case 2:
				this->mode = climate::CLIMATE_MODE_FAN_ONLY;
				break;
			case 3:
				this->mode = climate::CLIMATE_MODE_HEAT_COOL;
				break;
			case 4:
				this->mode = climate::CLIMATE_MODE_HEAT;
				break;
			default:
				ESP_LOGE(TAG, "received invalid operation mode from AC (%u)", mode_val);
				*had_error = true;
				return;
		}
	}

	uint8_t fan_val = b >> 5;
	switch (fan_val) {
		case 0:
			this->fan_mode = climate::CLIMATE_FAN_LOW;
			break;
		case 1:
			this->fan_mode = climate::CLIMATE_FAN_MEDIUM;
			break;
		case 2:
			this->fan_mode = climate::CLIMATE_FAN_HIGH;
			break;
		case 3:
			this->fan_mode = climate::CLIMATE_FAN_AUTO;
			break;
		case 4:
			this->fan_mode = climate::CLIMATE_FAN_QUIET;
			break;
		default:
			ESP_LOGE(TAG, "received unexpected fan mode from AC (%u)", fan_val);
			*had_error = true;
			return;
	}

	this->purifier_ = buffer[2] & 0x4;
	if(this->purifier_entity_ != nullptr)
		this->purifier_entity_->publish_state(this->purifier_);

	bool horiz_swing = buffer[2] & 0x40;
	bool vert_swing = buffer[2] & 0x80;
	if (horiz_swing && vert_swing) {
		this->set_swing_mode(climate::CLIMATE_SWING_BOTH);
	} else if (horiz_swing) {
		this->set_swing_mode(climate::CLIMATE_SWING_HORIZONTAL);
	} else if (vert_swing) {
		this->set_swing_mode(climate::CLIMATE_SWING_VERTICAL);
	} else {
		this->set_swing_mode(climate::CLIMATE_SWING_OFF);
	}

	float target = float((buffer[6] & 0xf) + 15);
	if (buffer[5] & 0x1) {
		target += 0.5;
	}
	if (this->fahrenheit_) {
		target = TempConversion::lgcelsius_to_celsius(target);
	}
	this->target_temperature = target;

	this->active_reservation_ = buffer[3] & 0x10;

	// Set or clear sleep timer.
	if (!this->slave_ && this->sleep_timer_ != nullptr) {
		if (this->sleep_timer_target_millis_.has_value() && !this->active_reservation_) {
			this->sleep_timer_->publish_state(0);
		} else if (((buffer[8] >> 3) & 0x7) == 3) {
			uint32_t minutes = (uint32_t(buffer[8] & 0x7) << 8) | buffer[9];
			this->sleep_timer_->publish_state(minutes);
		}
	}

	publish_state();
}

void LGControllerComponent::process_capabilities_message(MessageSender sender, const uint8_t* buffer) {
	// Capabilities message. The unit sends this with the other settings so we're now
	// initialized.

	if (sender != MessageSender::Unit) {
		ESP_LOGE(TAG, "ignoring capabilities message not from unit");
		return;
	}

	// Check if we need to update the capabilities message.
	if (this->nvs_storage_.capabilities_message[0] == 0 ||
		std::memcmp(this->nvs_storage_.capabilities_message, buffer, MsgLen - 1) != 0) {

		bool needsRestart = (this->nvs_storage_.capabilities_message[0] == 0);

		ESPPreferenceObject pref = global_preferences->make_preference<NVSStorage>(this->get_object_id_hash() ^ NVS_STORAGE_VERSION);
		memcpy(this->nvs_storage_.capabilities_message, buffer, MsgLen);

		// If no capabilities were stored in NVS before, restart to make sure we get the correct traits for climate
		// No restart just on changes (which is unlikely anyway) to make sure we don't end up in a bootloop for faulty devices / communication
		if (needsRestart) {
			pref.save(&this->nvs_storage_);
			global_preferences->sync();
			ESP_LOGD(TAG, "restarting to apply initial capabilities");
			App.safe_reboot();
		}
		else {
			ESP_LOGD(TAG, "updated device capabilities, manual restart required to take effect");
		}
	}

	this->is_initializing_ = false;
}

void LGControllerComponent::process_type_a_settings_message(MessageSender sender, const uint8_t* buffer) {
	// Send settings the first time we receive a 0xCA message.
	if (sender != MessageSender::Slave) {
		bool first_time = this->last_recv_type_a_settings_[0] == 0;
		memcpy(this->last_recv_type_a_settings_, buffer, MsgLen);
		if (first_time) {
			this->pending_type_a_settings_change_ = true;
		}
	}

	// Handle vane 1 position change
	uint8_t vane1 = buffer[7] & 0x0F;
	if (vane1 <= 6) {
		this->vane_position_[0] = vane1;
		if(this->vane_select_1_ != nullptr)
			this->vane_select_1_->publish_state(*this->vane_select_1_->at(vane1));
	} else {
		ESP_LOGE(TAG, "Unexpected vane 1 position: %u", vane1);
	}

	// Handle vane 2 position change
	uint8_t vane2 = (buffer[7] >> 4) & 0x0F;
	if (vane2 <= 6) {
		this->vane_position_[1] = vane2;
		if(this->vane_select_2_ != nullptr)
			this->vane_select_2_->publish_state(*this->vane_select_2_->at(vane2));
	} else {
		ESP_LOGE(TAG, "Unexpected vane 2 position: %u", vane2);
	}

	// Handle vane 3 position change
	uint8_t vane3 = buffer[8] & 0x0F;
	if (vane3 <= 6) {
		this->vane_position_[2] = vane3;
		if(this->vane_select_3_ != nullptr)
			this->vane_select_3_->publish_state(*this->vane_select_3_->at(vane3));
	} else {
		ESP_LOGE(TAG, "Unexpected vane 3 position: %u", vane3);
	}

	// Handle vane 4 position change
	uint8_t vane4 = (buffer[8] >> 4) & 0x0F;
	if (vane4 <= 6) {
		this->vane_position_[3] = vane4;
		if(this->vane_select_4_ != nullptr)
			this->vane_select_4_->publish_state(*this->vane_select_4_->at(vane4));
	} else {
		ESP_LOGE(TAG, "Unexpected vane 4 position: %u", vane4);
	}

	this->auto_dry_ = buffer[11] & 0x8;
	if(this->auto_dry_entity_ != nullptr)
		this->auto_dry_entity_->publish_state(this->auto_dry_);

	if (sender != MessageSender::Slave) {
		// Handle fan speed 0 (slow) change
		this->fan_speed_[0] = buffer[2];
		if(this->fan_speed_slow_ != nullptr)
			this->fan_speed_slow_->publish_state(this->fan_speed_[0]);

		// Handle fan speed 1 (low) change
		this->fan_speed_[1] = buffer[3];
		if(this->fan_speed_low_ != nullptr)
			this->fan_speed_low_->publish_state(this->fan_speed_[1]);

		// Handle fan speed 2 (medium) change
		this->fan_speed_[2] = buffer[4];
		if(this->fan_speed_medium_ != nullptr)
			this->fan_speed_medium_->publish_state(this->fan_speed_[2]);

		// Handle fan speed 3 (high) change
		this->fan_speed_[3] = buffer[5];
		if(this->fan_speed_high_ != nullptr)
			this->fan_speed_high_->publish_state(this->fan_speed_[3]);
	}
}

void LGControllerComponent::process_type_b_settings_message(MessageSender sender, const uint8_t* buffer) {
	// Ignore this message from other controllers.
	if (sender != MessageSender::Unit) {
		return;
	}

	// Send installer settings the first time we receive a 0xCB message.
	bool first_time = this->last_recv_type_b_settings_[0] == 0;
	memcpy(this->last_recv_type_b_settings_, buffer, MsgLen);
	if (first_time) {
		this->pending_type_b_settings_change_ = true;
	}

	this->last_sent_recv_type_b_millis_ = millis();

	uint8_t overheating = (buffer[2] >> 3) & 0b111;
	if (overheating <= 4) {
		this->overheating_ = overheating;
		if(this->overheating_select_ != nullptr)
			this->overheating_select_->publish_state(*this->overheating_select_->at(overheating));
	} else {
		ESP_LOGE(TAG, "Unexpected overheating value: %u", overheating);
	}

	// Table mapping a byte value to degrees Celsius based on values displayed by PREMTB100.
	// INT8_MIN indicates an invalid value.
	static constexpr int8_t PipeTempTable[] = {
		/* 0x00 */ INT8_MIN, INT8_MIN, INT8_MIN, INT8_MIN, INT8_MIN, INT8_MIN, INT8_MIN,
				   INT8_MIN, INT8_MIN, INT8_MIN, 108, 104, 101, 100, 98, 95,
		/* 0x10 */ 93, 91, 89, 87, 85, 84, 82, 81, 79, 78, 76, 75, 74, 73, 72, 71,
		/* 0x20 */ 70, 68, 68, 67, 66, 65, 64, 63, 62, 61, 60, 60, 59, 58, 57, 57,
		/* 0x30 */ 56, 55, 55, 54, 53, 53, 52, 52, 51, 50, 50, 49, 49, 48, 47, 47,
		/* 0x40 */ 46, 46, 45, 45, 44, 44, 43, 43, 42, 42, 41, 41, 40, 40, 39, 39,
		/* 0x50 */ 39, 38, 38, 37, 37, 36, 36, 36, 35, 35, 34, 34, 33, 33, 33, 32,
		/* 0x60 */ 32, 31, 31, 31, 30, 30, 30, 29, 29, 29, 28, 28, 27, 27, 27, 26,
		/* 0x70 */ 26, 26, 25, 25, 24, 24, 24, 23, 23, 23, 22, 22, 22, 21, 21, 21,
		/* 0x80 */ 20, 20, 20, 19, 19, 19, 18, 18, 18, 17, 17, 17, 16, 16, 16, 15,
		/* 0x90 */ 15, 15, 14, 14, 14, 13, 13, 13, 12, 12, 12, 11, 11, 11, 10, 10,
		/* 0xa0 */ 10, 9, 9, 9, 8, 8, 8, 7, 7, 6, 6, 6, 5, 5, 5, 4,
		/* 0xb0 */ 4, 4, 3, 3, 3, 2, 2, 2, 1, 1, 0, 0, 0, 0, 0, -1,
		/* 0xc0 */ -1, -2, -2, -2, -3, -3, -4, -4, -5, -5, -5, -6, -6, -7, -7, -8,
		/* 0xd0 */ -8, -9, -9, -9, -10, -10, -11, -11, -12, -12, -13, -14, -14, -15, -15, -16,
		/* 0xe0 */ -16, -17, -18, -18, -19, -20, -20, -21, -22, -22, -23, -24, -25, -26, -27,
				   -28,
		/* 0xf0 */ -29, INT8_MIN, INT8_MIN, INT8_MIN, INT8_MIN, INT8_MIN, INT8_MIN, INT8_MIN,
				   INT8_MIN, INT8_MIN, INT8_MIN, INT8_MIN, INT8_MIN, INT8_MIN, INT8_MIN, INT8_MIN
	};
	static_assert(sizeof(PipeTempTable) == 256);
	static_assert(PipeTempTable[UINT8_MAX] == INT8_MIN);

	int8_t pipe_temp_in = PipeTempTable[buffer[3]];
	if(this->pipe_temp_in_ != nullptr) {
		if (pipe_temp_in == INT8_MIN) {
			this->pipe_temp_in_->set_internal(true);
		} else {
			this->pipe_temp_in_->publish_state(pipe_temp_in);
		}
	}

	int8_t pipe_temp_out = PipeTempTable[buffer[4]];
	if(this->pipe_temp_out_ != nullptr) {
		if (pipe_temp_out == INT8_MIN) {
			this->pipe_temp_out_->set_internal(true);
		} else {
			this->pipe_temp_out_->publish_state(pipe_temp_out);
		}
	}

	int8_t pipe_temp_mid = PipeTempTable[buffer[5]];
	if(this->pipe_temp_mid_ != nullptr) {
		if (pipe_temp_mid == INT8_MIN) {
			this->pipe_temp_mid_->set_internal(true);
		} else {
			this->pipe_temp_mid_->publish_state(pipe_temp_mid);
		}
	}
}

void LGControllerComponent::update() {
	ESP_LOGD(TAG, "update");

	bool had_error = false;
	while (this->available() > 0) {
		if (!this->read_byte(&this->recv_buf_[this->recv_buf_len_])) {
			break;
		}
		this->last_recv_millis_ = millis();
		this->recv_buf_len_++;
		if (this->recv_buf_len_ == MsgLen) {
			process_message(this->recv_buf_, &had_error);
			this->recv_buf_len_ = 0;
		}
	}

	// If we did not receive the message we sent last time, try to send it again next time.
	// Ignore this when we're initializing because the unit then immediately responds by
	// sending a lot of messages and this introduces a delay.
	if (this->pending_send_ != PendingSendKind::None && !this->is_initializing_) {
		ESP_LOGE(TAG, "did not receive message we just sent");
		switch (this->pending_send_) {
			case PendingSendKind::Status:
				this->pending_status_change_ = true;
				break;
			case PendingSendKind::TypeA:
				this->pending_type_a_settings_change_ = true;
				break;
			case PendingSendKind::TypeB:
				this->pending_type_b_settings_change_ = true;
				break;
			case PendingSendKind::None:
				ESP_LOGE(TAG, "unreachable");
				break;
		}
		this->pending_send_ = PendingSendKind::None;
		return;
	}

	if (this->recv_buf_len_ > 0) {
		if (millis() - this->last_recv_millis_ > 15 * 1000) {
			ESP_LOGE(TAG, "discarding incomplete data %s",
					 format_hex_pretty(this->recv_buf_, this->recv_buf_len_).c_str());
			this->recv_buf_len_ = 0;
		}
		return;
	}

	if (had_error) {
		return;
	}

	uint32_t millis_now = millis();

	// Handle sleep timer.
	if (this->sleep_timer_target_millis_.has_value()) {
		int32_t diff = int32_t(this->sleep_timer_target_millis_.value() - millis_now);
		if (diff <= 0) {
			ESP_LOGD(TAG, "Turning off for sleep timer");
			this->sleep_timer_target_millis_.reset();
			this->active_reservation_= false;
			this->ignore_sleep_timer_callback_ = true;
			if(this->sleep_timer_ != nullptr)
				this->sleep_timer_->publish_state(0);
			this->ignore_sleep_timer_callback_ = false;
			this->mode = climate::CLIMATE_MODE_OFF;
			this->pending_status_change_ = true;
		} else if (optional<uint32_t> minutes = this->get_sleep_timer_minutes()) {
			if (this->sleep_timer_ != nullptr && this->sleep_timer_->state != *minutes) {
				this->ignore_sleep_timer_callback_ = true;
				this->sleep_timer_->publish_state(*minutes);
				this->ignore_sleep_timer_callback_ = false;
			}
		}
	}

	if (this->slave_ && this->is_initializing_) {
		ESP_LOGD(TAG, "Not sending, waiting for other controller or unit to send first");
		return;
	}

	// Make sure the RX pin is idle for at least 500 ms to avoid collisions on the bus as much
	// as possible. If there is still a collision, we'll likely both start sending at
	// approximately the same time and the message will hopefully be corrupt (and ignored)
	// anyway. Else the pending_send_/send_buf_ mechanism should catch it and we try again.
	//
	// Note: using digital_read is *much* better for this than using serial_ because that
	// interface has significant delays. It has to wait for a full byte to arrive and this
	// takes about 9-10 ms with our slow baud rate. There are also various buffers and
	// timeouts before incoming bytes reach us.
	//
	// 500 ms might be overkill, but the device usually sends the same message twice with a
	// short delay (about 200 ms?) between them so let's not send there either to avoid
	// collisions.
	auto check_can_send = [&]() -> bool {
		while (true) {
			if (this->available() > 0 || !rx_pin_.digital_read()) {
				ESP_LOGD(TAG, "line busy, not sending yet");
				return false;
			}
			if (millis() - millis_now > 500) {
				return true;
			}
			delay(5);
		}
	};

	if (this->pending_type_a_settings_change_) {
		if (check_can_send()) {
			this->send_type_a_settings_message();
		}
		return;
	}
	if (this->pending_type_b_settings_change_) {
		if (check_can_send()) {
			this->send_type_b_settings_message(/* timed = */ false);
		}
		return;
	}
	// Send an AB message every 10 minutes to request pipe temperature values.
	if (!this->slave_ && (millis_now - this->last_sent_recv_type_b_millis_) > 10 * 60 * 1000) {
		if (check_can_send()) {
			this->send_type_b_settings_message(/* timed = */ true);
		}
		return;
	}
	// Send a status message if there is a pending change
	// Additionally, queue a type a message after sending the status message
	// If the swing mode does not include vertical, then the vane will be set to the previous setting
	if (this->pending_status_change_) {
		if (check_can_send()) {
			this->send_status_message();
			this->pending_type_a_settings_change_ = true;
		}
		return;
	}
	// Send a status message every 20 seconds
	// Slave controllers only send this if needed.
	if ((!slave_ && millis_now - this->last_sent_status_millis_ > 20 * 1000)) {
		if (check_can_send()) {
			this->send_status_message();
		}
		return;
	}
}

void LGEntity::set_lg_controller_parent(LGControllerComponent *parent) {
	ESP_LOGD(TAG, "Setting parent for entity id %s", this->entity_id_.c_str());

	this->parent_ = parent;
}

}
}
