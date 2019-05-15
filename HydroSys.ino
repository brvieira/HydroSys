#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <DHT.h>
#include <DallasTemperature.h>
#include <OneWire.h>

#define ONE_WIRE_BUS 4
#define BLYNK_PRINT Serial
#define DHTPIN 0
#define DHTTYPE DHT11

int sendSensorId, saveDataId;

//Token para comunicação com o Blynk
char auth[] = "";

//Credenciais de WIFI
char ssid[] = "";
char pass[] = "";

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature dsSensor(&oneWire);
DHT dht(DHTPIN, DHTTYPE);
BlynkTimer timer;

float tempSolucao;
float umidInterna;
float tempInterna;

//Função responsável por ler os dados dos sensores e enviá-los para o Blynk
void sendSensor() {
  //Leitura dos sensores
  dsSensor.requestTemperatures();
  tempSolucao = dsSensor.getTempCByIndex(0);
  umidInterna = dht.readHumidity();
  tempInterna = dht.readTemperature();
  
  Serial.print("Temperatura da Solução: ");
  Serial.print(tempSolucao);
  Serial.println(" °C");
  Serial.print("Temperatura Interna: ");
  Serial.print(tempInterna);
  Serial.println(" °C");
  Serial.print("Umidade Interna: ");
  Serial.print(umidInterna);
  Serial.println(" %");
  Serial.println("");

  //Envia cada dados para um dos pinos virtuais do Blynk
  if(!isnan(umidInterna)) {
    Blynk.virtualWrite(V5, umidInterna);
  }
  if(!isnan(tempInterna)) {
    Blynk.virtualWrite(V6, tempInterna);
  }
  if(!isnan(tempSolucao)) {
    Blynk.virtualWrite(V4, tempSolucao);
  }
  
}

//Função responsável por ler os dados dos sensores, criar um arquivo JSON e enviá-lo via POST para a API.
void saveData() {
  //Leitura dos sensores
  dsSensor.requestTemperatures();
  tempSolucao = dsSensor.getTempCByIndex(0);
  umidInterna = dht.readHumidity();
  tempInterna = dht.readTemperature();

  if(!isnan(tempSolucao) && !isnan(umidInterna) && !isnan(tempInterna)) {
    //Criação do objeto JSON
    StaticJsonBuffer<300> JSONbuffer;
  
    JsonObject& data = JSONbuffer.createObject();
  
    data["temperaturaInterna"] = String(tempInterna);
    data["umidadeInterna"] = String(umidInterna);
    data["temperaturaSolucao"] = String(tempSolucao);
    data["token"] = String(auth);
  
    //Converte o objeto JSON para String em formato JSON
    String dataStr;
    data.printTo(dataStr);
  
    //Envia os dados em formato JSON para a API via POST
    HTTPClient http;
  
    //URL da API
    http.begin("http://hydroapi.herokuapp.com/dados/");
    http.addHeader("Content-Type", "application/json");
  
    //Variável responsável por receber o código da resposta Http
    int httpCode = http.POST(dataStr);
  
    //Imprimi na porta Serial o código da resposta Http para debug
    Serial.println(httpCode);
  
    //Encerra a conexão com a API
    http.end();
  }
}

void setup()
{
  //Debug console
  Serial.begin(115200);

  dsSensor.begin();
  dht.begin();
  Blynk.begin(auth, ssid, pass);
  
  //Timer responsável por chamar a função sendSensor a cada 5 segundos
  sendSensorId = timer.setInterval(5000L, sendSensor);
  //Timer responsável por chamar a função saveData a cada 30 minutos
  saveDataId = timer.setInterval(1800000L, saveData);
}

void loop()
{
  Blynk.run();
  timer.run();
}
