/******************************************************************************
  TinyGPSPlus_GPS_Shield.ino
  TinyGPS++ Library Example for the SparkFun GPS Logger Shield
  By Jim Lindblom @ SparkFun Electronics
  February 9, 2016
  https://github.com/sparkfun/GPS_Shield

  This example uses SoftwareSerial to communicate with the GPS module on
  pins 8 and 9. It uses the TinyGPS++ library to parse the NMEA strings sent
  by the GPS module, and prints interesting GPS information to the serial
  monitor.

  After uploading the code, open your serial monitor, set it to 9600 baud, and
  watch for latitude, longitude, altitude, course, speed, date, time, and the
  number of visible satellites.

  Resources:
  TinyGPS++ Library  - https://github.com/mikalhart/TinyGPSPlus/releases
  SoftwareSerial Library

  Development/hardware environment specifics:
  Arduino IDE 1.6.7
  GPS Logger Shield v2.0 - Make sure the UART switch is set to SW-UART
  Arduino Uno, RedBoard, Pro, etc.
******************************************************************************/
#include <SD.h> //SD card
#include <SPI.h> //SPI library

#include <TheThingsNetwork.h>

#include <TinyGPS++.h> // Include the TinyGPS++ library
TinyGPSPlus tinyGPS; // Create a TinyGPSPlus object

#define GPS_BAUD 9600 // GPS module baud rate. GP3906 defaults to 9600.

// If you're using an Arduino Uno, RedBoard, or any board that uses the
// 0/1 UART for programming/Serial monitor-ing, use SoftwareSerial:
#include <SoftwareSerial.h>
#define ARDUINO_GPS_RX 8 // GPS TX, Arduino RX pin
#define ARDUINO_GPS_TX 9 // GPS RX, Arduino TX pin
SoftwareSerial ssGPS(ARDUINO_GPS_TX, ARDUINO_GPS_RX); // Create a SoftwareSerial

int chipSelect = 4; //Chip select for SPI SD card
int firstPass = 0; //set flag for first pass for SD filename
String dateVal = "dateNow";
String timeVal = "timeNow";
String fileExt = ".CSV";
String fileName; //temporary store for SD card filename


// Set gpsPort to either ssGPS if using SoftwareSerial or Serial1 if using an
// Arduino with a dedicated hardware serial port
#define gpsPort ssGPS  // Alternatively, use Serial1 on the Leonardo

// Define the serial monitor port. On the Uno, and Leonardo this is 'Serial'
//  on other boards this may be 'SerialUSB'
#define SerialMonitor Serial

void setup()
{
  SerialMonitor.begin(9600);
  gpsPort.begin(GPS_BAUD);

  pinMode (10, OUTPUT); //Assigned to SD card but not used
  SD.begin(chipSelect); //Slave select on SPI for SD card

  

 // if (SD.exists("GPS.txt")) { //Check for file, delete if previous version
 //   SD.remove("GPS.txt");
 // }
}

void loop()
{
  // print position, altitude, speed, time/date, hdop and write to SD card
  printGPSInfo();

  // "Smart delay" looks for GPS data while the Arduino's not doing anything else
  smartDelay(30000); 
}

void printGPSInfo()
{
  // Print latitude, longitude, altitude in feet, course, speed, date, time,
  // and the number of visible satellites

  File GPSdata; //Declare variable for SD card object
  
  SerialMonitor.print("Lat: "); SerialMonitor.println(tinyGPS.location.lat(), 6);
  SerialMonitor.print("Long: "); SerialMonitor.println(tinyGPS.location.lng(), 6);
  SerialMonitor.print("Alt: "); SerialMonitor.println(tinyGPS.altitude.meters(), 1);
  SerialMonitor.print("HDOP: "); SerialMonitor.println(tinyGPS.hdop.value());
  SerialMonitor.print("Sats: "); SerialMonitor.println(tinyGPS.satellites.value());
  SerialMonitor.print("Date: "); printDate();
  SerialMonitor.print("Time: "); printTime();
  SerialMonitor.print("First Pass?: "); SerialMonitor.println(firstPass);
  SerialMonitor.println();

  if (tinyGPS.location.isValid())
    {
      if (firstPass == 0)
      {
        String dateVal = String(tinyGPS.date.value());
        String timeVal = String(tinyGPS.time.value());
        String fileName = String(dateVal + timeVal + fileExt); //create filename from current date and time
        SerialMonitor.print(dateVal); SerialMonitor.println(timeVal);
      } 

  SerialMonitor.print("Filename: "); SerialMonitor.println(String(fileName));
  
  GPSdata=SD.open(String(fileName),FILE_WRITE); //Open GPS data file on SD
  
  GPSdata.print (tinyGPS.date.value());      
  GPSdata.print (",");//Insert comma for parsing

  GPSdata.print(tinyGPS.time.value());
  GPSdata.print (",");//Insert comma for parsing

  GPSdata.print(tinyGPS.location.lng(), 6); //Write decimal longitude to SD card
  GPSdata.print(","); //Comma separator

  GPSdata.print(tinyGPS.location.lat(), 6); //Write decimal latitude to SD card
  GPSdata.print(","); //Comma separator

  GPSdata.print(tinyGPS.altitude.meters(), 1); //Write altitude
  GPSdata.print(","); //Comma separator
  
  GPSdata.println(tinyGPS.hdop.value()); //Write HDOP
      
  GPSdata.close(); //Close SD file

  firstPass = 1; //set firstpass complete flag
    }
}

// This custom version of delay() ensures that the tinyGPS object
// is being "fed". From the TinyGPS++ examples.
static void smartDelay(unsigned long ms)
{
  unsigned long start = millis();
  do
  {
    // If data has come in from the GPS module
    while (gpsPort.available())
      tinyGPS.encode(gpsPort.read()); // Send it to the encode function
    // tinyGPS.encode(char) continues to "load" the tinGPS object with new
    // data coming in from the GPS module. As full NMEA strings begin to come in
    // the tinyGPS library will be able to start parsing them for pertinent info
  } while (millis() - start < ms);
}

// printDate() formats the date into dd/mm/yy.
void printDate()
{
  SerialMonitor.print(tinyGPS.date.day());
  SerialMonitor.print("/");
  SerialMonitor.print(tinyGPS.date.month());
  SerialMonitor.print("/");
  SerialMonitor.println(tinyGPS.date.year());
}

// printTime() formats the time into "hh:mm:ss", and prints leading 0's
// where they're called for.
void printTime()
{
  SerialMonitor.print(tinyGPS.time.hour());
  SerialMonitor.print(":");
  if (tinyGPS.time.minute() < 10) SerialMonitor.print('0');
  SerialMonitor.print(tinyGPS.time.minute());
  SerialMonitor.print(":");
  if (tinyGPS.time.second() < 10) SerialMonitor.print('0');
  SerialMonitor.println(tinyGPS.time.second());
}
