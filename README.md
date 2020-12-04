# Sistema de Monitoreo IoT
- El query SQL para crear la base de datos se encuentra en el archivo **crearDB_estacion_9.txt**  
- El archivo **monitoreoGeneral.ino** es el código Arduino de la estación de monitoreo  
- El archivo **recibirDatos.ino** es el código Arduino de la estación receptora
- El archivo **insertaVal.php** es el código Php que inserta los datos en la base de datos. Se debe guardar en la ruta **C:\xampp\htdocs\Grabar\insertaVal.php**
- Se usan las librerías **DHT.h** **ESP8266WiFi.h**, **ESP8266HTTPClient.h**, **PubSubClient.h**, **Wire.h** **MAX30105.h** y **spo2_algorithm.h**
