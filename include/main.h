#ifndef MAIN_H
#define MAIN_H


struct Pin{

const uint8_t pin;
uint32_t analogValue;
uint32_t analogValueVoltage;
uint32_t pinOffset;
const char* name;

constexpr Pin(uint8_t pin,const char* name ,uint32_t offset):  pin(pin),
                                                analogValue(0),
                                                analogValueVoltage(0),
                                                pinOffset(offset),
                                                name(name)
                                                {}

};


void Measure();
void SerialCommands();
void CheckExcelInput();
















#endif