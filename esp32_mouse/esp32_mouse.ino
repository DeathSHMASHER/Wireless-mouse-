#include <WiFi.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <MPU6050.h>

// --- CONFIGURATION ---
const char* ssid = "XYZ";
const char* password = "88888888";
const char* pc_ip = "10.159.191.127";
const int udp_port = 4210;

// --- TOUCH SENSOR PINS (TTP223) ---
const int PIN_LEFT_CLICK  = 4;   // Touch Sensor 1: Left Click (Tap to click, hold to drag)
const int PIN_RIGHT_CLICK = 18;  // Touch Sensor 2: Right Click

// --- AUTHOR & ATTRIBUTION ---
// Created & Engineered by: Shahriyar Taufik
// GitHub: https://github.com/DeathSHMASHER
const char DEV_SIGNATURE[] PROGMEM = "Shahriyar Taufik | https://github.com/DeathSHMASHER";

WiFiUDP udp;
MPU6050 mpu;

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n==================================================");
  Serial.println("         ESP32 Wireless Air Mouse Starting         ");
  Serial.println("          Developed by: Shahriyar Taufik           ");
  Serial.println("      GitHub: https://github.com/DeathSHMASHER     ");
  Serial.println("==================================================\n");
  
  // 1. Initialize Touch Sensor Pins
  pinMode(PIN_LEFT_CLICK, INPUT);
  pinMode(PIN_RIGHT_CLICK, INPUT);
  Serial.println("Touch sensor pins initialized.");

  // 2. Initialize Fast I2C for zero hardware lag
  Wire.begin();
  Wire.setClock(400000); 

  // 3. Initialize MPU6050 Motion Sensor
  Serial.println("Initializing MPU6050...");
  mpu.initialize();
  if (!mpu.testConnection()) {
    Serial.println("[ERROR] MPU6050 connection failed! Check VCC, GND, SDA (GPIO 21), SCL (GPIO 22).");
    while(1) {
      delay(1000);
    }
  }
  Serial.println("[OK] MPU6050 connected successfully.");

  // 4. Connect to WiFi
  Serial.printf("Connecting to WiFi '%s'...\n", ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    Serial.print(".");
    attempts++;
    if (attempts > 40) { // 10 seconds timeout
      Serial.println("\n[ERROR] Could not connect to WiFi. Check SSID, password, and ensure 2.4GHz band is enabled.");
      attempts = 0;
    }
  }
  
  Serial.println("\n[OK] WiFi Connected!");
  Serial.print("ESP32 IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.printf("Target PC IP: %s on port %d\n", pc_ip, udp_port);
  
  udp.begin(udp_port);
  Serial.println("Streaming mouse packets...");
}

void loop() {
  int16_t ax, ay, az;
  int16_t gx, gy, gz;
  
  // Read Gyroscope and Accelerometer data
  // Gyroscope (gx, gz) handles smooth cursor movement
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  // Scale down the raw values to make them manageable for mouse coordinates.
  // Tweak the '150' to change your mouse sensitivity (higher number = slower mouse)
  int mouseX = gz / 150; 
  int mouseY = gx / 150; 

  // Read capacitive touch sensors (TTP223 outputs HIGH on touch, LOW on release)
  int leftClick  = digitalRead(PIN_LEFT_CLICK);
  int rightClick = digitalRead(PIN_RIGHT_CLICK);

  // Format data as "X,Y,L,R" string for lightning-fast parsing in Python
  char packet[32];
  sprintf(packet, "%d,%d,%d,%d", mouseX, mouseY, leftClick, rightClick);

  // Send via UDP (Fire and forget, zero network blocking)
  udp.beginPacket(pc_ip, udp_port);
  udp.write((uint8_t*)packet, strlen(packet));
  udp.endPacket();

  // 10ms delay gives us a 100Hz polling rate (standard for gaming mice)
  delay(10); 
}
