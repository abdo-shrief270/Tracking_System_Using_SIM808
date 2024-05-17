// #include <SoftwareSerial.h>
// #include <DFRobot_SIM808.h>

// #define rxPin 11
// #define txPin 10

// SoftwareSerial mySerial(txPin, rxPin);
// DFRobot_SIM808 sim808(&mySerial);


// void setup() {
//   mySerial.begin(9600);
//   mySerial.println("AT+IPR=9600");
//   Serial.begin(9600);
//   Serial.println("Starting...");
//   while (!sim808.init()) {
//     delay(1000);
//     Serial.print("Sim808 init error\r\n");
//   }
//   //************* Turn on the GPS power************
//   if (sim808.attachGPS()) {
//     Serial.println("Open the GPS power success");
//   } else {
//     Serial.println("Open the GPS power failure");
//   }
// }

// void loop() {
//   if (sim808.getGPS()) {
//     Serial.print(sim808.GPSdata.year);
//     Serial.print("/");
//     Serial.print(sim808.GPSdata.month);
//     Serial.print("/");
//     Serial.print(sim808.GPSdata.day);
//     Serial.print(" ");
//     Serial.print(sim808.GPSdata.hour);
//     Serial.print(":");
//     Serial.print(sim808.GPSdata.minute);
//     Serial.print(":");
//     Serial.print(sim808.GPSdata.second);
//     Serial.print(":");
//     Serial.println(sim808.GPSdata.centisecond);

//     Serial.print("latitude :");
//     Serial.println(sim808.GPSdata.lat, 6);

//     sim808.latitudeConverToDMS();
//     Serial.print("latitude :");
//     Serial.print(sim808.latDMS.degrees);
//     Serial.print("\^");
//     Serial.print(sim808.latDMS.minutes);
//     Serial.print("\'");
//     Serial.print(sim808.latDMS.seconeds, 6);
//     Serial.println("\"");
//     Serial.print("longitude :");
//     Serial.println(sim808.GPSdata.lon, 6);
//     sim808.LongitudeConverToDMS();
//     Serial.print("longitude :");
//     Serial.print(sim808.longDMS.degrees);
//     Serial.print("\^");
//     Serial.print(sim808.longDMS.minutes);
//     Serial.print("\'");
//     Serial.print(sim808.longDMS.seconeds, 6);
//     Serial.println("\"");

//     Serial.print("speed_kph :");
//     Serial.println(sim808.GPSdata.speed_kph);
//     Serial.print("heading :");
//     Serial.println(sim808.GPSdata.heading);
//   }
// }





#include <SoftwareSerial.h>
SoftwareSerial SIM808(7, 8); //Seleccionamos los pines 7 como Rx y 8 como Tx

void setup()
{
 SIM808.begin(9600);
 Serial.begin(9600);

 SIM808.println("AT+IPR=9600");

 delay(100);
}

void loop()
{
 //Envíamos y recibimos datos
 if (Serial.available() > 0)
 SIM808.write(Serial.read());
 if (SIM808.available() > 0)
 Serial.write(SIM808.read());
}