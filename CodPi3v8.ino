#include <NTPClient.h>

#include <DHT11.h>

#include <IOXhop_FirebaseESP32.h>
#include <IOXhop_FirebaseStream.h>

#include <ArduinoJson.h>
#include <ArduinoJson.hpp>

#include <WiFi.h>

#define WIFI_SSID "ANA JULIA INTERPLUS_2G" // your wifi SSID; GalaxyA30sB199 ; ANA JULIA INTERPLUS_2G; Galaxy A03ea78
#define WIFI_PASSWORD "18291825" //your wifi PASSWORD; mfsk4621 ; 18291825; vinitor3

#define LED_R 21
#define LED_G 22
#define POTAGUA 32
//#define PotN 32
#define PotP 35
#define PotCorrecao 34

#define FIREBASE_HOST "https://arduino-web-73b1a-default-rtdb.firebaseio.com/" // change here; toggle-button-3d8-rtdb.firebaseio.com
#define FIREBASE_AUTH "pUq6TKyxxA1OHTdU6vdL1IHrtbNFz9R1ePgeULxK"  // your private key; icOuLI7Mpe04Nv1ABHfAqyvlQn7HI0aYBwlp
FirebaseESP32 firebaseData;

float tempdht, umiddht;
DHT11 dht11(33);

int mudanca;
int horaatual;
unsigned long int tempoanterior = 0;
unsigned long int intervaloLeitura = 1800000;

const char* servidorNTP = "a.st1.ntp.br";
const int fusoHorario = -10800;
const int taxaDeAtualizacao = 1800000;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, servidorNTP, fusoHorario, 60000);

void setup ()
{
  pinMode(POTAGUA, INPUT);
  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  //pinMode(PotN, INPUT);
  pinMode(PotP, INPUT);
  pinMode(PotCorrecao, INPUT);

  Serial.begin(115200);

  // connect to wifi.
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("connecting");

  while (WiFi.status() != WL_CONNECTED) {
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("connected: ") ;
  Serial.println(WiFi.localIP());
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  //Firebase.reconnectWiFi(true);     

  timeClient.begin();

  //digitalWrite(LED_R,Firebase.getString("/Led1Status").toInt());
  //digitalWrite(LED_G,Firebase.getString("/Led2Status").toInt());

  horaatual = timeClient.getHours();
  mudanca = Firebase.getInt("/Mudanca");

  if(mudanca != horaatual){
    mudanca = horaatual;
    FuncaoProjeto();
    if(mudanca==23){
      LeituraNPK();
    }
  }

}
void loop (){

  horaatual = timeClient.getHours();
  
  if(millis() - tempoanterior >= intervaloLeitura){
    if(mudanca != horaatual){
      mudanca = horaatual;
      FuncaoProjeto();
      if(mudanca==23){
        LeituraNPK();
      }
    }
    tempoanterior = millis();
  }

}

void LeituraNPK(){

  float potcorrecao = map(analogRead(PotCorrecao),0,4095,0,100);
  float potassioread = map(analogRead(PotP),0,4095,0,200);
  float nitrogenioread = (potassioread/3)+(potcorrecao);
  float fosfororead = ((potassioread+10)/2)+(potcorrecao/2);

  Firebase.setFloat("/NPK/Nitrogenio",nitrogenioread);
  Firebase.setFloat("/NPK/Potassio",potassioread);
  Firebase.setFloat("/NPK/Fosforo",fosfororead);
}

void FuncaoProjeto(){
  //float potcorrecao = map(analogRead(PotCorrecao),0,4095,0,100);
  float potaguaread = map(analogRead(POTAGUA),0,4095,0,100);
  float umidIdeal = 70;
  tempdht = dht11.readTemperature();
  umiddht = dht11.readHumidity();
  //float potassioread = map(analogRead(PotP),0,4095,0,200);
  //float nitrogenioread = (potassioread/3)+(potcorrecao);
  //float fosfororead = ((potassioread+10)/2)+(potcorrecao/2);

  
  Firebase.setFloat("/Temperatura",tempdht);
  Firebase.setFloat("/UmidadeRelativa",umiddht);
  Firebase.setFloat("/TemperaturaSolo",tempdht+3);
  Firebase.setFloat("/UmidadeSolo",potaguaread);
  //Firebase.setFloat("/NPK/Nitrogenio",nitrogenioread);
  //Firebase.setFloat("/NPK/Potassio",potassioread);
  //Firebase.setFloat("/NPK/Fosforo",fosfororead);
  Firebase.setInt("/Mudanca",mudanca);//add logica para enviar leitura do solo as 23h da noite, discutir com grupo

  /*//Serial.println();
  //Serial.print("Led ON/OFF: ");*/
  if(Firebase.getString("/Led1Status")=="1"){
    digitalWrite(LED_R,HIGH);
    //Serial.println(Firebase.getString("/Led1Status"));
  }
  else{
    digitalWrite(LED_R,LOW);
    //Serial.println(Firebase.getString("/Led1Status"));
  }

  if(Firebase.getString("/Led2Status")=="1"){
    digitalWrite(LED_G,HIGH);
    //Serial.println(Firebase.getString("/Led2Status"));
  }
  else{
    digitalWrite(LED_G,LOW);
    //Serial.println(Firebase.getString("/Led2Status"));
  }

  if(potaguaread < umidIdeal){
    if(Firebase.getString("/Led1Status")=="0"){
      //bomba ja ligada
    }
    else{
      Firebase.setString("/Led1Status","0");
      digitalWrite(LED_R,LOW);
      Firebase.setString("/Led2Status","1");
      digitalWrite(LED_G,HIGH);
    }
  }
  else{
    if(Firebase.getString("/Led1Status")=="0"){
      Firebase.setString("/Led1Status","1");
      digitalWrite(LED_R,HIGH);
    }
    if(Firebase.getString("/Led2Status")=="1"){
      Firebase.setString("/Led2Status","0");
      digitalWrite(LED_G,LOW);
    }
  }
}



