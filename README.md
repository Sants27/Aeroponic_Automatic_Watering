# 🌱 IoT Smart Agriculture - Automatic Watering System (Aeroponic)
This project is an IoT-based automatic watering system for aeroponic farming, utilizing sensors and an ESP32 board to monitor and manage plant environmental conditions efficiently. The system supports automatic watering based on real-time conditions and manual control via the Blynk app.

🚀 Key Features :
- Real-Time Monitoring via Blynk App:
- Temperature & Humidity (DHT22)
- Water pH (Analog Sensor)
- Water Level (Ultrasonic HC-SR04)
- Light Intensity (LDR)
- Automatic Watering based on time intervals and water availability
- Manual Pump Control via: Physical Button and Blynk App
- Notifications & Safety: Buzzer alarm with LED alert when water level is too low
- Visual Indicator for UV lamp status

⚙️ Hardware Used
- ESP32 Dev Board
- DHT22 sensor
- Analog pH sensor
- Ultrasonic sensor HC-SR04
- LDR sensor (analog & digital)
- Water pump with relay
- UV lamp with relay
- Buzzer
- LED indicator
- Physical push button
- Breadboard
- Jumper wires

📲 Platform & Libraries
- Blynk IoT Platform
- Libraries: WiFi.h, BlynkSimpleEsp32.h, DHT.h, Wire.h

🧠 System Logic
- Sensor data is read and sent to Blynk every 5 seconds
- Buzzer activates and LED turns off if water level is below 20%
- Pump activates automatically every 4 hours if water is available
- Pump can also be activated manually via button or Blynk (only if water is sufficient)
- Pump is turned off automatically using timer after a certain duration
