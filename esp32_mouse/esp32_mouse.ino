#include <WiFi.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <MPU6050.h>

// --- CONFIGURATION ---
const char* ssid = "";
const char* password = "";
const char* pc_ip = " ";
const int udp_port = ;

WiFiUDP udp;
MPU6050 mpu;

void setup() {
  Serial.begin(115200);
  
  // 1. Initialize Fast I2C for zero hardware lag
  Wire.begin();
  Wire.setClock(400000); 

  // 2. Initialize Sensor
  mpu.initialize();
  if (!mpu.testConnection()) {
    Serial.println("MPU6050 connection failed!");
    while(1);
  }

  // 3. Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }
  Serial.println("\nConnected!");
  
  udp.begin(udp_port);
}

void loop() {
  int16_t ax, ay, az;
  int16_t gx, gy, gz;
  
  // Read both Accelerometer and Gyroscope data
  // Gyroscope (gx, gz) handles movement, Accelerometer (ax) handles tilt gestures
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  // Scale down the raw values to make them manageable for mouse coordinates.
  // Tweak the '150' to change your mouse sensitivity (higher number = slower mouse)
  int mouseX = gz / 150; 
  int mouseY = gx / 150; 

  // Gesture detection for clicks (Sudden Jerk left or right)
  // Gravity alone is only ~16384. By setting the threshold to +/- 22000,
  // we ensure that NO amount of tilting will accidentally trigger a click.
  // It will ONLY trigger when you make a sudden, sharp movement (jerk).
  int leftClick = (ax < -22000) ? 1 : 0;   // Sudden jerk left -> Left Click
  int rightClick = (ax > 22000) ? 1 : 0;   // Sudden jerk right -> Right Click

  // Format data as a simple string "X,Y,L,R" for lightning-fast parsing in Python
  char packet[32];
  sprintf(packet, "%d,%d,%d,%d", mouseX, mouseY, leftClick, rightClick);

  // Send via UDP (Fire and forget, zero network blocking)
  udp.beginPacket(pc_ip, udp_port);
  udp.write((uint8_t*)packet, strlen(packet));
  udp.endPacket();

  // 10ms delay gives us a 100Hz polling rate (standard for gaming mice)
  delay(10); 
}
