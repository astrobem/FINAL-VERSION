#include <Wire.h>
#include <SPI.h>
#include <Adafruit_BMP280.h>    //czujnik temperatury
Adafruit_BMP280 bmp;

#include <Adafruit_MPU6050.h>   //czujnik akcerelometr
Adafruit_MPU6050 mpu;

#include <Adafruit_Sensor.h>    
#include <CanSatKit.h>    //cansatowa biblioteka
using namespace CanSatKit;

#include <TinyGPSPlus.h>    //gps
TinyGPSPlus gps;
#define SerialGPS Serial1

#include <SD.h>    //biblioteka karty sd
File dataFile;

#define BMP_SCK  (13)
#define BMP_MISO (12)
#define BMP_MOSI (11)
#define BMP_CS   (10)

double force; //nacisk
double temperature;
double pressure;
//double altittude;
double acceleration_x;
double acceleration_y;
double acceleration_z;
double time;
double lat;
double lng;
double alt;
double rotation_x;
double rotation_y;
double rotation_z;
Radio radio(Pins::Radio::ChipSelect,
            Pins::Radio::DIO0,
            433.0,
            Bandwidth_125000_Hz,
            SpreadingFactor_9,
            CodingRate_4_8);

// create (empty) radio frame object that can store data
// to be sent via radio
Frame frame;

String JsonData;

// int readDiode = D0;

bool withSerial = true;

void setup() {
  if ( withSerial ) {
    SerialUSB.begin(9600); 
    SerialUSB.println(F("BMP280 test"));    
  }
  SerialGPS.begin(9600);  
  unsigned status;

  (!mpu.begin()); //dodalem jdiopeiro

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);  //bez tego sie lekko dekalibruje
  
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
 
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ); //dotad
 
  status = bmp.begin(0x76); //tu sie wpisuje port z i2c 
  /*
  if (!status) {
    SerialUSB.println(F("Could not find a valid BMP280 sensor, check wiring or "
                      "try a different address!"));
    SerialUSB.print("SensorID was: 0x"); SerialUSB.println(bmp.sensorID(),16);
    SerialUSB.print("        ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n");
    SerialUSB.print("   ID of 0x56-0x58 represents a BMP 280,\n");
    SerialUSB.print("        ID of 0x60 represents a BME 280.\n");
    SerialUSB.print("        ID of 0x61 represents a BME 680.\n");
    while (1) delay(10);
    */

  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,    
                  Adafruit_BMP280::SAMPLING_X2,   
                  Adafruit_BMP280::SAMPLING_X16,   
                  Adafruit_BMP280::FILTER_X16,      
                  Adafruit_BMP280::STANDBY_MS_500);

  SD.begin(11);

  radio.begin();
  if ( withSerial ) {
    SerialUSB.begin(9600);
    SerialUSB.println();
  }
}

bool save_to_file(String filename, String content, bool newlane = true)
{
  dataFile = SD.open(filename, FILE_WRITE);
  if (!dataFile) {  
    return false; 
  }
  if (newlane) {
    dataFile.println(content);    
  } else {
    dataFile.print(content);
  }

  return true;
}

void loop() {
  
  sensors_event_t a, g, temp; //dodalem dopeiro
  mpu.getEvent(&a, &g, &temp); //musi byc temp

  // digitalWrite(readDiode, HIGH);
  time = millis() / 1000;
  force = 1023.0 - analogRead(A0); //nacisk
  temperature = bmp.readTemperature();
  pressure = bmp.readPressure();
//altittude = bmp.readAltitude(999.25);
  acceleration_x = a.acceleration.x;
  acceleration_y = a.acceleration.y;
  acceleration_z = a.acceleration.z;
  lat = gps.location.lat();
  lng = gps.location.lng();
  alt = gps.altitude.meters();
  rotation_x = g.gyro.x;
  rotation_y = g.gyro.y;
  rotation_z = g.gyro.z;

  if ( withSerial ) {    
    SerialUSB.println("force: " + (String(force,2)));
    SerialUSB.println("temperature: " + (String(temperature,2)) + " C");
    SerialUSB.println("pressure: " + (String(pressure,2)) + " Pa");
/*    SerialUSB.println("approx. altitude = " + (String(altittude,2)) + " m");  a tutaj jeszcze do stringa => , \"altitude\": " + (String(altittude,2)) + " w razie co*/
    SerialUSB.println("acceleration_x = " + (String(acceleration_x,2)));
    SerialUSB.print("acceleration_y = " + (String(acceleration_y,2)));
    SerialUSB.print("acceleration_z = " + (String(acceleration_z,2)) + " m/s^2"); 
    SerialUSB.print("rotation_x = " + (String(rotation_x,2)));
    SerialUSB.print("rotation_y = " + (String(rotation_y,2)));
    SerialUSB.print("rotation_z = " + (String(rotation_z,2)) + " rad/s");
  }
  
  JsonData = "{\"force\":" + (String(force,2)) + ",\"temperature\": " + (String(temperature,2)) + ",\"pressure\": " + (String(pressure,2)) + ",\"acceleration_x\": " + (String(acceleration_x,2)) + ",\"acceleration_y\": " + (String(acceleration_y,2)) + ",\"acceleration_z\": " + (String(acceleration_z,2)) +",\"time\": " + (String(time,2)) + ",\"rotation_x\": " + (String(rotation_x,2)) + ",\"rotation_y\": " + (String(rotation_y,2)) + ",\"rotation_z\": " + (String(rotation_z,2)) + ",\"lat\": " + (String(lat,6)) + ",\"lng\": " + (String(lng,6)) + ",\"alt\": " + (String(alt,2)) + "}";

  frame.print(JsonData);

  radio.transmit(frame);
  frame.clear();

  save_to_file("final.txt", JsonData);
      dataFile.close();                    //ŁUKASZ musi byc close file ziomek
   
  while (SerialGPS.available() > 0)

  if (gps.encode(SerialGPS.read()));

    displayInfo();
  delay(1000);
}
void displayInfo()

{

 // SerialUSB.println(F("Location: "));

  if (gps.location.isValid()){
    if ( withSerial )
    {
      SerialUSB.println();
      SerialUSB.print("Lat: ");
      SerialUSB.print(lat, 6);
      SerialUSB.print(F(","));
      SerialUSB.print("Lng: ");
      SerialUSB.print(lng, 6);
      SerialUSB.print(F(","));
      SerialUSB.print("Alt: ");
      SerialUSB.print(alt, 2);
      SerialUSB.println();
    }
  }  


}
