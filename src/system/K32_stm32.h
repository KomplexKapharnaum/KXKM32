/*
  K32_stm32.h
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/
#ifndef K32_stm32_h
#define K32_stm32_h

#define STM32_CHECK 200           // task loop in ms
#define STM32_CHECK_BATT 5000     // check battery in ms

#include "Arduino.h"
#include "system/K32_log.h"
#include "K32_stm32_api.h"
#include "esp_task_wdt.h"
#include <WiFi.h>

class K32_stm32 {
  public:
    K32_stm32(bool startListening = true);

    void listen();      // Start monitoring thread
    void listen(bool btn, bool battery);      // Start monitoring thread (configurable)
    void stopListening(); // Stop monitoring thread

    void leds(uint8_t *values);      // Set Leds
    void gauge(int percent);          // Set led gauge %
    void blink(uint8_t *values, int duration_ms);  //Blink led value for a fixed duration
    void custom(int Ulow, int U1, int U2, int U3, int U4, int U5, int Umax); // set CUSTOM progile
    int firmware();

    int current();      // Get Load current (mA)
    int battery();      // Get Battery %
    int voltage();      // Get Battery %
    bool clicked();     // Get Btn Click
    bool dblclicked();  // Get Btn DblClick

    void wait();        // Wait for stm32
    void reset();       // Reset board
    void shutdown();    // Shutdown board

    void send(K32_stm32_api::CommandType cmd, int arg);
    void send(K32_stm32_api::CommandType cmd);
    long get(K32_stm32_api::CommandType cmd);

  private:
    SemaphoreHandle_t lock;
    bool running = false;
    int _battery = 0;
    bool _btn_click = false;
    bool _btn_dblclick = false;

    bool _btn_listen = true;
    bool _batt_listen = true;

    uint8_t *_blink_leds;
    int _blink_duration;

    static void task( void * parameter );
    static void blink_task (void * parameter);
    long read();
    void flush();


};

#endif
