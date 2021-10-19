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


String GAS_ID = "AKfycbyiq42vbOPFBfbQOaE3gwk3hEL92xLTt2MPgCbE2UoHYwpNvnXyyr12GJJDpOdvMh5d";  // Replace by your GAS service id

long lastTime = 0;
long minutes = 0;
long hours = 0;
int i=0;

int Pioggiapin = A0;
int PioggiaValue = 0;
int pioggia;
int count = 0;


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
              pinMode(Pioggiapin, INPUT);
              while (!Serial) continue;
}
float humi;
float temp;
float wind;
float mediatemp = 0;
float mediahumi = 0;
float mediawind = 0;

void loop() {
  if(millis()-lastTime > 60000){
    minutes++;
    lastTime = millis();
    int chk = DHT11.read(DHT11PIN);

    PioggiaValue = analogRead(A0);
    if(PioggiaValue > 500){
          pioggia = 0;
    }
    else if(PioggiaValue < 500){
          pioggia = 1;
    }

    humi = ((float)DHT11.humidity);
    temp = ((float)DHT11.temperature);

    
    StaticJsonBuffer<1000> jsonBuffer;
    JsonObject& data = jsonBuffer.parseObject(nodemcu);

    if (data == JsonObject::invalid()) {
    //Serial.println("Invalid Json Object");
      return;
    }

    Serial.println("JSON Object Recieved");
    
    float wind = data["wind"];
    
    mediatemp = mediatemp + temp;
    mediahumi = mediahumi + humi;
    mediawind = mediawind + wind;
  }
  if(minutes > 10){
    mediatemp = mediatemp/10;
    mediahumi = mediahumi/10;
    mediawind = mediawind/10;
    sendData(mediahumi,mediatemp,mediawind,pioggia);
    minutes = 0;
    mediatemp = 0;
    mediahumi = 0;
    mediawind = 0;
  }
}

void sendData(float x, float y, float z, int w)
{
  client.setInsecure();
  //Serial.print("connecting to ");
  //Serial.println(host);
  if (!client.connect(host, httpsPort)) {
    //Serial.println("connection failed");
    return;
  }

  float string_x     =  float(x);
  float string_y     =  float(y);
  float string_z     =  float(z);
  int string_w     =  int(w);
  String url = "/macros/s/" + GAS_ID + "/exec?value1=" + string_x + "&value2=" + string_y + "&value3=" + string_z + "&value8=" + string_w;
  //Serial.print("requesting URL: ");
  Serial.println(url);

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
}
