/**
 * @file lv_port_fs_templ.c
 *
 */

/*Copy this file as "lv_port_fs.c" and set this value to "1" to enable content*/
#if 1

/*********************
 *      INCLUDES
 *********************/
#include <Arduino.h>
#include "lv_port_fs.h"
#include <lvgl.h>
#include <SD.h>

/*********************
 *      Static Data
 *********************/
static lv_fs_drv_t fs_drv;
static bool SDready = false;
static File sd_file;  // SD entity
static File sd_dir;  // SD entity
static lv_fs_file_t lv_file; // lvgl entity
static lv_fs_dir_t lv_dir;
static char buff[50];  // local sprintf
/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void fs_init();

static void * fs_open(lv_fs_drv_t * drv, const char * path, lv_fs_mode_t mode);
static lv_fs_res_t fs_close(lv_fs_drv_t * drv, void * file_p);
static lv_fs_res_t fs_read(lv_fs_drv_t * drv, void * file_p, void * buf, uint32_t btr, uint32_t * br);
static lv_fs_res_t fs_write(lv_fs_drv_t * drv, void * file_p, const void * buf, uint32_t btw, uint32_t * bw);
static lv_fs_res_t fs_seek(lv_fs_drv_t * drv, void * file_p, uint32_t pos, lv_fs_whence_t whence);
static lv_fs_res_t fs_size(lv_fs_drv_t * drv, void * file_p, uint32_t * size_p);
static lv_fs_res_t fs_tell(lv_fs_drv_t * drv, void * file_p, uint32_t * pos_p);
static bool fs_ready(lv_fs_drv_t * drv);

static void * fs_dir_open(lv_fs_drv_t * drv, const char * path);
static lv_fs_res_t fs_dir_read(lv_fs_drv_t * drv, void * rddir_p, char * fn, uint32_t fn_len);
static lv_fs_res_t fs_dir_close(lv_fs_drv_t * drv, void * rddir_p);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
void lv_port_fs_end(void) {
    SD.end();
    SDready = false;
    LV_LOG_USER("SD unmounted");
}
void lv_port_fs_init(void)
{
    /*----------------------------------------------------
     * Initialize your storage device and File System
     * -------------------------------------------------*/
    fs_init();

    /*---------------------------------------------------
     * Register the file system interface in LVGL
     *--------------------------------------------------*/

    lv_fs_drv_init(&fs_drv);

    /*Set up fields...*/
    fs_drv.letter = 'S';
    fs_drv.open_cb = fs_open;
    fs_drv.close_cb = fs_close;
    fs_drv.read_cb = fs_read;
    fs_drv.write_cb = fs_write;
    fs_drv.seek_cb = fs_seek;
    fs_drv.tell_cb = fs_tell;

    fs_drv.dir_close_cb = fs_dir_close;
    fs_drv.dir_open_cb = fs_dir_open;
    fs_drv.dir_read_cb = fs_dir_read;

    fs_drv.ready_cb = fs_ready;


    lv_fs_drv_register(&fs_drv);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/*Initialize your Storage device and File system.*/
static void fs_init()
{
    SDready = SD.begin();
    LV_LOG_USER(SDready ? "SD init pass" : "SD init fail");
}

/**
 * Open a file
 * @param drv       pointer to a driver where this function belongs
 * @param path      path to the file beginning with the driver letter (e.g. S:/folder/file.txt)
 * @param mode      read: FS_MODE_RD, write: FS_MODE_WR, both: FS_MODE_RD | FS_MODE_WR
 * @return          a file descriptor or NULL on error
 */
static void * fs_open(lv_fs_drv_t * drv, const char * path, lv_fs_mode_t mode)
{
 //   lv_fs_res_t res = LV_FS_RES_NOT_IMP;
    LV_LOG_USER(path);

    void * rc = NULL;
    char pmode[] = {0,0,0,0};
    if (mode & LV_FS_MODE_RD)
        strcat(pmode,"r");
    if (mode & LV_FS_MODE_WR)
        strcat(pmode,"w");
    if (strlen(pmode) > 0) {
        switch (drv->letter) {
            case 'S':
                sd_file = SD.open(path,pmode);
                if (sd_file) {
                    LV_LOG_USER("fs_open success");
                    lv_file.drv = drv;
                    lv_file.file_d = (void*)&sd_file;
                    rc = &lv_file;
                }
                else
                    LV_LOG_USER("fs_open fail");

                break;
        }
    }
    return rc;
}

/**
 * Close an opened file
 * @param drv       pointer to a driver where this function belongs
 * @param file_p    pointer to a file_t variable. (opened with fs_open)
 * @return          LV_FS_RES_OK: no error or  any error from @lv_fs_res_t enum
 */
static lv_fs_res_t fs_close(lv_fs_drv_t * drv, void * file_p)
{
    lv_fs_res_t res = LV_FS_RES_NOT_IMP;
    sprintf(buff,"Close file %c",drv->letter);
    LV_LOG_USER(buff);
    switch (drv->letter) {
        case 'S':
            ((File*)(((lv_fs_file_t*)file_p)->file_d))->close();
            res = LV_FS_RES_OK;
            break;
    }
    return res;
}

/**
 * Read data from an opened file
 * @param drv       pointer to a driver where this function belongs
 * @param file_p    pointer to a file_t variable.
 * @param buf       pointer to a memory block where to store the read data
 * @param btr       number of Bytes To Read
 * @param br        the real number of read bytes (Byte Read)
 * @return          LV_FS_RES_OK: no error or  any error from @lv_fs_res_t enum
 */
static lv_fs_res_t fs_read(lv_fs_drv_t * drv, void * file_p, void * buf, uint32_t btr, uint32_t * br)
{
    lv_fs_res_t res = LV_FS_RES_NOT_IMP;
    sprintf(buff,"Request %d bytes",btr);
    LV_LOG_USER(buff);
    switch(drv->letter) {
        case 'S':
            size_t bread = ((File*)(((lv_fs_file_t*)file_p)->file_d))->readBytes((char *)buf,btr);
            sprintf(buff,"Read %d",bread);
            LV_LOG_USER(buff);
            // br -1 is error, 0 means EOF reached so dont know how many actually read
            *br = bread;
            if (bread > 0) {
                res = LV_FS_RES_OK;
            }
            break;
    }

    return res;
}

/**
 * Write into a file
 * @param drv       pointer to a driver where this function belongs
 * @param file_p    pointer to a file_t variable
 * @param buf       pointer to a buffer with the bytes to write
 * @param btw       Bytes To Write
 * @param bw        the number of real written bytes (Bytes Written). NULL if unused.
 * @return          LV_FS_RES_OK: no error or  any error from @lv_fs_res_t enum
 */
static lv_fs_res_t fs_write(lv_fs_drv_t * drv, void * file_p, const void * buf, uint32_t btw, uint32_t * bw)
{
    lv_fs_res_t res = LV_FS_RES_NOT_IMP;
    int bwritten = ((File*)(((lv_fs_file_t*)file_p)->file_d))->write((uint8_t *)buf,btw);
    if (bwritten == -1)
        res = LV_FS_RES_FS_ERR;
    else {
        res = LV_FS_RES_OK;
        *bw = bwritten;
    }
    return res;
}

/**
 * Set the read write pointer. Also expand the file size if necessary.
 * @param drv       pointer to a driver where this function belongs
 * @param file_p    pointer to a file_t variable. (opened with fs_open )
 * @param pos       the new position of read write pointer
 * @param whence    tells from where to interpret the `pos`. See @lv_fs_whence_t
 * @return          LV_FS_RES_OK: no error or  any error from @lv_fs_res_t enum
 */
static lv_fs_res_t fs_seek(lv_fs_drv_t * drv, void * file_p, uint32_t pos, lv_fs_whence_t whence)
{
    lv_fs_res_t res = LV_FS_RES_NOT_IMP;
    // Arduino SD library can only seek to absolute position
    uint32_t offset;
    switch ( whence) {
        case LV_FS_SEEK_SET:
            offset = pos;
            break;
        case LV_FS_SEEK_CUR:
            offset = ((File*)(((lv_fs_file_t*)file_p)->file_d))->position() + pos;
            break;
         case LV_FS_SEEK_END:
            offset = ((File*)(((lv_fs_file_t*)file_p)->file_d))->size() + pos;
            break;
   }
    sprintf(buff,"Seek %d to %d",whence,offset);
    LV_LOG_USER(buff);
    
    bool rc =  ((File*)(((lv_fs_file_t*)file_p)->file_d))->seek(offset);
    if (rc) {
        res = LV_FS_RES_OK;
        sprintf(buff,"offset %d", offset = ((File*)(((lv_fs_file_t*)file_p)->file_d))->position());
        LV_LOG_USER(buff);
    } 
    return res;
}
/**
 * Give the position of the read write pointer
 * @param drv       pointer to a driver where this function belongs
 * @param file_p    pointer to a file_t variable
 * @param pos_p     pointer to store the result
 * @return          LV_FS_RES_OK: no error or  any error from @lv_fs_res_t enum
 */
static lv_fs_res_t fs_tell(lv_fs_drv_t * drv, void * file_p, uint32_t * pos_p)
{
    lv_fs_res_t res = LV_FS_RES_NOT_IMP;
    switch (drv->letter) {
        case 'S':
            *pos_p = ((File*)(((lv_fs_file_t*)file_p)->file_d))->position();
            res = LV_FS_RES_OK;
            break;
    }

    return res;
}

/**
 * Initialize a 'lv_fs_dir_t' variable for directory reading
 * @param drv       pointer to a driver where this function belongs
 * @param path      path to a directory
 * @return          pointer to the directory read descriptor or NULL on error
 */
static void * fs_dir_open(lv_fs_drv_t * drv, const char * path)
{
    void *dir = NULL;
    LV_LOG_USER(path);
    switch (drv->letter)
    {
    case 'S':
        sprintf(buff, "Open SD path %s\n", path);
        LV_LOG_USER(buff);
        sd_dir = SD.open(path);
        if (sd_dir && sd_dir.isDirectory())
        {
            lv_dir.drv = drv;
            lv_dir.dir_d = (void *)&sd_dir;
            dir = &lv_dir;
        }
        break;
    }
    // returns pointer to directory object
    return dir;
}

/**
 * Read the next filename form a directory.
 * The name of the directories will begin with '/'
 * @param drv       pointer to a driver where this function belongs
 * @param rddir_p   pointer to an initialized 'lv_fs_dir_t' variable
 * @param fn        pointer to a buffer to store the filename
 * @param fn_len    length of the buffer to store the filename
 * @return          LV_FS_RES_OK: no error or  any error from @lv_fs_res_t enum
 */
static lv_fs_res_t fs_dir_read(lv_fs_drv_t * drv, void * rddir_p, char * fn, uint32_t fn_len)
{
    lv_fs_res_t res = LV_FS_RES_NOT_IMP;
    switch (drv->letter)
    {
    case 'S':
        memset(fn, 0, fn_len);
        sd_file = ((File *)(((lv_fs_dir_t *)rddir_p)->dir_d))->openNextFile();
        if (sd_file)
        {
            if (sd_file.isDirectory())
                strcpy(fn,"DIR:");
            strncat(fn, sd_file.name(),min(strlen(sd_file.name()),fn_len-1));
            LV_LOG_USER(sd_file.name());
        }
        res = LV_FS_RES_OK;
        break;
    }
    return res;
}

/**
 * Close the directory reading
 * @param drv       pointer to a driver where this function belongs
 * @param rddir_p   pointer to an initialized 'lv_fs_dir_t' variable
 * @return          LV_FS_RES_OK: no error or  any error from @lv_fs_res_t enum
 */
static lv_fs_res_t fs_dir_close(lv_fs_drv_t * drv, void * rddir_p)
{
    lv_fs_res_t res = LV_FS_RES_NOT_IMP;
    sprintf(buff,"Close dir %c\n",drv->letter);
    LV_LOG_USER(buff);
    switch (drv->letter)
    {
    case 'S':
        ((File *)(((lv_fs_dir_t *)rddir_p)->dir_d))->close();
        res = LV_FS_RES_OK;
        break;
    }
    return res;
}

#else /*Enable this file at the top*/

/*This dummy typedef exists purely to silence -Wpedantic.*/
typedef int keep_pedantic_happy;
#endif

/**
 * @brief Check if SD opened or not
 * 
 * @param drv 
 * @return lv_fs_res_t 
 */
static bool fs_ready(lv_fs_drv_t * drv) {
    bool rc = false;
    switch (drv->letter) {
        case 'S':
            rc = SDready;
            break;
        default:
            break;
    }
    return rc;
}
