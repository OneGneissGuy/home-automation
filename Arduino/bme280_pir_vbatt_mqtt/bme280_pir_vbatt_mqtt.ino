#include "config.h" //passwords
#include <ESP8266WiFi.h>
#include <Wire.h>
#include <PubSubClient.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

// In case you have more than one sensor, make each one a different number here
//#define sensor_number "1"

#define motion_topic "sensor/shed/motion" //PIR motion sensor
#define humidity_topic "sensor/shed/humidity"
//#define temperature_c_topic "sensor/" sensor_number "/temperature/degreeCelsius"
#define temperature_f_topic "sensor/shed/temperature"
//#define barometer_hpa_topic "sensor/" sensor_number "/barometer/hectoPascal"
#define barometer_inhg_topic "sensor/shed/pressure"
#define battery_topic "sensor/shed/battery"

// Lookup for your altitude and fill in here, units hPa
// Positive for altitude above sea level
#define baro_corr_hpa 4.1 // =34 masl,0= no alt correction//http://www.luizmonteiro.com/Altimetry.aspx#AltitudeCorrectionforPressureAltitude

#define red_led 0
#define blue_led 2
const int  motionPin = 12;    // the pin that the pushbutton is attached to
int motionState = 0;         // current state of the motion sensor
int lastMotionState = 0;     // previous state of the motion sensor

WiFiClient espClient;
PubSubClient client(espClient);
Adafruit_BME280 bme; // I2C
void blink_red() {
  digitalWrite(red_led, LOW);
  delay(20);
  digitalWrite(red_led, HIGH);
}

void blink_blue() {
  digitalWrite(blue_led, LOW);
  delay(20);
  digitalWrite(blue_led, HIGH);
}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);

  WiFi.begin(wifi_ssid, wifi_password);

  while (WiFi.status() != WL_CONNECTED) {
    blink_red();
    delay(480);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    // If you do not want to use a username and password, change next line to
    // if (client.connect("ESP8266Client")) {
    if (client.connect("ESP8266Client", mqtt_user, mqtt_password)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      blink_red();
      delay(200);
      blink_red();
      delay(4760);
    }
  }
}

bool checkBound(float newValue, float prevValue, float maxDiff) {
  return newValue < prevValue - maxDiff || newValue > prevValue + maxDiff;
}

long lastMsg = 0;
long lastForceMsg = 0;
bool forceMsg = false;
float temp = 0.0;
float hum = 0.0;
float baro = 0.0;
float diff = 1.0;
int batt_warn = 60;
void handleInterrupt() {
  //an interrupt routine
}
int battery_level() {
  // read the battery level from the ESP8266 analog in pin.
  // analog read level is 10 bit 0-1023 (0V-1V).
  // our 1M & 220K voltage divider takes the max
  // lipo value of 4.2V and drops it to 0.758V max.
  // this means our min analog read value should be 580 (3.14V)
  // and the max analog read value should be 774 (4.2V).
  int level = analogRead(A0);
  // convert battery level to percent
  level = map(level, 580, 774, 0, 100);
  Serial.print("Battery level: "); Serial.print(level); Serial.println("%");
  return level;
}
void setup() {
  // Setup the two LED ports to use for signaling status
  pinMode(red_led, OUTPUT);
  pinMode(blue_led, OUTPUT);
  pinMode(motionPin, INPUT);
  digitalWrite(red_led, HIGH);
  digitalWrite(blue_led, HIGH);
  Serial.begin(115200);
  //attachInterrupt(digitalPinToInterrupt(motionPin), handleInterrupt, RISING);

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  //initial post- closed
  client.publish(motion_topic, String(motionState).c_str(), true);

  // Set SDA and SDL ports
  //Wire.begin(2, 14);
  // Using I2C on the HUZZAH board SCK=#5, SDI=#4 by default

  // Start sensor
  if (!bme.begin()) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }

}


void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 30000) {
    lastMsg = now;
    int batt_level = battery_level();
    if (batt_level < batt_warn) {
      client.publish(battery_topic, String(batt_level).c_str(), true);
      Serial.println("The battery is below the warning level...");

    }
    // MQTT broker could go away and come back at any time
    // so doing a forced publish to make sure something shows up
    // within the first 5 minutes after a reset
    if (now - lastForceMsg > 300000) {
      lastForceMsg = now;
      forceMsg = true;
      Serial.println("Forcing publish every 5 minutes...");
    }

    float newTemp = bme.readTemperature();
    float newHum = bme.readHumidity();
    float newBaro = bme.readPressure() / 100.0F;

    if (checkBound(newTemp, temp, diff) || forceMsg) {
      temp = newTemp;
      float temp_c = temp; // Celsius
      float temp_f = temp * 1.8F + 32.0F; // Fahrenheit
      Serial.print("New temperature:");
      Serial.print(String(temp_c) + " degC   ");
      Serial.println(String(temp_f) + " degF");
      //  client.publish(temperature_c_topic, String(temp_c).c_str(), true);
      client.publish(temperature_f_topic, String(temp_f).c_str(), true);
      blink_blue();
    }

    if (checkBound(newHum, hum, diff) || forceMsg) {
      hum = newHum;
      Serial.print("New humidity:");
      Serial.println(String(hum) + " %");
      client.publish(humidity_topic, String(hum).c_str(), true);
      blink_blue();
    }

    if (checkBound(newBaro, baro, diff) || forceMsg) {
      baro = newBaro;
      float baro_hpa = baro + baro_corr_hpa; // hPa corrected to sea level
      float baro_inhg = (baro + baro_corr_hpa) / 33.8639F; // inHg corrected to sea level

      Serial.print("New barometer:");
      Serial.print(String(baro_hpa) + " hPa   ");
      Serial.println(String(baro_inhg) + " inHg");
      //    client.publish(barometer_hpa_topic, String(baro_hpa).c_str(), true);
      client.publish(barometer_inhg_topic, String(baro_inhg).c_str(), true);
      blink_blue();
    }

    motionState = digitalRead(motionPin);
    // compare the motionState to its previous state
    if (motionState != lastMotionState) {
      // if the state has changed, increment the counter
      if (motionState == HIGH) {
        // if the current state is HIGH then the button went from off to on:
        Serial.println("door opened");
      } else {
        // if the current state is LOW then the button went from on to off:
        Serial.println("door closed");
      }
      // Delay a little bit to avoid bouncing
      delay(50);
      client.publish(motion_topic, String(motionState).c_str(), true);
      client.publish(battery_topic, String(batt_level).c_str(), true);

    }
    // save the current state as the last state, for next time through the loop
    lastMotionState = motionState;

    forceMsg = false;
  }
}
