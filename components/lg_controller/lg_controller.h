#pragma once

#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/climate/climate.h"
#include "esphome/components/number/number.h"
#include "esphome/components/select/select.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/switch/switch.h"
#include "esphome/components/uart/uart.h"

#include <map>

namespace esphome {
namespace lg_controller {

static constexpr size_t MIN_TEMP_SETPOINT = 16;
static constexpr size_t MAX_TEMP_SETPOINT = 30;

#define LG_ENTITY_DEFROST "defrost"
#define LG_ENTITY_PREHEAT "preheat"
#define LG_ENTITY_OUTDOOR "outdoor"
#define LG_ENTITY_AUTO_DRY_ACTIVE "auto_dry_active"
#define LG_ENTITY_FAN_SPEED_SLOW "fan_speed_slow"
#define LG_ENTITY_FAN_SPEED_LOW "fan_speed_low"
#define LG_ENTITY_FAN_SPEED_MEDIUM "fan_speed_medium"
#define LG_ENTITY_FAN_SPEED_HIGH "fan_speed_high"
#define LG_ENTITY_SLEEP_TIMER "sleep_timer"
#define LG_ENTITY_VANE_POSITION_1 "vane_position_1"
#define LG_ENTITY_VANE_POSITION_2 "vane_position_2"
#define LG_ENTITY_VANE_POSITION_3 "vane_position_3"
#define LG_ENTITY_VANE_POSITION_4 "vane_position_4"
#define LG_ENTITY_OVERHEATING "overheating"
#define LG_ENTITY_ERROR_CODE "error_code"
#define LG_ENTITY_PIPE_TEMPERATURE_IN "pipe_temperature_in"
#define LG_ENTITY_PIPE_TEMPERATURE_MID "pipe_temperature_mid"
#define LG_ENTITY_PIPE_TEMPERATURE_OUT "pipe_temperature_out"
#define LG_ENTITY_AIR_PURIFIER "air_purifier"
#define LG_ENTITY_INTERNAL_THERMISTOR "internal_thermistor"
#define LG_ENTITY_AUTO_DRY "auto_dry"

class LGEntity;

enum class LgCapability {
	PURIFIER,
	FAN_AUTO,
	FAN_SLOW,
	FAN_LOW,
	FAN_LOW_MEDIUM,
	FAN_MEDIUM,
	FAN_MEDIUM_HIGH,
	FAN_HIGH,
	MODE_HEATING,
	MODE_FAN,
	MODE_AUTO,
	MODE_DEHUMIDIFY,
	HAS_ONE_VANE,
	HAS_TWO_VANES,
	HAS_FOUR_VANES,
	VERTICAL_SWING,
	HORIZONTAL_SWING,
	HAS_ESP_VALUE_SETTING,
	OVERHEATING_SETTING,
	AUTO_DRY,
};

class LGControllerComponent: public esphome::Component,
		public climate::Climate,
		public uart::UARTDevice {
public:
	LGControllerComponent(InternalGPIOPin* rx_pin)
	: rx_pin_(*rx_pin) {
	}
	void setup() override;
	void dump_config() override;
	void set_temperature_sensor(sensor::Sensor *temperature_sensor) {
		this->temperature_sensor_ = temperature_sensor;
	}
	void set_slave(bool slave) {
		this->slave_ = slave;
	}
	void set_fahrenheit(bool fahrenheit) {
		this->fahrenheit_ = fahrenheit;
	}
	float get_setup_priority() const override {
		return esphome::setup_priority::BUS;
	}
	void control(const climate::ClimateCall &call) override;
	climate::ClimateTraits traits() override {
		return this->supported_traits_;
	}
	void set_changed() {
		this->pending_status_change_ = true;
	}
	// Sets position of vane index (1-4) to position (0-6).
	void set_vane_position(int index, int position);
	// Sets installer setting fan speed index (0-3 for slow-high) to value (0-255), with "0" being the factory default
	void set_fan_speed(int index, int value);
	void set_overheating(int value);
	void set_sleep_timer(int minutes);

	void set_purifier(bool value) {
		this->purifier_ = value;
	}
	void set_internal_thermistor(bool value) {
		this->internal_thermistor_ = value;
	}
	void set_auto_dry(bool value) {
		this->auto_dry_ = value;
	}

	void add_entity(std::string entity_id, binary_sensor::BinarySensor *entity);
	void add_entity(std::string entity_id, number::Number *entity);
	void add_entity(std::string entity_id, select::Select *entity);
	void add_entity(std::string entity_id, sensor::Sensor *entity);
	void add_entity(std::string entity_id, switch_::Switch *entity);

	void set_defrost(bool defrost) {
		this->defrost_ = defrost;
	}
protected:
	static constexpr size_t MsgLen = 13;

	climate::ClimateTraits supported_traits_;

    InternalGPIOPin &rx_pin_;
	sensor::Sensor *temperature_sensor_{nullptr};

	select::Select *vane_select_1_{nullptr};
	select::Select *vane_select_2_{nullptr};
	select::Select *vane_select_3_{nullptr};
	select::Select *vane_select_4_{nullptr};
	select::Select *overheating_select_{nullptr};

	number::Number *fan_speed_slow_{nullptr};
	number::Number *fan_speed_low_{nullptr};
	number::Number *fan_speed_medium_{nullptr};
	number::Number *fan_speed_high_{nullptr};

	number::Number *sleep_timer_{nullptr};

	sensor::Sensor *error_code_{nullptr};
	sensor::Sensor *pipe_temp_in_{nullptr};
	sensor::Sensor *pipe_temp_mid_{nullptr};
	sensor::Sensor *pipe_temp_out_{nullptr};
	bool defrost_;
	binary_sensor::BinarySensor *defrost_entity_{nullptr};
	bool preheat_;
	binary_sensor::BinarySensor *preheat_entity_{nullptr};
	bool outdoor_;
	binary_sensor::BinarySensor *outdoor_entity_{nullptr};
	binary_sensor::BinarySensor *auto_dry_active_entity_{nullptr};
	uint32_t last_outdoor_change_millis_ = 0;

	bool purifier_ = false;
	switch_::Switch *purifier_entity_{nullptr};
	bool internal_thermistor_ = true;
	switch_::Switch *internal_thermistor_entity_{nullptr};
	bool auto_dry_ = true;
	switch_::Switch *auto_dry_entity_{nullptr};

	uint8_t recv_buf_[MsgLen] = { };
	uint32_t recv_buf_len_ = 0;
	uint32_t last_recv_millis_ = 0;

	// Last received 0xC8 message.
	uint8_t last_recv_status_[MsgLen] = { };

	// Last received 0xCA message.
	uint8_t last_recv_type_a_settings_[MsgLen] = { };

	// Last received 0xCB message.
	uint8_t last_recv_type_b_settings_[MsgLen] = { };

	uint8_t send_buf_[MsgLen] = { };
	uint32_t last_sent_status_millis_ = 0;
	uint32_t last_sent_recv_type_b_millis_ = 0;

	enum class PendingSendKind : uint8_t {
		None, Status, TypeA, TypeB
	};
	PendingSendKind pending_send_ = PendingSendKind::None;

	bool pending_status_change_ = false;
	bool pending_type_a_settings_change_ = false;
	bool pending_type_b_settings_change_ = false;

	bool is_initializing_ = true;

	uint8_t vane_position_[4] = { 0, 0, 0, 0 };
	uint8_t fan_speed_[4] = { 0, 0, 0, 0 };
	uint8_t overheating_ = 0;

	optional<uint32_t> sleep_timer_target_millis_ { };
	bool active_reservation_ = false;
	bool ignore_sleep_timer_callback_ = false;

	uint32_t NVS_STORAGE_VERSION = 2843654U; // Change version if the NVSStorage struct changes
	struct NVSStorage {
		uint8_t capabilities_message[13] = { };
	};
	NVSStorage nvs_storage_;

	bool fahrenheit_ = false;

	// Whether a message came from the HVAC unit, a master controller, or a slave controller.
	enum class MessageSender : uint8_t {
		Unit, Master, Slave
	};
	// Set if this controller is configured as slave controller.
	bool slave_ = false;

	bool parse_capability(LgCapability capability);
	void configure_capabilities();
	optional<float> get_room_temp();
	optional<uint32_t> get_sleep_timer_minutes();
	static uint8_t calc_checksum(const uint8_t* buffer);
	void set_swing_mode(climate::ClimateSwingMode mode);
	void send_status_message();
	void send_type_a_settings_message();
	void send_type_b_settings_message(bool timed);
	void process_message(const uint8_t* buffer, bool* had_error);
	void process_status_message(MessageSender sender, const uint8_t* buffer, bool* had_error);
	void process_capabilities_message(MessageSender sender, const uint8_t* buffer);
	void process_type_a_settings_message(MessageSender sender, const uint8_t* buffer);
	void process_type_b_settings_message(MessageSender sender, const uint8_t* buffer);
	void update();
};

class LGEntity {
public:
	void set_lg_controller_parent(LGControllerComponent *parent);
	void set_entity_id(std::string entity_id) {
		entity_id_ = entity_id;
	}
protected:
	LGControllerComponent *parent_{nullptr};
	std::string entity_id_;
};

}
}
