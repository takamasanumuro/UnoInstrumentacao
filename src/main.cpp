#include <Arduino.h>
enum class MeasureMode{
  RAW,
  AVG,
  OFF
}measureState;

const byte analogPin=A0;
char serialBuffer[64];
unsigned long measureTime=0;
unsigned long measureInterval=1000;
uint16_t bitNumber=9;
uint32_t digitalResolution=1023;
uint8_t numSamples=64;
uint32_t analogValue;
uint32_t analogValueVoltage;
void measure();
void setup() {
  Serial.begin(115200);
  analogReference(INTERNAL);
  pinMode(analogPin,INPUT);
  measureState=MeasureMode::RAW;
  digitalResolution=static_cast<uint32_t>(pow(2,bitNumber)-1);

}

void loop() {
  measure();

  while(Serial.available()){
    char inByte=Serial.read();
    switch(inByte){
      case 'o':
        measureState=MeasureMode::OFF;
        Serial.println("OFF");
        break;
      case 'd':
        sprintf(serialBuffer,"Digital Resolution: %lu",digitalResolution);
        Serial.println(serialBuffer);
        break;
      case 'b':
        bitNumber==10? bitNumber=8: ++bitNumber;
        digitalResolution=static_cast<uint32_t>(pow(2,bitNumber)); // CHECK WHY 1 IS BEING SUBTRACTED AUTOMATICALLY (seems to be sprintf)
        Serial.print("Res: "); Serial.println(digitalResolution);
        sprintf(serialBuffer,"BitNumber: %d\t Resolution: %lu",bitNumber,digitalResolution);
        Serial.println(serialBuffer);
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
        sprintf(serialBuffer,"MeasureMode: %d",static_cast<int>(measureState));
        Serial.println(serialBuffer);
        measureState=MeasureMode::RAW;
        break;

      case 'M':
        sprintf(serialBuffer,"MeasureMode: %d",static_cast<int>(measureState));
        Serial.println(serialBuffer);
        measureState=MeasureMode::AVG;
        break;     

      case 's':
        sprintf(serialBuffer,"Samples: %d",numSamples);
        Serial.println(serialBuffer);
        break;
 
      case 'S':
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
      
     
/*
      case 'I':
        uint16_t sum2=0;
        for(int i=0; i<Serial.available(); i++){
          char inByte=Serial.read();
          if(inByte=='\r' || inByte=='\n'){
            measureInterval=sum2;
            break;
          }
          if(inByte>='0' && inByte<='9'){
            sum2=10*sum2+inByte-'0';
            break;
          }
        }
        break;
      */
    }
  }

}

void measure(){
  if(millis()-measureTime>measureInterval){
    measureTime=millis();
    switch(measureState){
    case MeasureMode::OFF:
      break;  
    case MeasureMode::RAW:
      analogValue=analogRead(analogPin);
      analogValueVoltage=analogValue*1100/1023;
      sprintf(serialBuffer,"Raw: %lu, mV: %lu",analogValue,analogValueVoltage);
      Serial.println(serialBuffer);
      break;
    case MeasureMode::AVG:
      uint32_t sum=0;
      //uint32_t timer=micros();
      for(int i=0;i<numSamples;i++){
        sum+=analogRead(analogPin);
        //sprintf(serialBuffer,"Average sum: %lu",sum); // For debugging
        //Serial.println(serialBuffer);
      }
      analogValue=sum/numSamples;
      analogValueVoltage=analogValue*1100/1023;
      //timer-=micros()*1000;
      sprintf(serialBuffer,"Average: %lu, mV: %lu",analogValue,analogValueVoltage);
      Serial.println(serialBuffer);
      break;
    
    }
  }
  
}