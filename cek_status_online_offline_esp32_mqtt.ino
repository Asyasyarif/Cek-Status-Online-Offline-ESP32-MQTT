#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

const char* ssid = "";
const char* password = "";
const char* mqtt_server = "";
int mqtt_port = 1883;

String macAddressString;
// Set MQTT topics using the MAC address as a string
char lwt_topic[30];
char macAddress[18];
uint8_t mac[6];
bool cn_check = false;


WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;


void setup_wifi() {
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

   while (WiFi.status() != WL_CONNECTED) {
      Serial.print(".");
    }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  // Obtain the MAC address as a string
  WiFi.macAddress(mac);
  sprintf(macAddress, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

  // Convert macAddress to a string
  for (int i = 0; i < 6; i++) {
  char hexChar[8];
  sprintf(hexChar, "%02X", mac[i]);
  macAddressString += hexChar;
  }

  Serial.print("MAC Address: ");
  Serial.println(macAddress);

  //Serial.println(lwt_topic);
  sprintf(lwt_topic, "lwt_topic_%s", macAddress); // Generate LWT topic based on Mac address
  Serial.println(lwt_topic);
}


void connectBroker() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
     while (WiFi.status() != WL_CONNECTED) {     
      Serial.print("Wifi re-connect successfully");
    }
    // Create a random client ID
    String clientId = "ESP32Client-"+macAddressString;
    Serial.println(clientId);

  StaticJsonDocument<50> lwtJson;
  lwtJson["stt"] = false;
  char lwtMessage[200];
  serializeJson(lwtJson, lwtMessage);
    if (client.connect(clientId.c_str(),lwt_topic, 2, true, lwtMessage)) {
      Serial.println("Connected to Broker and setup state message");
      cn_check =true; 
        
      DynamicJsonDocument RtJson(50);
      RtJson["stt"] = true;
      String RtMessage;
      serializeJson(RtJson, RtMessage);
      // Once connected, publish an announcement...
      const char* pl_RtMessage = RtMessage.c_str();
      client.publish(lwt_topic, pl_RtMessage, true);
    } else {
      cn_check =false;
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(2000);
    }
  }
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
}

void loop() {
   if (!client.connected()) {
    cn_check =false;
    connectBroker();
  }
  client.loop();
  delay(200);
  
}