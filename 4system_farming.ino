//  Smart Farming IoT — Meta4Farm
//  Board   : ESP32
//  Sensors : DHT11, Soil Moisture (A0), CO2 analog (A1)
//  Output  : LCD I2C 16x2, Water Pump (pin 5), DC Fan (pin 6)
//  Cloud   : Adafruit IO via MQTT

#include "secrets.h"               // kredensial WiFi & Adafruit IO
#include <WiFi.h>
#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>
#include <DHT.h>
#include <LiquidCrystal_I2C.h>

#define DHTPIN            2
#define DHTTYPE           DHT11
#define SOIL_MOISTURE_PIN A0
#define CO2_PIN           A1
#define WATER_PUMP_PIN    5
#define DC_FAN_PIN        6

#define TEMP_FAN_ON       30      // °C — kipas nyala jika suhu >= nilai ini
#define SOIL_DRY          300     // nilai analog — pompa nyala jika tanah < nilai ini
#define HARVEST_HUMIDITY  50      // % — kondisi panen: humidity < nilai ini
#define HARVEST_TEMP      25      // °C — kondisi panen: suhu > nilai ini
#define HARVEST_CO2       50      // ppm — kondisi panen: co2 > nilai ini

#define SENSOR_INTERVAL   3000    // ms — interval baca sensor (non-blocking)
#define MQTT_RETRY_DELAY  5000    // ms — jeda sebelum retry koneksi MQTT

// Adafruit IO 
#define AIO_SERVER        "io.adafruit.com"
#define AIO_SERVERPORT    1883

// Objek global
DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);
WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

// Adafruit IO feeds
Adafruit_MQTT_Publish temperatureFeed  = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/temperature");
Adafruit_MQTT_Publish humidityFeed     = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/humidity");
Adafruit_MQTT_Publish soilMoistureFeed = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/soilMoisture");
Adafruit_MQTT_Publish co2Feed          = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/co2");
Adafruit_MQTT_Publish harvestFeed      = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/harvestNotification");

// Variabel sensor
float temperature     = 0.0;
float humidity        = 0.0;
int   soilMoistureValue = 0;
int   co2Value        = 0;

// Timing non-blocking 
unsigned long previousMillis = 0;

// Deklarasi fungsi
void connectWiFi();
void connectMQTT();
void ensureMQTT();
int  readCO2();
void readSensors();
bool sensorsValid();
void updateLCD();
void publishToCloud();
void controlActuators();
void showLCDMessage(const char* line1, const char* line2 = "");

//  SETUP
void setup() {
  Serial.begin(115200);

  // LCD
  lcd.init();
  lcd.backlight();
  showLCDMessage("Smart Farming", "Initializing...");

  // Sensor & aktuator
  dht.begin();
  pinMode(WATER_PUMP_PIN, OUTPUT);
  pinMode(DC_FAN_PIN,     OUTPUT);
  digitalWrite(WATER_PUMP_PIN, LOW);
  digitalWrite(DC_FAN_PIN,     LOW);

  // Koneksi
  connectWiFi();
  connectMQTT();

  harvestFeed.publish("Semangat! Panen kita hampir tiba!");
  showLCDMessage("System Ready", "Monitoring...");
  delay(1500);
}

//  LOOP  (non-blocking dengan millis)
void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= SENSOR_INTERVAL) {
    previousMillis = currentMillis;

    readSensors();

    if (!sensorsValid()) {
      Serial.println("[ERROR] DHT11 gagal membaca sensor!");
      showLCDMessage("Sensor Error!", "Cek DHT11");
      return;
    }

    // Debug serial
    Serial.print("Suhu: ");          Serial.print(temperature);   Serial.println(" C");
    Serial.print("Kelembapan: ");    Serial.print(humidity);       Serial.println(" %");
    Serial.print("Kelembapan Tanah: "); Serial.println(soilMoistureValue);
    Serial.print("CO2: ");           Serial.print(co2Value);       Serial.println(" ppm");
    Serial.println("---");

    updateLCD();
    ensureMQTT();
    publishToCloud();
    controlActuators();
  }
}

//  SENSOR
void readSensors() {
  temperature       = dht.readTemperature();
  humidity          = dht.readHumidity();
  soilMoistureValue = analogRead(SOIL_MOISTURE_PIN);
  co2Value          = readCO2();
}

bool sensorsValid() {
  return !isnan(temperature) && !isnan(humidity);
}

int readCO2() {
  int   analogValue = analogRead(CO2_PIN);
  float voltage     = analogValue * (3.3f / 4095.0f);  // ESP32: 3.3V, 12-bit ADC
  float co2PPM      = max(0.0f, (voltage - 0.2f) * 1000.0f);
  return (int)co2PPM;
}

//  LCD
void updateLCD() {
  // Baris 0: "T:28.5C  H:65%"
  // Baris 1: "Mst:450 CO2:120"
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("T:");
  lcd.print(temperature, 1);
  lcd.print("C");

  lcd.setCursor(8, 0);
  lcd.print("H:");
  lcd.print((int)humidity);
  lcd.print("%");

  lcd.setCursor(0, 1);
  lcd.print("Mst:");
  lcd.print(soilMoistureValue);

  lcd.setCursor(9, 1);
  lcd.print("CO2:");
  lcd.print(co2Value);
}

void showLCDMessage(const char* line1, const char* line2) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(line1);
  if (line2[0] != '\0') {
    lcd.setCursor(0, 1);
    lcd.print(line2);
  }
}

//  MQTT PUBLISH
void publishToCloud() {
  temperatureFeed.publish(temperature);
  humidityFeed.publish(humidity);
  soilMoistureFeed.publish(static_cast<int32_t>(soilMoistureValue));
  co2Feed.publish(static_cast<int32_t>(co2Value));

  bool harvestReady = (humidity < HARVEST_HUMIDITY &&
                       temperature > HARVEST_TEMP   &&
                       co2Value   > HARVEST_CO2);

  if (harvestReady) {
    harvestFeed.publish("Panen Siap!");
    Serial.println("[HARVEST] Kondisi panen tercapai!");
  } else {
    harvestFeed.publish("Semangat! Panen kita hampir tiba!");
  }
}

//  AKTUATOR
void controlActuators() {
  // Pompa air — aktif jika tanah kering
  if (soilMoistureValue < SOIL_DRY) {
    digitalWrite(WATER_PUMP_PIN, HIGH);
    Serial.println("[PUMP] Water pump ON");
  } else {
    digitalWrite(WATER_PUMP_PIN, LOW);
    Serial.println("[PUMP] Water pump OFF");
  }

  // Kipas — aktif jika suhu terlalu tinggi
  if (temperature >= TEMP_FAN_ON) {
    digitalWrite(DC_FAN_PIN, HIGH);
    Serial.println("[FAN] DC Fan ON");
  } else {
    digitalWrite(DC_FAN_PIN, LOW);
    Serial.println("[FAN] DC Fan OFF");
  }
}

//  KONEKSI WiFi & MQTT
void connectWiFi() {
  Serial.print("Menghubungkan ke WiFi: ");
  Serial.println(WIFI_SSID);
  showLCDMessage("Connecting WiFi", WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  int attempt = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
    attempt++;
    if (attempt > 30) {
      // Reboot jika WiFi tidak bisa konek dalam 30 detik
      Serial.println("\n[ERROR] WiFi timeout — restarting...");
      ESP.restart();
    }
  }
  Serial.println("\nTerhubung ke WiFi!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

void connectMQTT() {
  int attempt = 0;
  while (!mqtt.connected()) {
    Serial.print("Menghubungkan ke MQTT... ");
    showLCDMessage("Connecting MQTT", "Please wait...");

    if (mqtt.connect()) {
      Serial.println("Terhubung!");
    } else {
      attempt++;
      Serial.print("Gagal (attempt ");
      Serial.print(attempt);
      Serial.println(") — coba lagi dalam 5 detik");
      delay(MQTT_RETRY_DELAY);
      if (attempt > 5) {
        Serial.println("[ERROR] MQTT gagal — restarting...");
        ESP.restart();
      }
    }
  }
}

void ensureMQTT() {
  if (!mqtt.ping()) {
    Serial.println("[MQTT] Koneksi terputus — reconnecting...");
    connectMQTT();
  }
}
