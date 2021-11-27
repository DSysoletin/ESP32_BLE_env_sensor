#include "DHT.h"
#define DHT11PIN 16
#include "sys/time.h"

#include <Arduino.h>

#include "BLEDevice.h"
#include "BLEUtils.h"
#include "BLEBeacon.h"
#include "BLEAdvertising.h"
#include "BLEEddystoneURL.h"

#include "esp_sleep.h"

#define GPIO_DEEP_SLEEP_DURATION 60     // sleep x seconds and then wake up
RTC_DATA_ATTR static time_t last;    // remember last boot in RTC Memory
RTC_DATA_ATTR static uint32_t bootcount; // remember number of boots in RTC Memory

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/
BLEAdvertising *pAdvertising;
struct timeval nowTimeStruct;

time_t lastTenth;

#define BEACON_UUID "8ec76ea3-6668-48da-9866-75be8bc86f4d" // UUID 1 128-Bit (may use linux tool uuidgen or random numbers via https://www.uuidgenerator.net/)

DHT dht(DHT11PIN, DHT11);
void setup()
{
  //delay(1000);
  Serial.begin(115200);
/* Start the DHT11 Sensor */
  dht.begin();

  gettimeofday(&nowTimeStruct, NULL);

  Serial.printf("start ESP32 %d\n", bootcount++);

  Serial.printf("deep sleep (%lds since last reset, %lds since last boot)\n", nowTimeStruct.tv_sec, nowTimeStruct.tv_sec - last);

  last = nowTimeStruct.tv_sec;
  lastTenth = nowTimeStruct.tv_sec * 10; // Time since last reset as 0.1 second resolution counter

  // Create the BLE Device
  BLEDevice::init("TLMBeacon");

  BLEDevice::setPower(ESP_PWR_LVL_N12);

  pAdvertising = BLEDevice::getAdvertising();
}


void setBeacon()
{
  char beacon_data[25];
  uint16_t beconUUID = 0xFEAA;
  //uint16_t volt = random(2800, 3700); // 3300mV = 3.3V
  uint16_t volt = analogRead(34);
  volt = round(2.13*3300*(float(volt)/4096.0));
  Serial.printf("Voltage is %dmV\n", volt);
  //float tempFloat = random(2000, 3100) / 100.0f;
  float tempFloat = dht.readTemperature();  
  Serial.printf("Temperature is %.2fC\n", tempFloat);
  int temp = (int)(tempFloat * 256); //(uint16_t)((float)23.00);
  //Serial.printf("Converted to 8.8 format %0X%0X\n", (temp >> 8), (temp & 0xFF));

  float humFloat = dht.readHumidity();  
  Serial.printf("Humitidy is %.2fC\n", humFloat);
  int hum = (int)(humFloat * 256); //(uint16_t)((float)23.00);
  //Serial.printf("Converted to 8.8 format %0X%0X\n", (hum >> 8), (hum & 0xFF));

  BLEAdvertisementData oAdvertisementData = BLEAdvertisementData();
  BLEAdvertisementData oScanResponseData = BLEAdvertisementData();

  oScanResponseData.setFlags(0x06); // GENERAL_DISC_MODE 0x02 | BR_EDR_NOT_SUPPORTED 0x04
  oScanResponseData.setCompleteServices(BLEUUID(beconUUID));

  beacon_data[0] = 0x20;                // Eddystone Frame Type (Unencrypted Eddystone-TLM)
  beacon_data[1] = 0x00;                // TLM version
  beacon_data[2] = (volt >> 8);           // Battery voltage, 1 mV/bit i.e. 0xCE4 = 3300mV = 3.3V
  beacon_data[3] = (volt & 0xFF);           //
  beacon_data[4] = (temp >> 8);           // Beacon temperature
  beacon_data[5] = (temp & 0xFF);           //
  beacon_data[6] = ((bootcount & 0xFF000000) >> 24);  // Advertising PDU count
  beacon_data[7] = ((bootcount & 0xFF0000) >> 16);  //
  beacon_data[8] = ((bootcount & 0xFF00) >> 8);   //
  beacon_data[9] = (bootcount & 0xFF);        //
  beacon_data[10] = ((lastTenth & 0xFF000000) >> 24); // Time since power-on or reboot as 0.1 second resolution counter
  beacon_data[11] = ((lastTenth & 0xFF0000) >> 16);   //
  beacon_data[12] = ((lastTenth & 0xFF00) >> 8);    //
  beacon_data[13] = (lastTenth & 0xFF);       //
  beacon_data[14] = ((hum) >> 8);    //
  beacon_data[15] = (hum & 0xFF);       //
  //beacon_data[12] = ((hum) >> 8);    //
  //beacon_data[13] = (hum & 0xFF);       //

  oScanResponseData.setServiceData(BLEUUID(beconUUID), std::string(beacon_data, 16));
  oAdvertisementData.setName("TLMBeacon");
  pAdvertising->setAdvertisementData(oAdvertisementData);
  pAdvertising->setScanResponseData(oScanResponseData);
}


void loop()
{
  float humi = dht.readHumidity();
  float temp = dht.readTemperature();
  Serial.print("Temperature: ");
  Serial.print(temp);
  Serial.print("ºC ");
  Serial.print("Humidity: ");
  Serial.println(humi);
  

  setBeacon();
  // Start advertising
  pAdvertising->start();
  delay(1000);
  Serial.printf("enter deep sleep for 10s\n");
  esp_deep_sleep(1000000LL * GPIO_DEEP_SLEEP_DURATION);
  pAdvertising->stop();
}
