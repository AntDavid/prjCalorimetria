/* Por Antonio David de Jesus santana, em 11/04/2023  
    
 */
#include <Wire.h>
#include <Adafruit_MLX90614.h>
#include <SparkFunMLX90614.h>
#include <LiquidCrystal_I2C.h>
#include <SD.h>                        // Biblioteca SD inclusa

File myFile;                           // Cria um ponteiro para arquivo

// --- Mapeamento de Hardware ---
#define CS_pin 4               // Comunicação SPI, CS_pin no digital 10, alterado para CS_pin 4
#define IR1 0x5A // Sensor 1
#define IR2 0x5B // Sensor 2 

// Definir os números dos pinos
const int trigger = 12;
const int echo = 11;

// Define os pinos usados pelo driver L298N
#define enA 3
#define in1 5
#define in2 6

// Define o pino analógico usado pelo potenciômetro
#define potPin A0

Adafruit_MLX90614 mlx = Adafruit_MLX90614();
LiquidCrystal_I2C lcd(0x27, 16, 2);  // Define o endereco I2C do display e quantidade de colunas e linhas

// Array que desenha o símbolo de grau
byte grau[8] = {B00110, B01001, B01001, B00110, B00000, B00000, B00000, B00000};

// Definições gerais
double temp_amb1;
double temp_obj1;
double temp_amb2;
double temp_obj2;
unsigned long minuto = 1;  // Armazena os minutos transcorridos
long duracao;
float dist;

void setup() {
  Serial.begin(9600);

  // Inicializa o display LCD I2C
  lcd.init();
  lcd.backlight();
  
  // Inicializa a comunicação com o cartão SD
  if (!SD.begin(CS_pin)) {
    Serial.println("Falha ao inicializar o cartão SD.");
    return;
  }

  // Abre o arquivo no modo de escrita
  myFile = SD.open("dados_temperatura.txt", FILE_WRITE);

  // Se o arquivo foi aberto com sucesso, escreve um cabeçalho
  if (myFile) {
    myFile.println("Minuto,Temperatura_Ambiente_1,Temperatura_Objeto_1,Temperatura_Ambiente_2,Temperatura_Objeto_2,Distancia");
    myFile.close();
  } else {
    Serial.println("Falha ao abrir o arquivo.");
  }

  pinMode(trigger, OUTPUT);  // Configura o pino trigger como saída
  pinMode(echo, INPUT);  // Configura o pino echo como entrada
  
  // Configura os pinos como saídas ou entradas
  pinMode(enA, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  analogWrite(enA, 0);
}

void loop() {
  // Leitura da temperatura ambiente e do objeto
  mlx.begin(IR1);  // Inicializa o MLX90614 na porta 0x5A
  temp_amb1 = mlx.readAmbientTempC();
  temp_obj1 = mlx.readObjectTempC();

  mlx.begin(IR2);  // Inicializa o MLX90614 na porta 0x5B
  temp_amb2 = mlx.readAmbientTempC();
  temp_obj2 = mlx.readObjectTempC();

  // Realiza a leitura do sensor de distância ultrassônico
  digitalWrite(trigger, LOW);
  delayMicroseconds(5);
  digitalWrite(trigger, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigger, LOW);
  duracao = pulseIn(echo, HIGH);
  dist = duracao * 0.034 / 2;

  // Salva os dados no cartão SD
  myFile = SD.open("dados_temperatura.txt", FILE_WRITE);
  if (myFile) {
    myFile.print(minuto);
    myFile.print(",");
    myFile.print(temp_amb1);
    myFile.print(",");
    myFile.print(temp_obj1);
    myFile.print(",");
    myFile.print(temp_amb2);
    myFile.print(",");
    myFile.print(temp_obj2);
    myFile.print(",");
    myFile.println(dist);
    myFile.close();
  } else {
    Serial.println("Falha ao abrir o arquivo.");
  }

  // Mostra as informações no display
  lcd.setCursor(0, 0);
  lcd.print("Ambien1:");
  lcd.setCursor(9, 0);
  lcd.print(temp_amb1);
  lcd.setCursor(14, 0);
  lcd.write(1);
  lcd.print("C");
  lcd.setCursor(0, 1);
  lcd.print("Objeto1:");
  lcd.setCursor(9, 1);
  lcd.print(temp_obj1);
  lcd.setCursor(14, 1);
  lcd.write(1);
  lcd.print("C");

  // Aguarda 1 segundo para mudar a leitura no display
  delay(1000);
  lcd.clear();
  
  lcd.setCursor(0, 0);
  lcd.print("Ambien2:");
  lcd.setCursor(9, 0);
  lcd.print(temp_amb2);
  lcd.setCursor(14, 0);
  lcd.write(1);
  lcd.print("C");
  lcd.setCursor(0, 1);
  lcd.print("Objeto2:");
  lcd.setCursor(9, 1);
  lcd.print(temp_obj2);
  lcd.setCursor(14, 1);
  lcd.write(1);
  lcd.print("C");
  delay(1000);
  lcd.clear();

  // Mostra as informações no Serial Monitor
  Serial.print(temp_amb1);
  Serial.print("*C");
  Serial.print(" ");
  Serial.print(temp_obj1);
  Serial.print("*C");
  Serial.print(" ");
  Serial.print(temp_amb2);
  Serial.print("*C");
  Serial.print(" ");
  Serial.print(temp_obj2);
  Serial.print("*C");
  Serial.print(" ");
  Serial.print("Distancia em cm: ");
  Serial.println(dist);
  
  // Incrementa os minutos
  minuto++;

  // Lê o valor atual do potenciômetro
  int lerPot = analogRead(potPin);
  int motorSpeed = lerPot;

  // Define a direção e velocidade do motor de acordo com o valor do potenciômetro
  if (lerPot < 450) {   
    motorSpeed = map(lerPot, 500, 0, 0, 255);
    if (motorSpeed > 0) {
      digitalWrite(in1, LOW);
      analogWrite(in2, motorSpeed);  
      delay(500);
      Serial.println(lerPot);
    }
  } else if (lerPot > 650) {
    motorSpeed = map(lerPot, 515, 1023, 0, 255);
    if (motorSpeed > 0) {
      digitalWrite(in2, LOW);
      analogWrite(in1, motorSpeed);
      delay(500);
      Serial.println(lerPot);
    }
  } else {
    analogWrite(enA, 0);
    digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);
    Serial.println("Parado");
    Serial.println(lerPot);
  }

  // Atualiza a velocidade do motor
  analogWrite(enA, motorSpeed);

  // Aguarda 1 segundo para mudar a leitura no Serial
  delay(1000);
  lcd.clear();
}