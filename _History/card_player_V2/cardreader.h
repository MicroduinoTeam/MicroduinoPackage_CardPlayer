#include <Adafruit_NeoPixel.h>

#define PIN_key A0
#define PIN_led 6

#define paper_val_MIN 100
#define paper_val_MAX 600

Adafruit_NeoPixel strip = Adafruit_NeoPixel(1, PIN_led, NEO_RGB + NEO_KHZ800);

unsigned long time1, time2;
boolean key = HIGH;
boolean key_cache = LOW;
int paper_num = 0;
boolean paper_sta=false;

int LED_vol_sta=0;  //LED呼吸灯反转
int LED_vol=0;    //LED呼吸灯值


// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}

void reader_init()
{
  strip.begin();
  strip.show();
  time2 = millis();
  colorWipe(strip.Color(0, 255, 200), 10);
}

boolean vokey(boolean _vokey)
{
    key = analogRead(PIN_key)<paper_val_MIN;
    switch(_vokey)
    {
      case 0:
       if(!key && key_cache)    //按下松开后
       {
         key_cache=key;    //缓存作判断用
        return true;
       }
       else
       {
         key_cache=key;    //缓存作判断用
         return false;
       }
       break;
     case 1:
       if(key && !key_cache)   //按下松开后
       {
         key_cache=key;    //缓存作判断用
         return true;
       }
       else
       {
         key_cache=key;    //缓存作判断用
         return false;
       }
       break;
    }     
}

int card_read() 
{
  if(vokey(0))    //触发时
  {
    colorWipe(strip.Color(0, 0, 255), 10);
    paper_num++;  //计数
    paper_sta=true; //读取模式
    time1=millis();
  }

  if(millis()-time1>1500 && paper_num!=0) //触发，并且闲置超过一秒，结束扫描
  {
    Serial.println(paper_num);  
    paper_sta=false;  //读取模式结束
    return paper_num;
  }
  if(!paper_sta)    //非读取模式，LED呼吸
  {
    if(LED_vol>=247)
    {
      LED_vol_sta=1;
    }
    if(LED_vol<=0) 
    {
      LED_vol_sta=0;
    }

    if(!LED_vol_sta)
    {
      LED_vol+=8;
    }
    else
    {
      LED_vol-=8;
    }
    colorWipe(strip.Color(LED_vol, 0, 0), 10);    //呼吸灯显示亮度
  }
  return 0;
}


