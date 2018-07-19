#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <DHT.h>

#define BLYNK_PRINT Serial
#define DHTPIN 2
#define DHTTYPE DHT11

//Token para comunicação com o Blynk
char auth[] = "8d2b97a1033749029f243fecc9f39a5a";

//Credenciais de WIFI
char ssid[] = "Desktop_F3212335";
char pass[] = "bru268no";

DHT dht(DHTPIN, DHTTYPE);
BlynkTimer timer;


//Função responsável por ler os dados dos sensores e enviá-los para o Blynk
void sendSensor() {
  //Leitura dos sensores
  float umidInterna = dht.readHumidity();
  float tempInterna = dht.readTemperature();

  //"If" para verificar se a leitura dos dados dos sensores foi feita com sucesso
  if (isnan(umidInterna) || isnan(tempInterna)) {
    Serial.println("Falha ao ler sensor DHT11!");
    return;
  }

  //Envia cada dados para um dos pinos virtuais do Blynk
  Blynk.virtualWrite(V5, umidInterna);
  Blynk.virtualWrite(V6, tempInterna);

}

//Função responsável por ler os dados dos sensores, criar um arquivo JSON e enviá-lo via POST para a API.
void saveData() {
  //Leitura dos sensores
  float umidInterna = dht.readHumidity();
  float tempInterna = dht.readTemperature();

  //Laço para garantir que a leitura dos dados dos sensores seja feita com sucesso
  while (isnan(umidInterna) || isnan(tempInterna)) {
    umidInterna = dht.readHumidity();
    tempInterna = dht.readTemperature();
  }
  
  //Criação do objeto JSON
  StaticJsonBuffer<300> JSONbuffer;

  JsonObject& data = JSONbuffer.createObject();

  data["umidadeInterna"] = String(umidInterna);
  data["temperaturaInterna"] = String(tempInterna);

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

void setup()
{
  //Debug console
  Serial.begin(115200);
  
  dht.begin();
  Blynk.begin(auth, ssid, pass);

  //Timer responsável por chamar a função sendSensor a cada 1,5 segundos
  timer.setInterval(1500L, sendSensor);
  //Timer responsável por chamar a função saveData a cada 10 minutos
  timer.setInterval(600000L, saveData);
}

void loop()
{
  Blynk.run();
  timer.run();
}
