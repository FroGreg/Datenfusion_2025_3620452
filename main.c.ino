#include "action_decider.h"

// Initiate the setup of all sensors inside ActionDecider class.
ActionDecider ad;

void setup() {
  // Establish Serial communication.
  Serial.begin(9600);
  delay(500);
}

// Shorter delays for demonstration purposes, change to real-world values for real-workd use.
const int pump_on_time = 1000 * 10; // 1000 * 30;
const int after_water_delay = 1000 * 10;// 1000 * 60 * 10;
const int pump_off_delay = 1000 * 5; // 1000 * 60;
/**
* Pump toggling rules:
* If the pump was turned on => Keep it running for half minute => Turn it off => Initiate a 10 minute delay to let the newly fed water stabilize inside the pot before the next decision.
* If the pump was not turned on => Initiate a 1 Minute delay before the next decision.
*/
void loop() {
  bool pump = ad.DecidePump();
  // For Debug/Demonstration purposes.
  ad.PrintAll();

  if(pump){
    Serial.println("Pump turning on");
    delay(pump_on_time);
    Serial.println("Pump turning off");
    ad.TurnOffPump();
    Serial.println("Initiating after-watering delay");
    delay(after_water_delay);
  }
  else{
    Serial.println("Pump staying off");
    Serial.println("Initiating pump-off delay");
    delay(pump_off_delay);
  }
  Serial.println("Ready for next decision\n\n\n\n\n");
}
