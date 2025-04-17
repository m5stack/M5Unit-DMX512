/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */

/**
 * @file M5Core_DMXWrite.ino
 * @author Tinyu (tinyu@m5stack.com)
 * @brief Send data via Unit DMX512 with M5Core
 * @version 0.0.1
 * @date 2024-08-22
 *
 * @Hardwares: M5Core + Unit DMX512
 * @Platform Version: Arduino M5Stack Board Manager v2.1.0
 * @Dependent Library:
 * esp_dmx: https://github.com/someweisguy/esp_dmx/tree/v4.1.0
 */

#include <M5Stack.h>
#include <esp_dmx.h>

int transmitPin    = 17;
int receivePin     = 16;
int enablePin      = -1;
dmx_port_t dmxPort = 1;

byte data[DMX_PACKET_SIZE];
unsigned long lastUpdate = millis();
bool setupSuccess = false, baudState = false;  // 用于标志初始化是否成功
uint8_t sendCounter = 0;

void setup() {
    M5.begin(true, false, true, false);
    M5.Power.begin();
    M5.Lcd.setTextColor(YELLOW);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(80, 10);
    M5.Lcd.println("Unit-DMX TEST");
    M5.Lcd.setCursor(120, 35);
    M5.Lcd.setTextColor(BLUE);
    M5.Lcd.println("MASTER");

    dmx_config_t config               = DMX_CONFIG_DEFAULT;
    dmx_personality_t personalities[] = {};
    int personality_count             = 0;
    if (dmx_driver_install(dmxPort, &config, personalities, personality_count) != true) {
        Serial.println("Failed to install DMX driver");
        M5.Lcd.println("Failed to install DMX driver");
        return;
    }

    if (dmx_set_pin(dmxPort, transmitPin, receivePin, enablePin) != true) {
        Serial.println("Failed to set DMX pin");
        M5.Lcd.println("Failed to set DMX pin");
        return;
    }
    M5.Lcd.setCursor(3, 55);
    setupSuccess = true;  // 初始化成功
}

void loop() {
    M5.update();
    if (!setupSuccess) {
        M5.Lcd.setCursor(40, 225);
        M5.Lcd.setTextColor(RED);
        M5.Lcd.println("Restart");
        M5.Lcd.setCursor(100, 85);
        M5.Lcd.setTextColor(RED);
        M5.Lcd.println("Please restrat");
        if (M5.BtnA.wasReleased()) esp_restart();
    } else {
        if (baudState == false) {
            M5.Lcd.setCursor(35, 85);
            M5.Lcd.setTextColor(GREEN);
            M5.Lcd.println("Pls select baud rate");
            M5.Lcd.setCursor(30, 225);
            M5.Lcd.setTextColor(WHITE);
            M5.Lcd.println("250Kbps 500Kbps");

            if (M5.BtnA.wasPressed()) {
                dmx_set_baud_rate(dmxPort, DMX_BAUD_RATE);
                baudState = true;
                M5.Lcd.fillRect(0, 55, 320, 190, BLACK);
                M5.Lcd.setCursor(210, 35);
                M5.Lcd.println("250Kbps");
            }
            if (M5.BtnB.wasPressed()) {
                dmx_set_baud_rate(dmxPort, DMX_BAUD_RATE * 2);
                baudState = true;
                M5.Lcd.fillRect(0, 55, 320, 190, BLACK);
                M5.Lcd.setCursor(210, 35);
                M5.Lcd.println("500Kbps");
            }
        } else {
            unsigned long now = millis();
            if (now - lastUpdate >= 20) {
                for (int i = 1; i < DMX_PACKET_SIZE; i++) {
                    data[i]++;
                }
                dmx_write(dmxPort, data, DMX_PACKET_SIZE);
                sendCounter++;
                Serial.printf("Sending DMX 0x%02X  sendCounter:%d\n", data[1], sendCounter);
                M5.Lcd.printf("%X ", data[1]);
                if (sendCounter % 100 == 0) {
                    M5.Lcd.setCursor(3, 55);
                    M5.Lcd.fillRect(0, 51, 320, 190, BLACK);
                }
                lastUpdate = now;
            }
            dmx_send_num(dmxPort, DMX_PACKET_SIZE);
            dmx_wait_sent(dmxPort, DMX_TIMEOUT_TICK);
        }
    }
}