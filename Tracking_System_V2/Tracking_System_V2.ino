#include <SoftwareSerial.h>
#define PIN_TX 10
#define PIN_RX 11
SoftwareSerial SIM808(PIN_TX, PIN_RX);

String serialRes, smsindex, phone_number, message_text, MES;
String lat_str, lon_str, emgMes = "help", password = "weeka", gpsKeyword = "GET_GPS";
boolean GPS_Data_Flag;

void InitSIM808(void);
void WaitForRespSIM808(String key);
String SendComToSIM808(String value);
boolean SendComToSIM808AndCheckForResp(String Com, String res);
double ddm_to_dd(double ddm);
void getGPSDataFromSIM808(void);
void sendSMSUsingSIM808(String phone, String Mes);
void processOnSMSMessage(String mes);
void makeCallUsingSIM808(String phone);

void setup() {
  Serial.begin(9600);
  SIM808.begin(9600);
  InitSIM808();
  sendSMSUsingSIM808("+201270989676", "SIM808 is ready to use :)");
}

void loop() {
  while (!SIM808.available());
  serialRes = SIM808.readString();

  if (serialRes.indexOf("SM") > -1)  // Message Recieved
  {
    smsindex = serialRes.substring(14, 15);
    serialRes = SendComToSIM808("AT+CMGR=" + smsindex);
    phone_number = serialRes.substring(23, 34);
    message_text = serialRes.substring(63, serialRes.length() - 6);
    processOnSMSMessage(message_text);
    while (!SendComToSIM808AndCheckForResp("AT+CMGD=1,4", "OK"));
  } else if (serialRes.indexOf("+CLCC") > -1)  // New Call
  {
    delay(2000);
    while (!SendComToSIM808AndCheckForResp("ATH", "OK"));
    phone_number = serialRes.substring(20, 31);
    getGPSDataFromSIM808();
    processOnSMSMessage(password + " " + gpsKeyword);
  }
}

void InitSIM808(void) {

  while (!SendComToSIM808AndCheckForResp("AT", "OK"));

  if (!SendComToSIM808AndCheckForResp("AT+CGPSPWR?", "1")) {
    while (!SendComToSIM808AndCheckForResp("AT+CGPSPWR=1", "OK"));
  }

  if (!SendComToSIM808AndCheckForResp("AT+CGPSOUT?", "0")) {
    while (!SendComToSIM808AndCheckForResp("AT+CGPSOUT=0", "OK"));
  }

  if (!SendComToSIM808AndCheckForResp("AT+CMGF?", "1")) {
    while (!SendComToSIM808AndCheckForResp("AT+CMGF=1", "OK"));
  }
}

void WaitForRespSIM808(String key) {
  String str = "";
  waitResp:
  while (!SIM808.available());
  str = SIM808.readString();
  if (str.indexOf(key) >= 0 || str.indexOf("ROR") >= 0)
    return;
  else
    goto waitResp;
}

String SendComToSIM808(String value) {
  String Out;
  SIM808.println(value);
  delay(10);
  while (!SIM808.available());
  Out = SIM808.readString();
  Out.trim();
  Out.remove(0, value.length() + 3);
  return Out;
}

boolean SendComToSIM808AndCheckForResp(String Com, String res) {
  String str;
  SIM808.println(Com);
  delay(10);
  while (!SIM808.available());
  str = SIM808.readString();
  if (str.indexOf(res) >= 0)
    return true;
  else
    return false;
}

double ddm_to_dd(double ddm) { // convert gps cordenates from degree deciemal minute to degree deciemal
  double degrees = floor(ddm / 100.0);
  double minutes = ddm - degrees * 100.0;
  return (degrees + minutes / 60.0);
}

void getGPSDataFromSIM808(void) {
  String data = "";
  if (SendComToSIM808AndCheckForResp("AT+CGPSSTATUS?", "D")) {

    GPS_Data_Flag = true;
    serialRes = SendComToSIM808("AT+CGPSINF=0");
    serialRes = serialRes.substring(12, 35);
    lon_str = serialRes.substring(0, 11);
    lat_str = serialRes.substring(serialRes.indexOf(",")+1);

  } else {
    GPS_Data_Flag = false;
  }
}

void sendSMSUsingSIM808(String phone, String Mes) {
  if(SendComToSIM808AndCheckForResp("AT+CMGS=\""+phone+"\"\r\n",">"))
  {
    if(SendComToSIM808AndCheckForResp(Mes,">"))
    {
      SIM808.print((char)26); // CTRL+Z
    }
  }
  WaitForRespSIM808("OK");
}

void processOnSMSMessage(String mes) {
  String str;
  if (mes.indexOf(password) > -1)  // message contains password keyword
  {
    mes.remove(mes.indexOf(password), mes.indexOf(password) + password.length() + 1);
    if (mes.indexOf(gpsKeyword) > -1)  // send gps data
    {
      mes.remove(mes.indexOf(gpsKeyword), mes.indexOf(gpsKeyword) + gpsKeyword.length());
      getGPSDataFromSIM808();
      if (GPS_Data_Flag) {
        MES = "Enter the next link to view the location : \nhttps://www.google.com/maps?z=18&t=m&q=loc:";
        MES += ddm_to_dd(atof(lon_str.c_str()));;
        MES += "+";
        MES += ddm_to_dd(atof(lat_str.c_str()));;
        sendSMSUsingSIM808(phone_number, MES);
      } else {
        sendSMSUsingSIM808(phone_number, "Gps Location Is Not Fixed Yet :(");
      }
    }
    if (mes.indexOf("AT") > -1)  // perform AT command
    {
      if(mes.indexOf("ATD")>-1){
        sendSMSUsingSIM808(phone_number, "Call is goining now :) ");
        str = SendComToSIM808(mes);
        delay(10000);
        str = SendComToSIM808("ATH");
      }else{
        str = SendComToSIM808(mes);
        sendSMSUsingSIM808(phone_number, str);
      }
    }
  } else {
    if (mes.indexOf(emgMes) > -1)  // emergency message
    {
      str = SendComToSIM808("ATD+201270989676;");
      delay(10000);
      str = SendComToSIM808("ATH");
      sendSMSUsingSIM808(phone_number, "Emergency Call was Sent !");
    }
  }
}

void makeCallUsingSIM808(String phone) {
  SIM808.println("ATD" + phone + ";");
  WaitForRespSIM808("OK");
  delay(10000);
  SIM808.println("ATH");
  WaitForRespSIM808("OK");
}
