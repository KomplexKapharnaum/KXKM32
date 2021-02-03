/*
  K32_system.h
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/
#ifndef K32_system_h
#define K32_system_h

#include <Preferences.h>

#include "K32_version.h"
#include "K32_stm32.h"
#include "K32_intercom.h"


class K32_system {
  public:
    K32_system() {
      this->lock = xSemaphoreCreateMutex();

      xSemaphoreTake(this->lock, portMAX_DELAY);
      preferences.begin("k32-app", false);
      xSemaphoreGive(this->lock);
    };

    int id() {
      #ifdef K32_SET_NODEID
        return K32_SET_NODEID;
      #else
        int id;
        xSemaphoreTake(this->lock, portMAX_DELAY);
        id = preferences.getUInt("id", 0);
        xSemaphoreGive(this->lock);
        return id; 
      #endif
    }

    void id(int id) {
      xSemaphoreTake(this->lock, portMAX_DELAY);
      int old = preferences.getUInt("id", 0);
      if (id != old) preferences.putUInt("id", id);
      xSemaphoreGive(this->lock);
    }

    int channel() {
      int chan;
      xSemaphoreTake(this->lock, portMAX_DELAY);
      chan = preferences.getUInt("channel", 15);
      xSemaphoreGive(this->lock);
      return chan;
    }

    void channel(int channel) {
      xSemaphoreTake(this->lock, portMAX_DELAY);
      int old = preferences.getUInt("channel", 15);
      if (channel != old) preferences.putUInt("channel", channel);
      xSemaphoreGive(this->lock);
    }

    int hw() {
      #ifdef K32_SET_HWREVISION
        return K32_SET_HWREVISION;
      #elif HW_REVISION
        return HW_REVISION;
      #else
        int hw;
        xSemaphoreTake(this->lock, portMAX_DELAY);
        hw = preferences.getUInt("hw", 0);
        xSemaphoreGive(this->lock);
        return hw;
      #endif
    }

    void hw(int hwrevision) {
      xSemaphoreTake(this->lock, portMAX_DELAY);
      int old = preferences.getUInt("hw", 0);
      if (hwrevision != old) preferences.putUInt("hw", hwrevision);
      xSemaphoreGive(this->lock);
    }

    String name() {
      String name = "esp-" + String(this->id());
      return name;
    }

    void reset() {
      xSemaphoreTake(this->lock, portMAX_DELAY);
      preferences.end();
      xSemaphoreGive(this->lock);
      if (stm32) stm32->reset();
      while (true);
    }

    void shutdown() {
      xSemaphoreTake(this->lock, portMAX_DELAY);
      preferences.end();
      xSemaphoreGive(this->lock);
      if (stm32) stm32->shutdown();
    }

    int ledpin(int i) {
      return LEDS_PIN[hw()][i];
    }

    // EXECUTE standardized command
    void command(Orderz* order) 
    {
      // RESET
      if (strcmp(order->action, "reset") == 0) 
          this->reset();

      // SHUTDOWN
      else if (strcmp(order->action, "shutdown") == 0) 
          this->shutdown();

      // SET CHANNEL
      else if (strcmp(order->action, "channel") == 0)
      {
          if (order->count() < 1) return;
          byte chan = order->getData(0)->toInt();
          if (chan > 0) {
            this->channel(chan);
            delay(100);
            this->reset();
          }
      }
    }

    K32_stm32 *stm32 = NULL;
  
    Preferences preferences;
    
  private:
    SemaphoreHandle_t lock;

};

#endif
