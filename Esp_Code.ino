#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <SoftwareSerial.h>
#include <WiFiManager.h>
#include <ArduinoJson.h>

#include <dht11.h>
#define DHT11PIN 2

dht11 DHT11;


//D6 = Rx & D5 = Tx
SoftwareSerial nodemcu(D6, D5);

String ssid;
String password;
 

#define ON_Board_LED 2  

const char* host = "script.google.com";
const int httpsPort = 443;


WiFiClientSecure client;


String GAS_ID = "AKfycbyz3Brk2iZ07Uy9jtV7Ry_B1W_yPaO3mJH9n8TXgttFnMvMJKqmkxxaEz4Iqknqod0";  // Replace by your GAS service id

long lastTime = 0;
long minutes = 0;
long hours = 0;
int i=0;

void wificonnect(){
  ssid = WiFi.SSID();
  password = WiFi.psk();
  WiFi.begin(ssid, password);
    
  pinMode(ON_Board_LED,OUTPUT); 
  digitalWrite(ON_Board_LED, HIGH); 

  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(ON_Board_LED, LOW);
    delay(250);
    digitalWrite(ON_Board_LED, HIGH);
    delay(250);
    
  }

  client.setInsecure();
}

void setup()
{       
              WiFiManager wifiManager;
              wifiManager.autoConnect("AutoConnectAP");
              delay(500);
              wificonnect();
              Serial.begin(9600);
              nodemcu.begin(9600);
              while (!Serial) continue;
}

void loop() {
  
  if(millis()-lastTime > 60000){
    minutes++;
    lastTime = millis();
  }
  if(minutes > 60){
    hours++;
    int chk = DHT11.read(DHT11PIN);

    
    float humi = ((float)DHT11.humidity);
    float temp = ((float)DHT11.temperature);

    
    StaticJsonBuffer<1000> jsonBuffer;
    JsonObject& data = jsonBuffer.parseObject(nodemcu);

    if (data == JsonObject::invalid()) {
    //Serial.println("Invalid Json Object");
      return;
    }

    Serial.println("JSON Object Recieved");
    
    float wind = data["wind"];
    sendData(humi,temp,wind);
    minutes = 0;
  }
}

void sendData(float x, float y, float z)
{
  client.setInsecure();
  //Serial.print("connecting to ");
  //Serial.println(host);
  if (!client.connect(host, httpsPort)) {
    //Serial.println("connection failed");
    return;
  }

  int string_x     =  float(x);
  int string_y     =  float(y);
  float string_z     =  float(z);
  String url = "/macros/s/" + GAS_ID + "/exec?value1=" + string_x + "&value2=" + string_y + "&value3=" + string_z;
  //Serial.print("requesting URL: ");
  //Serial.println(url);

  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
         "Host: " + host + "\r\n" +
         "User-Agent: BuildFailureDetectorESP8266\r\n" +
         "Connection: close\r\n\r\n");

  //Serial.println("request sent");
  while (client.connected()) {
  String line = client.readStringUntil('\n');
  if (line == "\r") {
    //Serial.println("headers received");
    break;
  }
  }
  String line = client.readStringUntil('\n');
  if (line.startsWith("{\"state\":\"success\"")) {
  //Serial.println("esp8266/Arduino CI successfull!");
  } else {
  //Serial.println("esp8266/Arduino CI has failed");
  }
  
}
