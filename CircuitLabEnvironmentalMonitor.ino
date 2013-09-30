#include <SPI.h>
#include <Ethernet.h>
#include <HttpClient.h>
#include <G7Weather3.h>
#include <SHT1x.h>
#include <Xively.h>

char xivelyApiKey[] = "";
#define XIVELY_FEED_ID 1369761790

// define sensor ids
#define SENSOR_ID_TEMPERATURE "temperature"
#define SENSOR_ID_HUMIDITY    "humidity"
#define SENSOR_ID_ATOMOSPHERIC_PRESSURE    "atomospheric_pressure"
#define SENSOR_ID_MOISTURE    "plant_moisture"

// define datastream indexes
#define DATASTREAM_TEMPERATURE 0
#define DATASTREAM_HUMIDITY    1
#define DATASTREAM_ATOMOSPHERIC_PRESSURE    2
#define DATASTREAM_MOISTURE    3

// define I2C pin assing for BMP085
#define DATAPIN  2
#define CLOCKPIN 3

byte mac[] = { 0x90, 0xA2, 0xDA, 0x00, 0x53, 0x13 };

XivelyDatastream dataStreams[] = {
  XivelyDatastream(SENSOR_ID_TEMPERATURE, strlen(SENSOR_ID_TEMPERATURE), DATASTREAM_FLOAT),
  XivelyDatastream(SENSOR_ID_HUMIDITY, strlen(SENSOR_ID_HUMIDITY), DATASTREAM_FLOAT),
  XivelyDatastream(SENSOR_ID_ATOMOSPHERIC_PRESSURE, strlen(SENSOR_ID_ATOMOSPHERIC_PRESSURE), DATASTREAM_FLOAT),
};

XivelyFeed feed(XIVELY_FEED_ID, dataStreams, 3);

EthernetClient client;
XivelyClient xivelyClient(client);


SHT1x sht1x(DATAPIN, CLOCKPIN);
G7Weather weather;
float lastPressure, lastTemperature, lastHumidity, lastMoisture;

unsigned long lastUploadTime = 0;
const unsigned long uploadingInterval = 20 * 1000;

boolean isInitialized = false;

int numDatastreams = 0;


void setup ()
{
  Serial.begin(9600);
  
  Serial.println("Initializing...");
  Serial.println();
  
  while (1 != Ethernet.begin(mac))
  {
    Serial.println("Error getting IP address via DHCP, trying again...");
    delay(2000);
  }
  Serial.println("Successfully got IP via DHCP.");
  
  weather.init_bmp085();
  
  Serial.println("Successfully initialized!");
  Serial.println("Start multiple datastream upload.");
  Serial.println();
}

void loop ()
{
  Serial.println("--------------------------------");
  Serial.println("Read sensor values...");
  
  float temperature = sht1x.readTemperatureC();
  float humidity = sht1x.readHumidity();
  float pressure = weather.get_bmp085();
//  float moisture = analogRead(A0) / 10;
  
  if (lastTemperature && !(lastTemperature + 4.0f <= temperature || lastTemperature - 4.0f >= temperature)) {
    dataStreams[DATASTREAM_TEMPERATURE].setFloat(temperature);
  }
  if (lastHumidity && !(lastHumidity + 10.0f <= humidity || lastHumidity - 10.0f >= humidity)) {
    dataStreams[DATASTREAM_HUMIDITY].setFloat(humidity);
  }
  if (lastPressure && !(lastPressure + 100.0f <= pressure || lastPressure - 100.0f >= pressure)) {
    dataStreams[DATASTREAM_ATOMOSPHERIC_PRESSURE].setFloat(pressure);
  }
//  if (lastMoisture && !(lastMoisture + 8.0f <= moisture || lastMoisture - 8.0f >= moisture)) {
//    dataStreams[DATASTREAM_MOISTURE].setFloat(moisture);
//  }
  
  // store current value if it's over the threshold
  if (-5.0f < temperature) {
    lastTemperature = temperature;
  }
  if (10.0f < humidity) {
    lastHumidity = humidity;
  }
  if (800.0f < pressure) {
    lastPressure = pressure;
  }
//  if (0.0f < moisture) {
//    lastMoisture = moisture;
//  }
  
  Serial.print("    temperature = ");
  Serial.println(dataStreams[DATASTREAM_TEMPERATURE].getFloat());
  Serial.print("    humidity    = ");
  Serial.println(dataStreams[DATASTREAM_HUMIDITY].getFloat());
  Serial.print("    pressure    = ");
  Serial.println(dataStreams[DATASTREAM_ATOMOSPHERIC_PRESSURE].getFloat());
//  Serial.print("    moisture    = ");
//  Serial.println(datastreams[DATASTREAM_MOISTURE].getFloat());
  Serial.println("--------------------------------");
  
  if (isInitialized) {
    Serial.println("Uploading...");
    
    int result = xivelyClient.put(feed, xivelyApiKey);
    
    Serial.print("    xivelyClient.put status : ");
    Serial.println(result);
    Serial.println("--------------------------------");
    
    delay(uploadingInterval);
  } else {
    isInitialized = true;
    delay(1000);
  }
}
