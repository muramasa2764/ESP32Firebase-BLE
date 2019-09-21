/****************************************
 * 野良ハック B1144向けBLEライブラリ Ver.1.2
 * 2019/9/12 だーいー
 *****************************************/
#include "BLEDevice.h"
#include <string.h>
#include <misakiUTF16.h>
#include "NoraBLE.h"

static BLEUUID serviceUUID(BLE_SERVICE_UUID);
static BLEUUID cServiceUUID(BLE_CHARACTERISTIC_SERVICE_UUID);
static BLEUUID cDataUUID(BLE_CHARACTERISTIC_DATA_UUID);

/****************************************
 * コンストラクタ
 ****************************************/
CNoraBLE::CNoraBLE()
{
    m_bleform = NULL;
    m_showflash = 0;
    m_showmarqee = 0;
    m_showspeed = 0;
    m_datalen = 0;
    m_str = NULL;
    m_devicenum = 0;
    m_pClient = BLEDevice::createClient();
}
/****************************************
 * デストラクタ
 ****************************************/
CNoraBLE::~CNoraBLE()
{
  Clear();
}
/****************************************
 * [関数] Clear
 * [引数] 無し
 * [戻り値] 無し
 * [処理] メンバ変数初期化
 ****************************************/
void CNoraBLE::Clear(void)
{
    m_showflash = 0;
    m_showmarqee = 0;
    m_showspeed = 0;
    m_datalen = 0;
    m_devicenum = 0;
    if(m_bleform != NULL){
      delete[] m_bleform;
      m_bleform = NULL;
    }
    if(m_str != NULL){
      delete[] m_str;
      m_str = NULL;
    }
    return;
}
/****************************************
 * [関数] AddDevice
 * [引数] device BLEデバイス
 * [戻り値] 登録済のBLEデバイスの数 , 異常時 -1
 * [処理] BLEデバイスのアドレス追加
 ****************************************/
int CNoraBLE::AddDevice(BLEAdvertisedDevice device)
{
  if(m_devicenum >= DEVICENUM_MAX){
    return -1; // 16以上
  }
  // デバイスの追加
  m_devices[m_devicenum] = device;
  m_devicenum ++;
  return m_devicenum;
}
/****************************************
 * [関数] SetFlash
 * [引数] bool flg  true: ON　false: OFF
 * [戻り値] true 固定
 * [処理] フラッシュ設定
 ****************************************/
bool CNoraBLE::SetFlash(bool flg)
{
  if(flg){
    m_showflash = 0x01;
  }
  else{
    m_showflash = 0x00;    
  }
  return true;
}
/****************************************
 * [関数] SetMarqee
 * [引数] bool flg  true: ON　false: OFF
 * [戻り値] true 固定
 * [処理] マーキー設定
 ****************************************/
bool CNoraBLE::SetMarqee(bool flg)
{
  if(flg){
    m_showmarqee = 0x01;
  }
  else{
    m_showmarqee = 0x00;    
  }
  return true;
}
/****************************************
 * [関数] SetSpeed
 * [引数] uint8_t  spd  0～7 の8段階
 * [戻り値] true 正常 false 異常
 * [処理] スピード設定
 ****************************************/
bool CNoraBLE::SetSpeed(uint8_t spd)
{
  if(spd < 0 || spd > 7){
    return false;
  }
  m_showspeed = spd;
  return true;
}
/****************************************
 * [関数] SetMode
 * [引数] uint8_t  mde 表示モード
 *                0 左スクロール
 *                1 右スクロール
 *                2 上スクロール
 *                3 下スクロール
 *                4 静止
 *                5 アニメーション
 *                6 つもる
 *                7 ボリューム
 *                8 レーザー
 * [戻り値] true 正常 false 異常
 * [処理] 表示モード設定
 ****************************************/
bool CNoraBLE::SetMode(uint8_t mde)
{
  if(mde < 0 || mde > 8){
    return false;
  }
  m_showmode = mde;
  return true;  
}
/****************************************
 * [関数] SetString
 * [引数] char  *str  表示する文字列
 * [戻り値] true 固定
 * [処理] 表示文字列の設定（正規版）
 ****************************************/
bool CNoraBLE::SetString(char *str)
{
  int len = 0;
  if(m_str != NULL){
    delete[] m_str;
    m_str = NULL;
    m_datalen = 0;
  }
  // 文字数を獲得する
  len = u8len(str);

  if(strlen(str) <= 0 || len <= 0){
    m_datalen = 0;
    return true;
  }
  m_str = new char[strlen(str)+1];
  memset(m_str, '\0', strlen(str)+1);
  strcpy(m_str, str);
  m_datalen = len;
  return true;
}
/****************************************
 * [関数] SetString
 * [引数] char  *str  表示する文字列
 *        int   len　文字の数(半角も全角も１文字は1でカウント)
 * [戻り値] true 固定
 * [処理] 表示文字列を設定（ベータ版）
 ****************************************/
bool CNoraBLE::SetString(char *str, int len)
{
  if(m_str != NULL){
    delete[] m_str;
    m_str = NULL;
    m_datalen = 0;
  }
  if(strlen(str) <= 0 || len <= 0){
    m_datalen = 0;
    return true;
  }
  m_str = new char[strlen(str)+1];
  memset(m_str, '\0', strlen(str)+1);
  strcpy(m_str, str);
  m_datalen = len;
  return true;
}
/****************************************
 * [関数] GetDevicenum
 * [引数] なし
 * [戻り値] int 登録済デバイスの数
 * [処理] 登録したB1144デバイスの数
 ****************************************/
int CNoraBLE::GetDevicenum(void)
{
  return m_devicenum;
}
/****************************************
 * [関数] GetFlash
 * [引数] なし
 * [戻り値] bool フラッシュ設定状態(true: ON false: OFF)
 * [処理] フラッシュ設定のON/OFF取得
 ****************************************/
bool CNoraBLE::GetFlash(void)
{
  if(m_showflash){
    return true;
  }
  return false;
}
/****************************************
 * [関数] GetMarqee
 * [引数] なし
 * [戻り値] bool マーキー設定状態(true: ON false: OFF)
 * [処理] マーキー設定のON/OFF取得
 ****************************************/
bool CNoraBLE::GetMarqee(void)
{
  if(m_showmarqee){
    return true;
  }
  return false;
}
/****************************************
 * [関数] GetSpeed
 * [引数] なし
 * [戻り値] uint8_t スピード設定
 * [処理] スピード設定の取得
 ****************************************/
uint8_t CNoraBLE::GetSpeed(void)
{
  return m_showspeed;
}
/****************************************
 * [関数] GetMode
 * [引数] なし
 * [戻り値] uint8_t モード設定
 * [処理] モード設定の取得
 ****************************************/
uint8_t CNoraBLE::GetMode(void)
{
  return m_showmode;  
}
/****************************************
 * [関数] WriteBLE
 * [引数] int index 表示対象デバイスの指定 (0はすべてのデバイス)
 * [戻り値] bool true:成功 false:異常
 * [処理] BLEデバイスへデータ書き込み
 ****************************************/
bool CNoraBLE::WriteBLE(int index)
{
  int datasize = 0;
  int formsize = 0;

  if(m_datalen <= 0 || m_devicenum < index){
    return false;
  }  
  // データ作成
  if((formsize = MakeData()) < 0){
    return false;
  }
  // BLEデータ書き込み
  if(index == 0){
    // 全デバイスへ書き込み
    for(int i = 0; i < m_devicenum; i++){
      SendData(i+1, formsize);
    }
  }
  else{
    // 指定デバイスへの書き込み
    SendData(index, formsize);
  }
  return true;
}
/****************************************
 * [関数] MakeData
 * [引数] なし
 * [戻り値] int BLEフォーマットデータのサイズ -1:異常
 * [処理] BLE書き込み情報の作成
 ****************************************/
int CNoraBLE::MakeData(void)
{
  uint8_t font[8];
  char* p = m_str;
  int formsize = 0;

  // BLEフォーマットデータ領域確保
  if(m_bleform != NULL){
    delete[] m_bleform;
    m_bleform = NULL;
  }
  formsize = (B1144_1CHAR_SIZE * m_datalen) / B1144_SEPARATE_SIZE;
  formsize = (formsize + 1) * B1144_SEPARATE_SIZE;
  formsize = B1144_DATA_POS + formsize; 
  m_bleform = new uint8_t[formsize];
  memset(m_bleform, '\0', formsize);

  // ヘッダ情報作成
  memcpy(m_bleform, B1144_HEADER_KEY, 4); // Key
  m_bleform[B1144_FLASH_POS] = m_showflash; // Flash
  m_bleform[B1144_MARQEE_POS] = m_showmarqee; // Marqee
  uint8_t sm = 0;
  sm = m_showspeed << 4;
  sm = sm & 0xF0;
  sm = sm | m_showmode;
  m_bleform[B1144_SPEED_MODE_POS] = sm; // Speed & Mode
  m_bleform[B1144_LENGTH_POS+1] = m_datalen; // Length
  m_bleform[B1144_DATE_POS] = 0xE1; // Date(固定)
  m_bleform[B1144_DATE_POS+1] = 0x0C; // Date(固定)
  m_bleform[B1144_DATE_POS+2] = 0x06; // Date(固定)
  m_bleform[B1144_DATE_POS+3] = 0x17; // Date(固定)
  m_bleform[B1144_DATE_POS+4] = 0x2D; // Date(固定)
  m_bleform[B1144_DATE_POS+5] = 0x23; // Date(固定)

  // データ部作成
  for(int i = 0; i < m_datalen; i++){
    memset(font, '\0', sizeof(font));
     // 1文字分の美咲フォントパターンを取得
    if((p = getFontData(font, p, true)) == NULL){
      break;
    }
    // 1文字分のパターンをデータ領域に設定
    memcpy((m_bleform+B1144_DATA_POS)+(i*B1144_1CHAR_SIZE+2), font, MAX7219_1CHAR_SIZE);
  }
  
  return formsize;
}

/****************************************
 * [関数] SendData
 * [引数] int index  操作対象のデバイスインデックス
 *       int formsize 送信バイト数
 * [戻り値] bool true:成功 false:異常
 * [処理] BLEデバイスへデータ送信
 ****************************************/
bool CNoraBLE::SendData(int index, int formsize)
{
  int cnt;
  BLERemoteCharacteristic* cData;

  // LEDに接続 3回リトライ
  for(cnt = 0; cnt < 3; cnt ++){
    if(m_pClient->connect(&(m_devices[index-1]))){
      break;
    }
    sleep(1);
  }
  if(cnt == 3){
    return false; // 接続失敗
  }    
  // サービスに接続
  BLERemoteService *pRemoteService = m_pClient->getService(cServiceUUID);
  if(pRemoteService == nullptr){
    m_pClient->disconnect();
    return false;
  }
  // キャラクタリスティックを取得
  cData = pRemoteService->getCharacteristic(cDataUUID);
  if(cData == nullptr){
    cData = nullptr;
    m_pClient->disconnect();
    return false;
  }
  // データ送信
  for(int i = 0; i < formsize;  i+= B1144_SEPARATE_SIZE){
    cData->writeValue((uint8_t *)m_bleform+i, B1144_SEPARATE_SIZE, true);
    usleep(B1144_TIMEOUT);  // 16byte書いて少し待つ   
  }
  // 切断
  m_pClient->disconnect();
  sleep(2); // 切断されるまで少し時間がかかるらしいので少しまつ
  return true;
}

/****************************************
 * [関数] GetDeviceAddress
 * [引数] int index  対象のデバイスインデックス
 * [戻り値] デバイスのアドレス
 * [処理] 登録済BLEデバイスのアドレスを取得する
 ****************************************/
char* CNoraBLE::GetDeviceAddress(int index)
{
  if(m_devicenum < index){
    return NULL;
  }
  return (char *)(m_devices[index-1]).getAddress().toString().c_str();
}
/****************************************
 * [関数] SearchDevice
 * [引数] char *address  対象のデバイスアドレス
 * [戻り値] int デバイスのインデックス -1:異常
 * [処理] 登録済BLEデバイスのインデックスを取得する
 ****************************************/
int CNoraBLE::SearchDevice(char *address)
{
  for(int i=0; i < m_devicenum; i++){
    if(strcmp(m_devices[i].getAddress().toString().c_str(), address) == 0){
      return (i+1);
    }
  }
  return -1;
}

/****************************************
 * [関数] u8mb
 * [引数] const char chr  確認対象の1文字
 * [戻り値] UTF8の1文字分サイズ
 * [処理] UTF8の1文字分サイズを返す
 ****************************************/
int CNoraBLE::u8mb(const char chr)
{
  int byt = 1;
  if ((chr & 0x80) == 0x00) { //1byte文字は何もしない（[byt = 1]のまま）
  } else if ((chr & 0xE0) == 0xC0) { //2byte文字
    byt = 2;
  } else if ((chr & 0xF0) == 0xE0) { //3byte文字
    byt = 3;
  } else if ((chr & 0xF8) == 0xF0) { //4byte文字
    byt = 4;
  } else if ((chr & 0xFC) == 0xF8) { //5byte文字
    byt = 5;
  } else if ((chr & 0xFE) == 0xFC) { //6byte文字
    byt = 6;
  }
  return byt;
}

/****************************************
 * [関数] u8len
 * [引数] const char *str  文字数確認対象の文字列
 * [戻り値] 文字数
 * [処理] UFT8を含む文字列から文字数をカウントして返す
 ****************************************/
int CNoraBLE::u8len(const char *str)
{
  int cnt = 0;
  while (*str != '\0') {
    cnt++;
    str += u8mb(*str);
  }
  return cnt;
}
