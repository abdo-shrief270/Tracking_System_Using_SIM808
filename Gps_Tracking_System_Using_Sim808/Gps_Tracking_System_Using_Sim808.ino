/*
  Made on Jan 9, 2021
  By MehranMaleki @ Electropeak
  Home
*/

#include <SoftwareSerial.h>
#include <TinyGPS++.h>

boolean GetGPS_flag = false;
boolean Location_isValid_flag = false;
boolean Password_flag = false;


String value;
String phone_number;
String message_text;
String password = "Hello";
String Message;


#define PIN_TX    10
#define PIN_RX    11
SoftwareSerial mySerial(PIN_TX, PIN_RX);



// The TinyGPS++ object
TinyGPSPlus gps;

String SIM808(String value) {
  String Out;

  mySerial.println(value);
  delay(10);
  while (mySerial.available()) {
    Out = (mySerial.readString());
  }

  Out.trim();
  Out.remove(0, value.length() + 3);
  return Out;
}

String SIM808(String value, int count) {
  String Out;

  mySerial.println(value);
  delay(10);
  while (mySerial.available()) {
    Out = (mySerial.readString());
  }

  Out.trim();
  Out.remove(0, value.length() + 3 + count);
  return Out;
}

void setup() {
  mySerial.begin(9600);
  Serial.begin(9600);
  Serial.println("Start\n");
  Serial.println("Power UP The GPS");
  SIM808("AT+CGPSPWR=1");
}

void loop() {
  //******************* RECEIVING SMS *******************
  Serial.println("Check AT Connection");
  value = SIM808("AT");
  Serial.println(value);
  Serial.println("");

  Serial.println("Put The Module in SMS Text Mode");
  SIM808("AT+CMGF=1");
  delay(2000);


  Serial.println("Delete All Previous Messages");
  SIM808("AT+CMGD=1,4");
  delay(1000);

  do {
    Serial.println("No new Messages Received");
    value = SIM808("AT+CMGL=\"REC UNREAD\"");
    Serial.println(value);
    delay(1000);
  } while (value == "OK");

  Serial.println("Message Recieved");

  value = SIM808("AT+CMGL=\"ALL\"");
  Serial.println(value);
  phone_number = value;
  phone_number.remove(0, 20);
  phone_number.remove(15, phone_number.length());
  message_text = value;
  message_text.remove(0, 63);
  message_text.remove(message_text.length() - 6, message_text.length());

  Serial.println("Phone Number:");
  Serial.println(phone_number);

  Serial.println("Message Text:");
  Serial.println(message_text);


  if (message_text == password) {
    GetGPS_flag = true;
    Password_flag = true;
    Serial.println("password is correct!");
  }
  //******************* RECEIVING SMS END *******************



  //******************* GET GPS *******************
  if (GetGPS_flag) {
    do {

      Serial.println("Check The GPS Power");
      value = SIM808("AT+CGPSPWR?");
      value.remove(11, 17);
      Serial.println(value);
      //Serial.println("end");

      Serial.println("Disable The GPS Output");
      SIM808("AT+CGPSOUT=0");
      //Serial.println("end\n");

      Serial.println("Wait For The GPS To Find Location");
      while (!(SIM808("AT+CGPSSTATUS?" , 13).startsWith("Location 2D Fix") || SIM808("AT+CGPSSTATUS?" , 13).startsWith("Location 3D Fix"))) {
        Serial.println("Location Not Fixed Yet, Please Wait!");
        delay(1000);
      }
      Serial.println("Location Found!");
      Serial.println("end\n");



      //Get and Analyse The GPS Output
      Serial.println("Get and Analyse The GPS Output");
      String Out = "";
      mySerial.println("AT+CGPSOUT=2");

      delay(10);
      while (mySerial.available()) {
        mySerial.readStringUntil('\n');
        mySerial.readStringUntil('\n');
        Out = (mySerial.readStringUntil('\n'));  Out += "\r\n";
      }
      mySerial.println("AT+CGPSOUT=0");
      delay(100);
      mySerial.println("AT+CGPSOUT=32");
      delay(10);
      while (mySerial.available()) {
        mySerial.readStringUntil('\n');
        mySerial.readStringUntil('\n');
        Out += (mySerial.readStringUntil('\n'));  Out += "\r\n";
      }
      mySerial.println("AT+CGPSOUT=0");
      Out.trim();
      Serial.println(Out);
      Serial.println("");
      //GPS Output Analized


      int buffint = Out.length();
      char buff[buffint];
      Out.toCharArray(buff, buffint);
      const char *gpsStream = buff;

      while (*gpsStream)
        if (gps.encode(*gpsStream++))
          displayInfo();

      Serial.println("");
      Serial.println("");

      delay(100);
      if (gps.location.isValid())
      {
        Location_isValid_flag = true;
        Message = String(gps.location.lat(), 6);
        Message += ",";
        Message += String(gps.location.lng(), 6);
        Message += " ";
        Message += (char)26;
        Serial.println(Message);
      }
    } while (!Location_isValid_flag);

  }
  //******************* GET GPS END *******************




  //******************* SENDING SMS *******************
  Serial.println("Start Sending The SMS\n");


  Serial.println("Sending The SMS to");
  Serial.println(phone_number);

  SIM808("AT+CMGS=" + phone_number );
  delay(200);

  if (Password_flag == true) {
    mySerial.println("Open This Link:");
    mySerial.print("https://www.google.com/maps/place/");
    SIM808(Message);
  }
  else {
    mySerial.println("Password Incorrect! ");
  }
  Serial.println("Message Sent!");

  Serial.println("Delete All Previous Messages");
  SIM808("AT+CMGD=1,4");
  delay(2000);
  //******************* SENDING SMS END*******************

  //reinitializing flags
  GetGPS_flag = false;
  Location_isValid_flag = false;
  Password_flag = false;
}






void displayInfo()
{
  Serial.print(F("Location: "));
  if (gps.location.isValid())
  {
    Serial.print(gps.location.lat(), 6);
    Serial.print(F(","));
    Serial.print(gps.location.lng(), 6);
  }
  else
  {
    Serial.print(F("INVALID"));
  }

  Serial.println();
}

