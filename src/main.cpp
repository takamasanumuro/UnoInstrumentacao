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
uint8_t numSamples=64;
void measure();
void setup() {
  Serial.begin(115200);
  analogReference(INTERNAL);
  pinMode(analogPin,INPUT);
  measureState=MeasureMode::RAW;
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
      sprintf(serialBuffer,"RAW: ¨%i",analogRead(analogPin));
      Serial.println(serialBuffer);
      break;
    case MeasureMode::AVG:
      uint32_t sum=0;
      uint32_t timer=micros();
      for(int i=0;i<numSamples;i++){
        sum+=analogRead(analogPin);
        sprintf(serialBuffer,"Average sum: %lu",sum);
        Serial.println(serialBuffer);
      }
      timer-=micros()*1000;
      sprintf(serialBuffer,"AVG: %lu",sum/numSamples);
      Serial.println(serialBuffer);
      break;
    
    }
  }
  
}