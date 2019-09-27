


BLEServer *pServer = NULL;
BLECharacteristic * pTxCharacteristic;
bool deviceConnected = false;
bool oldDeviceConnected = false;
uint8_t txValue = 0;

#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E" // 0x2A3D

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      Serial.println("BT connected");
      deviceConnected = true;
      watch.setBTstate(true);

      watch.sendBTstate();
    };

    void onDisconnect(BLEServer* pServer) {
      Serial.println("BT disconnected");
      deviceConnected = false;
      watch.setBTstate(false);
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string rxValue = pCharacteristic->getValue();
      char *cstr = new char[rxValue.length() + 1];
      strcpy(cstr, rxValue.c_str());

      watch.checkBTdata(cstr);
    }
};

void M5BLEinit() {
  BLEDevice::init("HappyWatch");

  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);


  pTxCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID_TX,
                      BLECharacteristic::PROPERTY_NOTIFY | 
                      BLECharacteristic::PROPERTY_READ
                    );
                      
  pTxCharacteristic->addDescriptor(new BLE2902());
 
  BLECharacteristic *pRxCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID_RX,
                                         BLECharacteristic::PROPERTY_WRITE
                                       );
 
  pRxCharacteristic->setCallbacks(new MyCallbacks());

  pService->start();
  pServer->getAdvertising()->start();
}

void M5BLEloop() {
  if (deviceConnected && (watch.BLEdata != "")) {
    pTxCharacteristic->setValue(watch.BLEdata);
    pTxCharacteristic->notify();

    delay(50); // bluetooth stack will go into congestion, if too many packets are sent
  }
}
