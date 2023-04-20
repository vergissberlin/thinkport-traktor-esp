#include <Arduino.h>
#include <ArduinoJson.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <FastLED.h>
#include <PubSubClient.h>
//#include "ServoEasing.hpp"
#include <Wire.h>
#include <WiFiManager.h>

// Import configuration
#include "config.h"

// WiFi
WiFiManager wifiManager;
WiFiClient espClient;

// MQTT
PubSubClient client(espClient);

// LED
CRGB leds[LED_NUM];

// OLED Display
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Servo
// ServoEasing ServoArm;

// Variables
const char *angle;
const char *led_0 = "black";
const char *led_1 = "black";
const char *led_2 = "black";
const char *display_0;
const char *display_1;
const char *display_2;

/**
 * @brief Print the project header to the display
 */
void msgHeader()
{
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.print("Tractor");
    display.setTextSize(1);
    display.print(" v2.0");
    display.setCursor(0, 18);
    display.setTextSize(1);
    display.setTextColor(WHITE);
}

/**
 * @brief Set the colour of a LED  using fastLED
 *
 * @param ledNum Number of the LED
 * @param colour Colour of the LED as string
 */
void ledSetColor(int ledNum, const char *colour)
{
    if (strcmp(colour, "red") == 0)
        leds[ledNum] = CRGB::Green; // Red and green are swapped
    else if (strcmp(colour, "green") == 0)
        leds[ledNum] = CRGB::Red; // Red and green are swapped
    else if (strcmp(colour, "blue") == 0)
        leds[ledNum] = CRGB::Blue;
    else if (strcmp(colour, "yellow") == 0)
        leds[ledNum] = CRGB::Yellow;
    else if (strcmp(colour, "orange") == 0)
        leds[ledNum] = CRGB::Orange;
    else if (strcmp(colour, "purple") == 0)
        leds[ledNum] = CRGB::Purple;
    else if (strcmp(colour, "white") == 0)
        leds[ledNum] = CRGB::White;
    else
        leds[ledNum] = CRGB::Black;
}

/**
 * @brief Callback function for MQTT
 *
 * @param topic The topic with the tractor data
 * @param payload The payload with the tractor data
 * @param length The length of the payload
 */
void mqttCallback(char *topic, byte *payload, unsigned int length)
{
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.println("] ");
    StaticJsonDocument<200> data;
    DeserializationError error = deserializeJson(data, payload);
    if (error)
    {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return;
    }

    // Data
    angle = data["angle"];
    led_0 = data["led"][0];
    led_1 = data["led"][1];
    led_2 = data["led"][2];
    display_0 = data["display"][0];
    display_1 = data["display"][1];
    display_2 = data["display"][2];

    // Display
    msgHeader();
    display.drawRect(2, 20, display.width() - 2, 44, SSD1306_WHITE);
    display.drawFastHLine(2, 38, display.width() - 2, SSD1306_WHITE);

    display.setCursor(8, 22);
    display.setTextColor(WHITE);
    display.setTextSize(2);
    display.println(display_0);
    display.setTextSize(1);
    display.setCursor(8, 42);
    display.println(display_1);
    display.setCursor(8, 52);
    display.println(display_2);
    display.display();

    // Servo
    Serial.print("Arm angle: ");
    Serial.println(angle);

    if ((char)payload[0] == '1')
        digitalWrite(BUILTIN_LED, LOW);
    else
        digitalWrite(BUILTIN_LED, HIGH);
}

/**
 * @brief Connect to MQTT
 */
void mqttConnect()
{
    while (!client.connected())
    {
        if (client.connect(MQTT_CLIENTID, MQTT_USER, MQTT_PASSWORD))
        {
            client.subscribe(MQTT_TOPIC);
        }
        else
        {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            delay(5000);
        }
    }
}

void setup()
{
    // Serial
    Serial.begin(115200);
    Serial.println("Starting up ...");

    //  LED
    FastLED.setBrightness(LED_BRIGHTNESS);
    FastLED.addLeds<WS2812, LED_DATA, RGB>(leds, LED_NUM);
    FastLED.clear();

    // Display
    Wire.begin();
    if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS, true, false))
    {
        Serial.println(F("SSD1306 allocation failed"));
        for (;;)
            ;
    }
    display.display();
    delay(2000);
    msgHeader();
    display.println("Display activated");
    display.display();

    // Servo
    // ServoArm.attach(SERVO_PIN);
    Serial.println("Servo configured");
    display.println("Servo configured");

    // WiFi
    display.println("WiFi activated");
    display.display();
    wifiManager.autoConnect("Bayer Tractor");
    if (wifiManager.autoConnect())
    {
        Serial.println("WiFi connected");
        display.println("WiFi connected");
    }
    else
    {
        Serial.println("WiFi connection failed");
        display.println("WiFi connection failed");
    }
    display.display();

    // 4. MQTT
    display.println("MQTT connection");
    display.display();
    client.setServer(MQTT_SERVER, MQTT_PORT);
    client.setCallback(mqttCallback);
    mqttConnect();
    display.println("MQTT connected");
    display.display();
}

void loop()
{
    // LED
    ledSetColor(0, led_0);
    ledSetColor(1, led_1);
    ledSetColor(2, led_2);
    FastLED.show();

    // Servo
    // ServoArm.easeTo(135, 40); // Blocking call, runs on all platforms
    // ServoArm.easeTo(45, 40); // Blocking call, runs on all platforms

    // MQTT
    mqttConnect();

    // WiFi
    client.loop();
}
