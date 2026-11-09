#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ezTime.h>
#include <ArduinoJson.h>
#include <LiquidCrystal_I2C.h>

void conexaoWifi();
void reConexaoWifi();

const char *SSID = "SENAI EDUCACIONAL";
const char *SENHA = "info@134";

const unsigned long tempoConexao = 10000;
const unsigned long tempoReConexao = 5000;

// Variaveis para substituir o delay do loop
unsigned long tempoLeitura = 0;
const unsigned long tempoEspera = 10000;

const String enderecoAPI = "https://wttr.in/Sao+Caetano+do+Sul?lang=pt&format=";
float temperatura, sensacaoTermica;
int umidade, pressao;
String condicao, nascerdoSol;

// Instancia do LCD I2C (Endereco 0x27, 20 colunas, 4 linhas)
LiquidCrystal_I2C lcd(0x27, 20, 4);

// Variaveis para Data e Hora com ezTime
Timezone sp;
String dataAtual;
String horaAtual;

JsonDocument pack;
String msgJson;

String consultaAPI(String codigo); 

void setup() {
  Serial.begin(115200);

  // Inicializacao do Display LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Conectando WiFi...");

  conexaoWifi();

  // Configuração do ezTime para o fuso horario do Brasil
  lcd.setCursor(0, 1);
  lcd.print("Sincronizando hora..");
  waitForSync();
  sp.setLocation("America/Sao_Paulo");

  lcd.clear();
}

void loop() {
  reConexaoWifi();
  events(); // Mantem a sincronizacao do ezTime atualizada
  
  // Substituicao do delay(10000) por millis
  if (millis() - tempoLeitura >= tempoEspera) {
    tempoLeitura = millis();

    // Obtenção da Data (xx/xx/xxxx) e Horario completo (HH:MM:SS)
    dataAtual = sp.dateTime("d/m/Y");
    horaAtual = sp.dateTime("H:i:s");

    temperatura = consultaAPI("%t").toFloat();
    sensacaoTermica = consultaAPI("%f").toFloat();
    umidade = consultaAPI("%h").toInt();
    pressao = consultaAPI("%P").toInt();
    condicao = consultaAPI("%C");
    nascerdoSol = consultaAPI("%S");

    
    pack["data"] = dataAtual;
    pack["hora"] = horaAtual;
    pack["temp"] = temperatura;
    pack["sTerm"] = sensacaoTermica;
    pack["umi"] = umidade;
    pack["press"] = pressao;
    pack["cond"] = condicao;
    pack["NSol"] = nascerdoSol;

    serializeJson(pack, msgJson);

    lcd.setCursor(0, 0);
    lcd.print(dataAtual + " " + horaAtual);
    lcd.setCursor(0, 1);
    lcd.print("T:");
    lcd.print(temperatura, 1);
    lcd.print("C S:");
    lcd.print(sensacaoTermica, 1);
    lcd.print("C   ");

    
    lcd.setCursor(0, 2);
    lcd.print("U:");
    lcd.print(umidade);
    lcd.print("% P:");
    lcd.print(pressao);
    lcd.print("hPa ");

   
    lcd.setCursor(0, 3);
    lcd.print(condicao.substring(0, 12)); 

   
    Serial.println("Data: " + dataAtual);
    Serial.println("Hora: " + horaAtual);
    
    Serial.printf("Temp: %f ", temperatura );
    Serial.printf("sTerm: %f ", sensacaoTermica );
    Serial.printf("umi: %i ", umidade );
    Serial.printf("Press: %i ", pressao );
    Serial.println("Cond: " + condicao);
    Serial.println("NSol: " + nascerdoSol );

    

    Serial.println("-------------------------------------------------------------------------");
    
    msgJson = ""; 
  }
}

void conexaoWifi() {
  Serial.printf("Conectando ao Wi-Fi: %s\n", SSID);

  WiFi.begin(SSID, SENHA);

  unsigned long tempoWifi = millis();
  unsigned long tempoPonto = millis();

  while (WiFi.status() != WL_CONNECTED && millis() - tempoWifi < tempoConexao) {
    if (millis() - tempoPonto >= 500) {
      Serial.print(".");
      tempoPonto = millis();
    }
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWi-Fi conectado com sucesso!");
    Serial.print("Endereco IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nFalha ao se conectar.");
  }
}

void reConexaoWifi() {
  static unsigned long tempoUltimaConexao = 0;

  unsigned long tempoAtual = millis();

  if (tempoAtual - tempoUltimaConexao >= tempoReConexao) {

    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("Conexao Wi-Fi perdida!");
      conexaoWifi();
    }

    tempoUltimaConexao = millis();
  }
}

String consultaAPI(String codigo) { 
  HTTPClient http;

  http.begin(enderecoAPI + codigo);
  int codigoHTTP = http.GET();
  String resposta = "";

  if (codigoHTTP == 200)
  {
    resposta = http.getString();
  }
  else {
    Serial.println("Deu erro: codigo: " + String(codigoHTTP)); 
  }

  http.end();
  return resposta;
}