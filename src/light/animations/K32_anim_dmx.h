/*
  K32_anim_basics.h
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/
#ifndef K32_anim_dmx_h
#define K32_anim_dmx_h

#define STROB_ON_MS  2000/LEDS_SHOW_FPS

//
// NOTE: to be available, add #include to this file in K32_light.h !
//


// OUTILS
//

enum color_mode { 
  COLOR_NORM    = 0,      
  COLOR_PICKER  = 1,    
  COLOR_SD      = 2        
};

static  CRGBW colorPreset[25] = {
  {CRGBW::Black},         // 0
  {CRGBW::Red},           // 1
  {CRGBW::Green},         // 2
  {CRGBW::Blue},          // 3
  {CRGBW::White},         // 4
  {CRGBW::Yellow},        // 5
  {CRGBW::Magenta},       // 6
  {CRGBW::Cyan},          // 7
  {CRGBW::Orange}        // 8
};


// Transform DMX range into simple value, I.E.  0-10 => 0, 11-20 => 1, 21-30 => 2, ....
inline int simplifyDmxRange(int value) {
  if (value > 0) value -= 1;
  return value/10;
}

// Return rounded value after dividing by 255
inline int roundDivInt(int a, int b) {
  return a/b + ((a%b)>(b/2));
}

// Scale a number based on a 8bit value
inline int scale255(int a, uint8_t value) {
  return roundDivInt( a*value, 255 );
}

// Check if int is min <= value <= max
inline bool btw(int value, int min, int max) {
  return (value >= min) && (value <= max);
}

// ANIM DMX
//

class K32_anim_dmx : public K32_anim {
  public:

    K32_anim_dmx() {
      
      // Strobe use data channel LEDS_DATA_SLOTS-1 to modulate
      // param 0: width of pulse in ms (time strobe ON)
      this->modulate( LEDS_DATA_SLOTS-1, "strobe", new K32_mod_pulse)->mini(0)->param(0, STROB_ON_MS);  

      // Smooth use data channel LEDS_DATA_SLOTS-2 to modulate
      this->modulate( LEDS_DATA_SLOTS-2, "smooth", new K32_mod_sinus)->mini(0)->maxi(255);  

    }

    // Loop
    void draw ()
    {
      //
      // ONDMXFRAME PUSH
      //
      
      // Mirror & Zoom -> Segment size
      int mirrorMode  = simplifyDmxRange(data[15]);
      int zoomedSize  = max(1, scale255( this->size(), data[16]) );

      int segmentSize = zoomedSize;
      if (mirrorMode == 1 || mirrorMode == 4)       segmentSize /= 2;
      else if (mirrorMode == 2 || mirrorMode == 5)  segmentSize /= 3;
      else if (mirrorMode == 3 || mirrorMode == 6)  segmentSize /= 4;

      // Create Segment Buffer
      CRGBW segment[segmentSize];

      // Modes
      int colorMode   = simplifyDmxRange(data[14]);
      int pixMode     = simplifyDmxRange(data[5]);

      //
      // GENERATE BASE SEGMENT
      //

      // Color mode NORM
      //
      if (colorMode == COLOR_NORM) {

        // Bi-color
        //
        if (pixMode < 23) {
          
          // Colors
          CRGBW color1 {data[1], data[2], data[3], data[4]};
          CRGBW color2 {data[10], data[11], data[12], data[13]};

          // Dash Length + Offset
          int dashLength  = max(1, scale255(segmentSize, data[6]) );         // pix_start
          int dashOffset  =  scale255(segmentSize, data[7]);                // pix_pos

          // Fix = only color1
          if (pixMode == 0) 
          {
            for(int i=0; i<segmentSize; i++) segment[i] = color1;
          }

          // Ruban = color1 one-dash / color2 background
          else if (pixMode == 1) 
          {
            for(int i=0; i<segmentSize; i++) 
            {
              if (i >= dashOffset && i < dashOffset+dashLength) segment[i] = color1;
              else segment[i] = color2;
            }
          }

          // 01:02 = color1 + color2 dash 
          if (pixMode == 2) 
          {
            dashLength = data[6];   // on this one, length is absolute and not relative to segementSize

            for(int i=0; i<segmentSize; i++) 
            {
              if ( (i+dashOffset)/dashLength % 2 == 0 ) segment[i] = color1;
              else segment[i] = color2;
            }
          }

          // rus> + rusf> = color1 one-dash fade or blend R / color2 background
          else if (pixMode == 3 || pixMode == 9) 
          {
            for(int i=0; i<segmentSize; i++) 
            {
              if (i >= dashOffset && i < dashOffset+dashLength) 
              {
                int coef = ((dashOffset+dashLength-1-i) * 255 ) / (dashLength-1);

                segment[i] = color1 % (uint8_t)coef;
                if (pixMode == 9) segment[i] += color2 % (uint8_t)(255-coef);
              }
              else segment[i] = color2;
            }
          }

          // rus< + rusf< = color1 one-dash fade or blend L / color2 background
          else if (pixMode == 4 || pixMode == 10) 
          {
            for(int i=0; i<segmentSize; i++) 
            {
              if (i >= dashOffset && i < dashOffset+dashLength) 
              {
                int coef = ((i-dashOffset) * 255 ) / (dashLength-1);

                segment[i] = color1 % (uint8_t)coef;
                if (pixMode == 10) segment[i] += color2 % (uint8_t)(255-coef);
              }
              else segment[i] = color2;
            }
          }

          // rus<> + rusf<> = color1 one-dash fade or blend LR / color2 background
          else if (pixMode == 5 || pixMode == 11) 
          {
            for(int i=0; i<segmentSize; i++) 
            {    
              if (i >= dashOffset && i < dashOffset+dashLength/2) 
              {
                int coef = ((i-dashOffset) * 255 ) / (dashLength/2);

                segment[i] = color1 % (uint8_t)coef;
                if (pixMode == 11) segment[i] += color2 % (uint8_t)(255-coef); 
              }
              else if (i >= dashOffset+dashLength/2 && i < dashOffset+dashLength) 
              {
                int coef = ((dashOffset+dashLength-1-i) * 255 ) / (dashLength/2);

                segment[i] = color1 % (uint8_t)coef;
                if (pixMode == 11) segment[i] += color2 % (uint8_t)(255-coef); 
              }
              else segment[i] = color2;
            }
          }

        }

        // Tri-color + Quadri-color 
        //
        else if (pixMode == 23 || pixMode == 24) {
          
          // Colors
          CRGBW color[5] = {
            {data[10], data[11], data[12], data[13]}, // background
            colorPreset[ simplifyDmxRange(data[1]) ],                   // color1
            colorPreset[ simplifyDmxRange(data[2]) ],                   // color2
            colorPreset[ simplifyDmxRange(data[3]) ],                   // color3
            colorPreset[ simplifyDmxRange(data[4]) ]                    // color4
          };

          // Dash Length + Offset + Split 
          int dashLength  = max(3, scale255(2 * segmentSize, data[6]));          
          int dashOffset  = scale255((segmentSize-dashLength), data[7]);
          int dashSplit = pixMode % 20;
          int dashPart  = 0;

          // Multi-color dash / color0 backgound
          for(int i=0; i<segmentSize; i++) 
          {
            dashPart = (i-dashOffset)*dashSplit/dashLength + 1;       // find in which part of the dash we are
            if (dashPart < 0 || dashPart > dashSplit) dashPart = 0;   // use 0 if outside of the dash

            segment[i] = color[ dashPart ];
          }          

        }       

      }

      // Color mode PICKER
      else if (colorMode == COLOR_PICKER) {

        // Hue range
        int hueStart = data[10] + data[7];
        int hueEnd = data[11] + data[7];

        // Channel master
        CRGBW rgbwMaster {data[1], data[2], data[3], data[4]};
        
        // Color wheel
        CRGBW colorWheel;
        for(int i=0; i<segmentSize; i++) {
          segment[i] = colorWheel.setHue( (hueStart + ((hueEnd - hueStart) * i) / segmentSize) );
          segment[i] %= rgbwMaster;
        }

      }

      // Color mode SD
      else if (colorMode == COLOR_SD) {
        // TODO: load image from SD !
      }


      //
      // STROBE: modulators on master
    
      int strobeMode  = simplifyDmxRange(data[8]);
      int strobePeriod = map(data[9]*data[9], 0, 255*255, STROB_ON_MS*4, 1000);

      // strobe
      if (strobeMode == 1 || btw(strobeMode, 3, 10)) 
      { 
        this->modulate("strobe")->period( strobePeriod );

        // OFF
        if (data[LEDS_DATA_SLOTS-1] == 0) {
          this->clear();
          return;
        }
      }

      // strobe blink (3xstrobe -> blind 1+s)
      else if (strobeMode == 11 || btw(strobeMode, 12, 19)) 
      {
        K32_modulator* strobe = this->modulate("strobe");
        int count = strobe->periodCount() % 3;  // ERROR: periodCount is moving.. -> count is not linear !
        // LOG(count);

        if (count == 0)       strobe->period( strobePeriod*100/225 );
        else if (count == 1)  strobe->period( strobePeriod/4 );
        else if (count == 2)  strobe->period( strobePeriod*116/100 + 1000 );
        
        // OFF
        if (data[LEDS_DATA_SLOTS-1] == 0) {
          this->clear();
          return;
        }
      }

      // smooth
      if (strobeMode == 2) 
      {
        K32_modulator* smooth = this->modulate("smooth");
        smooth->period( strobePeriod * 10 );

        for(int i=0; i<segmentSize; i++) segment[i] %= data[LEDS_DATA_SLOTS-2];
      }

      // random w/ threshold
      if (btw(strobeMode, 3, 10) || btw(strobeMode, 12, 19) || btw(strobeMode, 20, 25)) 
      {
        int strobeSeuil = 1000; 
        if (btw(strobeMode, 3, 10))       strobeSeuil = (data[8] - 31)*1000/69;         // 0->1000    strobeMode >= 3 && strobeMode <= 10
        else if (btw(strobeMode, 12, 19)) strobeSeuil = (data[8] - 121)*1000/79;        // 0->1000    strobeMode >= 12 && strobeMode <= 19
        else if (btw(strobeMode, 20, 25)) strobeSeuil = (data[8] - 201)*1000/54;        // 0->1000    strobeMode >= 20

        for(int i=0; i<segmentSize; i++) 
          if (random(1000) > strobeSeuil) segment[i] = {CRGBW::Black};
      }



      //
      // MASTER
      //
      uint8_t master = data[0];
      for(int i=0; i<segmentSize; i++) segment[i] %= master;

      //
      // DRAW ON STRIP WITH ZOOM & MIRROR
      //

      // Clear
      this->clear();

      // Mirroring alternate (1 = copy, 2 = alternate)
      int mirrorAlternate = 1 + (mirrorMode == 1 || mirrorMode == 2 || mirrorMode == 3); 

      // Zoom offset
      int zoomOffset = (this->size() - zoomedSize)/2;      

      // Copy pixels into strip
      for(int i=0; i<zoomedSize; i++) 
      {
        int pix  = i % segmentSize;           // pix cursor into segment
        int iter = i / segmentSize;           // count of mirror copy 
        
        if (iter && iter % mirrorAlternate)   // alternate: invert pix cursor
          pix = segmentSize - pix - 1;    

        this->pixel(i+zoomOffset, segment[pix]);
      }      

    }

};


#endif