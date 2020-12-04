// Este código funciona con NodeMCU
// Actualiza los valores con el identificador de tu red

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ESP8266HTTPClient.h>

const char* ssid = "NETGEAR67"; //Modifica con el SSID de tu red
const char* password = "magicaltrumpet853"; //Password WEP/WPA
// Direccion http
String urlBase = "http://10.0.0.9/Grabar/insertaVal.php?idPersona=";
String url ="";
String usuario = "-1"; 
String temp = "0";
String pulso = "0";
String spo2 = "0";

const char* mqtt_server = "broker.hivemq.com";
const char *topicValorTemp = "valorTemp";
const char *topicValorPulso = "valorPulso";
const char *topicValorOxim = "valorSpo2";
const char *topicNumUsuario = "numUsuarioDB";
const char *mensajeSistema = "sistema";
 
WiFiClient espClient;
WiFiClient clienteWiFi;
PubSubClient client(espClient);
HTTPClient http;

// En este espacio debes inicializar los pines y declarar las variables que emplearás
int value = 0;
int threshold=50; // you might need to adjust this value to define light on/off status

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

void setup_wifi() {
   delay(100);
   // Nos conectamos a la red
   Serial.print("Conectando a ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED){
      Serial.print(".");
      parpadearLento();
    }
    Serial.println("");
    parpadearRapido();
} // Termina setup_wifi


void enviarDatos(){
  if(usuario[0] != '-'){
  url = ( urlBase + usuario + "&valorTemp=" + temp + "&valorPulso=" + pulso + "&valorSpo2=" + spo2);
  Serial.println(url);
  // Enviarlos por la red al servicio
  // Solicitar la conexión al servicio
  if ( http.begin(clienteWiFi, url) ) { 
    int codigo = http.GET();  // Realizar petición
    Serial.printf("Código: %d\n", codigo);
   // if ( codigo > 0) {
      if (codigo == HTTP_CODE_OK || codigo == HTTP_CODE_MOVED_PERMANENTLY) {
        String respuesta = http.getString();  // Respuesta
        Serial.println(respuesta);
      } else {
      Serial.printf("GET falló, error: %s\n", http.errorToString(codigo).c_str());
    }
  } else {
    Serial.println("No es posible hacer la conexión");
  }
  http.end();
  parpadearRapido();
  }else{
    Serial.println("No se ha especificado un usuario");
    client.publish(mensajeSistema, "Seleccionar usuario");
  }
}
 
void callback(char* topic, byte* payload, unsigned int length){
  char *cstring = (char *) payload;
  cstring[length] = '\0';    // Agrega un caracter de terminación
  if(topic[5] == 'u'){
    Serial.println("Usuario recibido");
    usuario = cstring;
    client.publish(mensajeSistema,"conectado");
  }else if(topic[5] == 'T'){
      temp = cstring;
    }else if(topic[5] == 'P'){
      pulso = cstring;
    }else if(topic[5] == 'S'){
      spo2 = cstring;
    }
  
} //termina callback 
 
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()){
    Serial.print("Attempting MQTT connection...");
    Serial.println("");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    //if you MQTT broker has clientID,username and password
   //please change following line to    if (client.connect(clientId,userName,passWord))
    if (client.connect(clientId.c_str())){
      Serial.println("connected");
     //once connected to MQTT broker, subscribe command if any
      client.subscribe(topicValorTemp);
      client.subscribe(topicValorPulso);
      client.subscribe(topicValorOxim);
      client.subscribe(topicNumUsuario);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 6 seconds before retrying
      delay(6000);
    }
  }
} //termina reconnect()
 
void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(57600);
  setup_wifi();
  client.setServer(mqtt_server, 1883);//the default mqqt port is 1883.
  client.setCallback(callback);
} //termina setup
 
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  // checar mqtt por 10s
  for( uint32_t tStart = millis();  (millis()-tStart) < 10000;  ){
   client.loop();
  }
  enviarDatos();
  
  
} // termina loop
