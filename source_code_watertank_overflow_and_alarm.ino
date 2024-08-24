/* Fill-in your Template ID (only if using Blynk.Cloud) */
#define BLYNK_TEMPLATE_ID "TMPL3hwr1k8qD"
#define BLYNK_TEMPLATE_NAME "waterlevel"
#define BLYNK_AUTH_TOKEN "ZUCngmnS_y4qif8lDcEkr6R9t1ZPusEh"

#include <Arduino.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <AceButton.h>

using namespace ace_button;

// Your WiFi credentials.
// Set password to "" for open networks.
#define WIFI_SSID "redmi pro 8"
#define WIFI_PASS "prajwal@07"

// Set Water Level Distance in CM
int emptyTankDistance = 70; // Distance when tank is empty
int fullTankDistance = 30; // Distance when tank is full

// Set trigger value in percentage
int triggerPer = 10; // Alarm will start when water level drops below triggerPer

// Define connections to sensor
#define TRIGPIN    27 // D27
#define ECHOPIN    26 // D26
#define wifiLed    2  // D2
#define ButtonPin1 12 // D12
#define BuzzerPin  13 // D13
#define GreenLed   14 // D14

// Change the virtual pins according to the rooms
#define VPIN_BUTTON_1 V1
#define VPIN_BUTTON_2 V2

float duration;
float distance;
int waterLevelPer;
bool toggleBuzzer = HIGH; // Define to remember the toggle state

char auth[] = BLYNK_AUTH_TOKEN;
ButtonConfig config1;
AceButton button1(&config1);

void handleEvent1(AceButton*, uint8_t, uint8_t);
BlynkTimer timer;

void checkBlynkStatus() { // Called every 3 seconds by SimpleTimer
    bool isconnected = Blynk.connected();
    if (isconnected == false) {
        // Serial.println("Blynk Not Connected");
        digitalWrite(wifiLed, LOW);
    }
    if (isconnected == true) {
        digitalWrite(wifiLed, HIGH);
        // Serial.println("Blynk Connected");
    }
}

BLYNK_CONNECTED() {
    Blynk.syncVirtual(VPIN_BUTTON_1);
    Blynk.syncVirtual(VPIN_BUTTON_2);
}

void measureDistance() {
    // Set the trigger pin LOW for 2μs
    digitalWrite(TRIGPIN, LOW);
    delayMicroseconds(2);

    // Set the trigger pin HIGH for 20μs to send pulse
    digitalWrite(TRIGPIN, HIGH);
    delayMicroseconds(20);

    // Return the trigger pin to LOW
    digitalWrite(TRIGPIN, LOW);

    // Measure the width of the incoming pulse
    duration = pulseIn(ECHOPIN, HIGH);

    // Determine distance from duration
    // Use 343 meters per second as speed of sound
    // Divide by 1000 as we want millimeters
    distance = ((duration / 2) * 0.343) / 10;

    if (distance > (fullTankDistance - 10) && distance < emptyTankDistance) {
        waterLevelPer = map((int)distance, emptyTankDistance, fullTankDistance, 0, 100);
        Blynk.virtualWrite(VPIN_BUTTON_1, waterLevelPer);
        Blynk.virtualWrite(VPIN_BUTTON_2, (String(distance) + " cm"));

        // Print result to serial monitor
        Serial.print("Distance: ");
        Serial.print(distance);
        Serial.println(" cm");

        if (waterLevelPer < triggerPer) {
            digitalWrite(GreenLed, HIGH);
            if (toggleBuzzer == HIGH) {
                digitalWrite(BuzzerPin, HIGH);
            }
        }
        if (distance < fullTankDistance) {
            digitalWrite(GreenLed, LOW);
            if (toggleBuzzer == HIGH) {
                digitalWrite(BuzzerPin, HIGH);
            }
        }

        if (distance > (fullTankDistance + 5) && waterLevelPer > (triggerPer + 5)) {
            toggleBuzzer = HIGH;
            digitalWrite(BuzzerPin, LOW);
        }
    }

    // Delay before repeating measurement
    delay(100);
}

void setupWiFi() {
    Serial.printf("\r\n[Wifi]: Connecting");
    WiFi.begin(WIFI_SSID, WIFI_PASS);

    while (WiFi.status() != WL_CONNECTED) {
        Serial.printf(".");
        delay(250);
    }
    digitalWrite(wifiLed, HIGH);
    Serial.printf("connected!\r\n[WiFi]: IP-Address is %s\r\n", WiFi.localIP().toString().c_str());
}

void setup() {
    // Set up serial monitor
    Serial.begin(115200);

    // Set pin modes for sensor connections
    pinMode(ECHOPIN, INPUT);
    pinMode(TRIGPIN, OUTPUT);
    pinMode(wifiLed, OUTPUT);
    pinMode(GreenLed, OUTPUT);
    pinMode(BuzzerPin, OUTPUT);
    pinMode(ButtonPin1, INPUT_PULLUP);

    digitalWrite(wifiLed, LOW);
    digitalWrite(GreenLed, LOW);
    digitalWrite(BuzzerPin, LOW);

    config1.setEventHandler(button1Handler);
    button1.init(ButtonPin1);

    setupWiFi();
    Blynk.config(auth);
    timer.setInterval(2000L, checkBlynkStatus); // Check if Blynk server is connected every 2 seconds
    delay(1000);
}

void loop() {
    measureDistance();
    Blynk.run();
    timer.run(); // Initiates SimpleTimer
    button1.check();
}

void button1Handler(AceButton* button, uint8_t eventType, uint8_t buttonState) {
    Serial.println("EVENT1");
    switch (eventType) {
        case AceButton::kEventReleased:
            // Serial.println("kEventReleased");
            digitalWrite(BuzzerPin, LOW);
            toggleBuzzer = LOW;
            break;
    }
}
