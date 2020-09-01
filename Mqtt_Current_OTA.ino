 /*
 ____                          ____                            
|  _ \ _   _ _ __ ___  _ __   / ___|  ___ _ __  ___  ___  _ __      Daniele Facco
| |_) | | | | '_ ` _ \| '_ \  \___ \ / _ \ '_ \/ __|/ _ \| '__| 
|  __/| |_| | | | | | | |_) |  ___) |  __/ | | \__ \ (_) | |        farsidevirtual@airmail.cc
|_|    \__,_|_| |_| |_| .__/  |____/ \___|_| |_|___/\___/|_|   
                      |_|                                      
*/
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "ACS712.h"
#include <ArduinoOTA.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>

ACS712  ACS(A0, 5.0, 4095, 255);

const char* ssid = "xxxxxx";
const char* password = "xxxxxx";
const char* mqtt_server = "xxxxxx";

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE  (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;
int mA = 0;
uint32_t Intervall_time=0;
char message_buff[1];
char message[4];

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void setup_ota(){
  // Port defaults to 3232
   ArduinoOTA.setPort(3232);

  // Hostname defaults to esp3232-[MAC]
   ArduinoOTA.setHostname("NodeMCU");

  // No authentication by default
  // ArduinoOTA.setPassword("admin");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

    ArduinoOTA.onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type);
    });
    ArduinoOTA.onEnd([]() {
      Serial.println("\nEnd");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });

  ArduinoOTA.begin();

  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  Intervall_time=millis();
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("hometest/test321/test", "connected");
      // ... and resubscribe
      client.subscribe("hometest/test321/test");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  setup_ota();
  client.setServer(mqtt_server, 1883);
}

void loop() {
  //OTA
  ArduinoOTA.handle();
  
  //CURRENT
  mA = ACS.mA_AC();
  Serial.print("mA: ");
  Serial.print(mA);
  dtostrf( mA, 5, 0, message);

  //MQTT
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  if(mA > 500){
    client.publish("hometest/test321/test", message);
  }
  else{
    client.publish("hometest/test321/test", message); 
  }

  delay(5000);
}
//EOF
