#define BLYNK_TEMPLATE_ID "TMPL66fIpuam0"
#define BLYNK_TEMPLATE_NAME "Nhayled"
#define BLYNK_AUTH_TOKEN "kjs8zcNKsNICd3_wGKq5AtU8rWcK5h7y"

#include "Arduino.h"
#include "BlynkSimpleEsp32.h"
#include "WiFiManager.h"
#include "SPI.h"
#include "MFRC522.h"
#define Led 2
#define UART2_RX 16
#define UART2_TX 17

HardwareSerial Serial_2(2); // Định nghĩa UART2
uint8_t button;

#define RSS_PIN 5
#define RST_PIN 0
MFRC522 rfid(RSS_PIN, RST_PIN);

String ma_the = "";
String ma_hex = "";

const char auth[] = BLYNK_AUTH_TOKEN;
const char ssid[] = "P202";
const char pass[] = "23456789";
int soluong_the =0;
String danh_sach_the[10];

const String ma_loc = "254173622";
const String ma_loc1 = "194602041";

bool kiemtra(String the_){
  for(byte i=0; i<soluong_the; i++){
    if(danh_sach_the[i]==the_){
      return true;
    }
  }
  return false;
}

void rfid_config() {
    SPI.begin();
    rfid.PCD_Init();
}

void doc_the() {
    if (!rfid.PICC_IsNewCardPresent()) {
        return;
    } else {
        Serial.println("Received your card");
        if (!rfid.PICC_ReadCardSerial()) {
            Serial.println("Your card might be wrong!!");
        } else {
            ma_the = "";
            ma_hex = "";
            Serial.println("Your card work fine!! -> Reading the card is in progress!");
            for (byte i = 0; i < 4; i++) {
                ma_the += rfid.uid.uidByte[i];
                ma_hex += String(rfid.uid.uidByte[i], HEX);
                if (i != 3) {
                    ma_hex += " ";
                }
            }
            if(kiemtra(ma_the)){
              Serial.println("The nay da duoc quet qua!!");
            }
            else{
              if(soluong_the<10){
                danh_sach_the[soluong_the]=ma_the;
                soluong_the++;
              }
              else{
                Serial.println("Danh sach the da day! Nhung toi se lam moi lai no!");
                for(int i=0;i<soluong_the;i++){
                  danh_sach_the[i]="";
                }
                soluong_the=0;
              }
            }
            Serial.println("Ma the: " + ma_the);
            Serial.println("Ma_hex: " + ma_hex);
            if (ma_the == ma_loc || ma_the == ma_loc1) {
                digitalWrite(Led, HIGH);
                delay(1200);
                digitalWrite(Led, LOW);
            }
        }
    }
    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
}

void setup() {
    Serial.begin(9600);
    Serial_2.begin(115200, SERIAL_8N1, UART2_RX, UART2_TX);
    Blynk.begin(auth, ssid, pass);
    pinMode(Led, OUTPUT);
    rfid_config();
}

BLYNK_WRITE(V2) {
    button = param.asInt();
    if (button == 1) {
        digitalWrite(Led, HIGH);
    } else {
        digitalWrite(Led, LOW);
    }
}

void loop() {
    Blynk.run();
    doc_the();
    
}
