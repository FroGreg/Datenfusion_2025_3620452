#ifndef ACTION_DECIDER_H
#define ACTION_DECIDER_H

#include "analog_sensors.h"
#include "pump_driver.h"

/**
* @brief Contains all logic for determining when to run the pump
* @note Pins A4 and A5 are reserved for possible I2C sensors.
*/
class ActionDecider{
private:
  static constexpr unsigned int SM_PIN = A1;
  SoilMoistureSensor sm;
  // PH sensor currently faulty (reads a constant PH value of around 8.6 without regard of the actual PH value of the water, tested with copious amounts of citric acid...), use a hardcoded OK instead...
//  static constexpr unsigned int PH_PIN = A2;
//  PHSensor ph;
  static constexpr unsigned int WL_PIN = A3;
  WaterLevelSensor wl;
  static constexpr unsigned int WD_PIN = A6;
  WaterDetectionSensor wd;


  static constexpr unsigned int PD_PIN = 2;
  PumpDriver pd;
public:
  ActionDecider()
  : sm(SM_PIN),
//  ph(PH_PIN),
    wl(WL_PIN),
    wd(WD_PIN),
    pd(PD_PIN)
    {

    }
  void PrintAll() const{
    sm.SerialPrint();
    // PH sensor is faulty...
//  ph.SerialPrint();
    wl.SerialPrint();
    wd.SerialPrint();  
    Serial.print("\n\nPump is: ");
    this->pd.is_on() ? Serial.print(" On") : Serial.print("Off");
    Serial.print("\n\n\n");
  }

  /**
  * The fuzzy rules are as follows:
  * Turn the pump off if:
  *   - Any sensor reports an invalid state. *all sensors INVALID_STATE
  *   - There is no water detected in the tank anymore. *wl sensor TOO_LOW.
  *   - All other cases.
  * Turn the pump on, if
  *   - PH ranges from DANGER_LOW to DANGER_HIGH, inclusive. *ph sensor DANGER_LOW - DANGER_HIGH
  *   - There is no water detected at the bottom of the pot. *wd sensor OK
  *   - The water level is OK or higher. *wl sensor OK - TOO_HIGH
  *   - The soil moisture is not soaking wet (TOO_HIGH = soaking wet). *sm sensor TOO_LOW - DANGER_HIGH
  * Special cases:
  *   Turn pump on if:
  *     - PH is in range. *ph sensor DANGER_LOW - PH_DANGER_HIGH
  *     - Water is detected at the bottom. *wd OK/TOO_HIGH
  *     - The soil moisture at the top is bone dry. sm *TOO_LOW
  * @returns bool Whether the pump shall be turned on (true) or off (false).
  */
  bool DecideAction(){
    SensorStateLevel sm_state = this->sm.get_state();
    // Since the ph sensor is faulty and only displays one value, irregardless of the actual ph value of the water (tested by adding massive amounts of citric acid into the testing solution, without any change to the read value), set it to be always OK.
    SensorStateLevel ph_state = SensorStateLevel::OK;
    SensorStateLevel wl_state = this->wl.get_state();
    SensorStateLevel wd_state = this->wd.get_state();

    // Step 1
    // Any of the sensors reports an invalid state.
    if( 
        sm_state == SensorStateLevel::INVALID_STATE ||
        ph_state == SensorStateLevel::INVALID_STATE ||
        wl_state == SensorStateLevel::INVALID_STATE ||
        wd_state == SensorStateLevel::INVALID_STATE  
      ) {
        return false;
      }
    // There is no more water in the tank
    if(wl_state == SensorStateLevel::TOO_LOW){
      return false;
    }

    // Step 2
    // Handle PH out of range first.
    // PH has to be in the range of DANGER_LOW - DANGER_HIGH. Or rather, not TOO_LOW OR TOO_HIGH.
    if(ph_state == SensorStateLevel::TOO_LOW || ph_state == SensorStateLevel::TOO_HIGH){
      return false;
    }

    // Step 3
    // Standard case
    // PH range already implicitly checked in Step 2
    if(
      wd_state == SensorStateLevel::OK &&
      ((int)wl_state >= (int)SensorStateLevel::OK && (int)wl_state <= (int)SensorStateLevel::TOO_HIGH) && // Check if the water level is in range OK - TOO_HIGH
      ((int)sm_state <= (int)SensorStateLevel::DANGER_HIGH) // Check if soil moisture is in range TOO_LOW - DANGER_HIGH. Or rather, if not TOO_HIGH.
    ){
      return true;
    }

    // Step 4
    // Special cases
    // Case 1
    // As in Step 3, the PH range has already been implicitly validated prior, in Step 2.
    if(
      (wd_state == SensorStateLevel::OK || wd_state == SensorStateLevel::TOO_HIGH) &&
      (sm_state == SensorStateLevel::TOO_LOW)
    ){
      return true;
    }

    // All other cases, turn pump off.
    return false;
  }

  /**
  * @brief Calls DecideAction, and turns the pump on or off, depending on the decision result.
  * @returns Whether the pump was turned on or off.
  */
  bool DecidePump() {
    if(this->DecideAction()){
       this->pd.turn_on();
       return true;
    }
    else this->pd.turn_off();
    return false;
  }
  /**
  * @brief Allow the users of the class to manually turn off the pump.
  */
  void TurnOffPump(){
    this->pd.turn_off();
  }
};

#endif