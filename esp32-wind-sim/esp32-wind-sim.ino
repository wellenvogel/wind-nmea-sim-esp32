#include <WiFi.h>
#include <WiFiClient.h>
#include <ESPmDNS.h>

const char *testdata[]={
  "$WIMWV,62.00,R,15.55,N,A*23",
  "$WIVWR,62.00,R,15.55,N,8.00,M,28.80,K*6d",
  "$WIVPW,0.00,N,0.00,M*4c",
  "$PWINF,0,62.00,D,0.20,D,8.00,M,28.80,K,15.55,N,4,B,*6e",
  NULL
};
const char *testdata2[]={
  "$WIMWV,57.00,R,13.61,N,A*24",
  "$WIVWR,57.00,R,13.61,N,7.00,M,25.20,K*62",
  "$WIVPW,0.00,N,0.00,M*4c",
  "$PWINF,0,57.00,D,0.20,D,7.00,M,25.20,K,13.61,N,4,B,*61",
  NULL
};

const char **testsequence[]={
  testdata,
  testdata2,
  NULL
};
const char *ssid="esp32test";
const char *password="esp32abc"; //pw must be at least 8 characters
const int SERVER_PORT=10110;
const char * SERVICE_KIND="_nmea-0183";
//const char * SERVICE_KIND="_http";
const char * SERVICE_PROTO="_tcp";
WiFiServer server(SERVER_PORT);

const int MAXCLIENTS=5;
WiFiClient clients[MAXCLIENTS];
int numClients=0;
int sequenceCount=0;
unsigned long lastRun=0;

void setup() {
  Serial.begin(19200);
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);
 
  WiFi.disconnect();
  delay(100);
  Serial.println("Setup done");
  Serial.println(WiFi.softAPIP());
  server.begin();
  if (! MDNS.begin("WindSim")){
    Serial.println("unable to start MDNS");
  }
  MDNS.addService(SERVICE_KIND,SERVICE_PROTO,SERVER_PORT);
  lastRun=millis();
  
}
void loop() {
  bool hasChanged=false;
  for (int i=0;i<numClients;i++){
    if (! clients[i].connected()){
      hasChanged=true;
      Serial.print("Client disconnect: ");
      Serial.println(i);
      clients[i].stop();
      if (i < (numClients-1)){
        for (int j=i;j<(numClients-1);j++){
          clients[j]=clients[j+1];
        }
        i--;
      }
      numClients--;
    }
  }
  WiFiClient client=server.available();
  if (client){
    if (numClients >= MAXCLIENTS){
      Serial.println("too many clients");
      client.stop();
    }
    else{
      hasChanged=true;
      Serial.print("Client connected:");
      Serial.println(client.remoteIP());
      client.println("Hello\n");
      clients[numClients]=client;
      numClients++;
    }
  }
  unsigned long now=millis();
  if (now < lastRun){
    //overflow
    lastRun=now;
    return;
  }
  if ((lastRun + 1000) > now){
    delay(10);
    return;
  }
  Serial.print("loop, numclients=");
  Serial.println(numClients);
  lastRun=now;
  if (testsequence[sequenceCount] != NULL){
    sequenceCount++;
  }
  if (testsequence[sequenceCount] == NULL){
    sequenceCount=0;
  }
  const char **current=testsequence[sequenceCount];
  for (int i=0;i<numClients;i++){
    for (int j=0;current[j] != NULL;j++){
      clients[i].println(current[j]);
    }
  }
}
