#include "LoRaWan_APP.h"
#include "Arduino.h"

#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>

#include <Wire.h>
#include <AsyncDelay.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>


//Lora var
#define RF_FREQUENCY                                915000000
#define TX_OUTPUT_POWER                             5
#define LORA_BANDWIDTH                              0
#define LORA_SPREADING_FACTOR                       7
#define LORA_CODINGRATE                             1
#define LORA_PREAMBLE_LENGTH                        8
#define LORA_SYMBOL_TIMEOUT                         0
#define LORA_FIX_LENGTH_PAYLOAD_ON                  false
#define LORA_IQ_INVERSION_ON                        false
#define RX_TIMEOUT_VALUE                            1000
#define BUFFER_SIZE                                 30
char txpacket[BUFFER_SIZE];
char rxpacket[BUFFER_SIZE];
double txNumber;
bool lora_idle=true;
static RadioEvents_t RadioEvents;
void OnTxDone( void );
void OnTxTimeout( void );


// wifi var
const char *ssid = "Kataiga";
const char *password = "Rikka123";
WiFiMulti wifiMulti;

// BME280
#define SEALEVELPRESSURE_HPA
#define TEMP_CORR (-2)
#define ELEVATION (505)
Adafruit_BMP280 bmp(&Wire);
TwoWire Wire2(1);
float SLpressure_hPa;

#define SDA 41
#define SCL 42

// Main function

void setup() {
  Serial.begin(115200);
  initLora();
  initWifi();

  //Wire2.begin(41,42); /* (GPIO SDA and SCL) */
  Wire.begin(41, 42);
  bool bme_status;

  // bme_status = bmp.begin(0x76);  //address either 0x76 or 0x77
  // if (!bme_status) {
  //     Serial.println("Could not find a valid BME280 sensor, check wiring!");
  // }

}

void loop() {
  sendDataLora();
  clientCall();
  delay(5000);
}

// WIFI/CLIENT FUNCTION

void initWifi() {
  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }
  wifiMulti.addAP(ssid, password);
}

void clientCall() {
  if ((wifiMulti.run() == WL_CONNECTED)) {
    HTTPClient http;
    http.begin("http://example.com/index.html");  //HTTP
    int httpCode = http.GET();
    if (httpCode > 0) {
      Serial.printf("[HTTP] GET... code: %d\n", httpCode);
      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        Serial.println(payload);
      }
    } else {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
  }
}

// // LORA FUNCTION

void initLora() {
  Mcu.begin(HELTEC_BOARD,SLOW_CLK_TPYE);
  txNumber=0;
  RadioEvents.TxDone = OnTxDone;
  RadioEvents.TxTimeout = OnTxTimeout;
  Radio.Init( &RadioEvents );
  Radio.SetChannel( RF_FREQUENCY );
  Radio.SetTxConfig( 
    MODEM_LORA, 
    TX_OUTPUT_POWER, 
    0, 
    LORA_BANDWIDTH,
    LORA_SPREADING_FACTOR, 
    LORA_CODINGRATE,
    LORA_PREAMBLE_LENGTH, 
    LORA_FIX_LENGTH_PAYLOAD_ON,
    true, 
    0, 
    0, 
    LORA_IQ_INVERSION_ON, 
    3000 
  ); 
}

void sendDataLora(){
  if(lora_idle == true) {
    delay(10000);
		txNumber += 0.01;
		sprintf(txpacket,"Hello world number %0.2f",txNumber);  //start a package
   
		Serial.printf("\r\nsending packet \"%s\" , length %d\r\n",txpacket, strlen(txpacket));

		Radio.Send( (uint8_t *)txpacket, strlen(txpacket) ); //send the package out	
    lora_idle = false;
	}
  Radio.IrqProcess( );
}

void OnTxDone( void ) {
	Serial.println("TX done......");
	lora_idle = true;
}

void OnTxTimeout( void ) {
    Radio.Sleep( );
    Serial.println("TX Timeout......");
    lora_idle = true;
}
