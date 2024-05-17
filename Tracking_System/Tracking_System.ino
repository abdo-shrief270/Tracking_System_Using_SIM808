#include <SoftwareSerial.h>
#define PIN_TX    10
#define PIN_RX    11
SoftwareSerial mySerial(PIN_TX, PIN_RX);

String value,phone_number,message_text,password = "kaabosh",Message,Out,messageDetectedFlag ="SM";
String lat,lon;
double ddm_to_dd(double ddm) {
    double degrees = floor(ddm / 100.0);
    double minutes = ddm - degrees * 100.0;
    double decimal_degrees = degrees + minutes / 60.0;
    return decimal_degrees;
}

void SendComToSIM808(String value){
  mySerial.println(value);
  delay(10);
  while (mySerial.available()) {
    Out = (mySerial.readString());
  }
  Out.trim();
  Out.remove(0, value.length() + 3);
  Serial.println(Out);
}
void WaitForRespSIM808(){
  String str="";
  waitResp :
  while(!mySerial.available());
  str=mySerial.readString();
  if(str.indexOf("OK")>=0||str.indexOf("ROR")>=0)
    return;
  else
    goto waitResp;
}
String SendComToSIM808WithValue(String value){
  mySerial.println(value);
  delay(10);
  while (mySerial.available()) {
    Out = (mySerial.readString());
  }
  Out.trim();
  Out.remove(0, value.length() + 3);
  return Out;
}

void SendMesWithSIM808(String phoneNum,String Message){
  SendComToSIM808("AT+CMGS=\"" + phone_number +"\"\r\n");
  SendComToSIM808(Message);
  mySerial.print((char)26);
  WaitForRespSIM808();
}

void MakeCallWithSIM808(String phoneNum){
  // mySerial.println(value);
  // delay(10);
  // while (mySerial.available()) {
  //   Out = (mySerial.readString());
  // }
  // Out.trim();
  // Out.remove(0, value.length() + 3);
  // Serial.println(Out);
}

void getGPSDataFromSIM808(){
  String data="",temp="",rtn="";
  SendComToSIM808("AT+CGPSOUT=0");
  data = SendComToSIM808WithValue("AT+CGPSSTATUS?");
  if(data.indexOf("D Fix")>=0){
    Serial.println("GPS Is Fixed :)");
  }else{
    Serial.println("GPS Is Not Fixed :)");
  }
  data = SendComToSIM808WithValue("AT+CGPSINF=0");
  Serial.println(data);
  data.remove(0,12);
  data.remove(23,data.length());
  Serial.println(data);
  lon=data;
  lon.remove(lon.indexOf(","),lon.length());
  Serial.println(lon.c_str());
  lat=data;
  lat.remove(0,lat.indexOf(",")+1);
  Serial.println(lat.c_str());
}


void setup() {
  mySerial.begin(9600);
  Serial.begin(9600);
  Serial.println("Start");
  Serial.print("Check AT Connection : ");
  SendComToSIM808("AT");
  Serial.print("Power UP The GPS : ");
  SendComToSIM808("AT+CGPSPWR=1");
  Serial.print("Put The Module in SMS Text Mode : ");
  SendComToSIM808("AT+CMGF=1");
}

void loop() {
  String index="";
  String rtnSerial="";
  Serial.print("Delete All Previous Messages : ");
  SendComToSIM808("AT+CMGD=1,4");
  Serial.print("Waiting for new Messages : ");
  // || SendComToSIM808WithValue("AT+CMGR=1").indexOf("ROR")>=0
  while(!(mySerial.available())){
    Serial.print(".");
    delay(250);
  }
  rtnSerial=mySerial.readString();
  if(rtnSerial.indexOf(messageDetectedFlag)>=0){
    rtnSerial.remove(0,rtnSerial.indexOf(messageDetectedFlag) + 4);
    rtnSerial.remove(1,rtnSerial.length());
    Serial.println("\nMessage Recieved :)");
    Serial.println(rtnSerial);
    goto mesProcess;
  }
  return;



  // Serial.print("Waiting for new Messages : ");
  // do {
    
  //   value = SendComToSIM808WithValue("AT+CMGL");
  //   Serial.println(value);
  // } while (value == "OK");
  // Serial.println();
  // Serial.println("\nMessage Recieved :)");

  mesProcess :
  value = SendComToSIM808WithValue("AT+CMGR="+rtnSerial);
  Serial.println(value);
  // Get Phone Number
  phone_number = value;
  phone_number.remove(0, 21);
  phone_number.remove(13, phone_number.length());
  // Get Message Text
  message_text = value;
  message_text.remove(0, 63);
  // message_text.remove(message_text.length() - 6, message_text.length());

  Serial.print("Phone Number : ");
  Serial.println(phone_number);
  Serial.print("Message Text : ");
  Serial.println(message_text);
  if(message_text.indexOf(password)>=0){
    Serial.println("Message From Admin :)");
    if(message_text.indexOf("GET_GPS")>=0){
      Serial.println("Admin Want GPS Location :)");
      getGPSDataFromSIM808();
      Message="";
      Message="https://www.google.com/maps?z=18&t=m&q=loc:";
      Message+=ddm_to_dd(atof(lon.c_str()));
      Message+="+";
      Message+=ddm_to_dd(atof(lat.c_str()));
      Serial.println("mes is : "+Message);
      SendMesWithSIM808(phone_number,Message);
      Serial.println("GPS Data Was Sent To : "+phone_number);
    }else if(message_text.indexOf("AT") >= 0){
      message_text.remove(0,message_text.indexOf("AT"));
      Message =SendComToSIM808WithValue(message_text);
      SendMesWithSIM808(phone_number,Message);
      Serial.println("AT Response was sent to : "+phone_number);
    }
  }else{
    Serial.println("Message From UnKnown User !!");
    SendMesWithSIM808(phone_number,"Call The Creator To Know The Password, Foul one :\" ");
    Serial.println("Message Sent To : " + phone_number + " With Content : Call The Creator To Know The Password :\" ");
  }
}
