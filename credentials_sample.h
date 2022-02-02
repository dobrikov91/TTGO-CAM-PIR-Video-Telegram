/*
 * Copy this file to credentials.h and fill in your data
 */

#pragma once

#include <vector>
// Initialize Wifi connection to the router and Telegram BOT

char ssid[] = "WiFiName";     // your network SSID (name)
char password[] = "wifiPASS"; // your network key
// https://sites.google.com/a/usapiens.com/opnode/time-zones  -- find your timezone here
String TIMEZONE = "MSK-3MSD,M3.5.0/2,M10.5.0/3";

// you can enter your home chat_id, so the device can send you a reboot message, otherwise it responds to the chat_id talking to telegram
std::vector <String> chat_ids {"111222", "111222333"};
#define BOTtoken "xxxxxxxxxx:yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy"  // your Bot Token (Get from Botfather)

// see here for information about getting free telegram credentials
// https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot
// https://randomnerdtutorials.com/telegram-esp32-motion-detection-arduino/