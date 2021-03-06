#include "JQ6500.h"
#include "JQ6500_def.h"
#include "U8glib.h"
#include <SoftwareSerial.h>
#include <Adafruit_NeoPixel.h>
#include <EEPROM.h>
//用户自定义部分------------------------
#include "key.h"
#include "Ui.h"
#include "audio.h"
#include "cardreader.h"

#define DRAW_LOOP 100	//主界面刷新时间

SoftwareSerial mySerial(2, 3); // RX, TX
JQ6500 AUDIO(&mySerial);

int musicVol=20;               //音量0~30
int musicMode = MODE_ALL;
boolean music_status = false;  //歌曲播放状态
int fileNum = 0;
int fileNumCache = 0;
int totalNum = 0;
int totalTime = 0;
int playTime = 0;

String nameCache = "";

void setup() 
{
  Serial.begin(9600);
  reader_init();

  config = eeprom_READ();
  musicMode = config.EEPROM_music_mode;
  musicVol = config.EEPROM_music_vol;
  AUDIO.init(DEVICE_TF, musicMode, musicVol);
 
  delay(1000);
  totalNum = AUDIO.queryTF();
  totalNum = AUDIO.queryTF();
  totalNum = AUDIO.queryTF();
  AUDIO.choose(1);
  AUDIO.pause();
}

void loop() 
{ 
  int key = keyGet();
  
  if(key == 1){
     music_status=!music_status;  //播放或暂停
     music_status? AUDIO.play():AUDIO.pause();
     
  }else if(key == 2){
     musicMode++;
     if(musicMode > 4)
        musicMode = 0;
     AUDIO.setMode(musicMode);
     eeprom_WRITE(musicMode, musicVol);
     
  }else if(key == 3||key == 4){
     (key-3)? musicVol++ : musicVol--;
     if(musicVol>30)
         musicVol=30;
     else if(musicVol<1)
         musicVol=1;
     AUDIO.volumn(musicVol);
     eeprom_WRITE(musicMode, musicVol);
     
  }else if(key == 5||key == 6){
     (key-5)? AUDIO.next() : AUDIO.prev(); 
  }

  if(card_read()> 0)
  {
      AUDIO.choose(paper_num);  
      paper_num = 0;
  } 

  if(!paper_sta)
  {
  fileNum = AUDIO.queryTFFile();  
  if(fileNum != fileNumCache){
      fileNumCache = fileNum;
      nameCache = AUDIO.queryName(); 
      totalTime = AUDIO.queryTotalTime(); 
  }
  playTime = AUDIO.queryPlayTime();   
  UI(fileNum,totalNum,musicMode,musicVol,music_status,nameCache,playTime,totalTime);
  }
////  delay(DRAW_LOOP);
}


