// project LAN/SNMP enviromental sensor
// wiring
// Ethernet W5500 lite
// needs external power
// STM32	<-> w5500
// PA4	<-> CS
// PA5	<-> SCK
// PA6	<-> MISO
// PA7	<-> MOSI
// sensors DHT21
// sensor1 <-> PA1, sensor2 <-> PA2 


#include <SPI.h>
#include <Ethernet.h>
#include <SNMP.h>
#include <DHT.h>
#include <FlashStorage_STM32.h>

#define SERIAL_BAUD_RATE 9600
#define SETUP_DELAY 5000

#define LED_STATUS PC13
#define LED_ON LOW
#define LED_OFF HIGH

struct UserSettings{
  char devicename[40];
  char location[40];
  char description[40];
  char contact[40];
  boolean dhcp;
  byte ip1;
  byte ip2;
  byte ip3;
  byte ip4;
  byte mask1;
  byte mask2;
  byte mask3;
  byte mask4;
  byte gate1;
  byte gate2;
  byte gate3;
  byte gate4;
};

typedef enum StatusLed {
  DHCP_WAITING,
  RUNNING,
};

typedef enum StatusLedRate {
  DHCP_WAITING_ON = 500000,
  DHCP_WAITING_OFF = 500000,
  RUNNING_ON = 50000,
  RUNNING_OFF = 3000000,
};

#if defined(TIM1)
  TIM_TypeDef *Instance = TIM1;
#else
  TIM_TypeDef *Instance = TIM2;
#endif

HardwareTimer *statusLedTimer = new HardwareTimer(Instance);

StatusLed statusLed;
StatusLedRate statusLedRateOn;
StatusLedRate statusLedRateOff;

#define pinDHT1 PA1
#define typeDHT1 DHT21
#define pinDHT2 PA2
#define typeDHT2 DHT21

#define ETHERNET_INIT PA4
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEC };

const char *SNMP_VERSION[2]{
  "v1",  // 0
  "v2c"  // 1
};

EthernetUDP udp;
SNMP::Agent snmp;

using SNMP::OctetStringBER;
using SNMP::IntegerBER;
using SNMP::VarBind;
using SNMP::VarBindList;
using SNMP::ObjectIdentifierBER;



// SNMP - community
const PROGMEM char *ALLOWED_COMMUNITY = "read";
// SNMMP - implemented OID's
const PROGMEM char *SYSOBJECTID_OID = "1.3.6.1.2.1.1.2.0";
const PROGMEM char *SYSNAME_OID = "1.3.6.1.2.1.1.5.0";
const PROGMEM char *SYSDESCRIPTION_OID = "1.3.6.1.2.1.1.1.0";
const PROGMEM char *SYSLOCATION_OID = "1.3.6.1.2.1.1.6.0";
const PROGMEM char *SYSCONTACT_OID = "1.3.6.1.2.1.1.4.0";
const PROGMEM char *SYSUPTIME_OID = "1.3.6.1.2.1.1.3.0";

const PROGMEM char *SENSOR1TEMP_OID = "1.3.6.1.4.1.148.1.1.1";
const PROGMEM char *SENSOR1HUM_OID = "1.3.6.1.4.1.148.1.1.2";
const PROGMEM char *SENSOR2TEMP_OID = "1.3.6.1.4.1.148.1.2.1";
const PROGMEM char *SENSOR2HUM_OID = "1.3.6.1.4.1.148.1.2.2";
// SNMP default values
const PROGMEM char *SYSOBJECTID_DEFAULT_VALUE = "1.3.6.1.4.1.8072.1.1";
const PROGMEM char *SYSNAME_DEFAULT_VALUE = "STM32 Enviro sensor (LAN/SNMP)";
const PROGMEM char *SYSDESCRIPTION_DEFAULT_VALUE = "snmp description";
const PROGMEM char *SYSLOCATION_DEFAULT_VALUE = "snmp location";
const PROGMEM char *SYSCONTACT_DEFAULT_VALUE = "snmp contact";


const PROGMEM String HTTP_HEADER_GET_STRING = "GET ";


DHT sensor1(pinDHT1, typeDHT1);
DHT sensor2(pinDHT2, typeDHT2);

EthernetServer server(80);

enum class Status {REQUEST, CONTENT_LENGTH, EMPTY_LINE, BODY};
Status status = Status::REQUEST;


UserSettings userSettings;
int eeAddress = 0;

void handleClientRequest(){

}


void setup() {
  delay(SETUP_DELAY); // delay 6sec because of enought time to manual restart putty

  // SETUP serial communication and begin communication
  Serial.begin(SERIAL_BAUD_RATE);

  // STATUS LED init
  pinMode(LED_STATUS, OUTPUT);
  digitalWrite(LED_STATUS, LED_OFF);

  // Setup LED Timer
  statusLed = StatusLed::DHCP_WAITING;
  statusLedTimer->setOverflow(5000, MICROSEC_FORMAT);
  statusLedTimer->attachInterrupt(handler_status_led);
  statusLedTimer->resume();

  // ETHERNET W5100 initialization
  Ethernet.init(ETHERNET_INIT);
  
  while (Ethernet.linkStatus() == Unknown) {
    delay(SETUP_DELAY);
  }
  while (Ethernet.linkStatus() == LinkOFF) {
    delay(SETUP_DELAY);
  }
  while (Ethernet.begin(mac) == 0) {

  }
  sensor1.begin();
  sensor2.begin();

  snmp.begin(&udp);
  snmp.onMessage(onMessage);

  // server.on("/", HTTP_POST, hadleClientRequest);
  server.begin();

  statusLed = StatusLed::RUNNING;
}

void loop() {
  snmp.loop();

  // testClient();
  checkForClient();

}

void handler_status_led(void) {
  switch (statusLed) {
    case DHCP_WAITING:
      statusLedRateOn = StatusLedRate::DHCP_WAITING_ON;
      statusLedRateOff = StatusLedRate::DHCP_WAITING_OFF;
      break;

    case RUNNING:
      statusLedRateOn = StatusLedRate::RUNNING_ON;
      statusLedRateOff = StatusLedRate::RUNNING_OFF;
      break;
  }

  if (digitalRead(LED_STATUS) == LED_OFF) {
    digitalWrite(LED_STATUS, LED_ON);
    statusLedTimer->setOverflow(statusLedRateOff, MICROSEC_FORMAT);
  } else {
    digitalWrite(LED_STATUS, LED_OFF);
    statusLedTimer->setOverflow(statusLedRateOn, MICROSEC_FORMAT);
  }
}

String ipToString(IPAddress address) {
  return String(address[0]) + "." + String(address[1]) + "." + String(address[2]) + "." + String(address[3]);
}

float getTemperature(DHT &sensor) {
  float temp = sensor.readTemperature();
  if (isnan(temp))             
  {
    //Serial.println("ERROR reading sensor!");       
  }
  return temp;
}

float getHumidity(DHT &sensor) {
  float hum = sensor.readHumidity();
  if (isnan(hum))             
  {
    //Serial.println("ERROR reading sensor!");       
  }
  return hum;
}

void onMessage(const SNMP::Message *message, const IPAddress remote, const uint16_t port) {
  //echo("received message from: ", remote, "");
  VarBindList *varbindlist = message->getVarBindList();
  if (strcmp(ALLOWED_COMMUNITY, message->getCommunity()) == 0) {
    for (unsigned int index = 0; index < varbindlist->count(); ++index) {
      VarBind *varbind = (*varbindlist)[index];
      const char *name = varbind->getName();
      OctetStringBER *value = nullptr;
      IntegerBER *valueInt = nullptr;
      ObjectIdentifierBER *valueObject = nullptr;

      //Serial.print("RequestID: " + (String)message->getRequestID());
      //Serial.println(", OID: " + (String)name);

      if (strcmp(SYSOBJECTID_OID, name) == 0)
        value = new OctetStringBER(SYSOBJECTID_DEFAULT_VALUE);
      else if (strcmp(SYSNAME_OID, name) == 0)
        value = new OctetStringBER(SYSNAME_DEFAULT_VALUE);
      else if (strcmp(SYSCONTACT_OID, name) == 0)
        value = new OctetStringBER(SYSCONTACT_DEFAULT_VALUE);
      else if (strcmp(SYSLOCATION_OID, name) == 0)
        value = new OctetStringBER(SYSLOCATION_DEFAULT_VALUE);
      else if (strcmp(SYSDESCRIPTION_OID, name) == 0)
        value = new OctetStringBER(SYSDESCRIPTION_DEFAULT_VALUE);
      else if (strcmp(SYSUPTIME_OID, name) == 0) {
        unsigned long millisec = millis();
        valueInt = new IntegerBER(millisec/10);
      } else if (strcmp(SENSOR1TEMP_OID, name) == 0) {
        int tempInt = getTemperature(sensor1) * 100;
        valueInt = new IntegerBER(tempInt);
      } else if (strcmp(SENSOR1HUM_OID, name) == 0) {
        int tempHum = getHumidity(sensor1) * 100;
        valueInt = new IntegerBER(tempHum);
      }  else if (strcmp(SENSOR2TEMP_OID, name) == 0) {
        int tempInt = getTemperature(sensor2) * 100;
        valueInt = new IntegerBER(tempInt);
      } else if (strcmp(SENSOR2HUM_OID, name) == 0) {
        int tempHum = getHumidity(sensor2) * 100;
        valueInt = new IntegerBER(tempHum);
      } else {
        value = NULL;
      }


      SNMP::Message *response = new SNMP::Message(SNMP::VERSION2C, ALLOWED_COMMUNITY, SNMP::TYPE_GETRESPONSE);

      response->setRequestID(message->getRequestID());

      if (valueInt != NULL)
        response->add(name, valueInt);
      else if (valueObject != NULL)
        response->add(name, valueObject);
      else
        response->add(name, value);

      if (value == NULL && valueInt == NULL && valueObject == NULL) {

      } else {
        snmp.send(response, remote, port);
      }

      delete response;
    }
  } else {
    // neodpovidame, tvarime se ze nezijeme
  }
}

String millisToString(unsigned long tmillisec, int type = 0) {
  int millisec = tmillisec % 100;
  int tseconds = tmillisec / 1000;
  int tminutes = tseconds / 60;
  int thours = tminutes / 60;
  int tdays = thours / 24;
  int seconds = tseconds % 60;
  int minutes = tminutes % 60;
  int hours = thours % 24;
  if(type == 0)
    return String(String(tdays) + "days " + String(hours) + "h " + String(minutes) + "m " + String(seconds) + "." + String(millisec) + "s");
  else if (type == 1)
    return String(String(tseconds) + "." + String(millisec) + "s");
}

void parseAndStore(String pair) {
  int equalsIndex = pair.indexOf('=');
  if (equalsIndex != -1) {
    String key = pair.substring(0, equalsIndex);
    String value = pair.substring(equalsIndex + 1);
    value.replace("+", " ");

    if(key == "devicename"){
      value.toCharArray(userSettings.devicename, value.length()+1);
    }
    if(key == "description"){
      value.toCharArray(userSettings.description, value.length()+1);
    }
    if(key == "location"){
      value.toCharArray(userSettings.location, value.length()+1);
    }
    if(key == "contact"){
      value.toCharArray(userSettings.contact, value.length()+1);
    }
  }
}

void checkForClient(){
    // listen for incoming clients
  EthernetClient client = server.available();
  String clientRequest = "";
  if (client) {
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        //Serial.write(c);
        clientRequest += c;
        //Serial.println(String(c).indexOf("/save?"));
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          //client.println("Refresh: 5");  // refresh the page automatically every 5 sec
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");

          UserSettings loadedSettings;
          if(EEPROM.isValid()){
            EEPROM.get(eeAddress, loadedSettings);
          }
          client.println("<h1>Device name: " + String(loadedSettings.devicename) + "</h1>");
          client.println("<h2>Description: " + String(loadedSettings.description) + "</h2>");
          client.println("<h3>Location: " + String(loadedSettings.location) + "</h3>");
          client.println("<h3>Contact: " + String(loadedSettings.contact) + "</h3>");
          client.println("<p>UpTime: " + millisToString(millis()) + "</p>");
          client.println("<table><thead><tr><th>Sensor</th><th>Teplota</th><th>Vlhkost</th></tr></thead>");
          client.println("<tbody><tr><td>Sensor 1</td>");
          client.println("<td>" + String((float)getTemperature(sensor1)) + "&nbsp;&deg;C</td>");
          client.println("<td>vlhkost: " + String((float)getHumidity(sensor1)) + "&nbsp;&percnt;</td>");
          client.println("</tr><tr><td>Sensor 2 <i>(ext)</i></td>");
          client.println("<td>" + String((float)getTemperature(sensor2)) + "&nbsp;&deg;C</td>");
          client.println("<td>vlhkost: " + String((float)getHumidity(sensor2)) + "&nbsp;&percnt;</td>");
          client.println("</tbody></table>");

          client.println("<a href=\"\">home</a>");

          client.println(F("<form action=\"save\" method=\"get\">"));
          client.println(F("<table><tr><th>Device name:</th><td>"));
          client.println(F("<input type=\"text\" name=\"devicename\" maxlength=\"20\" />"));
          client.println(F("</td></tr>"));
          client.println(F("<tr><th>Location:</th><td>"));
          client.println(F("<input type=\"text\" name=\"location\" maxlength=\"20\" />"));
          client.println(F("</td></tr>"));
          client.println(F("<tr><th>Description:</th><td>"));
          client.println(F("<input type=\"text\" name=\"description\" maxlength=\"20\" />"));
          client.println(F("</td></tr>"));
          client.println(F("<tr><th>Contact:</th><td>"));
          client.println(F("<input type=\"text\" name=\"contact\" maxlength=\"20\" />"));
          client.println(F("</td></tr>"));
          client.println(F("<tr><th>&nbsp;</th><td>"));
          client.println(F("<input type=\"submit\" value=\"Send\" name=\"Action\" />"));
          client.println(F("</table></form>"));
          client.println(F("</html>"));
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
  
    if(clientRequest.indexOf(HTTP_HEADER_GET_STRING) >= 0){
      //Serial.println(clientRequest.indexOf(HTTP_HEADER_GET_STRING) + HTTP_HEADER_GET_STRING.length());
      String url = clientRequest.substring(clientRequest.indexOf(HTTP_HEADER_GET_STRING) + HTTP_HEADER_GET_STRING.length(), clientRequest.indexOf(" ", clientRequest.indexOf(HTTP_HEADER_GET_STRING) + HTTP_HEADER_GET_STRING.length()));
      //Serial.println("url : " + url);
      String path = "";
      String query = "";
      if(url.indexOf("?") >= 0){
        path = url.substring(0, url.indexOf("\?"));
        query = url.substring(url.indexOf("\?")+1);
      }

      String inputString = query;

      int startIndex = 0;
      int endIndex = inputString.indexOf('&');

      // První tokenizace na základě '&'
      while (endIndex != -1) {
        String pair = inputString.substring(startIndex, endIndex);
        parseAndStore(pair);

        startIndex = endIndex + 1;
        endIndex = inputString.indexOf('&', startIndex);
      }
      // Poslední (nebo jediný) pár
      parseAndStore(inputString.substring(startIndex));

      if(path == "/save"){
        EEPROM.put(eeAddress, userSettings);
        EEPROM.commit();
      }
    }
  }
}


