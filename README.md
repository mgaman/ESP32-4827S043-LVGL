# Introduction

<p align="center"><img width="50%" src="Images/IMG_0179.png"></p>

Recently I bought a couple of [Cheap Yellow Displays](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display). They were both based on SPI controlled graphics chips and well covered by the [TFT_eSPI](https://github.com/Bodmer/TFT_eSPI) and [Widgets](https://github.com/Bodmer/TFT_eWidget.git) libraries.  
The first 2 boards were 2.8" and 3.2" so I bought another, 4.3" [board](https://www.aliexpress.com/item/1005004788147691.html?spm=a2g0o.order_list.order_list_main.11.54531802TOrJoA), thinking it would be the same as before, just larger.  
As I soon discovered the [ESP32-4827S043](https://www.openhasp.com/0.7.0/hardware/sunton/esp32-4827s043) is quite a different beast. While a serial interface like SPI may give reasonable performance for a small screen it can be a bottle-neck for larger screens, with more data to shift. Accordingly the ESP32-4827S043 uses a parallel interface with faster throughput. Sadly it is not supported by TFT_eSPI so we need a different solution.  
[Arduino_GFX](https://github.com/moononournation/Arduino_GFX) does support the ESP32 parallel interface and is used to control the hardware, [lvgl](https://github.com/lvgl/lvgl) for screen control, widgets and other goodies.  
<b>This is NOT a comparison of the two solutions. They both do an excellent job but in different ways</b>.  
NOTE: Do not confuse Arduino_GFX and Adafruit_GFX. Googling the former often brings up results for the latter.  They both have similar objectives but Adafruit, as far as I can see, does not support the ESP32 16 bit parallel interface of the ESP32-4827S043.  

## Objective
To quickly get up and running with the ESP32-4827S043.  
This is NOT a tutorial for Arduino_GFX and lvgl.  
<b>NEW!!</b> How to get up and running with [Squareline Studio](#squareline-studio).
## Caveat
The AliExpress page from which I bought my board has a link to download a bundle of documentation, library and examples source code. I strongly advise you NOT to use the Arduino_GFX and lvgl libraries there as they are older versions (GFX 1.2.8 LVGL 8.3) incompatible with the current versions (GFX 1.4.6 LVGL  9.1 at time of writing).  

## Development
My development environment is VS Code/PlatformIO so this documentation is aimed at that audience.
### Board
In PlatformIO choose the ESP32-S3-DevkitC board.  
<p align="center"><img width="76%" src="Images/boardselect.png"></p>
In Arduino IDE choose ESP32S3 Dev Module. Note I also selected OSI PSRAM. Not needed for this example but used in the LVGL example.
<p align="center"><img width="76%" src="Images/IDE.png"></p>

### Libraries
Define the libraries to be loaded in platformIO.ini/lib_deps
```
[env:esp32-s3-devkitc-1]
platform = espressif32
board = esp32-s3-devkitc-1
framework = arduino
lib_deps = 	moononournation/GFX Library for Arduino@^1.4.6
```
# Examples
## HelloWorldNoLVGL
Demonstrates how to install the Arduino_GFX library and define the ESP32-4827S043 device.  
The screen color is set and some text written in the center of the screen,
## TouchNoLVGL

<video width="640" height="480" controls>
  <source src="Images/IMG_0182.mp4" type="video/mp4">
</video>

An extension of the <b>HelloWorldNoLVGL</b> example. The touch screen is defined and polled regularily. If a touch event is detected then a red circle is drawn at that point.  
My board has the capacative touch screen powered by the GT911 controller so the comments below are only relevant to that device. If you have the resistive touch version of the board, feel free to update this document.  
Modifiy platformio.ini to install the GT911 library.  
```
[env:esp32-s3-devkitc-1]
platform = espressif32
board = esp32-s3-devkitc-1
framework = arduino
monitor_speed = 115200
lib_deps = 	moononournation/GFX Library for Arduino@^1.4.6
	tamctec/TAMC_GT911@^1.0.2
```

Note that we have to define the rotations of the screen and touch separately. Unfortunately because of the way they are wired the 2 components do not use the same values for rotation. See the following table.  
Assuming that Normal Landscape mode is with the USB port to the left, normal portrait USB port below.
| Orientation | Screen | Touch |
| ----------- | ------ | ----- |
| Landscape | 0 | 1 ROTATION_INVERTED |
| Portrait | 1 | 2 ROTATION RIGHT |
| Inverted Landscape | 2 | 3 ROTATION_NORMAL |
| Inverted Portrait | 3 | 0 ROTATION LEFT |
<p>
Note that the width and height of the touch screen is defined in the constructor according to the declared orientation. The GT911 library does not handle dynamic rotation change.
<p>
A convenient way to set touch orientation directly from screen orientation is

```
gfx->setRotation(2);
ts.setRotation((gfx->getRotation()+1)%4);
```
## lvglWidget
lvgl needs a lot of RAM. While a smaller screen e.g. 320x240 might be handled within the confines of the 512KB of an ESP32 our larger 480x272 screen cannot. The ESP32-4827S043 has 8MB of PSRAM that needs to be unlocked. The 2 items in *build_flags* relating to PSRAM and the *board_build* line must be entered as-is to platformio.ini to make use of PSRAM.  
lvgl also needs a configuration file *lv_conf.h* to know which of the many options to include and which to omit. The location of the configuration file is the object of much confusion. Each lvgl project may need its own, different, configuration file. To make life simple, place *lv_conf.h* in the *include* folder of the project and add the *-Iinclude/* item to *build_flags*. Don't forget the trailing slash.
```
[env:esp32-s3-devkitc-1]
platform = espressif32
board = esp32-s3-devkitc-1
framework = arduino
monitor_speed = 115200
lib_deps = 	moononournation/GFX Library for Arduino@^1.4.6
	lvgl/lvgl@^9.1.0
	tamctec/TAMC_GT911@^1.0.2
build_flags = -Iinclude/
    -DBOARD_HAS_PSRAM -mfix-esp32-psram-cache-issue
board_build.arduino.memory_type = qio_opi
```
## lvglButtonExample
<video width="640" height="480" controls>
  <source src="Images/IMG_0190.mp4" type="video/mp4">
</video>

This is an implementation of the *lv_example_button_1* example from the lvgl examples library. Library installation and PSRAM unlocking is as shown in the previous example.
## lvglFileSystem
lvgl has an abstraction of a file system that interfaces to a physical file system via callback functions.  
I have implemented a library that provides file I/O (open/close/read/write/seek/tell) and directory(open/read/close) for SD & LittleFS devices.  
My source only allows for a single file/directory to be open at any point in time.  
See the source in the lib/FileSystem folder.  

To choose SD or LittleFS uncomment the appropriate line in lib/FileSystem/lv_port_fs.h.  
If choosing LittleFS you also need to let PlatformIO. See platformio.ini.  
The directory listing example raises and interesting point. lvgl has no way to identify a file as a directory, so how can one do a recursive directory listing? In lv_fs_dir_read
I prepend a filename that is a directory by DIR: however this is just my solution.  
Note that I did NOT define a logical drive in the *lv_conf.h* file. It is defined in my library source and I arbitarily assigned the letter 'S' to both SD and LittleFS devices.
# Squareline Studio
While this tool is aimed at the TFT_eSPI/Arduino IDE user it does not mean that Arduino_GFX/PlatformIO users are excluded. The method to get up and running is actually quite simple. Just follow the following steps. Don't panic, parts 1-5 below are just run once.  
1. Open VSCode/PlatformIO and create an Arduino project. Let the pathname of the project (the folder containing the platformIO.ini file) be \<ArduinoPath\>.  
2. Open Squareline Studio and create a new Arduino project as follows:
    - Select Major LVGL version 9.1
    - Select create/Arduino IDE
    - In the Project Settings panel
    - Set the project name e.g. first.spj  (remember the name <b>first</b>, needed later).
    - Set the path for saving the SquareLine project. This will be \<ArduinoPath\> as defined above.
    - Set resolution (480, 272) for Landscape, (272,480) for Portrait.
    - Set Color Depth to 16.
    - Click on the Create button.
    - Develop your SquareLine project. A single screen is sufficient at this point.
    - Save the project, it goes to \<ArduinoPath\>. 
    - In the export menu, click on Create Template Project. When asked to select a folder, use \<ArduinoPath\>. This has the effect of copying a load of auto-generated source files to the folder \<ArduinoPath\>/first.
3. The folder \<ArduinoPath\>/first contains stuff we dont't need and may be deleted i.e. the copies of the lvgl and TFT_eSPI libraries in \<ArduinoPath\>/first/libraries. The \<ArduinoPath\>/first/ui folder contains the generated source files for our SquareLine project and must be preserved.
4. The platformIO.ini file is now modified to
    - Include compilation of the generated ui files.
    - Tell the compiler where to find the lv_conf.h file
    - Define the libraries to be loaded.
```platformio.ini
[env:esp32-s3-devkitc-1]
platform = espressif32
board = esp32-s3-devkitc-1
framework = arduino
;------ Above this line generated in the original Arduino project
monitor_speed = 115200
; ---  point the compiler to the location of lv_conf.h.  Unlock PSRAM on this MCU
build_flags = -Ifirst/libraries  -DBOARD_HAS_PSRAM -mfix-esp32-psram-cache-issue
; --- Force compilation of the generated ui source files
lib_extra_dirs = first/libraries/ui
; --- Define libraries to be loaded
lib_deps = lvgl/lvgl@^9.1.0
	moononournation/GFX Library for Arduino@
	tamctec/TAMC_GT911
; -- More PSRAM info
board_build.arduino.memory_type = qio_opi
```
5. SquareLine also creates a base Arduino application in \<ArduinoPath\>/first/ui/ui.ino. This file is configured for TFT_eSPI and must be altered for use with Arduino_GFX and the GT911 touch screen. You may ignore the generated file and replace \<ArduinoPath\>/src/main.cpp with this modified version (only needs to be done once).
```c++
//Arduino-TFT_eSPI board-template main routine. There's a TFT_eSPI create+flush driver already in LVGL-9.1 but we create our own here for more control (like e.g. 16-bit color swap).
#include <Arduino.h>
#include <lvgl.h>
#include <Arduino_GFX_Library.h>
#include <TAMC_GT911.h>
#include "TouchGT911.h"

#include <ui.h>

/*Don't forget to set Sketchbook location in File/Preferences to the path of your UI project (the parent foder of this INO file)*/

/*Change to your screen resolution*/
static const uint16_t screenWidth  = 480;
static const uint16_t screenHeight = 272;

enum { SCREENBUFFER_SIZE_PIXELS = screenWidth * screenHeight / 10 };
static lv_color_t buf [SCREENBUFFER_SIZE_PIXELS];

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

Arduino_GFX *tft = new Arduino_RGB_Display(screenWidth, screenHeight, panel);
#define TFT_BL 2

void backlightSetup()
{
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);
}

TAMC_GT911 ts = TAMC_GT911(TOUCH_GT911_SDA, TOUCH_GT911_SCL, TOUCH_GT911_INT, TOUCH_GT911_RST, screenWidth, screenHeight);

#if LV_USE_LOG != 0
/* Serial debugging */
void my_print(const char * buf)
{
    Serial.printf(buf);
    Serial.flush();
}
#endif

/* Display flushing */
void my_disp_flush (lv_display_t *disp, const lv_area_t *area, uint8_t *pixelmap)
{
    uint32_t w = ( area->x2 - area->x1 + 1 );
    uint32_t h = ( area->y2 - area->y1 + 1 );

    if (LV_COLOR_16_SWAP) {
        size_t len = lv_area_get_size( area );
        lv_draw_sw_rgb565_swap( pixelmap, len );
    }
    tft->draw16bitRGBBitmap(area->x1, area->y1, (uint16_t *)pixelmap, w, h);
    lv_disp_flush_ready( disp );
}

/*Read the touchpad*/
void my_touchpad_read (lv_indev_t * indev_driver, lv_indev_data_t * data)
{
    uint16_t touchX = 0, touchY = 0;

    ts.read();
    bool touched = ts.isTouched;

    if (!touched)
    {
        data->state = LV_INDEV_STATE_REL;
    }
    else
    {
        data->state = LV_INDEV_STATE_PR;

        /*Set the coordinates*/
        data->point.x = ts.points[0].x; // touch_last_x;
        data->point.y = ts.points[0].y; // touch_last_y;

     //   Serial.print( "Data x " );
     //   Serial.println( data->point.x);

     //   Serial.print( "Data y " );
     //   Serial.println( data->point.y );
    }
}

/*Set tick routine needed for LVGL internal timings*/
static uint32_t my_tick_get_cb (void) { return millis(); }


void setup ()
{
    Serial.begin( 115200 ); /* prepare for possible serial debug */

    String LVGL_Arduino = "Hello Arduino! ";
    LVGL_Arduino += String('V') + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();

    Serial.println( LVGL_Arduino );
    Serial.println( "I am LVGL_Arduino" );

    lv_init();

#if LV_USE_LOG != 0
    lv_log_register_print_cb( my_print ); /* register print function for debugging */
#endif
    backlightSetup();
    tft->begin();         
    tft->setRotation( 2 ); /* Landscape orientation, flipped */
    ts.begin();
    ts.setRotation(3);

    static lv_disp_t* disp;
    disp = lv_display_create( screenWidth, screenHeight );
    lv_display_set_buffers( disp, buf, NULL, SCREENBUFFER_SIZE_PIXELS * sizeof(lv_color_t), LV_DISPLAY_RENDER_MODE_PARTIAL );
    lv_display_set_flush_cb( disp, my_disp_flush );

    static lv_indev_t* indev;
    indev = lv_indev_create();
    lv_indev_set_type( indev, LV_INDEV_TYPE_POINTER );
    lv_indev_set_read_cb( indev, my_touchpad_read );

    lv_tick_set_cb( my_tick_get_cb );

    ui_init();

    Serial.println( "Setup done" );
}

void loop ()
{
    lv_timer_handler(); /* let the GUI do its work */
    delay(5);
}
```
By comparing with the generated ui.ino file you can see the I just modified the definition of <i>tft</i> and replaced calls to <i>tft</i> by <i>tft-\></i>. I also added a definition and use of the touchscreen <i>ts</i>.  
You may now compile and upload the project to the 4827SO343 device.  
6. All of the above only need to be done once. From now on after modifying the SquareLine Studio project it is sufficient to hit the export/Export UI Files menu item which just updates the generated source files in \<ArduinoPath\>/first/libraries/ui. All the necessary configuration is already in <i>platformIO.h</i> so gets compiled automatically.
## Usercode in a Squareline Studio project
When an event triggers a call to user code, SquareLine generates a source file \<ArduinoPath\>/first/libraries/ui/src/ui_events.c containing an instance of the called function. <b>Do not edit this file</b> as it gets overwritten each time you do an export/UI files.  
Instead copy ui_events.c to \<ArduinoPath\>/src and rename it to ui_events.cpp. Replace the line
```c++
#include "ui.h"
```
by the lines
```c++
#include <Arduino.h>
#include <ui.h>
```
Use this file to develop your event triggered code.
