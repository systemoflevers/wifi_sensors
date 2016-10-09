#include <dummy.h>


#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFiMulti.h>
#include <WiFiClientSecure.h>
#include <DHT.h>

#define DHTTYPE DHT11   // DHT Shield uses DHT 11
#define DHTPIN D4       // DHT Shield uses pin D4

ESP8266WiFiMulti wifiMulti;
// Existing WiFi network
const char* ssid     = "GoogleGuest";
const char* password = "";

// Initialize DHT sensor
// Note that older versions of this library took an optional third parameter to
// tweak the timings for faster processors.  This parameter is no longer needed
// as the current DHT reading algorithm adjusts itself to work on faster procs.
DHT dht(DHTPIN, DHTTYPE);

float humidity, temperature;                 // Raw float values from the sensor
char str_humidity[10], str_temperature[10];  // Rounded  sensor values and as strings
float avg_humidity, avg_temperature;
char str_avg_humidity[10], str_avg_temperature[10];

const int kWinSize = 20;
float humidity_window[kWinSize], temperature_window[kWinSize]; // Sliding window of sensor values
float humidity_win_sum = 0, temperature_win_sum = 0;
int samples = 0;
int next_sample = 0;

String str_mac;



// Generally, you should use "unsigned long" for variables that hold time
unsigned long previousMillis = 0;            // When the sensor was last read
const long interval = 60000;                  // Wait this long until reading again

int id; // ID for this sensor, derived from the MAC.

int get_id() {
  byte mac[6];

  WiFi.macAddress(mac);

  
  str_mac += String(mac[5],HEX);
  str_mac += String(":");
  str_mac += String(mac[4],HEX);
  str_mac += String(":");
  str_mac += String(mac[3],HEX);
  str_mac += String(":");
  str_mac += String(mac[2],HEX);
  str_mac += String(":");
  str_mac += String(mac[1],HEX);
  str_mac += String(":");
  str_mac += String(mac[0],HEX);
  Serial.println();
  Serial.println("MAC: " + str_mac);

  return 0;
}

void update_window(float h, float t) {
  if (samples == kWinSize) {
    humidity_win_sum -= humidity_window[next_sample];
    temperature_win_sum -= temperature_window[next_sample];
  } else {
    ++samples;
  }

  humidity_win_sum += h;
  temperature_win_sum += t;

  humidity_window[next_sample] = h;
  temperature_window[next_sample] = t;

  next_sample = (next_sample + 1) % kWinSize;

  avg_humidity = humidity_win_sum / samples;
  avg_temperature = temperature_win_sum / samples;

  dtostrf(avg_humidity, 1, 2, str_avg_humidity);
  dtostrf(avg_temperature, 1, 2, str_avg_temperature);

  Serial.print("Humidity avg: ");
  Serial.print(str_avg_humidity);
  Serial.print(" %\t");
  Serial.print("Temperature avg: ");
  Serial.print(str_avg_temperature);
  Serial.println(" °C");
}

bool interval_met() {
  unsigned long currentMillis = millis();
  if (currentMillis < previousMillis)
    return true;

  if (currentMillis - previousMillis < interval)
    return false;
  
  previousMillis = currentMillis;
  return true;
}

bool read_sensor() {
  // Reading temperature and humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (it's a very slow sensor)
  humidity = dht.readHumidity();        // Read humidity as a percent
  temperature = dht.readTemperature();  // Read temperature as Celsius

  // Check if any reads failed and exit early (to try again).
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
    return false;
  }

  // Convert the floats to strings and round to 2 decimal places
  dtostrf(humidity, 1, 2, str_humidity);
  dtostrf(temperature, 1, 2, str_temperature);

  Serial.print("Humidity: ");
  Serial.print(str_humidity);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(str_temperature);
  Serial.println(" °C");
  return true;
}

void send_reading() {

  WiFiClientSecure https;
  Serial.println("beginning https...");

  String domain = "docs.google.com";


  if (!https.connect(domain.c_str(), 443)) {
    Serial.println("connection failed");
    return;
  }
  // String url = "/forms/d/1H7CHmU7KHQBBgvnAjmgFacooQ8jib-b1XCgQ2_mI39g/formResponse?";
  String url = "/forms/d/e/1FAIpQLScKaDPsxFOnHd-uxXpHar4DZ78dYbqJWGic27b-Bc1Ihft_JA/formResponse?";
  url = String(url + "entry.928714953=" + str_mac);
  url = String(url + "&entry.70098726=" + avg_temperature);
  url = String(url + "&entry.2062043784=" + avg_humidity);
  Serial.print("url: ");
  Serial.println(url);


  https.print(String("GET ") + url + " HTTP/1.1\r\n" +
              "Host: " + domain + "\r\n" +
              "User-Agent: DHTSensorESP8266\r\n" +
              "Connection: close\r\n\r\n");

  Serial.println("request sent");

    Serial.println("request sent");
  while (https.connected()) {
    String line = https.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("headers received");
      break;
    }
  }
  Serial.println("closing connection");
}
void sense() {
  if (read_sensor()) 
    update_window(humidity, temperature);
}

void sense_and_send() {
  sense();

  while (wifiMulti.run() != WL_CONNECTED) {
    Serial.println("Not connected to WiFi.");
    delay(1000);
  }

  send_reading();
}

void setup(void)
{
  // Open the Arduino IDE Serial Monitor to see what the code is doing
  Serial.begin(9600);
  dht.begin();

  id = get_id();

  Serial.println("WeMos DHT Client");
  Serial.println("");

  // Connect to your WiFi network
  wifiMulti.addAP(ssid); // password too if needed
  wifiMulti.addAP("jessnet", "jesselovesjessica");
  wifiMulti.addAP("jessnet_tv_2", "jesselovesjessica");
  wifiMulti.addAP("jessnet-slow", "jesselovesjessica");
  Serial.print("Connecting");

  sense_and_send();
}

void loop(void)
{
  if (interval_met()) {
    sense_and_send();
    return;
  }

  sense();
  delay(10000);
}

// sensor entry.928714953
// temp entry.70098726
// humidity entry.2062043784
