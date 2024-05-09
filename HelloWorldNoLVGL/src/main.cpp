#include <Arduino.h>
#include <Arduino_GFX_Library.h>

Arduino_ESP32RGBPanel *panel = new Arduino_ESP32RGBPanel(
    40 /* DE */, 41 /* VSYNC */, 39 /* HSYNC */, 42 /* DCLK */,
    45 /* R0 */, 48 /* R1 */, 47 /* R2 */, 21 /* R3 */, 14 /* R4 */,
    5 /* G0 */, 6 /* G1 */, 7 /* G2 */, 15 /* G3 */, 16 /* G4 */, 4 /* G5 */,
    8 /* B0 */, 3 /* B1 */, 46 /* B2 */, 9 /* B3 */, 1 /* B4 */,
    0 /*hsync_polarity*/, 8 /* hsync_front_porch*/, 4 /* hsync_pulse_width*/, 43 /* hsync_back_porch*/,
      0 /*vsync_polarity*/ , 8 /*vsync_front_porch*/ , 4 /*vsync_pulse_width*/ , 12 /*vsync_back_porch*/ ,
      1 /*pclk_active_neg*/ , 9000000 /*prefer_speed*/ , false /*useBigEndian*/ ,
      0 /*de_idle_high*/, 0 /*pclk_idle_high*/
);

/* More display class: https://github.com/moononournation/Arduino_GFX/wiki/Display-Class */
  //bus, DF_GFX_RST, 0 /* rotation */, false /* IPS */);
  /*Set to your screen resolution*/
#define TFT_HOR_RES   480
#define TFT_VER_RES   272

Arduino_GFX *gfx = new Arduino_RGB_Display(TFT_HOR_RES,TFT_VER_RES,panel);
#define TFT_BL 2

void backlightSetup() {
  pinMode(TFT_BL,OUTPUT);
  digitalWrite(TFT_BL,HIGH);
}

const char *hw = "Hello World";
void setup() {
  backlightSetup();
  Serial.begin(115200);
  gfx->begin();
  gfx->setRotation(2);
  gfx->fillScreen(CYAN);
  gfx->setTextSize(4);
  gfx->setTextColor(BLACK);
  // calculate size of text then calculate x,y co-ordinates to place in center of the screen
  int16_t x,y;
  uint16_t w,h;
  gfx->getTextBounds(hw,0,0,&x,&y,&w,&h);
  gfx->setCursor((gfx->width()-w)/2,(gfx->height()-h)/2);
  gfx->print(hw);

}

void loop() {
  // put your main code here, to run repeatedly:
}
