#ifndef _NORA_BLE_H_
#define _NORA_BLE_H_

#include "BLEDevice.h"

/* BLEデバイス定義 */
#define BLE_DEVICE_NAME "LSLED"
#define BLE_SERVICE_UUID "0000fee7-0000-1000-8000-00805f9b34fb"
#define BLE_CHARACTERISTIC_SERVICE_UUID "0000fee0-0000-1000-8000-00805f9b34fb" 
#define BLE_CHARACTERISTIC_DATA_UUID "0000fee1-0000-1000-8000-00805f9b34fb"

/* b1144フォーマット定義 */
#define B1144_HEADER_KEY  "wang"
#define B1144_FLASH_POS 6
#define B1144_MARQEE_POS 7
#define B1144_SPEED_MODE_POS 8
#define B1144_LENGTH_POS 16
#define B1144_DATE_POS 38
#define B1144_DATA_POS 64
#define B1144_SEPARATE_SIZE 16
#define B1144_1CHAR_SIZE 11
#define B1144_TIMEOUT 100000
#define MAX7219_1CHAR_SIZE 8
#define DEVICENUM_MAX  8

/* クラス定義 */
class CNoraBLE
{
  private:
    uint8_t     *m_bleform;
    uint8_t     m_showflash;
    uint8_t     m_showmarqee;
    uint8_t     m_showspeed;
    uint8_t     m_showmode;
    uint8_t     m_datalen;
    char        *m_str;
    BLEAdvertisedDevice  m_devices[DEVICENUM_MAX];    
    int         m_devicenum;
    BLEClient   *m_pClient;
    int         MakeData(void);
    bool        SendData(int index, int formsize);
    int         u8mb(const char chr);
    int         u8len(const char *str);
  public:
    CNoraBLE(); // コンストラクタ
    ~CNoraBLE();  // デストラクタ
    void Clear();
    int AddDevice(BLEAdvertisedDevice device);
    bool SetFlash(bool flg);
    bool SetMarqee(bool flg);
    bool SetSpeed(uint8_t spd);
    bool SetMode(uint8_t mde);
    bool SetString(char *str);
    bool SetString(char *str, int len); // ベータ用の関数
    int GetDevicenum(void);
    char* GetDeviceAddress(int index);
    bool GetFlash(void);
    bool GetMarqee(void);
    uint8_t GetSpeed(void);
    uint8_t GetMode(void);
    bool WriteBLE(int index=0);
    int SearchDevice(char *address);
};

#endif
