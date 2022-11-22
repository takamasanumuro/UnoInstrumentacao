#include <Arduino.h>
#include "main.h"
enum class MeasureMode{
  RAW,
  AVG,
  EXCEL,
  OFF
}measureState;

typedef uint32_t excel;
Pin strainPin=Pin(A0,"Strain");
Pin currentPin=Pin(A1,"Current");
Pin pins[]={strainPin,currentPin};
const uint8_t bufferSize=64;
char bufferSerial[bufferSize];
char bufferAux[bufferSize/2];
unsigned long measureTime=0;
unsigned long measureInterval=1000;
uint16_t bitNumber=9;
uint32_t digitalResolution=1023;
uint8_t numSamples=64;
uint32_t analogValue;
uint32_t analogValueVoltage;
uint32_t linha=0; // for EXCEL


void setup() {
  Serial.begin(19200);
  Serial.println("CLEARDATA");
  Serial.println("LABEL,Horario,Valor Codigo,Valor mV,Linha");
  analogReference(INTERNAL);
  for(auto Pin:pins) pinMode(Pin.pin,INPUT);
  measureState=MeasureMode::RAW;
  digitalResolution=static_cast<uint32_t>(pow(2,bitNumber)-1);

}

void loop() {
  Measure();
  SerialCommands();

}

void Measure(){
  if(millis()-measureTime<measureInterval) return;
    measureTime=millis();
    switch(measureState){
    case MeasureMode::OFF:
      break;  
    case MeasureMode::RAW:
      bufferSerial[0]='\0'; 
      for(auto Pin:pins){   
        Pin.analogValue=analogRead(Pin.pin);
        Pin.analogValueVoltage=analogValue*1100/1023;
        sprintf(bufferAux,"%s--Raw: %lu, mV: %lu ",Pin.name,Pin.analogValue,Pin.analogValueVoltage);
        strlcat(bufferSerial,bufferAux,bufferSize);
      }
      Serial.println(bufferSerial);
      
      break;
    
    case MeasureMode::AVG:{
      uint32_t sum=0;
      bufferSerial[0]='\0'; 
      for(auto Pin:pins){
        for(uint8_t i=0;i<numSamples;i++){
          sum+=analogRead(Pin.pin);
        }
        Pin.analogValue=sum/numSamples;
        Pin.analogValueVoltage=analogValue*1100/1023;
        sprintf(bufferAux,"[%s]Average: %lu, mV: %lu ",Pin.name,Pin.analogValue,Pin.analogValueVoltage);
        strlcat(bufferSerial,bufferAux,bufferSize);
      }
      Serial.println(bufferSerial);
      break;
      }
    case MeasureMode::EXCEL:
      linha++;
      uint32_t sum=0;
      bufferSerial[0]='\0'; 
      for(auto Pin:pins){
        for(uint8_t i=0;i<numSamples;i++){
          sum+=analogRead(Pin.pin);
        }
        Pin.analogValue=sum/numSamples;
        Pin.analogValueVoltage=analogValue*1100/1023;
        sprintf(bufferAux,"%s,%lu,%lu,%lu",Pin.name,Pin.analogValue,Pin.analogValueVoltage,linha);
        strlcat(bufferSerial,bufferAux,bufferSize);
      }
      Serial.println(bufferSerial);
      
      bufferSerial[0]='\0';
      bufferAux[0]='\0';
      strlcat(bufferSerial,"DATA,TIME,",bufferSize);
      for(auto Pin:pins){
        sprintf(bufferAux,"%lu,%lu,",Pin.analogValue,Pin.analogValueVoltage);
        strlcat(bufferSerial,bufferAux,bufferSize);
      }
      //sprintf(bufferSerial,"DATA,TIME,%lu,%lu,%lu",analogValue,analogValueVoltage,linha);
      //Serial.println(bufferSerial);

      if(linha>200){
        linha=0;
        sprintf(bufferSerial,"ROW,SET,2");
        Serial.println(bufferSerial);
      }
      break;
    }
  }
  


void SerialCommands(){
  while(Serial.available()){
    char inByte=Serial.read();
    switch(inByte){
      case 'o'||'O':
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
      /*case 'B':
        Serial.print("Bit number 10: "); Serial.println(pow(2,10)-1);
        Serial.print("Bit number 9: "); Serial.println(pow(2,9)-1);
        Serial.print("Bit number 8: "); Serial.println(pow(2,8)-1);
        Serial.print("Bit number 10: "); Serial.println(static_cast<uint32_t>(pow(2,10)-1));
        Serial.print("Bit number 9: "); Serial.println(static_cast<uint32_t>(pow(2,9)-1));
        Serial.print("Bit number 8: "); Serial.println(static_cast<uint32_t>(pow(2,8)-1));
        break;
      */
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