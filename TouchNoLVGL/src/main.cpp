#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include <TAMC_GT911.h>
#include "Touch.h"

#define SCREEN_ROTATION 2  // 0,2 landscape 1,3 portrait
#if SCREEN_ROTATION==0||SCREEN_ROTATION==2
#define SCREEN_PORTRAIT
#endif

Arduino_ESP32RGBPanel *panel = new Arduino_ESP32RGBPanel(
    40 /* DE */, 41 /* VSYNC */, 39 /* HSYNC */, 42 /* DCLK */,
    45 /* R0 */, 48 /* R1 */, 47 /* R2 */, 21 /* R3 */, 14 /* R4 */,
    5 /* G0 */, 6 /* G1 */, 7 /* G2 */, 15 /* G3 */, 16 /* G4 */, 4 /* G5 */,
    8 /* B0 */, 3 /* B1 */, 46 /* B2 */, 9 /* B3 */, 1 /* B4 */,
    0 /*hsync_polarity*/, 8 /* hsync_front_porch*/, 4 /* hsync_pulse_width*/, 43 /* hsync_back_porch*/,
    0 /*vsync_polarity*/, 8 /*vsync_front_porch*/, 4 /*vsync_pulse_width*/, 12 /*vsync_back_porch*/,
    1 /*pclk_active_neg*/, 9000000 /*prefer_speed*/, false /*useBigEndian*/,
    0 /*de_idle_high*/, 0 /*pclk_idle_high*/
);

#define TFT_HOR_RES 480
#define TFT_VER_RES 270

Arduino_GFX *gfx = new Arduino_RGB_Display(TFT_HOR_RES, TFT_VER_RES, panel);
#define TFT_BL 2

void backlightSetup()
{
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);
}

TAMC_GT911 ts = TAMC_GT911(TOUCH_GT911_SDA, TOUCH_GT911_SCL, TOUCH_GT911_INT, TOUCH_GT911_RST,
#ifdef SCREEN_PORTRAIT
                                                                                                TFT_HOR_RES, TFT_VER_RES);
#else
                                                                                                TFT_VER_RES, TFT_HOR_RES);
#endif

void setup()
{
  Serial.begin(115200);
  while (!Serial) delay(50);
  // initialise screen and touch device
  backlightSetup();
  gfx->begin();
  gfx->setRotation(SCREEN_ROTATION);
  gfx->fillScreen(CYAN);
  gfx->setTextColor(BLACK);
  gfx->print("Touch Test");
  ts.begin();
  ts.setRotation((gfx->getRotation()+1)%4);
}

void loop()
{
  int x = 0, y = 0;
  ts.read();
  if (ts.isTouched)
  {
    // get mean value of x,y
    for (int i = 0; i < ts.touches; i++)
    {
      x += ts.points[i].x;
      y += ts.points[i].y;
    }
    x /= ts.touches;
    y /= ts.touches;
    // draw red circle at x,y
    Serial.printf("x: %d y: %d\n",x,y);
    gfx->fillCircle(x, y, 10, RED);
    delay(50);
  }
}
