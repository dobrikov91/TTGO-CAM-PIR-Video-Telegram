/*******************************************************************

 ESP32-CAM-Video-Telegram

  This program records an mjpeg avi video in the psram of a ESP32-CAM, and sends a jpeg and a avi video to Telegram.

  https://github.com/jameszah/ESP32-CAM-Video-Telegram is licensed under the
    GNU General Public License v3.0

  by James Zahary  June 1, 2020
  jamzah.plc@gmail.com


  The is Arduino code, with standard setup for ESP32-CAM
    - Board ESP32 Wrover Module
    - Partition Scheme Huge APP (3MB No OTA)
    - or with AI Thinker ESP32-CAM
  Compiled with Arduino 1.8.13 and arduino-esp32 1.0.6 (latest release version) on Jun 10, 2021

  Jan 18, 2022 ver 8.9
  - updates from Arduino 1.8.19
    - return from void problem re-runs the function if you dont do a return ???
      https://stackoverflow.com/questions/22742581/warning-control-reaches-end-of-non-void-function-wreturn-type
  - updates for esp32-arduino 2.0.2
    - bug with 2.0.2 handshake timeout - added timeout resets in this file as a workaround
      https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot/issues/270#issuecomment-1003795884
   - updates for esp32-arduino 2.0.2
     - esp-camera seems to have changed to fill all free fb buffers in sequence, so must empty them to get a snapshot

  Based on these two:

  https://github.com/jameszah/ESP32-CAM-Video-Recorder-junior
  https://github.com/jameszah/ESP32-CAM-Video-Recorder

  and using a modified old version of:
  https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot



~~~~~~~~~~~~

Using library WiFi at version 2.0.0 in folder: C:\ArduinoPortable\arduino-1.8.19\portable\packages\esp32\hardware\esp32\2.0.2\libraries\WiFi
Using library WiFiClientSecure at version 2.0.0 in folder: C:\ArduinoPortable\arduino-1.8.19\portable\packages\esp32\hardware\esp32\2.0.2\libraries\WiFiClientSecure
Using library ArduinoJson at version 6.18.5 in folder: C:\ArduinoPortable\arduino-1.8.19\portable\sketchbook\libraries\ArduinoJson
Using library ESPmDNS at version 2.0.0 in folder: C:\ArduinoPortable\arduino-1.8.19\portable\packages\esp32\hardware\esp32\2.0.2\libraries\ESPmDNS
"C:\\ArduinoPortable\\arduino-1.8.19\\portable\\packages\\esp32\\tools\\xtensa-esp32-elf-gcc\\gcc8_4_0-esp-2021r2/bin/xtensa-esp32-elf-size" -A "C:\\Users\\James\\AppData\\Local\\Temp\\arduino_build_57156/ESP32-CAM-Video-Telegram_8.9.ino.elf"
Sketch uses 956193 bytes (30%) of program storage space. Maximum is 3145728 bytes.
Global variables use 63080 bytes (19%) of dynamic memory, leaving 264600 bytes for local variables. Maximum is 327680 bytes.
C:\ArduinoPortable\arduino-1.8.19\portable\packages\esp32\tools\esptool_py\3.1.0/esptool.exe --chip esp32 --port COM7 --baud 460800 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 80m --flash_size 4MB 0xe000 C:\ArduinoPortable\arduino-1.8.19\portable\packages\esp32\hardware\esp32\2.0.2/tools/partitions/boot_app0.bin 0x1000 C:\Users\James\AppData\Local\Temp\arduino_build_57156/ESP32-CAM-Video-Telegram_8.9.ino.bootloader.bin 0x10000 C:\Users\James\AppData\Local\Temp\arduino_build_57156/ESP32-CAM-Video-Telegram_8.9.ino.bin 0x8000 C:\Users\James\AppData\Local\Temp\arduino_build_57156/ESP32-CAM-Video-Telegram_8.9.ino.partitions.bin


*******************************************************************/


/*******************************************************************
  -  original opening from Brian Lough telegram bot demo

   A Telegram bot for taking a photo with an ESP32Cam

   Parts used:
   ESP32-CAM module* - http://s.click.aliexpress.com/e/bnXR1eYs

    = Affiliate Links

   Note:
   - Make sure that you have either selected ESP32 Wrover Module,
           or another board which has PSRAM enabled
   - Choose "Huge App" partion scheme

   Some of the camera code comes from Rui Santos:
   https://randomnerdtutorials.com/esp32-cam-take-photo-save-microsd-card/

   Written by Brian Lough
    YouTube: https://www.youtube.com/brianlough
    Tindie: https://www.tindie.com/stores/brianlough/
    Twitter: https://twitter.com/witnessmenow

    Aug 7, 2020 - jz
    Mods to library and example to demonstrate
     - bugfix with missing println statement
     - method to send big and small jpegs
     - sending a caption with a pictire

    Mar 26, 2021 - jz
    Mods for esp32 version 1.05
    See line 250 of UniversalTelegramBot.cpp in the current github software
    https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot/issues/235#issue-842397567
    See https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot/blob/65e6f826cbab242366d69f00cebc25cdd1e81305/src/UniversalTelegramBot.cpp#L250


*******************************************************************/

// ----------------------------
// Standard Libraries - Already Installed if you have ESP32 set up
// ----------------------------

#include <WiFiClientSecure.h>
#include "esp_camera.h"

// ----------------------------
// Additional Libraries - each one of these will need to be installed.
// ----------------------------

//#include <UniversalTelegramBot.h>
#include "UniversalTelegramBot.h"  // use local library which is a modified copy of an old version
// Library for interacting with the Telegram API
// Search for "Telegegram" in the Library manager and install
// The universal Telegram library
// https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot

#include <ArduinoJson.h>
// Library used for parsing Json from the API responses
// Search for "Arduino Json" in the Arduino Library manager
// https://github.com/bblanchon/ArduinoJson

// own headers
#include "credentials.h"
#include "avi.h"
#include "config.h"
#include "hardware.h"

bool reboot_request = false;

#include "esp_system.h"

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

int Bot_mtbs = 5000; //mean time between scan messages
long Bot_lasttime;   //last time messages' scan has been done

camera_fb_t * fb = NULL;
camera_fb_t * vid_fb = NULL;

TaskHandle_t the_camera_loop_task;
void the_camera_loop(void* pvParameter) ;
static void IRAM_ATTR PIR_ISR(void* arg) ;

bool video_ready = false;
bool picture_ready = false;
bool active_interupt = false;
bool pir_enabled = false;
bool avi_enabled = false;

time_t now;
struct tm timeinfo;
char strftime_buf[64];

////////////////////////////////  send photo as 512 byte blocks or jzblocksize
int currentByte;
uint8_t* fb_buffer;
size_t fb_length;

bool isMoreDataAvailable() {
  return (fb_length - currentByte);
}

uint8_t getNextByte() {
  currentByte++;
  return (fb_buffer[currentByte - 1]);
}

////////////////////////////////  send avi as 512 byte blocks or jzblocksize
int avi_ptr;
uint8_t* avi_buf;
size_t avi_len;

bool avi_more() {
  return (avi_len - avi_ptr);
}

uint8_t avi_next() {
  avi_ptr++;
  return (avi_buf[avi_ptr - 1]);
}

bool dataAvailable = false;

///////////////////////////////

void handleNewMessages(int numNewMessages) {
  //Serial.println("handleNewMessages");
  //Serial.println(String(numNewMessages));

  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    String text = bot.messages[i].text;

    Serial.printf("\nGot a message %s\n", text);

    String from_name = bot.messages[i].from_name;
    if (from_name == "") from_name = "Guest";

    String hi = "Got: ";
    hi += text;
    bot.sendMessage(chat_id, hi, "Markdown");
    client.setHandshakeTimeout(120000);

    if (text == "/status") {
      String stat = "Device: " + devstr + "\nVer: " + String(vernum) + "\nRssi: " + String(WiFi.RSSI()) + "\nip: " +  WiFi.localIP().toString() + "\nEnabled: " + pir_enabled + "\nAvi Enabled: " + avi_enabled;
      if (frame_interval == 0) {
        stat = stat + "\nFast 3 sec";
      } else if (frame_interval == 125) {
        stat = stat + "\nMed 10 sec";
      } else {
        stat = stat + "\nSlow 40 sec";
      }
      stat = stat + "\nQuality: " + quality;

      bot.sendMessage(chat_id, stat, "");
    }

    if (text == "/reboot") {
      reboot_request = true;
    }

    if (text == "/enable") {
      pir_enabled = true;
    }

    if (text == "/disable") {
      pir_enabled = false;
    }

    if (text == "/enavi") {
      avi_enabled = true;
    }

    if (text == "/disavi") {
      avi_enabled = false;
    }

    if (text == "/fast") {
      max_frames = 150;
      frame_interval = 0;
      speed_up_factor = 0.5;
      pir_enabled = true;
      avi_enabled = true;
    }

    if (text == "/med") {
      max_frames = 150;
      frame_interval = 125;
      speed_up_factor = 1;
      pir_enabled = true;
      avi_enabled = true;
    }

    if (text == "/slow") {
      max_frames = 150;
      frame_interval = 500;
      speed_up_factor = 5;
      pir_enabled = true;
      avi_enabled = true;
    }

    /*
    if (fb) {
      esp_camera_fb_return(fb);
      Serial.println("Return an fb ???");
      if (fb) {
        esp_camera_fb_return(fb);
        Serial.println("Return another fb ?");
      }
    }
    */

    for (int j = 0; j < 4; j++) {
    camera_fb_t * newfb = esp_camera_fb_get();
    if (!newfb) {
      Serial.println("Camera Capture Failed");
    } else {
      //Serial.print("Pic, len="); Serial.print(newfb->len);
      //Serial.printf(", new fb %X\n", (long)newfb->buf);
      esp_camera_fb_return(newfb);
      delay(10);
    }
  }
    if ( text == "/photo" || text == "/caption" ) {

      fb = NULL;

      // Take Picture with Camera
      fb = esp_camera_fb_get();
      if (!fb) {
        Serial.println("Camera capture failed");
        bot.sendMessage(chat_id, "Camera capture failed", "");
        return;
      }

      currentByte = 0;
      fb_length = fb->len;
      fb_buffer = fb->buf;

      if (text == "/caption") {

        Serial.println("\n>>>>> Sending with a caption, bytes=  " + String(fb_length));

        String sent = bot.sendMultipartFormDataToTelegramWithCaption("sendPhoto", "photo", "img.jpg",
                      "image/jpeg", "Your photo", chat_id, fb_length,
                      isMoreDataAvailable, getNextByte, nullptr, nullptr);

        Serial.println("\ndone!");

      } else {

        Serial.println("\n>>>>> Sending, bytes=  " + String(fb_length));

        bot.sendPhotoByBinary(chat_id, "image/jpeg", fb_length,
                              isMoreDataAvailable, getNextByte,
                              nullptr, nullptr);

        dataAvailable = true;

        Serial.println("\ndone!");
      }
      esp_camera_fb_return(fb);
    }

    if (text == "/vga" ) {

      fb = NULL;

      //sensor_t * s = esp_camera_sensor_get();
      //s->set_framesize(s, FRAMESIZE_VGA);

      Serial.println("\n\n\nSending VGA");

      // Take Picture with Camera
      fb = esp_camera_fb_get();
      if (!fb) {
        Serial.println("Camera capture failed");
        bot.sendMessage(chat_id, "Camera capture failed", "");
        return;
      }

      currentByte = 0;
      fb_length = fb->len;
      fb_buffer = fb->buf;

      Serial.println("\n>>>>> Sending as 512 byte blocks, with jzdelay of 0, bytes=  " + String(fb_length));

      bot.sendPhotoByBinary(chat_id, "image/jpeg", fb_length,
                            isMoreDataAvailable, getNextByte,
                            nullptr, nullptr);

      esp_camera_fb_return(fb);
    }


    if (text == "/clip") {

      // record the video
      bot.longPoll =  0;

      xTaskCreatePinnedToCore( the_camera_loop, "the_camera_loop", 10000, NULL, 1, &the_camera_loop_task, 1);
      //xTaskCreatePinnedToCore( the_camera_loop, "the_camera_loop", 10000, NULL, 1, &the_camera_loop_task, 0);  //v8.5

      if ( the_camera_loop_task == NULL ) {
        //vTaskDelete( xHandle );
        Serial.printf("do_the_steaming_task failed to start! %d\n", the_camera_loop_task);
      }
    }

    if (text == "/start") {
      String welcome = "ESP32Cam Telegram bot.\n\n";
      welcome += "/photo: take a photo\n";
      welcome += "/caption: photo with caption\n";
      welcome += "/clip: short video clip\n";
      welcome += "\n Configure the clip\n";
      welcome += "/enable: enable pir\n";
      welcome += "/disable: disable pir\n";
      welcome += "/enavi: enable avi\n";
      welcome += "/disavi: disable avi\n";
      welcome += "\n/fast: 25 fps - 3  sec - play .5x speed\n";
      welcome += "/med: 8  fps - 10 sec - play 1x speed\n";
      welcome += "/slow: 2  fps - 40 sec - play 5x speed\n";
      welcome += "\n/status: status\n";
      welcome += "/reboot: reboot\n";
      welcome += "/start: start\n";
      bot.sendMessage(chat_id, welcome, "Markdown");
    }
  }
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// the_camera_loop()

void the_camera_loop (void* pvParameter) {
  for (int i = 0; i < 3; ++i) {
    esp_camera_fb_return(vid_fb);
    vid_fb = esp_camera_fb_get();
    if (!vid_fb) {
      Serial.println("Camera capture failed");
      for (int i = 0; i < chat_ids.size(); ++i) {
        bot.sendMessage(chat_ids[i], "Camera capture failed", "");
      }
      return;
    }
  }
  picture_ready = true;

  if (avi_enabled) {
    time(&now);
    localtime_r(&now, &timeinfo);
    strftime(strftime_buf, sizeof(strftime_buf), "CatCam %F %H.%M.%S.avi", &timeinfo);

    record_movie();
    video_ready = true;
  }
  Serial.println("Deleting the camera task");
  delay(100);
  vTaskDelete(the_camera_loop_task);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// setup some interupts during reboot
//
//  int read13 = digitalRead(13); -- pir for video

int PIRpin = 19;

static void setupinterrupts() {
  pinMode(PIRpin, INPUT_PULLDOWN);

  Serial.print("Setup PIRpin = ");
  for (int i = 0; i < 5; i++) {
    Serial.print( digitalRead(PIRpin) ); Serial.print(", ");
  }
  Serial.println(" ");

  esp_err_t err = gpio_isr_handler_add((gpio_num_t)PIRpin, &PIR_ISR, NULL);

  if (err != ESP_OK) Serial.printf("gpio_isr_handler_add failed (%x)", err);
  gpio_set_intr_type((gpio_num_t)PIRpin, GPIO_INTR_POSEDGE);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//  PIR_ISR - interupt handler for PIR  - starts or extends a video
//
static void IRAM_ATTR PIR_ISR(void* arg) {
  int PIRstatus = digitalRead(PIRpin) + digitalRead(PIRpin) + digitalRead(PIRpin) ;
  if (PIRstatus == 3) {
    Serial.print("PIR Interupt>> "); Serial.println(PIRstatus);

    if (!active_interupt && pir_enabled) {
      active_interupt = true;
      Serial.print("PIR Interupt ... start recording ... ");
      xTaskCreatePinnedToCore( the_camera_loop, "the_camera_loop", 10000, NULL, 1, &the_camera_loop_task, 1);
      //xTaskCreatePinnedToCore( the_camera_loop, "the_camera_loop", 10000, NULL, 1, &the_camera_loop_task, 0);  //v8.5

      if ( the_camera_loop_task == NULL ) {
        Serial.printf("do_the_steaming_task failed to start! %d\n", the_camera_loop_task);
      }
    }
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("---------------------------------");
  Serial.printf("TTGO-CAM PIR Video Telegram %s\n", vernum);
  Serial.println("---------------------------------");

  //pinMode(12, INPUT_PULLUP);        // pull this down to stop recording

  allocatePsram();
  if (!setupCamera()) {
    Serial.println("Camera Setup Failed!");
    while (true) {
      delay(100);
    }
  }


  for (int j = 0; j < 3; j++) {
    camera_fb_t * fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Camera Capture Failed");
    } else {
      Serial.print("Pic, len="); Serial.print(fb->len);
      Serial.printf(", new fb %X\n", (long)fb->buf);
      esp_camera_fb_return(fb);
      delay(50);
    }
  }

  bool wifi_status = initWifi();

  // Make the bot wait for a new message for up to 60seconds
  bot.longPoll = 60;
  //bot.longPoll = 5;

  client.setInsecure();

  setupinterrupts();

  String stat = "Reboot\nDevice: " + devstr + "\nVer: " + String(vernum) + "\nRssi: " + String(WiFi.RSSI()) + "\nip: " +  WiFi.localIP().toString() + "\n/start";
  for (int i = 0; i < chat_ids.size(); ++i) {
    bot.sendMessage(chat_ids[i], stat, "");
  }

  pir_enabled = true;
  avi_enabled = true;
}

int loopcount = 0;

void loop() {
  loopcount++;
  delay(10);

  //client.setHandshakeTimeout(120000); // workaround for esp32-arduino 2.02 bug https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot/issues/270#issuecomment-1003795884

  if (reboot_request) {
    String stat = "Rebooting on request\nDevice: " + devstr + "\nVer: " + String(vernum) + "\nRssi: " + String(WiFi.RSSI()) + "\nip: " +  WiFi.localIP().toString() ;
    for (int i = 0; i < chat_ids.size(); ++i) {
      bot.sendMessage(chat_ids[i], stat, "");
    }
    delay(10000);
    ESP.restart();
  }

  if (picture_ready) {
    picture_ready = false;
    send_the_picture();
  }

  if (video_ready) {
    video_ready = false;
    send_the_video();
  }

  if (botReacts && millis() > Bot_lasttime + Bot_mtbs) {

    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("***** WiFi reconnect *****");
      WiFi.reconnect();
      delay(5000);
      if (WiFi.status() != WL_CONNECTED) {
        Serial.println("***** WiFi rerestart *****");
        initWifi();
      }
    }

    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages) {
      //Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    Bot_lasttime = millis();
  }
}


void send_the_picture() {
  fb_length = vid_fb->len;
  fb_buffer = vid_fb->buf;

  Serial.println("\n>>>>> Sending as 512 byte blocks, with jzdelay of 0, bytes=  " + String(fb_length));

  String messageText = "Telegram Request";
  if (active_interupt) {
    messageText = "Cot p'et!";
  }

  for (int i = 0; i < chat_ids.size(); ++i) {
    currentByte = 0;
    String sent = bot.sendMultipartFormDataToTelegramWithCaption("sendPhoto", "photo", "img.jpg",
                  "image/jpeg", messageText, chat_ids[i], fb_length,
                  isMoreDataAvailable, getNextByte, nullptr, nullptr);
  }

  esp_camera_fb_return(vid_fb);
  bot.longPoll =  0;
  if (!avi_enabled) active_interupt = false;
}

void send_the_video() {
  Serial.println("\n\n\nSending clip with caption");
  Serial.println("\n>>>>> Sending as 512 byte blocks, with a caption, and with jzdelay of 0, bytes=  " + String(getPsramAviLen()));
  avi_buf = getPsramAviBuf();
  avi_len = getPsramAviLen();

  String messageText = "Drinking process";
  for (int i = 0; i < chat_ids.size(); ++i) {
    avi_ptr = 0;
    String sent = bot.sendMultipartFormDataToTelegramWithCaption("sendDocument", "document", strftime_buf,
                  "image/jpeg", messageText, chat_ids[i], getPsramAviLen(),
                  avi_more, avi_next, nullptr, nullptr);
    Serial.println("\ndone");
  }

  bot.longPoll = 5;
  active_interupt = false;
}
