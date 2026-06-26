# Meta4Farm – Smart Farming IoT System

Meta4Farm is an Internet of Things (IoT)-based smart farming system designed to automate greenhouse monitoring and environmental control. Built with ESP32, the system collects real-time sensor data, controls irrigation and cooling devices automatically, and publishes monitoring data to Adafruit IO using the MQTT protocol.

This project demonstrates the implementation of embedded systems, IoT communication, sensor integration, and basic automation for precision agriculture.

---

## Features

- Real-time temperature monitoring
- Real-time humidity monitoring
- Soil moisture monitoring
- CO₂ level monitoring
- Automatic irrigation using a water pump
- Automatic cooling using a DC fan
- LCD 16×2 local monitoring
- MQTT communication with Adafruit IO
- Harvest readiness notification based on environmental conditions

---

## Hardware

| Component | Description |
|-----------|-------------|
| ESP32 | Main microcontroller |
| DHT11 | Temperature & humidity sensor |
| Soil Moisture Sensor | Soil moisture monitoring |
| Analog CO₂ Sensor | Air quality monitoring |
| LCD I2C 16×2 | Local display |
| Water Pump | Automatic irrigation |
| DC Fan | Automatic cooling |

---

## Software

- Arduino IDE
- ESP32 Board Package
- WiFi Library
- Adafruit MQTT Library
- DHT Sensor Library
- LiquidCrystal I2C Library

---

## System Workflow

1. ESP32 reads sensor data periodically.
2. Sensor values are displayed on the LCD.
3. Data is sent to Adafruit IO through MQTT.
4. The irrigation system is activated when soil moisture falls below the threshold.
5. The cooling fan is activated when the temperature exceeds the threshold.
6. A harvest notification is published when all predefined environmental conditions are met.

---

## Pin Configuration

| Device | ESP32 Pin |
|---------|----------:|
| DHT11 | GPIO 2 |
| Soil Moisture Sensor | A0 |
| CO₂ Sensor | A1 |
| Water Pump | GPIO 5 |
| DC Fan | GPIO 6 |

> **Note:** Replace the analog pin definitions according to the ESP32 board being used.

---

## Adafruit IO Feeds

The project publishes data to the following MQTT feeds:

- temperature
- humidity
- soilMoisture
- co2
- harvestNotification

---

## Project Structure

```
Meta4Farm/
│
├── 4system_farming.ino
├── secrets.h.example
├── README.md
├── LICENSE
├── .gitignore
└── assets/
    ├── prototype.jpg
    ├── dashboard.png
    ├── wiring-diagram.png
    └── architecture.png
```

---

## Installation

### 1. Clone the repository

```bash
git clone https://github.com/nnaelmuna/Meta4Farm.git
```

### 2. Install required libraries

- Adafruit MQTT
- DHT Sensor Library
- LiquidCrystal I2C
- ESP32 Board Package

### 3. Configure credentials

Create a file named `secrets.h`.

```cpp
#define WIFI_SSID "YOUR_WIFI_NAME"
#define WIFI_PASS "YOUR_WIFI_PASSWORD"

#define AIO_USERNAME "YOUR_ADAFRUIT_USERNAME"
#define AIO_KEY "YOUR_ADAFRUIT_IO_KEY"
```

### 4. Upload

Compile and upload the firmware to the ESP32 using Arduino IDE.

---

## Project Preview

### Prototype

*Insert a photo of the hardware prototype.*

### Dashboard

*Insert a screenshot of the Adafruit IO dashboard.*

### Wiring Diagram

*Insert the circuit wiring diagram.*

### System Architecture

*Insert the system architecture diagram.*

---

## Future Improvements

- OTA firmware updates
- ESP32-CAM integration for plant disease detection
- AI-based crop prediction
- Mobile application support
- Historical data logging
- Weather API integration

---

## Security

Sensitive information such as WiFi credentials and Adafruit IO keys is excluded from this repository through the `.gitignore` file.

---

## License

This project is licensed under the MIT License.

---

## Author

**Nael**

Software Engineering Student with interests in:

- Internet of Things (IoT)
- Embedded Systems
- Full-Stack Development
- Robotics
