// Include necessary libraries
#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include <DHT.h>
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>
#include <Wire.h>
#include <DallasTemperature.h>
#include <Adafruit_NeoPixel.h>
#include <ESP32Servo.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// define gpio pins
#define DHTPIN 4                  // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11             // DHT 11
#define RST_PIN 14                // Reset pin connected to GPIO 14 (D14)
#define SS_PIN 21                 // Slave Select pin connected to GPIO 5 (D5)
#define SERVO_PIN 26              // Pin to which the servo is connected
#define RELAY_PIN 27              // Change this to the GPIO pin connected to the relay module
#define ONE_WIRE_BUS 22           // one wire bus to DS18B20
#define MAT_PIN 5                 // PIN FOR heatmat
#define LED_PIN 32                // Pin D32 on ESP32
#define LED_COUNT 30              // Number of LEDs in your strip
const int trigPin = 12;           // Trig pin of HC-SR04
const int echoPin = 13;           // Echo pin of HC-SR04
const int soilSensorPin = 35;     // Analog pin to which the sensor is connected
const int photoresistorPin = 34;  // Digital pin 35 on ESP32

// Replace with your network credentials
const char* ssid = "telenet-C0937";
const char* password = "NxefST4pcycx";

// MQTT Broker data
const char* mqtt_server = "tharspi.local";
const char* mqtt_user = "thars";
const char* mqtt_password = "thars";

// create wifi for MQTT
WiFiClient espClient;
PubSubClient client(espClient);

Servo myServo;                                                      // Create a servo object
MFRC522 mfrc522(SS_PIN, RST_PIN);                                   // Create MFRC522 instance.
DHT dht(DHTPIN, DHTTYPE);                                           // create DHT object
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);  // create ledstrip object
LiquidCrystal_I2C lcd(0x27, 16, 2);                                 // Set the LCD I2C address and the number of columns and rows

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS);

// Pass oneWire reference to DallasTemperature library
DallasTemperature sensors(&oneWire);

// parameters voor de kweekbak
float lichtintensiteit = 0;
float bodemvochtigheid = 5000;
float bodemtemperatuur = 0;
float luchtemperatuur = 0;
float WaterAfstand = 15;
String plantname = "";

// extra parameters voor extra grafana data
int lcdState = 1;
int waterLeft = 0;
float moistpercent = 0;
float lightpercent = 0;
float pumpState = 0;
float windowState = 0;
float lightState = 0;
float hundred = 100;
float null = 0;

// Define your mappings between RFID tags and settings
const String TAG_ID_1 = "03 31 79 13";
const String TAG_ID_2 = "53 80 02 29";

// Function to compare two tag IDs
bool compareTagIDs(String tag1, String tag2) {
  return tag1.equals(tag2);
}

// Function to update settings based on the detected RFID tag
void updateSettings(String tagID) {
  if (compareTagIDs(tagID, TAG_ID_1)) {
    Serial.println("Settings updated for Tag 1");
    plantname = "plant1";
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(plantname);
    lichtintensiteit = 1000;
    bodemvochtigheid = 2000;
    bodemtemperatuur = 15;
    luchtemperatuur = 15;
  } else if (compareTagIDs(tagID, TAG_ID_2)) {
    Serial.println("Settings updated for Tag 2");
    plantname = "plant2";
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(plantname);
    lichtintensiteit = 3000;
    bodemvochtigheid = 5000;
    bodemtemperatuur = 50;
    luchtemperatuur = 30;
  } else {
    Serial.println("Unknown tag ID");
  }
}

void colorWipe(uint32_t color, int wait) {
  for (int i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, color);  // Set pixel color
    strip.show();                   // Update strip
    delay(wait);                    // Pause for a moment
  }
}

void moveServo(int degrees) {
  myServo.write(degrees);  // Move the servo to the specified angle
}

void setup_wifi() {
  delay(10);
  // Connecting to a WiFi network
  Serial.begin(115200);
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");  // print ..... if it cant connect to wifi
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP32Client", mqtt_user, mqtt_password)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);  // Initialize serial communications with the PC
  while (!Serial)
    ;                                   // Wait for serial port to connect
  SPI.begin();                          // Init SPI bus
  setup_wifi();                         //setup wifi
  client.setServer(mqtt_server, 1883);  //set MQTT server
  client.setCallback(callback);
  mfrc522.PCD_Init();             // Init MFRC522
  dht.begin();                    // start DHT11
  Wire.begin(25, 33);             // Start the I2C communication on pins 25 (SDA) and 33 (SCL)
  lcd.init();                     // Initialize the LCD
  lcd.backlight();                // Turn on the backlight
  lcd.clear();                    //clear the lcd
  sensors.begin();                //start the DS18B20
  pinMode(trigPin, OUTPUT);       // Set trig pin as output
  pinMode(echoPin, INPUT);        // Set echo pin as input
  pinMode(soilSensorPin, INPUT);  //set soilsensor as input
  pinMode(RELAY_PIN, OUTPUT);     // set relay as output
  pinMode(MAT_PIN, OUTPUT);       // set led
  myServo.attach(SERVO_PIN);      // Attach the servo to the specified pin
  strip.begin();                  // Initialize the strip
  strip.show();                   // Initialize all pixels to 'off'
  strip.setBrightness(50);        // Set brightness (0 to 255)
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long duration, distance;

  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);  // send short ultrasonic pulse
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);


  // Measure the duration of the echo pulse
  duration = pulseIn(echoPin, HIGH);

  // Calculate distance in centimeters
  distance = duration * 0.034 / 2;

  // Call sensors.requestTemperatures() to issue a global temperature
  // request to all devices on the bus
  sensors.requestTemperatures();

  // Get the temperature in Celsius
  float temperatureC = sensors.getTempCByIndex(0);

  // Check if temperature reading is valid
  if (temperatureC != DEVICE_DISCONNECTED_C) {
    // Print temperature to serial monitor
    if (temperatureC <= bodemtemperatuur) {
      Serial.print("ZET WARMTEMATLEDJE AAN || ");
      digitalWrite(MAT_PIN, HIGH);
    } else {
      digitalWrite(MAT_PIN, LOW);
    }
    Serial.print("Soil Temperature: ");
    Serial.print(temperatureC);
    Serial.println(" °C");
  } else {
    // Print an error message if temperature reading failed
    Serial.println("Error: Unable to read soil temperature data");
  }

  // Print the distance to the serial monitor
  if (distance > WaterAfstand) {
    Serial.print("VOEG WATER TOE AAN DE TANK || ");
  }
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.print(" cm || ");
  // calculate waterleft in the tank
  waterLeft = distance - 15;
  waterLeft = -waterLeft;
  Serial.print("Water Left in tank : ");
  Serial.print(waterLeft);
  Serial.println(" cm");

  // measure the soilmoisture
  float soilMoisture = analogRead(soilSensorPin);
  moistpercent = (bodemvochtigheid / soilMoisture) * 100;
  if (soilMoisture >= bodemvochtigheid) {
    Serial.print("ZET WATERPOMPJE AAN || ");
    digitalWrite(RELAY_PIN, HIGH);
    pumpState = 1;
  } else {
    digitalWrite(RELAY_PIN, LOW);
    pumpState = 0;
  }
  Serial.print("Soil Moisture: ");
  Serial.print(soilMoisture);
  Serial.print(" || percent: ");
  Serial.println(moistpercent);

  float lightValue = analogRead(photoresistorPin);
  lightpercent = (lichtintensiteit / lightValue) * 100;
  if (lightValue >= lichtintensiteit) {
    // Turn on LED strip in pink color
    Serial.print("zet ledstrip aan || ");
    colorWipe(strip.Color(255, 0, 255), 50);  // Pink color
    lightState = 1;
  } else {
    // Otherwise, turn off the LED strip
    colorWipe(strip.Color(0, 0, 0), 50);  // Off
    lightState = 0;
  }
  // Print the sensor value to the serial monitor
  Serial.print("Light Value: ");
  Serial.print(lightValue);
  Serial.print(" || percent: ");
  Serial.println(lightpercent);

  float temperature = dht.readTemperature();  // read DHT11 temperature in celsius Celsius

  if (isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  if (temperature >= luchtemperatuur) {
    Serial.print("ZET Raampje open AAN || ");
    moveServo(90);  // Move servo to 90 degrees
    windowState = 1;
  } else {
    moveServo(0);  // Move servo back to 0 degrees
    windowState = 0;
  }

  Serial.print("Air Temperature: ");
  Serial.print(temperature);
  Serial.println(" *C");

  // Create JSON objects with data for grafana
  StaticJsonDocument<400> doc1;
  doc1["soilTemperature"] = static_cast<float>(temperatureC);
  doc1["distance"] = static_cast<float>(distance);
  doc1["soilMoisture"] = static_cast<float>(soilMoisture);
  doc1["moistpercent"] = static_cast<float>(moistpercent);
  doc1["lightpercent"] = static_cast<float>(lightpercent);
  doc1["waterLeft"] = static_cast<float>(waterLeft);
  doc1["lightValue"] = static_cast<float>(lightValue);
  doc1["airTemperature"] = static_cast<float>(temperature);
  // second object beacause first was too big
  StaticJsonDocument<400> doc2;
  doc2["pumpState"] = static_cast<float>(pumpState);
  doc2["windowState"] = static_cast<float>(windowState);
  doc2["lightState"] = static_cast<float>(lightState);
  doc2["lichtintensiteit"] = static_cast<float>(lichtintensiteit);
  doc2["bodemvochtigheid"] = static_cast<float>(bodemvochtigheid);
  doc2["bodemtemperatuur"] = static_cast<float>(bodemtemperatuur);
  doc2["luchtemperatuur"] = static_cast<float>(luchtemperatuur);
  doc2["hundred"] = static_cast<float>(hundred);
  doc2["WaterAfstand"] = static_cast<float>(WaterAfstand);
  doc2["null"] = static_cast<float>(null);



  char jsonBuffer1[400];
  char jsonBuffer2[400];
  size_t n1 = serializeJson(doc1, jsonBuffer1);
  size_t n2 = serializeJson(doc2, jsonBuffer2);

  Serial.print("JSON size: ");
  Serial.println(n1);
  Serial.print("JSON size: ");
  Serial.println(n2);
  serializeJson(doc1, jsonBuffer1);
  serializeJson(doc2, jsonBuffer2);

  // Publish JSON data to MQTT
  client.publish("esp32/sensors", jsonBuffer1);
  client.publish("esp32/sensors", jsonBuffer2);

  // every 5 loops lcd print shows different data
  if (lcdState > 0) {
    int procent = 0;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(plantname);
    lcd.setCursor(0, 1);
    lcd.print(temperatureC);
    lcd.setCursor(8, 1);
    lcd.print("SoilTemp");
  }
  if (lcdState > 5) {

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(plantname);
    lcd.setCursor(0, 1);
    lcd.print(waterLeft);
    lcd.setCursor(4, 1);
    lcd.print("cm WaterLeft");
  }
  if (lcdState > 10) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(plantname);
    lcd.setCursor(0, 1);
    lcd.print(moistpercent);
    lcd.setCursor(6, 1);
    lcd.print("%SoilMoist");
  }
  if (lcdState > 15) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(plantname);
    lcd.setCursor(0, 1);
    lcd.print(lightpercent);
    lcd.setCursor(10, 1);
    lcd.print("%Light");
  }
  if (lcdState > 20) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(plantname);
    lcd.setCursor(0, 1);
    lcd.print(temperature);
    lcd.setCursor(9, 1);
    lcd.print("AirTemp");
  }
  if (lcdState == 25) { // when lcd state = 25 return to 0 
    lcdState = 0;
  }
  lcdState++;

  delay(500);

  // Look for new cards
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  // Select one of the cards
  if (!mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  //Show UID on serial monitor
  Serial.print("UID tag :");
  String content = "";
  byte letter;
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
    content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  Serial.println();
  Serial.print("Message : ");
  content.toUpperCase();
  Serial.println(content.substring(1));

  // Update settings based on the detected RFID tag
  updateSettings(content.substring(1));
  Serial.println(lichtintensiteit);
  Serial.println(bodemvochtigheid);
  Serial.println(bodemtemperatuur);
  Serial.println(luchtemperatuur);



  delay(1000);
}