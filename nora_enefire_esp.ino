#include "BLEDevice.h"
#include <IOXhop_FirebaseESP32.h>
#include <WiFi.h>
#include <string.h>
#include <EEPROM.h>
#include "NoraBLE.h"
#include <rom/rtc.h>
#include <ArduinoJson.h>

// LED表示情報を含むデータを確保数るEEPROMのサイズ
#define EEPROM_SIZE 512

// Wi-Fi,Firebaseの接続情報
// 各々の環境を設定してください。
#define WIFI_SSID             "SSID"
#define WIFI_PASSWORD         "PASSWORD"
#define FIREBASE_DATABASE_URL "FIREBASE-DATABASE-URL"
#define FIREBASE_STREAM       "FIREBASE-STREAM"


// LED表示情報
// EEPROMに保存するLED表示に関する情報
struct NORA_FB_INFO
{
  int mode;           // 再起動前のモード
  int display_speed;  // 表示スピード
  int display_mode;   // 表示モード
  int strlen;         // payload 文字数
  int bytelen;        // payload バイト数
  char payload[256];  // payload ドットマトリクスデータ
};

// FireBaseからの読み込み情報カウンタ
// FireBase接続時に読み込むデータの1回目は更新前のもののため、無視するために使用する。
// 今となってはboolにしておけばよかった。
int FireBaseWriteCount = 0;

// 野良BLEコントラスタ NoraBLE.h 参照
CNoraBLE noraBLE;
// BLE Pelipheralの起動判定フラグ
static bool peripheral_start = false;


// BLEスキャン成功時のコールバック
// ライブラリ参照
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks{
  void onResult(BLEAdvertisedDevice advertisedDevice){
    int num = 0;  
      
    // 野良バッジを検出したらLEDデバイスとして1件クラスに追加する
    if(strcmp(advertisedDevice.getName().c_str(), BLE_DEVICE_NAME) == 0){
      num = noraBLE.AddDevice(advertisedDevice);
      if(num >= DEVICENUM_MAX){
        advertisedDevice.getScan()->stop();
      }
    }
  }
};

// 初期設定
void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("[start setup()]");
  
  // FireBase読み込みカウンタ初期化
  FireBaseWriteCount = 0;

  // EEPROMを準備
  if (!EEPROM.begin(EEPROM_SIZE))
  {
    Serial.println("failed to initialise EEPROM");
    delay(1000);
  }  
  
  // reset理由をチェック
  RESET_REASON r = rtc_get_reset_reason(0);
  Serial.print("CPU0 reset reason = ");
  Serial.println(r);
  delay(100);
  
  // software reset 以外の場合
  if (r != 12)
  {
    Serial.print("Hard Reset");

    // BLE Pelipheralの起動判定フラグはOFF
    peripheral_start = false;
    // Wi-FiでFirabaseに接続
    nora_firebaseConnector();
    
  }else{
    // software reset の場合
    NORA_FB_INFO readEEPdata;
    // EEPROM読み込み
    EEPROM.get<NORA_FB_INFO>(0, readEEPdata);
    //前モードが Wi-Fiの場合
    Serial.print("起動前モード：");
    Serial.println(readEEPdata.mode);
    if(readEEPdata.mode == 0xF0){
      // BLE Pelipheralの起動判定フラグをON
      peripheral_start = true;

      // BLEScan開始
      nora_BLE_Scanner();
            
    // 前モードがBluetoothの場合
    }else{
      peripheral_start = false;
      //Wi-FiでFirabaseに接続
      nora_firebaseConnector();
    }
  }
}

void loop() {
  // EEPROM 読み込み/書き込み 用データ格納領域確保
  NORA_FB_INFO readFireBaseData;
  NORA_FB_INFO writeFireBaseData;

  // BLE Pelipheralの起動判定フラグがONの場合
  if(peripheral_start == true){
    Serial.println("LED Scan End!!\n");
    Serial.println("EEPROM読み込み開始");
    // LED表示情報の読み込み開始
    EEPROM.get<NORA_FB_INFO>(0, readFireBaseData);
    int dspspeed = readFireBaseData.display_speed;
    int dspmode  = readFireBaseData.display_mode;
    int splen    = readFireBaseData.strlen;
    int bplen    = readFireBaseData.bytelen;
    char blesenddata[bplen];
    
    Serial.print("EEPROM:文字列：");

    // Payloadの格納
    for(int loop = 0; loop < bplen; loop++){
      blesenddata[loop] = readFireBaseData.payload[loop];
      Serial.print(blesenddata[loop]);
    }
    Serial.println();
    Serial.print("EEPROM:文字列長：");
    Serial.println(splen);
    Serial.print("EEPROM:文字列バイト数：");
    Serial.println(bplen);
    Serial.print("EEPROM:表示スピード：");
    Serial.println(dspspeed);
    Serial.print("EEPROM:表示モード：");
    Serial.println(dspmode);

    // BLE送信開始
    nora_ble_send(blesenddata, splen, dspspeed, dspmode);

    // EEPROMにモード書き込み(Wi-FI[0xF0] → BLE[0xF1])
    writeFireBaseData.mode = 0xF1;
    EEPROM.put<NORA_FB_INFO>(0, writeFireBaseData);
    EEPROM.commit();

    // 1分待機
    delay(60000);
    
    Serial.println("1分後に再起動");

    // ソフトリセット開始
    softReset();
  }
}

// software reset
void softReset()
{
  Serial.println("Going to software reset now...");
  delay(100);
  ESP.restart();
}

// nora badge sender
void nora_ble_send(char *norastr, int noralen, int noraspeed, int noramode){
  noraBLE.SetSpeed(noraspeed);
  noraBLE.SetMode(noramode);
  noraBLE.SetString(norastr, noralen);
  noraBLE.WriteBLE(0);
}


// firebase connector
void nora_firebaseConnector(){
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
        
  // WiFi Connected
  Serial.println("\nWiFi Connected.");
  Serial.println(WiFi.localIP());

  //Firebase接続開始
  Firebase.begin(FIREBASE_DATABASE_URL);
  Firebase.stream( FIREBASE_STREAM, [](FirebaseStream stream) {
    String eventType = stream.getEvent();
    eventType.toLowerCase();
    Serial.print("event: ");
    Serial.println(eventType);
    if (eventType == "put") {
      String path = stream.getPath();
      String data = stream.getDataString();
      Serial.print("data: ");
      Serial.println(stream.getDataString());
      if(FireBaseWriteCount == 0){
        //最初の読み込みは無視する。
        FireBaseWriteCount++;
      }else{
        FireBaseWriteCount = 0;
        //文字列抽出
        StaticJsonBuffer<512> fbgetJsonData;
        JsonObject& reciveJson = fbgetJsonData.parseObject(data);
        int data_len           = reciveJson["len"];
        String payloadStrings  = reciveJson["value"];
        int payloadSpeed       = reciveJson["speed"];
        int payloadMode        = reciveJson["mode"];
        int dataByeLength      = payloadStrings.length() + 1; 
        char sendStringData[dataByeLength];
        payloadStrings.toCharArray(sendStringData, dataByeLength);
        Serial.print("抽出文字列：");
        Serial.println(payloadStrings);
        Serial.print("文字サイズ：");
        Serial.println(data_len);
        Serial.print("文字バイトサイズ：");
        Serial.println(dataByeLength);
        Serial.print("表示スピード：");
        Serial.println(payloadSpeed);
        Serial.print("表示モード：");
        Serial.println(payloadMode);
            
        NORA_FB_INFO writeData;
        writeData.mode          = 0xF0;
        writeData.display_speed = payloadSpeed;
        writeData.display_mode  = payloadMode;
        writeData.strlen        = data_len;
        writeData.bytelen       = dataByeLength;
        for(int wloop = 0; wloop < dataByeLength; wloop++){
          writeData.payload[wloop] = sendStringData[wloop];
        }
        EEPROM.put<NORA_FB_INFO>(0, writeData);
        EEPROM.commit();
        Serial.println("ESP32 SoftReset");
        delay(500);
        softReset();
       }
    }
  });
}

//BLE Periperal scanner
void nora_BLE_Scanner(){
  Serial.println("Start Application...\n Please Wait 5sec\n");
  // BLE init
  BLEDevice::init("");
  // 20秒間Scan
  BLEScan *pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
  pBLEScan->start(20);  
}
