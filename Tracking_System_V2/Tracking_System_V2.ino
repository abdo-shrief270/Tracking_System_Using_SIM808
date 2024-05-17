#include <SoftwareSerial.h> // Include the SoftwareSerial library

#define PIN_TX 10 // Define the TX pin
#define PIN_RX 11 // Define the RX pin

SoftwareSerial SIM808(PIN_TX, PIN_RX); // Create a SoftwareSerial object for SIM808

// Define global variables
String ADMIN_NUMBER = "01555440882"; // Admin phone number

String serialRes; // Stores responses from SIM808
String smsindex; // Stores the index of the SMS
String phone_number; // Stores the phone number of the sender
String message_text; // Stores the text of the received message
String MES; // General purpose message string
String lat_str; // Latitude string
String lon_str; // Longitude string
String emgMes = "help"; // Emergency message keyword
String password = "weeka"; // Password for authorized commands
String gpsKeyword = "GET_GPS"; // Keyword to get GPS data
boolean GPS_Data_Flag; // Flag to indicate if GPS data is available

// Function prototypes
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
  Serial.begin(9600); // Initialize Serial communication
  SIM808.begin(9600); // Initialize SIM808 communication
  InitSIM808(); // Initialize SIM808 module
  sendSMSUsingSIM808(ADMIN_NUMBER, "SIM808 is ready to use :)"); // Send initialization message
}

void loop() {
  while (!SIM808.available()); // Wait for data from SIM808
  serialRes = SIM808.readString(); // Read the response

  if (serialRes.indexOf("SM") > -1) { // Check if a message is received
    smsindex = serialRes.substring(14, 15); // Extract SMS index
    serialRes = SendComToSIM808("AT+CMGR=" + smsindex); // Read the SMS
    phone_number = serialRes.substring(23, 34); // Extract phone number
    message_text = serialRes.substring(63, serialRes.length() - 6); // Extract message text
    processOnSMSMessage(message_text); // Process the message
    while (!SendComToSIM808AndCheckForResp("AT+CMGD=1,4", "OK")); // Delete all SMS
  } else if (serialRes.indexOf("+CLCC") > -1) { // Check if there's a new call
    delay(2000); // Wait for 2 seconds
    while (!SendComToSIM808AndCheckForResp("ATH", "OK")); // Hang up the call
    phone_number = serialRes.substring(20, 31); // Extract phone number
    getGPSDataFromSIM808(); // Get GPS data
    processOnSMSMessage(password + " " + gpsKeyword); // Process GPS request
  }
}

void InitSIM808(void) {
  while (!SendComToSIM808AndCheckForResp("AT", "OK")); // Check if SIM808 is ready

  if (!SendComToSIM808AndCheckForResp("AT+CGPSPWR?", "1")) { // Check if GPS power is on
    while (!SendComToSIM808AndCheckForResp("AT+CGPSPWR=1", "OK")); // Turn on GPS power
  }

  if (!SendComToSIM808AndCheckForResp("AT+CGPSOUT?", "0")) { // Check if GPS output is off
    while (!SendComToSIM808AndCheckForResp("AT+CGPSOUT=0", "OK")); // Turn off GPS output
  }

  if (!SendComToSIM808AndCheckForResp("AT+CMGF?", "1")) { // Check if SMS mode is set to text
    while (!SendComToSIM808AndCheckForResp("AT+CMGF=1", "OK")); // Set SMS mode to text
  }
}

void WaitForRespSIM808(String key) {
  String str = "";
  waitResp:
  while (!SIM808.available()); // Wait for response
  str = SIM808.readString(); // Read response
  if (str.indexOf(key) >= 0 || str.indexOf("ROR") >= 0) // Check for key or error
    return; // Return if key or error found
  else
    goto waitResp; // Wait again if key or error not found
}

String SendComToSIM808(String value) {
  String Out;
  SIM808.println(value); // Send command to SIM808
  delay(10); // Short delay
  while (!SIM808.available()); // Wait for response
  Out = SIM808.readString(); // Read response
  Out.trim(); // Trim whitespace
  Out.remove(0, value.length() + 3); // Remove command echo
  return Out; // Return response
}

boolean SendComToSIM808AndCheckForResp(String Com, String res) {
  String str;
  SIM808.println(Com); // Send command to SIM808
  delay(10); // Short delay
  while (!SIM808.available()); // Wait for response
  str = SIM808.readString(); // Read response
  if (str.indexOf(res) >= 0) // Check if response contains expected string
    return true; // Return true if found
  else
    return false; // Return false if not found
}

double ddm_to_dd(double ddm) { // Convert DDM to DD
  double degrees = floor(ddm / 100.0); // Extract degrees
  double minutes = ddm - degrees * 100.0; // Extract minutes
  return (degrees + minutes / 60.0); // Return converted value
}

void getGPSDataFromSIM808(void) {
  String data = "";
  if (SendComToSIM808AndCheckForResp("AT+CGPSSTATUS?", "D")) { // Check GPS status

    GPS_Data_Flag = true; // Set GPS data flag
    serialRes = SendComToSIM808("AT+CGPSINF=0"); // Get GPS information
    serialRes = serialRes.substring(12, 35); // Extract relevant data
    lon_str = serialRes.substring(0, 11); // Extract longitude
    lat_str = serialRes.substring(serialRes.indexOf(",")+1); // Extract latitude

  } else {
    GPS_Data_Flag = false; // Clear GPS data flag
  }
}

void sendSMSUsingSIM808(String phone, String Mes) {
  if(SendComToSIM808AndCheckForResp("AT+CMGS=\""+phone+"\"\r\n",">")) { // Send SMS command
    if(SendComToSIM808AndCheckForResp(Mes,">")) { // Send message text
      SIM808.print((char)26); // Send Ctrl+Z to indicate end of message
    }
  }
  WaitForRespSIM808("OK"); // Wait for OK response
}

void processOnSMSMessage(String mes) {
  String str;
  if (mes.indexOf(password) > -1) { // Check if message contains password
    mes.remove(mes.indexOf(password), mes.indexOf(password) + password.length() + 1); // Remove password
    if (mes.indexOf(gpsKeyword) > -1) { // Check if message contains GPS keyword
      mes.remove(mes.indexOf(gpsKeyword), mes.indexOf(gpsKeyword) + gpsKeyword.length()); // Remove GPS keyword
      getGPSDataFromSIM808(); // Get GPS data
      if (GPS_Data_Flag) { // If GPS data is available
        MES = "Enter the next link to view the location : \nhttps://www.google.com/maps?z=18&t=m&q=loc:"; // Create Google Maps link
        MES += ddm_to_dd(atof(lon_str.c_str())); // Append longitude
        MES += "+";
        MES += ddm_to_dd(atof(lat_str.c_str())); // Append latitude
        sendSMSUsingSIM808(phone_number, MES); // Send SMS with location
      } else {
        sendSMSUsingSIM808(phone_number, "Gps Location Is Not Fixed Yet :("); // Send error message
      }
    }
    if (mes.indexOf("AT") > -1) { // Check if message contains AT command
      if(mes.indexOf("ATD")>-1){
        sendSMSUsingSIM808(phone_number, "Call is going now :) "); // Send SMS indicating call
        str = SendComToSIM808(mes); // Execute AT command
        delay(10000); // Wait for 10 seconds
        str = SendComToSIM808("ATH"); // Hang up the call
      } else {
        str = SendComToSIM808(mes); // Execute AT command
        sendSMSUsingSIM808(phone_number, str); // Send response
      }
    }
  } else {
    if (mes.indexOf(emgMes) > -1) { // Check if message contains emergency keyword
      str = SendComToSIM808("ATD"+ADMIN_NUMBER+";"); // Make an emergency call
      delay(10000); // Wait for 10 seconds
      str = SendComToSIM808("ATH"); // Hang up the call
      sendSMSUsingSIM808(phone_number, "Emergency Call was Sent !"); // Send confirmation
    }
  }
}

void makeCallUsingSIM808(String phone) {
  SIM808.println("ATD" + phone + ";"); // Send AT command to make a call
  WaitForRespSIM808("OK"); // Wait for OK response
  delay(10000); // Wait for 10 seconds
  SIM808.println("ATH"); // Send AT command to hang up
  WaitForRespSIM808("OK"); // Wait for OK response
}
