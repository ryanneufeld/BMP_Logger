/***************************************************
  Adafruit MQTT Library Ethernet Example

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Alec Moore
  Derived from the code written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 ****************************************************/
#include <SPI.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

#include <Ethernet.h>
#include <EthernetClient.h>
#include <Dns.h>
#include <Dhcp.h>
#include <Adafruit_BMP085.h>
#include "DHT.h"

/******************* DHT CONFIG *********************/
#define DHTPIN 2     // what pin we're connected to
#define DHTTYPE DHT21   // DHT 21 (AM2301)

DHT dht(2, DHTTYPE);

/**
 * BMP setup
 */
Adafruit_BMP085 bmpsensor;

/************************* Ethernet Client Setup *****************************/
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};

//Uncomment the following, and set to a valid ip if you don't have dhcp available.
IPAddress iotIP (192, 168, 1, 20);
//Uncomment the following, and set to your preference if you don't have automatic dns.
IPAddress dnsIP (208, 67, 222, 220);
//If you uncommented either of the above lines, make sure to change "Ethernet.begin(mac)" to "Ethernet.begin(mac, iotIP)" or "Ethernet.begin(mac, iotIP, dnsIP)"


/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883
#define AIO_USERNAME    "ryan_neufeld"
#define AIO_KEY         "f7381e533d8446f791e7593eb3c78698"

/************ Global State (you don't need to change this!) ******************/

//Set up the ethernet client
EthernetClient client;

Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

// You don't need to change anything below this line!
#define halt(s) { Serial.println(F( s )); while(1);  }


/****************************** Feeds ***************************************/

// Setup a feed called 'photocell' for publishing.
// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
Adafruit_MQTT_Publish bmp = Adafruit_MQTT_Publish(&mqtt,  AIO_USERNAME "/feeds/barometer.kpa");
Adafruit_MQTT_Publish temp = Adafruit_MQTT_Publish(&mqtt,  AIO_USERNAME "/feeds/barometer.temp");
Adafruit_MQTT_Publish humidity = Adafruit_MQTT_Publish(&mqtt,  AIO_USERNAME "/feeds/barometer.humidity");

/*************************** Sketch Code ************************************/

void setup() {
  Serial.begin(115200);

  if (!bmpsensor.begin()) {
    Serial.println("Could not find a valid BMP085 sensor, check wiring!");
    while (1) {}
  }

  // Initialise the Client
  Serial.print(F("\nInit the Client..."));
  Ethernet.begin(mac, iotIP, dnsIP);
  delay(1000); //give the ethernet a second to initialize
  Serial.println(F("Ethernet should be up"));

  dht.begin();
}

void loop() {
  // Ensure the connection to the MQTT server is alive (this will make the first
  // connection and automatically reconnect when disconnected).  See the MQTT_connect
  // function definition further below.
   
  publishPressure();
  publishTemperature();
  publishHumidity();
  
  Serial.println(F("Sleeping 60 for seconds"));
  delay(60000);

}

void publishPressure() {
  MQTT_connect();
  float pressure = bmpsensor.readPressure() / 1000.0;
    
  // Now we can publish stuff!
  Serial.print(F("\nSending bmp pressure: "));
  Serial.print(pressure);
  Serial.print(F("..."));
  if (! bmp.publish(pressure)) {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("OK!"));
  }
  
  // ping the server to keep the mqtt connection alive
  if(! mqtt.ping()) {
    mqtt.disconnect();
  }
}

void publishTemperature() {
  float degreesc = bmpsensor.readTemperature();
  
  MQTT_connect();
  // Now we can publish stuff!
  Serial.print(F("\nSending temp in C: "));
  Serial.print(degreesc);
  Serial.print(F("..."));
  if (! temp.publish(degreesc)) {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("OK!"));
  }

  // ping the server to keep the mqtt connection alive
  if(! mqtt.ping()) {
    mqtt.disconnect();
  }
}

void publishHumidity() {
  float h = dht.readHumidity();
  MQTT_connect();
  // Now we can publish stuff!
  Serial.print(F("\nSending humidity: "));
  Serial.print(h);
  Serial.print(F("..."));
  if (! humidity.publish(h)) {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("OK!"));
  }

  // ping the server to keep the mqtt connection alive
  if(! mqtt.ping()) {
    mqtt.disconnect();
  }
  
}

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println(F("Retrying MQTT connection in 5 seconds..."));
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
  }
  Serial.println(F("MQTT Connected!"));
}
