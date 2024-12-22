#pragma once

#include "esp_camera.h"

void allocatePsram();
uint8_t * getPsramAviBuf();
size_t getPsramAviLen();

void start_avi();
void another_save_avi(camera_fb_t * fb);
void end_avi();
bool record_movie();
