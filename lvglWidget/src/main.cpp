/*Using LVGL with Arduino requires some extra steps:
 *Be sure to read the docs here: https://docs.lvgl.io/master/get-started/platforms/arduino.html  */
#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include <lvgl.h>
#include <TAMC_GT911.h>
#include "Touch.h"

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

void setup()
{
    backlightSetup();

    gfx->begin();
    gfx->setRotation(2);
    String LVGL_Arduino = "Hello Arduino! LVGL ";
    LVGL_Arduino += String('V') + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();

    Serial.begin(115200);
    Serial.println(LVGL_Arduino);

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
    /* Create a simple label  */
    lv_obj_t *label = lv_label_create(lv_scr_act());
    lv_obj_t *label2 = lv_label_create(lv_scr_act());
    lv_obj_t *label3 = lv_label_create(lv_scr_act());
    static lv_style_t style;
    lv_color_t color;
    color.red = 0xff;
    color.green = 0x0;
    color.blue = 0xff;
    lv_style_init(&style);
    lv_style_set_text_color(&style, color);
    //    lv_style_set_text_font(&style, &lv_font_montserrat_24); // <--- you have to enable other font sizes in menuconfig
    lv_style_set_text_font(&style, &lv_font_dejavu_16_persian_hebrew); // <--- you have to enable other font sizes in menuconfig
    lv_obj_add_style(label, &style, 0);                                // <--- obj is the label
                                                                       //    lv_label_set_text( label, "שלום עולם" ); // BIDI automatically works out direction RTL
    lv_label_set_text(label, "Hello World");
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

    lv_label_set_text(label2, LVGL_Arduino.c_str());
    lv_obj_align(label2, LV_ALIGN_BOTTOM_LEFT, 0, 0); // LTR

    lv_label_set_text(label3, "Top Left");
    lv_obj_align(label3, LV_ALIGN_TOP_LEFT, 0, 0); // LTR
    Serial.println("Setup done");
}

void loop()
{
    lv_task_handler(); /* let the GUI do its work */
    delay(5);          /* let this time pass */
}