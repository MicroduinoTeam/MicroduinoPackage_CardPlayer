#include "arduino.h"

int vol,volCache;
unsigned long time_cache=0;

int keyRead()
{
  if(analogRead(A6)<10)
  {
    delay(50);
    if(analogRead(A6)<10)
      return 1;
  }
  if(analogRead(A6)>200 && analogRead(A6)<300)
  {
    delay(50);
    if(analogRead(A6)>200 && analogRead(A6)<300)
      return 2;
  }
  if(analogRead(A6)>300 && analogRead(A6)<400)
  {
    delay(50);
    if(analogRead(A6)>300 && analogRead(A6)<400)
      return 3;
  }
  return 0;
}

int keyGet()
{
   int key = 0;
   vol = keyRead();  //检测输入动作
   if(vol==0){
      if(volCache == 1){
          key = 1;
      }else if(volCache > 1){
          key = volCache + 3; 
      }else{
          key = 0; 
      } 
      time_cache=millis();
  }
  volCache = vol;
  if(millis() > time_cache + 1000){
      key = vol + 1;
      volCache = 0;
  }  
  return key;
}


