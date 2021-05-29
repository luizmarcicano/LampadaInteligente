#include <SPI.h>
#include <Ethernet.h>

#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>

#include <PubSubClient.h>

#define pinLampada 2  //o número é o pin q a lâmpada vai ser ligada/desligada (não é o de 5V)
#define pinLigaSensorL 6  //o número é o pin q o sensor de Luminosidade vai ser ligado
#define pinLigaSensorS 7  //o número é o pin q o sensor de Som vai ser ligado/desligado

#define pinSensorL 12  //o número é o pin q o sensor de Luminosidade vai enviar a leitura
#define pinSensorS 13  //o número é o pin q o sensor de Som vai enviar a leitura

//Ethernet
const char* SSID = "REDE_CASA";                // SSID / nome da rede que deseja se conectar
const char* PASSWORD = "SENHA_REDE_CASA";      // Senha da rede que deseja se conectar
IPAddress ip(//ip do seu roteador)
EthernetServer server(80); 
EthernetClient client;                     
byte mac[6] ={0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};

//MQTT Server - Opção Eclipse 
const char* BROKER_MQTT = "iot.eclipse.org"; //URL do broker MQTT que se deseja utilizar
int BROKER_PORT = 1883;                      // Porta do Broker MQTT

/// OPÇÃO ADAFRUIT 
#define AIO SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883
#define IO_USERNAME     "lmarcicano"
#define IO_KEY          "aio_ttGy7370qcoe3y25G8HOhrjxEf2x"

Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, IO_USERNAME, IO_KEY);
Adafruit_MQTT_Publish lumin = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/lumin")
Adafruit_MQTT_Publish som = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/som")
Adafruit_MQTT_Publish statusSom = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/statussom")
Adafruit_MQTT_Subscribe timer = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/timer")



#define ID_MQTT  "ARDUINO"                    //Informe um ID unico e seu. Caso sejam usados IDs repetidos a ultima conexão irá sobrepor a anterior. 
#define TOPIC_SUBSCRIBE "dealer/timer"       //Informe um Tópico único. Caso sejam usados tópicos em duplicidade, o último irá eliminar o anterior.
#define TOPIC_PUBLISH "dealer/luminosidade"
#define TOPIC_PUBLISH1 "dealer/som"
#define TOPIC_PUBLISH2 "dealer/status"
PubSubClient MQTT(client);                  // Instancia o Cliente MQTT passando o objeto EthernetClient

//Declaração das Funções
void mantemConexoes();  //Garante que as conexoes com WiFi e MQTT Broker se mantenham ativas
void conectaMQTT();     //Faz conexão com Broker MQTT
void recebePacote(char* topic, byte* payload, unsigned int length);

void setup() {
  pinMode(pinLampada, OUTPUT);   
  pinMode(pinLigaSensorL, OUTPUT);   
  pinMode(pinLigaSensorS, OUTPUT);  
   
  pinMode(pinSensorL, INPUT); 
  pinMode(pinSensorS, INPUT);         

  Serial.begin(115200);
  Ethernet.begin(mac,ip);

  if(Ethernet.hardwareStatus() == EthernetNoHardware){
    Serial.println("Ethernet shield não encontrada. verifique o hardware");
    while(true){
      delay(1);
    }
  }

  if(Ethernet.linkStatus() == LinkOFF){
    Serial.println("Cabo desconectado.");
  }
  server.begin();
  Serial.print("IP do servidor: ");
  Serial.println(Ethernet.localIP();


  MQTT.setServer(BROKER_MQTT, BROKER_PORT);  
  MQTT.setCallback(recebePacote); 

  
}

void loop() {
  mantemConexoes();
  enviaValores();
  MQTT.loop();
}

void mantemConexoes() {
    if (!MQTT.connected()) {
       conectaMQTT(); 
    }
}


void conectaMQTT() { 
    while (!MQTT.connected()) {
        Serial.print("Conectando ao Broker MQTT: ");
        Serial.println(BROKER_MQTT);
        if (MQTT.connect(ID_MQTT)) {
            Serial.println("Conectado ao Broker com sucesso!");
            MQTT.subscribe(TOPIC_SUBSCRIBE);
        } 
        else {
            Serial.println("Noo foi possivel se conectar ao broker.");
            Serial.println("Nova tentatica de conexao em 10s");
            delay(10000);
        }
    }
}

void recebePacote(char* topic, byte* payload, unsigned int length) 
{
    String msg;

    //obtem a string do payload recebido
    for(int i = 0; i < length; i++) 
    {
       char c = (char)payload[i];
       msg += c;
    }

    if (msg == "0") {
       digitalWrite(pinLigaSensorS, LOW);
    }

    if (msg == "1") {
       digitalWrite(pinLigaSensorS, HIGH);
    }
}

void enviaValores() {
static bool pinLampada = HIGH;
static int pinSensorL;
static int pinSensorS;
  leituraSensorLIni = analogRead(pinSensorL);
  delay(5000);
  leituraSensorL = analogRead(pinSensorL);
  leituraSensorS = analogRead(pinSensorS);
  leituraSensorLIni = leituraSensorLIni - 40;
     if (leituraSensorL <  leituraSensorLIni) {
        //Diminuiu consideravelmente a Luz     
        MQTT.publish(TOPIC_PUBLISH, leituraSensorL);
        MQTT.publish(TOPIC_PUBLISH2, "1");
        MQTT.publish(TOPIC_PUBLISH1, leituraSensorS);
        Serial.println("Luminosidade Baixa. Payload enviado.");    
     }
}

// Por: Luiz Guilherme Lima Marcicano e Heloisa Prado Cavichiolli
