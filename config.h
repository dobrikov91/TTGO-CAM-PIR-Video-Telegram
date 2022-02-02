#pragma once

#include "esp_camera.h"

static const char vernum[] = "pir-cam 8.9";
static String devstr =  "deskpir";
static int max_frames = 150;
static framesize_t configframesize = FRAMESIZE_SVGA; // FRAMESIZE_ + QVGA|CIF|VGA|SVGA|XGA|SXGA|UXGA
static int frame_interval = 500; // 0 = record at full speed, 100 = 100 ms delay between frames
static int quality = 10;
static int qualityconfig = 5;
static float speed_up_factor = 2.5; // 1 = play at realtime, 0.5 = slow motion, 10 = speedup 10x
static int framesize = configframesize; //FRAMESIZE_SVGA;

static int avi_buf_size = 3000 * 1024; // = 3000 kb = 60 * 50 * 1024;
static int idx_buf_size = 200 * 10 + 20;

static bool botReacts = false;

/*
 * Setup correct pins for your camera module
 */

/*
#define CAMERA_MODEL_AI_THINKER
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22
*/
#define CAMERA_MODEL_ESP_EYE
#define PWDN_GPIO_NUM    -1
#define RESET_GPIO_NUM   -1
#define XCLK_GPIO_NUM    4
#define SIOD_GPIO_NUM    18
#define SIOC_GPIO_NUM    23

#define Y9_GPIO_NUM      36
#define Y8_GPIO_NUM      37
#define Y7_GPIO_NUM      38
#define Y6_GPIO_NUM      39
#define Y5_GPIO_NUM      35
#define Y4_GPIO_NUM      14
#define Y3_GPIO_NUM      13
#define Y2_GPIO_NUM      34
#define VSYNC_GPIO_NUM   5
#define HREF_GPIO_NUM    27
#define PCLK_GPIO_NUM    25