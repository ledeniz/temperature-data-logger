/*
  Temperature Data Logger

  Data is written as a CSV file,
  structured as following:
    - temperature
    - unit (C or F)
    - timestamp (human readable)
    - timestamp (machine readable, unix)

    example: 2023-10-8 22:21:45,24.00,C,3782774626

*/

//-- includes ------------------------------
// for microSD 
#include <SPI.h>
#include <SD.h>

// for RTC
#include <Wire.h>
#include <I2C_RTC.h>

// for thermometer
#include <OneWire.h>
#include <DallasTemperature.h>

//-- config ------------------------------
#define UNIT 'C'

const unsigned short thermometerPin  = 10;
const unsigned short sdChipSelectPin = 7;

const unsigned int measurementInterval = 10000;

//-- setup ------------------------------
OneWire oneWire(thermometerPin);
DallasTemperature thermometer(&oneWire);

static DS3231 RTC;

float temperature;
String filename;

void setup() {
  Serial.begin(9600);
  delay(2000); // instead of `while (!Serial)` - primary use case is without serial connection
  Serial.println(__TIMESTAMP__);

  RTC.begin();
  if (false == RTC.isRunning()) {
    Serial.println("RTC NOT running");
    setTime();
  } else {
    Serial.println("RTC is running");
  }

  if (false == SD.begin(sdChipSelectPin)) {
    Serial.println("SD card initialization failed");
    while (1);
  }

  thermometer.begin();

  // sprintf(filename, "temperaturelog-%d%d%d-%d%d%d.txt", RTC.getYear(), RTC.getMonth(), RTC.getDay(), RTC.getHours(), RTC.getMinutes(), RTC.getSeconds());
  
  // filename = String(
  //   String("temperaturelog-") +
  //   String(2023)  + 
  //   String(10)    + 
  //   String(6)     +
  //   String("-")   + 
  //   String(20)    + 
  //   String(6)     + 
  //   String(42)    +
  //   String(".txt")
  // );

  filename = String("temperaturelog.txt");
  Serial.println(String("Begin logging to file: ") + String(filename));

  writeData("/////////////////////////////////////");
}

void loop() {
  temperature = readTemperature();

  String dataString = getDataString();
  Serial.println(dataString);

  writeData(dataString);

  delay(measurementInterval);
}

void setTime() {
  RTC.setHourMode(CLOCK_H24);  
  RTC.setDateTime(__DATE__, __TIME__);
  RTC.updateWeek();

  RTC.startClock();

  Serial.println("RTC time set to: " + getFormattedTimestamp());
}

float readTemperature() {
  thermometer.requestTemperatures();

  if (UNIT == 'C') {
    return thermometer.getTempCByIndex(0);
  } else {
    return thermometer.getTempFByIndex(0);
  }
}

String getDataString() {
  String dataString;

  dataString = String(
    String(temperature)     + ',' +
    String(UNIT)            + ',' +
    getFormattedTimestamp() + ',' +
    getTimestamp()
  );

  return dataString;
}

String getTimestamp() {
  String timestamp = String(RTC.getEpoch());

  return timestamp;
}

String getFormattedTimestamp() {
  return String(
    String(RTC.getYear())
    + "-" + String(RTC.getMonth())
    + "-" + String(RTC.getDay())
    + " " + String(RTC.getHours())
    + ":" + String(RTC.getMinutes())
    + ":" + String(RTC.getSeconds())
  );
}

bool writeData(String data) {
  File file = SD.open("data.txt", FILE_WRITE);

  if (file) {
    file.println(data);
    file.close();

    return true;
  } else {
    return false;
  }
}

