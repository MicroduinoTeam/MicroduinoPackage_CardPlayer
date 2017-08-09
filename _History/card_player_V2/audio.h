#include "arduino.h"
#include <EEPROM.h>

#define EEPROM_write(address, p) {int i = 0; byte *pp = (byte*)&(p);for(; i < sizeof(p); i++) EEPROM.write(address+i, pp[i]);}
#define EEPROM_read(address, p)  {int i = 0; byte *pp = (byte*)&(p);for(; i < sizeof(p); i++) pp[i]=EEPROM.read(address+i);}

struct config_type
{
  int EEPROM_music_mode;
  int EEPROM_music_vol;
};

config_type config;      // 定义结构变量config，并定义config的内容

void eeprom_WRITE(int mode, int vol)
{
  config_type config;      // 定义结构变量config，并定义config的内容
  config.EEPROM_music_mode=mode;
  config.EEPROM_music_vol=vol;

  EEPROM_write(0, config);  // 变量config存储到EEPROM，地址0写入
}

config_type eeprom_READ()
{
  config_type config_readback;
  EEPROM_read(0, config_readback);
  return config_readback;
}
