/*
   Microduino_开源条码刷卡音乐播放器示例程序
   套件WIKI地址:https://wiki.microduino.cn/index.php/%E5%BC%80%E6%BA%90%E6%9D%A1%E7%A0%81%E5%88%B7%E5%8D%A1%E9%9F%B3%E4%B9%90%E6%92%AD%E6%94%BE%E5%99%A8/zh
   使用Microduino_AudioPro、Microduino_SD模块、Sensor Joystick（摇杆）/Sensor-Line Finder（灰度）传感器、OLED屏等，详细清单见维基界面
   本例程适配新版 Arduino IDE 1.6.9 for Microduino

   *注意：请在开机前在A0口接入Sensor Joystick（摇杆）传感器或Sensor-Line Finder（灰度）传感器
          如果接入了Sensor Joystick传感器，可用摇杆控制音乐播放，
          如果接入了灰度传感器，可通过刷条码卡来切歌
   
   更新时间：2017-08-04
*/

#include <Microduino_Key.h>      //按键库
#include <Microduino_AudioPro.h> //AudioPro库
#include <SD.h>                  //SD库
#include "Ui.h"                  //OLED显示库文件

#define PIN_LINEA A0     //灰度传感器所用端口号
#define DOCK_MIN  90     //灰度传感器检测的触发值。

AudioPro_FilePlayer musicPlayer =  AudioPro_FilePlayer(SD);  //AudioPro_FilePlayer实例化

AnalogKey keyAnalog[5] {(A0), (A0), (A0), (A0), (A0)};   //摇杆传感器引用按键库里的AnalogKey类
AnalogKey keyLine(PIN_LINEA); //灰度传感器引用按键库里的AnalogKey类

enum KeyName {
  UP, DOWN, LEFT, RIGHT, PRESS
};

bool music_status;              //定义音乐播放状态变量（暂停/播放）
int musicVol;                   //定义用户设置音量值变量（音量从小到大：0~20）
int setVol;                     //定义实际设置音量值变量（音量从小到大：127~0，单位-dB）

uint8_t musicNum = 1; //歌曲序号
uint8_t fileNum = 0;  //文件数量
String musicName;     //音乐名（英文，若为中文会识别成错误字符）

uint32_t cardTimer = 0;
uint8_t cardNum = 0; 
uint32_t oledTimer = millis();
bool controlMode;   //音乐播放的控制模式，若为1，摇杆控制音乐播放，若为0，刷条码卡来切歌

//**********播放第num号音乐
void playNum(uint8_t num) {
  if (!musicPlayer.paused() || !musicPlayer.stopped()) {
    musicPlayer.stopPlaying();  //必要，否则SD类得不到关闭，内存溢出
  }
  musicName = musicPlayer.getMusicName(num);
  Serial.print(F("Playing:"));
  if (!musicPlayer.playMP3(musicName)) {
    Serial.println(F("ERROR"));
  }
  else {
    Serial.print(F("OK \t File: "));
    Serial.println(musicName);
  }
}

//判断条形码
void getCard() {
  if (keyLine.readEvent(DOCK_MIN, 1023) == SHORT_PRESS) {
    oledTimer = millis();
    cardNum++;
    cardTimer = millis();
  }
}

void setup() {
  //**********串口初始化
  Serial.begin(115200);

  //***通过A0口接入的传感器判断控制模式
  pinMode(A0, INPUT);
  Serial.print(F("A0："));
  Serial.println(analogRead(A0));
  if (analogRead(A0) > 1020) {
    controlMode = 1;
  }
  else {
    controlMode = 0;
  }

  //****如果A0接入了摇杆
  if (controlMode) {
    for (uint8_t a = 0; a < 5; a++) {
      keyAnalog[a].begin(INPUT);//摇杆初始化
    }
  }
  //***如果A0接入了灰度
  else {
    keyLine.begin(INPUT);//灰度传感器初始化
  }

  //**********AudioPro初始化
  pinMode(SD_PIN_SEL, OUTPUT);
  digitalWrite(SD_PIN_SEL, HIGH);
  delay(500);
  if (! musicPlayer.begin()) { // initialise the music player
    Serial.println(F("Couldn't find VS1053, do you have the right pins defined?"));
    while (1);
  }
  Serial.println(F("VS1053 found"));

  //**********SD初始化
  if (!SD.begin(SD_PIN_SEL)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");

  //**********初始音量设置
  musicVol = 16;   //设置初始音量
  setVol = map(musicVol, 0, 20, 127, 0); //将用户设置的音量映射到实际设置的音量范围
  musicPlayer.setVolume(setVol, setVol); //设置音量
  Serial.print("Volume:");   //串口打印信息
  Serial.println(musicVol);  //串口打印用户设置的音量值

  //**********获取音乐文件信息，并通过串口打印出来
  Serial.println(F("Enter Index of File to play"));
  fileNum = musicPlayer.getMusicNum();  //可以获取SD卡中曲目列表以及数量
  Serial.print(F("Music Files : "));
  Serial.println(fileNum);
  for (uint8_t a = 0; a < fileNum; a++) {
    Serial.print("\t File[");
    Serial.print(a);
    Serial.print("]: ");
    Serial.println(musicPlayer.getMusicName(a));
  }
  musicName = "pressToStart!";
  delay(200);
}

void loop() {
  //**如果A0接入摇杆
  if (controlMode) {
    //**********摇杆向上拨动，音量增大
    switch (keyAnalog[UP].readEvent(700 - 50, 700 + 50)) {
      case SHORT_PRESS:  { //短按，每按一次音量加1
          musicVol++;
          if (musicVol > 20) {
            musicVol = 20;
          }
          setVol = map(musicVol, 0, 20, 127, 1);
          musicPlayer.setVolume(setVol, setVol);
          Serial.print(F("Volume changed to:"));
          Serial.print(musicVol);
          Serial.println(F("[dB]"));
        }
        break;
      case LONG_PRESS: { //长按，音量不断增大
          musicVol++;
          if (musicVol > 20) {
            musicVol = 20;
          }
          setVol = map(musicVol, 0, 20, 127, 1);
          musicPlayer.setVolume(setVol, setVol);
          Serial.print(F("Volume changed to:"));
          Serial.print(musicVol);
          Serial.println(F("[dB]"));
          delay(100);
          break;
        }
    }
    //**********摇杆向下拨动，音量减小
    switch (keyAnalog[DOWN].readEvent(330 - 50, 330 + 50)) {
      case SHORT_PRESS: { //短按，每按一次音量减1
          musicVol--;
          if (musicVol < 1) {
            musicVol = 0;
          }
          setVol = map(musicVol, 0, 20, 127, 1);
          musicPlayer.setVolume(setVol, setVol);
          Serial.print(F("Volume changed to:"));
          Serial.print(musicVol);
          Serial.println(F("[dB]"));
        }
        break;
      case LONG_PRESS:  //长按，音量不断减小
        musicVol--;
        if (musicVol < 1) {
          musicVol = 0;
        }
        setVol = map(musicVol, 0, 20, 127, 1);
        musicPlayer.setVolume(setVol, setVol);
        Serial.print(F("Volume changed to:"));
        Serial.print(musicVol);
        Serial.println(F("[dB]"));

        delay(100);
        break;
    }
    //**********摇杆向左拨动，播放上一曲
    switch (keyAnalog[LEFT].readEvent(512 - 50, 512 + 50)) {
      case SHORT_PRESS:
        musicNum--;
        if (musicNum < 1 ) {
          musicNum = fileNum;
        }
        playNum(musicNum - 1);
        break;
    }
    //**********摇杆向右拨动，播放下一曲
    switch (keyAnalog[RIGHT].readEvent(860 - 50, 860 + 50)) {
      case SHORT_PRESS:
        musicNum++;
        if (musicNum > fileNum) {
          musicNum = 1;
        }
        playNum(musicNum - 1);
        break;
    }
    //**********摇杆按下，切换暂停或播放状态
    switch (keyAnalog[PRESS].readEvent(0, 50)) {
      case SHORT_PRESS:
        if (musicPlayer.stopped()) {
          Serial.println(F("Playing!"));
          playNum(musicNum - 1);
        }
        else if (! musicPlayer.paused()) {
          Serial.println("Paused");
          musicPlayer.setAmplifier(false);  //关闭运放
          musicPlayer.pausePlaying(true);   //暂停
        } else {
          Serial.println("Resumed");
          musicPlayer.setAmplifier(true);   //开启运放
          musicPlayer.pausePlaying(false);  //取消暂停
        }
        break;
      case LONG_PRESS:
        Serial.println(F("Stopping"));
        musicPlayer.stopPlaying();
        delay(500);
        break;
    }
  }
  //***如果A0接入了灰度
  else {
    getCard();
    if (millis() - cardTimer > 2000 && cardTimer > 0) //2秒钟内没有检测到新的条纹，即认为刷卡结束。
    {
      musicNum = constrain(cardNum, 1, fileNum); //根据扫描到的黑色条纹数播放相应的音乐。
      Serial.print(F("musicNum: "));
      Serial.println(musicNum);
      cardNum = 0;
      cardTimer = 0;
      playNum(musicNum - 1);
    }
  }

  //**OLED屏显示音乐播放信息
  if (millis() - oledTimer > 800 ) { //800ms执行一次
    music_status = (!musicPlayer.paused() && !musicPlayer.stopped());
    UI(musicNum, fileNum, musicPlayer.getMonoMode(), musicVol,
       music_status, musicName, musicPlayer.decodeTime());
    oledTimer = millis();
  }

  delay(2); //延时2ms
}


