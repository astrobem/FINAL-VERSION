#include <CanSatKit.h>
using namespace CanSatKit;

#include <TinyGPSPlus.h>
TinyGPSPlus gps;


Radio radio(Pins::Radio::ChipSelect,
            Pins::Radio::DIO0,
            433.0,                  
            Bandwidth_125000_Hz,   
            SpreadingFactor_9,     
            CodingRate_4_8);       

void setup() {
  SerialUSB.begin(9600);  
  Serial1.begin(9600);
 
  radio.begin();
 
}

void loop() {
  
  while (Serial1.available() > 0)

    if (gps.encode(Serial1.read()))

      displayInfo();
      
     
  if (millis() > 5000 && gps.charsProcessed() < 10)
 

  {

    SerialUSB.println(F("No GPS detected: check wiring."));

    while (true);

  }

  char data[256];

  radio.receive(data);

    SerialUSB.print("Received (RSSI = ");
  SerialUSB.print(radio.get_rssi_last());
  SerialUSB.print("): ");

  SerialUSB.println(data);
}
void displayInfo()

{

  SerialUSB.print(F("Location: "));

  if (gps.location.isValid()){
    
    SerialUSB.print("Lat: ");

    SerialUSB.print(gps.location.lat(), 6);

    SerialUSB.print(F(","));

    SerialUSB.print("Lng: ");

    SerialUSB.print(gps.location.lng(), 6);

    SerialUSB.print(F(","));

    SerialUSB.print("Alt: ");

    SerialUSB.print(gps.altitude.meters(), 2);

    SerialUSB.println();
   

  }  

  else

  {

    SerialUSB.print(F("INVALID"));
    

  }

}