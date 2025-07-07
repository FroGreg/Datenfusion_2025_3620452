#ifndef STATES_H
#define STATES_H

/**
* @brief Represents the state of the environmental sensors reading in human readable terms.
*/
enum SensorStateLevel{
  TOO_LOW = 0,
  DANGER_LOW = 1,
  OK = 2,
  DANGER_HIGH = 3,
  TOO_HIGH = 4,
  INVALID_STATE = INT32_MAX
};


#endif