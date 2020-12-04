// programa para nodemcu que envia datos de sensores a traves de mqtt
// los sensores se activan segun las suscripciones mqtt indicadas
// Por Ruben Sanchez Mayen y Lorena Palomino Castillo

/* --------------------------- librerias ------------------------------------ */

#include <DHT.h> // libreria sensor temperatura
#include <ESP8266WiFi.h> // libreria wifi nodemcu
#include <ESP8266HTTPClient.h> // libreria http nodemcu
#include <PubSubClient.h> // mqtt
#include <Wire.h>
#include "MAX30105.h"
#include "spo2_algorithm.h"

/* --------------------------- variables ------------------------------------ */

// sensores
DHT dht(D3, DHT11); // Objeto que representa el sensor de temperatura. Se conecta al pin D3
MAX30105 particleSensor; // sensor de pulso y oximetria
// datos wifi
const char *red = "**********"; // nombre de la red wifi
const char *password = "1234"; // contraseña de la red wifi

WiFiClient clienteWiFi; // cliente wifi

// indicadores para saber que sensores están activados
bool leerTemp = false;
bool leerPulso = false;
bool leerOxim = false;

// broker mqtt
const char* mqtt_server = "broker.hivemq.com";
// topicos mqtt
const char *topicSwitchTemp = "switchTemp";
const char *topicSwitchPulso = "switchPulso";
const char *topicSwitchOxim = "switchOxim";
const char *topicValorTemp = "valorTemp";
const char *topicValorPulso = "valorPulso";
const char *topicValorOxim = "valorSpo2";
const char *topicMensajesSistema = "sistema";
// cliente mqtt
PubSubClient client(clienteWiFi);

// Spo2
const int32_t bufferLength = 25; //data length
#define MAX_BRIGHTNESS 255
#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__)
//Arduino Uno doesn't have enough SRAM to store 100 samples of IR led data and red led data in 32-bit format
//To solve this problem, 16-bit MSB of the sampled data will be truncated. Samples become 16-bit data.
uint16_t irBuffer[bufferLength]; //infrared LED sensor data
uint16_t redBuffer[bufferLength];  //red LED sensor data
#else
uint32_t irBuffer[bufferLength]; //infrared LED sensor data
uint32_t redBuffer[bufferLength];  //red LED sensor data
#endif
int32_t spo2; //SPO2 value
// int32_t lastSpo2; // last spo2 value
int8_t validSPO2; //indicator to show if the SPO2 calculation is valid
int32_t heartRate; //heart rate value
// int32_t lastHearRate;  // last heart rate
int8_t validHeartRate; //indicator to show if the heart rate calculation is valid
byte pulseLED = 13; //Must be on PWM pin
byte readLED = 16; //Blinks with each data read
bool maxConectado = false;
/* --------------------------- funciones ------------------------------------ */

// mandar mensaje con led
void parpadearLento(){
  digitalWrite(LED_BUILTIN, LOW); // prender led
  delay(1000);
  digitalWrite(LED_BUILTIN, HIGH); // apagar led
  delay(1000);
}
// mandar mensaje con led
void parpadearRapido(){
  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_BUILTIN, LOW); // prender led
    delay(200);
    digitalWrite(LED_BUILTIN, HIGH); // apagar led
    delay(200);
  }
}

// Conectarse a la red, se indica con el led del nodemcu
void conectarARed() {
  Serial.println("Conectando a red");
  WiFi.begin(red, password);
  while (WiFi.status() != WL_CONNECTED) {
    parpadearLento();
    Serial.print('.');
  }
  Serial.println();
  parpadearRapido();
}

// calcular pulso y oximetria
void calibrarSensor(){
    //bufferLength = 100; //buffer length of 100 stores 4 seconds of samples running at 25sps

  //read the first 100 samples, and determine the signal range
  for (byte i = 0 ; i < bufferLength ; i++)
  {
    while (particleSensor.available() == false) //do we have new data?
      particleSensor.check(); //Check the sensor for new data

    redBuffer[i] = particleSensor.getRed();
    irBuffer[i] = particleSensor.getIR();
    particleSensor.nextSample(); //We're finished with this sample so move to next sample

//    Serial.print(F("red="));
//    Serial.print(redBuffer[i], DEC);
//    Serial.print(F(", ir="));
//    Serial.println(irBuffer[i], DEC);
  }

  //calculate heart rate and SpO2 after first 100 samples (first 4 seconds of samples)
  maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);

}

// setup del sensor MAX30105
void setupSpo2(){
  while (!particleSensor.begin(Wire, I2C_SPEED_FAST)) //Use default I2C port, 400kHz speed
  {
    client.publish(topicMensajesSistema,"Revisar sensor de pulso");
    Serial.println(F("El sensor MAX30105 no fue detecado. Checar cableado/alimentacion."));
    parpadearRapido();
    delay(3000);
  }
  byte ledBrightness = 60; //Options: 0=Off to 255=50mA
  byte sampleAverage = 4; //Options: 1, 2, 4, 8, 16, 32
  byte ledMode = 2; //Options: 1 = Red only, 2 = Red + IR, 3 = Red + IR + Green
  byte sampleRate = 100; //Options: 50, 100, 200, 400, 800, 1000, 1600, 3200
  int pulseWidth = 411; //Options: 69, 118, 215, 411
  int adcRange = 4096; //Options: 2048, 4096, 8192, 16384

  particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange); //Configure sensor with these settings
}

// callback mqtt, se activa cada que llega un mensaje
void callback(char* topic, byte* payload, unsigned int length)
{
  char *cstring = (char *) payload;
  cstring[length] = '\0';    // Adds a terminate terminate to end of string based on length of current payload
  if(topic[6] == 'T'){
    if (cstring[0] == '1') { // arreglar que no sirve
      Serial.println("TEMP ON");
      leerTemp = true;
    }else{
      Serial.println("TEMP OFF");
      leerTemp = false;
    }
  }else if(topic[6] == 'P'){
    if (cstring[0] == '1') { // arreglar que no sirve
      Serial.println("PULSO ON");
      leerPulso = true;
    }else{
      Serial.println("PULSO OFF");
      leerPulso = false;
    }
  }else if(topic[6] == 'O'){
    if (cstring[0] == '1') { // arreglar que no sirve
      Serial.println("SPO2 ON");
      leerOxim = true;
    }else{
      Serial.println("SPO2 OFF");
      leerOxim = false;
    }
  }
} //end callback

// conectarse a mqtt
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected())
  {
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    //if you MQTT broker has clientID,username and password
   //please change following line to    if (client.connect(clientId,userName,passWord))
    if (client.connect(clientId.c_str()))
    {
      parpadearRapido();
      client.subscribe(topicSwitchTemp);
      client.subscribe(topicSwitchPulso);
      client.subscribe(topicSwitchOxim);
      client.publish(topicMensajesSistema,"Conectado");
      Serial.println("Conectado a MQTT");
    } else {
      // esperar 6s
      client.publish(topicMensajesSistema,"Desconectado");
      parpadearLento();
      parpadearLento();
      parpadearLento();
      Serial.println("No se pudo conectar a MQTT");
    }
  }
} //end reconnect()

// leer temperatura y publicarla en mqtt
void lecturaTemp(){
  Serial.println("Leer temperatura");
  float t = dht.readTemperature();
    Serial.println(t);
    if(!isnan(t)){
      client.publish(topicValorTemp, String(t).c_str());
    }else{
      parpadearRapido();
      client.publish(topicMensajesSistema,"Error al medir la temperatura");
    }
}

// lectura sensor de pulso y oximetría. Si están activados por el usuario se mandan los datos filtrados a la interfaz
void lecturaMax(){
  calibrarSensor();

  if(leerPulso && leerOxim){
    Serial.println("Leer pulso y oximetría");
    if(validHeartRate == 1 && heartRate > 40 && heartRate < 190){
      Serial.print(String(heartRate).c_str());
      Serial.println(" BPM");
      client.publish(topicValorPulso,String(heartRate).c_str());
    }else{
      Serial.println("Calculando pulso");
    }
    if(validSPO2 == 1 && spo2 > 50 && spo2 < 99){
      Serial.print(String(spo2).c_str());
      Serial.println("%");
      client.publish(topicValorOxim,String(spo2).c_str());
    }else{
      Serial.println("Calculando Spo2");
    }
  }else if(leerPulso){
    Serial.println("Leer pulso");
    if(validHeartRate == 1 && heartRate > 40 && heartRate < 190){
      Serial.print(String(heartRate).c_str());
      Serial.println(" BPM");
      client.publish(topicValorPulso,String(heartRate).c_str());
    }else{
      Serial.println("Calculando pulso");
    }
  }else if(leerOxim){
    Serial.println("Oximetría");
    if(validSPO2 == 1 && spo2 > 50 && spo2 < 99){
      Serial.print(String(spo2).c_str());
      Serial.println("%");
      client.publish(topicValorOxim,String(spo2).c_str());
    }else{
      Serial.println("Calculando Spo2");
    }
  }
}

/* --------------------------- setup y loop ------------------------------------ */

// setup
void setup() {
  pinMode(LED_BUILTIN, OUTPUT); // led del nodemcu
  analogWrite(LED_BUILTIN, 1);
  Serial.begin(57600); // descomentar para activar serial
  conectarARed(); // conectarse a la red
  client.setServer(mqtt_server, 1883);//the default mqqt port is 1883
  client.setCallback(callback);
  // Inicializar el sensores
  dht.begin();
  setupSpo2();
}

// main
void loop() {
  // valores medidos
  float valorTemp;
  
  // Revisar conexion mqtt
  if (!client.connected()) {
    reconnect();
  }

  client.loop();

  if(leerTemp && (leerPulso || leerOxim)){
    client.publish(topicMensajesSistema,"Leyendo datos");
    lecturaTemp();
    lecturaMax();
    Serial.println("\n----------------------------------------------\n");
  }else if(leerPulso || leerOxim){
    client.publish(topicMensajesSistema,"Leyendo datos");
    lecturaMax();
    Serial.println("\n----------------------------------------------\n");
  }else if(leerTemp){
    client.publish(topicMensajesSistema,"Leyendo datos");
    lecturaTemp();
    delay(3000);
    Serial.println("\n----------------------------------------------\n");
  }else{
    client.publish(topicMensajesSistema,"conectado");
    Serial.println("No leer nada");
    delay(1000);
  }
 
  
}
