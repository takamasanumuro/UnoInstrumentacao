#include <Arduino.h>
#include "main.h"

enum class MeasureMode: uint8_t{
  RAW,
  AVG,
  EXCEL,
  OFF
}measureState;

constexpr uint16_t strainOffset=15;
constexpr uint16_t currentOffset=15;
typedef uint32_t excel;
Pin strainPin=Pin(A0,"Strain",strainOffset);
Pin currentPin=Pin(A5,"Current",currentOffset);
Pin pins[]={strainPin,currentPin};
constexpr uint8_t bufferSize=64;
char bufferSerial[bufferSize];
char bufferAux[bufferSize];
unsigned long measureTime=0;
unsigned long measureInterval=1000;
uint16_t bitNumber=9;
uint32_t digitalResolution=1023;
uint8_t numSamples=64;
uint16_t linha=0; // for EXCEL


void setup() {
  Serial.begin(19200);
  Serial.println("CLEARDATA");
  Serial.println("LABEL,Horario,ADC Ponte,mV Ponte,ADC Corrente,mV Corrente,Linha");
  analogReference(INTERNAL);
  for(auto& Pin:pins) pinMode(Pin.pin,INPUT);
  measureState=MeasureMode::EXCEL;
  digitalResolution=static_cast<uint32_t>(pow(2,bitNumber)-1);
  
}

void loop() {
  //CheckExcelInput();
  SerialCommands();
  Measure();
}

void Measure(){
  static uint32_t sum=0;
  bufferSerial[0]='\0';
  if(millis()-measureTime<measureInterval) return;
    measureTime=millis();
    switch(measureState){
      case MeasureMode::OFF:
        break;  
      case MeasureMode::RAW:
        for(auto& Pin:pins){   
          Pin.analogValue=analogRead(Pin.pin);
          Pin.analogValueVoltage=Pin.analogValue*1100/1023+Pin.pinOffset;
          sprintf(bufferAux,"[%s]--Raw: %lu, mV: %lu ",Pin.name,Pin.analogValue,Pin.analogValueVoltage);
          strlcat(bufferSerial,bufferAux,bufferSize);
        }
        Serial.println(bufferSerial);
        break;
      case MeasureMode::AVG:
        for(auto& Pin:pins){
          for(uint8_t i=0;i<numSamples;i++){
            sum+=analogRead(Pin.pin);
          }
          Pin.analogValue=sum/numSamples; // to check division arithmetics
          Pin.analogValueVoltage=Pin.analogValue*1100/1023+Pin.pinOffset;
          sprintf(bufferAux,"[%s]Average: %lu, mV: %lu ",Pin.name,Pin.analogValue,Pin.analogValueVoltage);
          strlcat(bufferSerial,bufferAux,bufferSize);
          sum=0;
        }
        Serial.println(bufferSerial);
        break;
        
      case MeasureMode::EXCEL:
        linha++;
        for(auto& Pin:pins){
          for(uint8_t i=0;i<numSamples;i++){
            sum+=analogRead(Pin.pin);
          }
          Pin.analogValue=sum/numSamples;
          Pin.analogValueVoltage=Pin.analogValue*1100/1023+Pin.pinOffset;
          sprintf(bufferAux,"[%s]:%lu,%lu ",Pin.name,Pin.analogValue,Pin.analogValueVoltage);
          strlcat(bufferSerial,bufferAux,bufferSize);
          sum=0;
        }
        sprintf(bufferAux,",%u\n",linha); // subtract memory addresses
        strlcat(bufferSerial,bufferAux,bufferSize);
        Serial.println(bufferSerial);
        
        sprintf(bufferSerial,"DATA,TIME");
        for(auto& Pin:pins){
          sprintf(bufferAux,",%lu,%lu",Pin.analogValue,Pin.analogValueVoltage);
          strlcat(bufferSerial,bufferAux,bufferSize);
        }
        sprintf(bufferAux,",%u",linha);
        strlcat(bufferSerial,bufferAux,bufferSize);
        Serial.println(bufferSerial);
        break;
      }
}
  
void CheckExcelInput(){
  static uint32_t excelCheckTimer=millis();
  if(millis()-excelCheckTimer<15000) return;
  excelCheckTimer=millis();
  Serial.println("CELL,GET,J1");
  delay(15);
  measureState=static_cast<MeasureMode>(Serial.read()-'0');
}

void SerialCommands(){
  static char inByte='\0';
  while(Serial.available()){
    inByte=Serial.read();
    switch(inByte){
      case 'J':
        delay(10);
        inByte=Serial.read();
        sprintf(bufferSerial,"Before: %c/%d",inByte,inByte);
        Serial.println(bufferSerial);
        if((inByte<'0') || (inByte>'9')) return;
        sprintf(bufferSerial,"Excel: %d",inByte-'0');
        Serial.println(bufferSerial);
        measureState=static_cast<MeasureMode>(inByte-'0');
        sprintf(bufferSerial,"MeasureMode: %d",static_cast<int>(measureState));
        Serial.println(bufferSerial);
        break;

      case 'o':
        measureState=MeasureMode::OFF;
        Serial.println("OFF");
        break;
      
      case 'd':
        sprintf(bufferSerial,"Digital Resolution: %lu",digitalResolution);
        Serial.println(bufferSerial);
        break;
      case 'b':
        bitNumber==10? bitNumber=8: ++bitNumber;
        digitalResolution=static_cast<uint32_t>(pow(2,bitNumber)); // CHECK WHY 1 IS BEING SUBTRACTED AUTOMATICALLY (seems to be sprintf)
        Serial.print("Res: "); Serial.println(digitalResolution);
        sprintf(bufferSerial,"BitNumber: %d\t Resolution: %lu",bitNumber,digitalResolution);
        Serial.println(bufferSerial);
        break;

      case 'z':
        sprintf(bufferSerial,"MeasureMode: %d",static_cast<int>(measureState));
        Serial.println(bufferSerial);
        break;
      case 'm':
        measureState=MeasureMode::RAW;
        sprintf(bufferSerial,"MeasureMode: RAW");
        Serial.println(bufferSerial);
        break;

      case 'M':
        measureState=MeasureMode::AVG;
        sprintf(bufferSerial,"MeasureMode: AVG");
        Serial.println(bufferSerial);
        break; 
      case 'E':
        measureState=MeasureMode::EXCEL;
        sprintf(bufferSerial,"MeasureMode: EXCEL");
        Serial.println(bufferSerial);
        break;
      case 's':
        sprintf(bufferSerial,"Samples: %d",numSamples);
        Serial.println(bufferSerial);
        break;
      case 'C':
        sprintf(bufferSerial,"CLEARDATA");
        Serial.println(bufferSerial);
        break;
      case 'S':{
        
        uint16_t sum=0;
        Serial.println("debug S");
        for(int i=0; i<Serial.available(); i++){
          Serial.println("debug S2");
          char inByte=Serial.read();
          if(inByte=='\r' || inByte=='\n'){
            numSamples=sum;
            break;
          }
          if(inByte>='0' && inByte<='9'){
            sum=10*sum+inByte-'0';
          }
        }
        break;
        }
      
     

      case 'I':{
        uint16_t sum=0;
        for(int i=0; i<Serial.available(); i++){
          char inByte=Serial.read();
          if(inByte=='\r' || inByte=='\n'){
            measureInterval=sum;
            break;
          }
          if(inByte>='0' && inByte<='9'){
            sum=10*sum+inByte-'0';
            break;
          }
        }
        break;
      }
    }
  }
}

