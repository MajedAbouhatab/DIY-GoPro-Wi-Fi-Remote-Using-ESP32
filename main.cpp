#include <Arduino.h>
#include "BLEDevice.h"

#define ServiceCharacteristic(S, C) ThisClient->getService(S)->getCharacteristic(C)

static BLEDevice *ThisDevice;
static BLEClient *ThisClient = ThisDevice->createClient();
BLEScan *ThisScan = ThisDevice->getScan();
static BLEUUID ServiceUUID((uint16_t)0xFEA6);
static BLEUUID CommandWriteCharacteristicUUID("b5f90072-aa8d-11e3-9046-0002a5d5c51b");
static bool ItsOn = false;
unsigned long TimeStamp;

bool ScanAndConnect(void)
{
  ThisScan->clearResults();
  ThisScan->start(3);
  for (int i = 0; i < ThisScan->getResults().getCount(); i++)
    if (ThisScan->getResults().getDevice(i).haveServiceUUID() && ThisScan->getResults().getDevice(i).isAdvertisingService(BLEUUID(ServiceUUID)))
    {
      ThisScan->stop();
      ThisClient->connect(new BLEAdvertisedDevice(ThisScan->getResults().getDevice(i)));
      return true;
    }
  return false;
}

void setup(void)
{
  pinMode(25, INPUT_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, 0);
  ThisDevice->init("");
  ThisDevice->setEncryptionLevel(ESP_BLE_SEC_ENCRYPT);
}

void loop(void)
{
  if (!digitalRead(25))
  {
    if (millis() - TimeStamp > 1000)
    {
      TimeStamp = millis();
      noTone(LED_BUILTIN);
      digitalWrite(LED_BUILTIN, 1);
      if (ItsOn)
      {
        ServiceCharacteristic(ServiceUUID, CommandWriteCharacteristicUUID)->writeValue({0x03, 0x01, 0x01, 0x00});
        while (millis() - TimeStamp < 5000)
          yield();
        ServiceCharacteristic(ServiceUUID, CommandWriteCharacteristicUUID)->writeValue({0x01, 0x05});
        ItsOn = false;
        digitalWrite(LED_BUILTIN, 0);
      }
      else
      {
        if (ScanAndConnect())
        {
          ServiceCharacteristic(ServiceUUID, CommandWriteCharacteristicUUID)->writeValue({0x03, 0x01, 0x01, 0x01});
          ItsOn = true;
          tone(LED_BUILTIN, 1);
        }
        else
          ESP.restart();
      }
    }
  }
}
