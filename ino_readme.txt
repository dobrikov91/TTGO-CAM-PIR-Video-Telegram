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