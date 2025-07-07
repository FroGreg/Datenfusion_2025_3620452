#ifndef PUMP_DRIVER_H
#define PUMP_DRIVER_H

struct PumpDriver{
private:
  const unsigned short pin;
  bool on;
public:
  PumpDriver(unsigned short pin) : pin(pin), on(false) {
    // Set the pin under given pin number to strictly output mode
    pinMode(pin, OUTPUT);
    digitalWrite(this->pin, LOW);
  };

  void turn_on() {
    this->on = true;
    digitalWrite(this->pin, HIGH);
  }
  void turn_off() {
    this->on = false;
    digitalWrite(this->pin, LOW);
  }

  bool is_on() const {
    return this->on;
  }
};

#endif