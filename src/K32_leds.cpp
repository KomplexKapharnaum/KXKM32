/*
  K32_leds.cpp
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/

#include "Arduino.h"
#include "K32_leds.h"


K32_leds::K32_leds() {
  // ANIMATOR
  this->activeAnim = NULL;
  this->leds = new K32_leds_rmt();
  this->book = new K32_leds_animbook();
}


K32_leds_anim* K32_leds::anim( String animName) {
  return this->book->get(animName);
}


void K32_leds::play( K32_leds_anim* anim ) {
  // ANIM task
  this->activeAnim = anim;
  this->stop();
  xTaskCreate( this->animate,        // function
                "leds_anim_task", // task name
                1000,             // stack memory
                (void*)this,      // args
                3,                      // priority
                &this->animateHandle);  // handler
}

void K32_leds::play( String animName ) {
  this->play( this->anim( animName ) );
}

void K32_leds::stop() {
  if (this->animateHandle) {
    vTaskDelete( this->animateHandle );
    this->animateHandle = NULL;
  }
  this->leds->blackout();
}


/*
 *   PRIVATE
 */

 void K32_leds::animate( void * parameter ) {
   K32_leds* that = (K32_leds*) parameter;

   if (that->activeAnim)
     while(that->activeAnim->loop( that->leds ));

   that->animateHandle = NULL;
   vTaskDelete(NULL);
 }


 /////////////////////////////////////////////
