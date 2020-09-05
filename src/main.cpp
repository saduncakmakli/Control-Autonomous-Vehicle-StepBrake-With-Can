#include <Arduino.h>
#include <SPI.h>
#include <mcp_can.h>

//UNO İCİN
//MOSI 11 İLE KISA DEVRE!
//MISO 12 İLE KISA DEVRE!
//SCK 13 İLE KISA DEVRE!
//ISCP ÜZERİNDEN SPI KULLANIRKEN BU PINLERI KULLANMA!
//SMD OLMAYAN DIP SOKETLİ ARDUİNOLARDA 2.ICSP PINLARI 11,12,13DEN BAĞIMSIZ KULLANILABİLİR.
//UNO PWM PINLER 3,5,6,9,10
//UNO INTERRUPT PINLER 2,3

const int spiCSPin = 10; //12 MISO, 11 MOSI, 13 SCK
MCP_CAN CAN(spiCSPin);

const unsigned short STEP_PULSE = 6;
const unsigned short STEP_DIRECTION = 5;
const unsigned short STEP_ENABLE = 4; //HIGH İSE KAPALI, LOW İSE AÇIK :)

const unsigned short MAX_FREN_PULSE = 50;

short FrenKonum = 0;
short canFrenIstek = 0;

void setup()
{
  //SERIAL BEGIN
  Serial.begin(9600);

  //MOTOR SETUP
  pinMode(STEP_DIRECTION, OUTPUT);
  pinMode(STEP_ENABLE, OUTPUT);
  pinMode(STEP_PULSE, OUTPUT);

  digitalWrite(STEP_ENABLE, LOW);
  digitalWrite(STEP_DIRECTION, LOW);
  digitalWrite(STEP_PULSE, LOW);

  //full CURRENT  4.2 AMPER 800 STEP AYARI İLE 75 DARBE İLE FRENE TAM

  //CAN CONNETION BEGIN
  while (CAN_OK != CAN.begin(CAN_500KBPS))
  {
    Serial.println("CAN BUS Init Failed");
    delay(100);
  }
  Serial.println("CAN BUS Init OK!");
}

void loop()
{
  //CANDAN OKUMA BOLUMU
  unsigned char len = 0;
  unsigned char buf[8];

  if (CAN_MSGAVAIL == CAN.checkReceive())
  {
    CAN.readMsgBuf(&len, buf);

    unsigned long canId = CAN.getCanId();

    Serial.println("-----------------------------");
    Serial.print("Data from ID: 0x");
    Serial.println(canId, HEX);

    for (int i = 0; i < len; i++)
    {
      Serial.print(buf[i]);
      Serial.print(" | ");
    }
    Serial.println();

    //HEDEF HIZ VERISINE GORE MOTOR KONTROL
    if (canId == 50) //Motor hız olcum bilgisi
    {
      canFrenIstek = map(buf[1], 0, 255, 0, MAX_FREN_PULSE); //0-255 arasında gelen olcum sinyalini 0-MAX_FREN_PULSE arasında ölçekler.
    }
  }

  if (canFrenIstek == 0) digitalWrite(STEP_ENABLE, HIGH); //HIGH -> KAPALI
  else digitalWrite(STEP_ENABLE, LOW); //LOW -> AÇIK

  //Fren konumunun ayarlanması.
  if (canFrenIstek != FrenKonum)
  {
    if (canFrenIstek > FrenKonum)
    {
      digitalWrite(STEP_DIRECTION, HIGH);
      FrenKonum ++;
    }
    else if (canFrenIstek < FrenKonum)
    {
      digitalWrite(STEP_DIRECTION, LOW);
      FrenKonum --;
    }

    digitalWrite(STEP_PULSE, HIGH);
    delayMicroseconds(20);
    digitalWrite(STEP_PULSE, LOW);
    delay(20);
  }
  

}