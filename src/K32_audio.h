/*
  K32_audio.h
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/
#ifndef K32_audio_h
#define K32_audio_h

class K32_audio;      // prevent cicular include error
#include "K32.h"

#include "Arduino.h"
#include "K32_log.h"

//https://github.com/earlephilhower/ESP8266Audio
//https://github.com/Gianbacchio/ESP8266_Spiram
#include "AudioGenerator.h"
#include "AudioFileSourceSD.h"
#include "AudioOutputI2S.h"
// #include "AudioFileSourceBuffer.h"

//https://github.com/tommag/PCM51xx_Arduino
#include "PCM51xx.h"


class K32_audio {
  public:
    K32_audio(K32* engine);

    bool isEngineOK();
    bool isSdOK();

    void setGainLimits(int min, int max);
    void volume(int vol);
    void loop(bool doLoop);

    bool play(String filePath, int velocity = 127);
    bool play();
    void stop();

    bool isPlaying();
    String media();
    String error();

    PCM51xx* pcm;

  private:
    SemaphoreHandle_t lock;
    SemaphoreHandle_t runflag;
    static void task( void * parameter );

    K32* engine;

    void applyVolume();
    void initSoundcard();

    AudioOutputI2S *out;
    AudioGenerator* gen;
    AudioFileSourceSD *file;
    // AudioFileSourceBuffer *buff;

    String currentFile = "";
    bool doLoop = false;
    bool sdOK = false;
    bool engineOK = false;
    String errorPlayer = "";
    int _volume = 100;
    int _velocity = 100;

    int gainMin = 150;
    int gainMax = 75;

};

#endif
