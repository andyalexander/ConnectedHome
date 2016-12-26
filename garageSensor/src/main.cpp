
#include "secret.h"
#include <Losant.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>

// 433mhz switch
#include <RCSwitch.h>
RCSwitch mySwitch = RCSwitch();

#define LED     D0        // Led in NodeMCU at pin GPIO16 (D0).
#define SWITCH1 D2

#define SWITCH_GROUP 1    // main group for switch, the primary dial on the switch istelf

// Our first sensor, a cheap DHT11 temperature and humidty sensor
#include <DHT.h>
#define DHTTYPE DHT11 //21 or 22 also an option
#define DHTPIN 5
DHT dht(DHTPIN, DHTTYPE);

// BME280 sensor
#include <BME280I2C.h>
#define BME_SDA   D5    // since this is on an esp8266 can choose the pins
#define BME_SCL   D6
BME280I2C bme;

unsigned long readTime;

// #define wifi_ssid "ssid"
// #define wifi_password "password"

// #define mqtt_server "ip address"
//#define mqtt_user "your_username"
//#define mqtt_password "your_password"


#define humidity_topic "openhab/garage/humidity"
#define temperature_topic "openhab/garage/temperature"
#define pressure_topic "openhab/garage/pressure"
#define switch1_topic "openhab/garage/door1"
#define switch2_topic "openhab/switch/433"

WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);

  WiFi.begin(wifi_ssid, wifi_password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("Wifi setup complete...");
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("garageSensor")) {
    //if (client.connect("ESP8266Client", mqtt_user, mqtt_password)) {
      client.subscribe(switch1_topic);
      client.subscribe(switch2_topic);
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

bool checkBound(float newValue, float prevValue, float maxDiff) {
  return !isnan(newValue) &&
         (newValue < prevValue - maxDiff || newValue > prevValue + maxDiff);
}

// function called when message received, set using the setCallback function
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  if (!strcmp(topic, switch1_topic)) {
    Serial.print("Door message received: ");

    // Switch on the LED if an 1 was received as first character
    if ((char)payload[0] == '1') {
      digitalWrite(LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
      digitalWrite(SWITCH1, LOW);
      Serial.println("opened");
    } else {
      digitalWrite(LED, HIGH);  // Turn the LED off by making the voltage HIGH
      digitalWrite(SWITCH1, HIGH);
      Serial.println("closed");
    }
  }

  if (!strcmp(topic, switch2_topic)) {
    Serial.print("433 mhz switch: ");

    //int switchNumber = atoi((char)payload[0]);
    int switchNumber = (char)payload[0] - '0'; // assumes max 9 switches - hack to conver the char to int

    if ((char)payload[2] == '1') {
      mySwitch.switchOn(switchNumber, 1);
      Serial.print(switchNumber);
      Serial.println(" - on");
    } else {
      mySwitch.switchOff(switchNumber, 1);
      Serial.print(switchNumber);
      Serial.println(" - off");
    }
  }


}


long lastMsg = 0;
float temp = 0.0;
float hum = 0.0;
float diff = 1.0;
float press = 0.0;

void setup() {
  pinMode(LED, OUTPUT);   // LED pin as output.
  digitalWrite(LED, HIGH);    // inverse low /high

  mySwitch.enableTransmit(D4);

  pinMode(SWITCH1, OUTPUT);
  digitalWrite(SWITCH1, HIGH);    // for security, close the switch if reset...

  Serial.begin(9600);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);     // define the callback function when we receive ANY message

  // Start sensors
  dht.begin();

  while(!bme.begin(BME_SDA, BME_SCL)){
    Serial.println("Could not find BME280I2C sensor!");
    delay(1000);
  }

  Serial.println("Setup complete...");
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // only update every 10 seconds
  long now = millis();
  if (now - lastMsg > 10000) {
    lastMsg = now;

    // float newTemp = dht.readTemperature();
    float newHum = dht.readHumidity();
    float newTemp = bme.temp(true);
    // float newHum = bme.hum();
    float newPress = bme.press(0x1);

    // float newHum2 = bme.hum();
    // Serial.print("Humidity2: ");
    // Serial.println(newHum2);

    if (checkBound(newTemp, temp, diff)) {
      temp = newTemp;
      Serial.print("New temperature:");
      Serial.println(String(temp).c_str());
      client.publish(temperature_topic, String(temp).c_str(), true);
    }

    if (checkBound(newHum, hum, diff)) {
      hum = newHum;
      Serial.print("New humidity:");
      Serial.println(String(hum).c_str());
      client.publish(humidity_topic, String(hum).c_str(), true);
    }

    if (checkBound(newPress, press, diff)) {
      press = newPress;
      Serial.print("New pressure:");
      Serial.println(String(press).c_str());
      client.publish(pressure_topic, String(press).c_str(), true);
    }
  }
}
