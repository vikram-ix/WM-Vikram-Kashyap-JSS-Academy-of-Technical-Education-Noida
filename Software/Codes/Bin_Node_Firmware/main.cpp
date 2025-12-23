/*
 * Project: Smart Waste Bin Node
 * Hardware: ESP32 LoRa (TTGO) + JSN-SR04T (Ultrasonic Sensor)
 * Features: Deep Sleep, Median Filtering, LoRaWAN Packet Sending
 */

#include <SPI.h>
#include <LoRa.h>

// --- Pin Definitions ---
#define TRIG_PIN 12
#define ECHO_PIN 14
#define SS_PIN   18  // LoRa CS
#define RST_PIN  14  // LoRa Reset
#define DIO0_PIN 26  // LoRa IRQ

// --- Configuration ---
const int BIN_HEIGHT_CM = 100; // Total depth of the bin
const int FULL_THRESHOLD = 20; // Distance (cm) indicating bin is full (trash near top)
#define uS_TO_S_FACTOR 1000000  // Conversion factor for micro seconds to seconds
[span_4](start_span)#define TIME_TO_SLEEP  3600     // Sleep for 1 hour (in seconds)[span_4](end_span)

// --- Function to get distance ---
long getDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  long duration = pulseIn(ECHO_PIN, HIGH);
  long distance = duration * 0.034 / 2;
  return distance;
}

[span_5](start_span)// --- Fault Handling: Median Filter[span_5](end_span) ---
long getFilteredDistance() {
  long readings[5];
  // Take 5 readings
  for (int i = 0; i < 5; i++) {
    readings[i] = getDistance();
    delay(50); 
  }
  
  // Simple sort to find median
  for (int i = 0; i < 4; i++) {
    for (int j = i + 1; j < 5; j++) {
      if (readings[i] > readings[j]) {
        long temp = readings[i];
        readings[i] = readings[j];
        readings[j] = temp;
      }
    }
  }
  return readings[2]; // Return the median value
}

void setup() {
  Serial.begin(115200);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // Initialize LoRa
  LoRa.setPins(SS_PIN, RST_PIN, DIO0_PIN);
  if (!LoRa.begin(915E6)) { // Set frequency (915MHz or 868MHz depending on region)
    Serial.println("LoRa init failed!");
    while (1);
  }

  // 1. Read Sensor
  long dist = getFilteredDistance();
  
  // 2. Calculate Fill Percentage
  // If distance is large (empty), fill is low. If distance is small (full), fill is high.
  int fillLevel = map(dist, BIN_HEIGHT_CM, FULL_THRESHOLD, 0, 100);
  fillLevel = constrain(fillLevel, 0, 100);

  // 3. Prepare Data Packet (JSON format for simplicity)
  String packet = "{\"id\":\"BIN01\", \"fill\":" + String(fillLevel) + "}";
  
  // 4. Send Packet via LoRa
  LoRa.beginPacket();
  LoRa.print(packet);
  LoRa.endPacket();
  Serial.println("Sent: " + packet);

  [span_6](start_span)// 5. Power Management: Deep Sleep[span_6](end_span)
  // Configure wake up source
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  Serial.println("Going to sleep now...");
  Serial.flush(); 
  esp_deep_sleep_start();
}

void loop() {
  // Loop is not used because of Deep Sleep
}
