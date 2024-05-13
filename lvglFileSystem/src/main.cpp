/*Using LVGL with Arduino requires some extra steps:
 *Be sure to read the docs here: https://docs.lvgl.io/master/get-started/platforms/arduino.html  */
#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include <lvgl.h>
#include <TAMC_GT911.h>
#include "Touch.h"
#include <lv_port_fs.h>

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

/* More display class: https://github.com/moononournation/Arduino_GFX/wiki/Display-Class */
// bus, DF_GFX_RST, 0 /* rotation */, false /* IPS */);
/*Set to your screen resolution*/
#define TFT_HOR_RES 480
#define TFT_VER_RES 272

Arduino_GFX *gfx = new Arduino_RGB_Display(TFT_HOR_RES, TFT_VER_RES, panel);
#define TFT_BL 2

void backlightSetup()
{
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);
}

TAMC_GT911 ts = TAMC_GT911(TOUCH_GT911_SDA, TOUCH_GT911_SCL, TOUCH_GT911_INT, TOUCH_GT911_RST, TFT_HOR_RES, TFT_VER_RES);

/*LVGL draw into this buffer, 1/10 screen size usually works well. The size is in bytes*/
#define DRAW_BUF_SIZE (TFT_HOR_RES * TFT_VER_RES / 10 * (LV_COLOR_DEPTH / 8))
uint32_t draw_buf[DRAW_BUF_SIZE / 4];

#if LV_USE_LOG != 0
void my_print(lv_log_level_t level, const char *buf)
{
  LV_UNUSED(level);
  Serial.println(buf);
  Serial.flush();
}
#endif
/* LVGL calls it when a rendered image needs to copied to the display*/
/*
    Is this the only direct interface between lvgl and gfx?
*/
void my_disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

  gfx->draw16bitRGBBitmap(area->x1, area->y1, (uint16_t *)px_map, w, h);

  lv_disp_flush_ready(disp);
}

/*Read the touchpad*/
void my_touchpad_read(lv_indev_t *touchDev, lv_indev_data_t *data)
{
  char buffer[50];
  ts.read();
  if (ts.isTouched)
  {
    data->state = LV_INDEV_STATE_PR;

    /*Set the coordinates*/
    data->point.x = ts.points[0].x; // touch_last_x;
    data->point.y = ts.points[0].y; // touch_last_y;
    sprintf(buffer, "down x %d y %d\r\n", data->point.x, data->point.y);
    LV_LOG_USER(buffer);
  }
  else
  {
    data->state = LV_INDEV_STATE_REL;
  }
}

static lv_fs_dir_t dir;
static lv_fs_res_t res;
static lv_fs_file_t file;

static char mybuff[100];

void setup()
{
  backlightSetup();

  gfx->begin();
  gfx->setRotation(2);

  Serial.begin(115200);
  while (!Serial)
    delay(50);
  lv_init();
  delay(10);
  ts.begin();
  ts.setRotation((gfx->getRotation() + 1) % 4);

  /*Set a tick source so that LVGL will know how much time elapsed. */
  lv_tick_set_cb((lv_tick_get_cb_t)millis);

  /* register print function for debugging */
#if LV_USE_LOG != 0
  lv_log_register_print_cb(my_print);
#endif

  lv_display_t *disp;
  disp = lv_display_create(TFT_HOR_RES, TFT_VER_RES);
  lv_display_set_flush_cb(disp, my_disp_flush);
  lv_display_set_buffers(disp, draw_buf, NULL, sizeof(draw_buf), LV_DISPLAY_RENDER_MODE_PARTIAL);

  /*Initialize the touchpad driver*/
  lv_indev_t *touchDev = lv_indev_create();
  lv_indev_set_type(touchDev, LV_INDEV_TYPE_POINTER); /*Touchpad should have POINTER type*/
  lv_indev_set_read_cb(touchDev, my_touchpad_read);

// create a text area
  lv_obj_t * ta = lv_textarea_create(lv_screen_active());
  lv_textarea_set_one_line(ta, false);
  lv_obj_align(ta, LV_ALIGN_TOP_LEFT, 10, 10);
  
  lv_textarea_set_text(ta,"hello there");
    
  // initialize FS driver
  lv_port_fs_init();
  if (lv_fs_is_ready('S'))
  {
    LV_LOG_USER("FS ready");
    String text = "";
#if 1   // read directory example
    if (lv_fs_dir_open(&dir,"S:/fonts") == LV_FS_RES_OK) {
      LV_LOG_USER("Directory opened");
      text += "directory opened\n";
      lv_textarea_set_text(ta,text.c_str());
      while (true) {
        if (lv_fs_dir_read(&dir,mybuff,sizeof(mybuff)) != LV_FS_RES_OK) {
          break;
        }
        if (strlen(mybuff) == 0)
          break;
        text += mybuff;
        text += "\n";
        lv_textarea_set_text(ta,text.c_str());
      }
      if (lv_fs_dir_close(&dir)== LV_FS_RES_OK) {
        text += "directory closed\n";
        lv_textarea_set_text(ta,text.c_str());
      }
      else
        LV_LOG_USER("Directory not closed");
    }
    else
      LV_LOG_USER("Directory not opened");
#endif
#if 0 // read file example
    if ( lv_fs_open(&file,"S:/people/lists/names.txt",LV_FS_MODE_RD) == LV_FS_RES_OK) {
      LV_LOG_USER("file opened");
      uint32_t bread=0,totalread=0,position;
      lv_fs_res_t res;
      memset(mybuff,0,sizeof(mybuff));
      res = lv_fs_read(&file,mybuff,8,&bread);
      String text = "";
      while  ( res == LV_FS_RES_OK && bread > 0) {
        mybuff[bread] = 0; // add end marker
        //LV_LOG_USER(mybuff);
 //       Serial.print(mybuff);
        text += mybuff;
        lv_textarea_set_text(ta,text.c_str());
        memset(mybuff,0,bread);
//        if (lv_fs_tell(&file,&position) == LV_FS_RES_OK) {
  //        sprintf(mybuff,"position %d",position);
    //      LV_LOG_USER(mybuff);
      //  }
        if (bread > 0)
          totalread += bread;
        else
          totalread += strlen(mybuff);
        res = lv_fs_read(&file,mybuff,8,&bread);
      }
      sprintf(mybuff,"Total read %d",totalread);
      LV_LOG_USER(mybuff);
      if (lv_fs_close(&file) == LV_FS_RES_OK)
        LV_LOG_USER("file closed");
      else
        LV_LOG_USER("file not closed");
    }
    else {
      LV_LOG_USER("file not opened");
    }
#endif
#if 0 // write example
    if (lv_fs_open(&file, "S:/people/lists/stuff.txt", LV_FS_MODE_WR) == LV_FS_RES_OK)
    {
      LV_LOG_USER("file opened");
//      if (lv_fs_seek(&file, 0, LV_FS_SEEK_END) == LV_FS_RES_OK)  // append to end
//      if (lv_fs_seek(&file, 0, LV_FS_SEEK_SET) == LV_FS_RES_OK)    // rewind to beginning
      if (lv_fs_seek(&file, 50, LV_FS_SEEK_SET) == LV_FS_RES_OK)   // start at an arbitary place
      {
        LV_LOG_USER("seek success");
        // write to the chosen position
        uint32_t bwritten;
        for (int i = 0; i < 20; i++)
        {
          sprintf(mybuff, "Line %d\n", i);
          if (lv_fs_write(&file, mybuff, strlen(mybuff), &bwritten) == LV_FS_RES_OK)
          {
            sprintf(mybuff, "%d written", bwritten);
            LV_LOG_USER(mybuff);
          }
          else
            LV_LOG_USER("write error");
        }
      }
      else
        LV_LOG_USER("Seek failed");
      if (lv_fs_close(&file) == LV_FS_RES_OK)
        LV_LOG_USER("file closed");
      else
        LV_LOG_USER("file not closed");
    }
    else
    {
      LV_LOG_USER("file not opened");
    }
#endif
  }
  else
    LV_LOG_USER("FS not ready");
  lv_port_fs_end();
  LV_LOG_USER("Setup done");
}

void loop()
{
  lv_task_handler(); /* let the GUI do its work */
  delay(5);          /* let this time pass */
}