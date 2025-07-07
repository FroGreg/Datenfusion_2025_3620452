#ifndef ANALOG_SENSORS_H
#define ANALOG_SENSORS_H

#include "states.h"

/**
* @brief Abstract base class for analog based sensors.
*/
struct AnalogSensor{
  const unsigned short PIN_NUMBER;

  AnalogSensor(unsigned short pin) : PIN_NUMBER(pin) {};
  /**
  * @brief Reads the value from the sensor and returns it as is.
  * @returns unsigned int Ranging from 0 to 1023.
  */
  virtual unsigned int read_raw() const final {
    return analogRead(PIN_NUMBER);
  }
  /**
  * @brief Reads the value from the sensor and maps it to a min...max range.
  * @param min  The lower bound of the mapping.
  * @param max The upper bound of the mapping.
  * @returns int Ranging from min to max.
  */
  virtual unsigned int read_mapped_int(int min, int max) const final{
    return map(this->read_raw(), 0, 1023, min, max);
  }
  /**
  * @brief Reads the value from the sensor and maps it to a min...max range.
  * @param min The lower bound of the mapping.
  * @param max The upper bound of the mapping.
  * @returns float Ranging from min to max.
  */
  virtual float read_mapped_float(float min, float max) const final {
    float raw = static_cast<float>(this->read_raw());
    return (raw / 1023.0f) * (max - min) + min;
  }
  /**
  * @brief Reads the value from the sensor and maps it to a 0...100 percentage range.
  * @returns unsigned int ranging from 0% to 100%.
  */
  virtual unsigned int read_percent() const final {
    return this->read_mapped_int(0, 100);
  }
  virtual SensorStateLevel get_state() const = 0;
  /**
  * @brief Default implementation, prints just the sensors raw value.
  */
  virtual void SerialPrint() const {
    Serial.print("#### Generic Analog Sensor reading ####\n");
    Serial.print("Raw value: ");
    Serial.print(this->read_raw());
    Serial.print("\n");
    Serial.print("#### Reading end ####\n");
  }
  /**
  * @brief Transforms a SensorStateLevel value to its' string representation.
  */
  static const char* state_to_str(SensorStateLevel state) {
    switch (state) {
      case SensorStateLevel::TOO_LOW: return "TOO_LOW";
      case SensorStateLevel::DANGER_LOW: return "DANGER_LOW";
      case SensorStateLevel::OK: return "OK";
      case SensorStateLevel::DANGER_HIGH: return "DANGER_HIGH";
      case SensorStateLevel::TOO_HIGH: return "TOO_HIGH";
      case SensorStateLevel::INVALID_STATE: return "INVALID_STATE";
    }
  }
};

/**
* @brief Concrete implementation for capacitive soil moisture sensors.
* @note Uses full range of values of SensorStateLevel
*/
struct SoilMoistureSensor : public AnalogSensor {
  // Soaking wet soil, or stagnant water.
  static constexpr unsigned int THRESH_TOO_WET           = 350;
  // Soil is well saturated with water.
  static constexpr unsigned int THRESH_DANGEROUSLY_WET   = 450;
  // Perfect soil-water saturation.
  static constexpr unsigned int THRESH_OK                = 550;
  // Damp soil, on the verge of drying out.
  static constexpr unsigned int THRESH_DANGEROUSLY_DRY   = 650;
  // Bone dry soil.
  static constexpr unsigned int THRESH_TOO_DRY           = 1023;

  SoilMoistureSensor(unsigned short pin) : AnalogSensor(pin) {}

  /** 
  * @brief Reads the value from the sensor and assigns it its' corresponding state.
  * @returns SensorStateLevel Full range of SensorStateLevel, TOO_LOW to TOO_HIGH.
  */
  SensorStateLevel get_state() const override {
    unsigned int value = read_raw();

    if (value <= THRESH_TOO_WET)          return SensorStateLevel::TOO_HIGH;
    if (value <= THRESH_DANGEROUSLY_WET)  return SensorStateLevel::DANGER_HIGH;
    if (value <= THRESH_OK)               return SensorStateLevel::OK;
    if (value <= THRESH_DANGEROUSLY_DRY)  return SensorStateLevel::DANGER_LOW;
    if (value <= THRESH_TOO_DRY)          return SensorStateLevel::TOO_LOW;

    return SensorStateLevel::INVALID_STATE;
  }
  /**
  * @brief Prints the soil moisture sensors current metrics.
  */
  void SerialPrint() const override {
    int raw = this->read_raw();
    SensorStateLevel state = this->get_state();
    const char* state_str = AnalogSensor::state_to_str(state);
    Serial.print("Soil Moisture\n");
    Serial.print("Raw sensor value: ");
    Serial.print(raw);
    Serial.print("\n");
    Serial.print("State: ");
    Serial.print(state_str);
    Serial.print("\n");
  }
};

/**
* @brief Concrete implementation for BNC PH Sensor + PH sensor module.
* @note Uses full range of values of SensorStateLevel
*/
struct PHSensor : public AnalogSensor {
private:
  // Values taken from my own experience with commonly found plants.
  static constexpr float PH_TOO_LOW      = 5.8f;
  static constexpr float PH_DANGER_LOW  = 6.1f;
  static constexpr float PH_OK          = 7.0f;
  static constexpr float PH_DANGER_HIGH = 7.5f;
  static constexpr float PH_TOO_HIGH    = 14.0f;
public:
  PHSensor(unsigned short pin) : AnalogSensor(pin) {}
  /** 
  * @brief Reads the value from the sensor and assigns it its' corresponding state.
  * @returns SensorStateLevel Full range of SensorStateLevel, TOO_LOW to TOO_HIGH.
  */
  SensorStateLevel get_state() const override {
    // The sensor board of the PH probe handles the logarithmic aspect of the reading. The value only needs to be mapped from 0.0f to 14.0f.
    float value = read_mapped_float(0.0f, 14.0f);
    if (value <= PH_TOO_LOW)      return SensorStateLevel::TOO_LOW;
    if (value <= PH_DANGER_LOW)   return SensorStateLevel::DANGER_LOW;
    if (value <= PH_OK)           return SensorStateLevel::OK;
    if (value <= PH_DANGER_HIGH)  return SensorStateLevel::DANGER_HIGH;
    if (value <= PH_TOO_HIGH)     return SensorStateLevel::TOO_HIGH;
    return SensorStateLevel::INVALID_STATE;
  }
  /**
  * @brief Prints the PH sensors current metrics.
  */
  void SerialPrint() const override {
    int raw = this->read_raw();
    SensorStateLevel state = this->get_state();
    const char* state_str = AnalogSensor::state_to_str(state);
    Serial.print("PH\n");
    Serial.print("Raw sensor value: ");
    Serial.print(raw);
    Serial.print("\n");
    Serial.print("State: ");
    Serial.print(state_str);
    Serial.print("\n");
  }
};


/**
* @brief Concrete implementation for capacitive water level sensor.
* @note Uses limited range of values of SensorStateLevel
*/
struct WaterLevelSensor : public AnalogSensor {
private:
  // There is enough water still in the reservoir.
  static constexpr unsigned int THRESH_OK = 1024;
  // The water in the reservoir is running low.
  static constexpr unsigned int THRESH_DANGER_LOW = 450;
  // There is no more, or barely any at all water left in the reservoir.
  static constexpr unsigned int THRESH_DRY = 200;
public:
  WaterLevelSensor(unsigned int pin) : AnalogSensor(pin) {}
  /** 
  * @brief Reads the value from the sensor and assigns it its' corresponding state.
  * @returns SensorStateLevel Limited range of SensorStateLevel, TOO_LOW, DANGER_LOW and OK only.
  */
  SensorStateLevel get_state() const override {
    int val = this->read_raw();
    if(val <= THRESH_DRY) return SensorStateLevel::TOO_LOW;
    if(val <= THRESH_DANGER_LOW) return SensorStateLevel::DANGER_LOW;
    if(val <= THRESH_OK) return SensorStateLevel::OK;
    return SensorStateLevel::INVALID_STATE;
  }
  /**
  * @brief Prints the water level sensors current metrics.
  */
  void SerialPrint() const override {
    int raw = this->read_raw();
    SensorStateLevel state = this->get_state();
    const char* state_str = AnalogSensor::state_to_str(state);
    Serial.print("Water level\n");
    Serial.print("Raw sensor value: ");
    Serial.print(raw);
    Serial.print("\n");
    Serial.print("State: ");
    Serial.print(state_str);
    Serial.print("\n");
  }
};

/**
* @brief Concrete implementation for capacitive water detection sensor.
* @note Uses limited range of values of SensorStateLevel
*/
struct WaterDetectionSensor : public AnalogSensor {
private:
  // Water is detected
  static constexpr unsigned int THRESH_ON = 1024;
  // No water detected
  static constexpr unsigned int THRESH_OFF = 50;
public:
  WaterDetectionSensor(unsigned int pin) : AnalogSensor(pin) {}
  /** 
  * @brief Reads the value from the sensor and assigns it its' corresponding state.
  * @returns SensorStateLevel Limited range of SensorStateLevel, TOO_HIGH and OK only.
  */
  SensorStateLevel get_state() const override {
    int val = this->read_raw();
    if(val <= THRESH_OFF) return SensorStateLevel::OK;
    if(val <= THRESH_ON) return SensorStateLevel::TOO_HIGH;
    return SensorStateLevel::INVALID_STATE;
  }
  /**
  * @brief Prints the water detection sensors current metrics.
  */
  void SerialPrint() const override {
    int raw = this->read_raw();
    SensorStateLevel state = this->get_state();
    const char* state_str = AnalogSensor::state_to_str(state);
    Serial.print("Water detection\n");
    Serial.print("Raw sensor value: ");
    Serial.print(raw);
    Serial.print("\n");
    Serial.print("State: ");
    Serial.print(state_str);
    Serial.print("\n");
  }
};

#endif