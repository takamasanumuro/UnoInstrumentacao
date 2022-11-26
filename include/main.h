#ifndef MAIN_H
#define MAIN_H


struct Pin{

const uint8_t pin;
uint32_t analogValue;
uint32_t analogValueVoltage;
const char* name;

Pin(uint8_t pin,const char* name): pin(pin),analogValue(0),analogValueVoltage(0),name(name){}

};


void Measure();
void SerialCommands();
void printPin(Pin& pin);















#endif