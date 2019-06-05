#include <BLEPeripheral.h>
#include <Adafruit_Microbit.h>

// Device Name: Maximum 20 bytes
#define DEVICE_NAME "LineIoT"
// Local Name in advertising packet: Maximum 29 bytes
#define LOCAL_NAME "LineIoT"

// User Service UUID: Change this to your generated service UUID
#define USER_SERVICE_UUID "e9255128-5aae-4f57-a9e5-d1368c2cedda"
// User service characteristics
#define WRITE_CHARACTERISTIC_UUID "E9062E71-9E62-4BC6-B0D3-35CDCD9B027B"
#define NOTIFY_CHARACTERISTIC_UUID "62FBD229-6EDD-4D1A-B554-5C4E1BB29169"

// PSDI Service UUID: Fixed value for Developer Trial
#define PSDI_SERVICE_UUID "e625601e-9e55-4597-a598-76018a0d293d"
#define PSDI_CHARACTERISTIC_UUID "26e2b12b-85f0-4f3f-9fdd-91d114270e6e"


// PSDI Service UUID: Fixed value for Developer Trial
#define PSDI_SERVICE_UUID "e625601e-9e55-4597-a598-76018a0d293d"
#define PSDI_CHARACTERISTIC_UUID "26e2b12b-85f0-4f3f-9fdd-91d114270e6e"

BLEPeripheral blePeripheral;
BLEBondStore bleBondStore;

// Setup User Service
BLEService userService(USER_SERVICE_UUID);
BLEUnsignedCharCharacteristic writeCharacteristic(WRITE_CHARACTERISTIC_UUID, BLEWrite);
BLEUnsignedCharCharacteristic notifyCharacteristic(NOTIFY_CHARACTERISTIC_UUID, BLENotify);
// Setup PSDI Service
BLEService psdiService(PSDI_SERVICE_UUID);
BLECharacteristic psdiCharacteristic(PSDI_CHARACTERISTIC_UUID, BLERead, sizeof(uint32_t) * 2);

Adafruit_Microbit_Matrix microbit;

volatile int btnAction = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("Initializing...");

  pinMode(PIN_BUTTON_A, INPUT_PULLUP);
  pinMode(PIN_BUTTON_B, INPUT_PULLUP);

  // Clear bond store if push button A+B for 3 secs on start up
  // You can bond only one central device on each peripheral device by library restriction
  if (!digitalRead(PIN_BUTTON_A) && !digitalRead(PIN_BUTTON_B)) {
    delay(3000);
    if (!digitalRead(PIN_BUTTON_A) && !digitalRead(PIN_BUTTON_B)) {
      bleBondStore.clearData();
      Serial.println("Cleared bond store");
    }
  }

  attachInterrupt(PIN_BUTTON_A, buttonAction, CHANGE);

  blePeripheral.setDeviceName(DEVICE_NAME);
  blePeripheral.setLocalName(LOCAL_NAME);
  blePeripheral.setBondStore(bleBondStore);
  blePeripheral.setAdvertisedServiceUuid(userService.uuid());

  blePeripheral.addAttribute(userService);
  blePeripheral.addAttribute(writeCharacteristic);
  blePeripheral.addAttribute(notifyCharacteristic);
  blePeripheral.addAttribute(psdiService);
  blePeripheral.addAttribute(psdiCharacteristic);

  // Set callback
  blePeripheral.setEventHandler(BLEConnected, blePeripheralConnectHandler);
  blePeripheral.setEventHandler(BLEDisconnected, blePeripheralDisconnectHandler);
  writeCharacteristic.setEventHandler(BLEWritten, writeLEDCallback);

  // Set PSDI (Product Specific Device ID) value
  uint32_t deviceAddr[] = { NRF_FICR->DEVICEADDR[0], NRF_FICR->DEVICEADDR[1] };
  psdiCharacteristic.setValue((unsigned char *)deviceAddr, sizeof(deviceAddr));

  blePeripheral.begin();
  Serial.println("Ready to Connect");

  microbit.begin();
}

void loop() {
  BLECentral central = blePeripheral.central();

  if (central && central.connected() && btnAction > 0) {
    uint8_t btnRead = !digitalRead(PIN_BUTTON_A);
    btnAction = 0;
    notifyCharacteristic.setValue(btnRead);
    delay(20);
  }

  blePeripheral.poll();
}

void buttonAction() {
  btnAction++;
}

void writeLEDCallback(BLECentral& central, BLECharacteristic& characteristic) {
  if (writeCharacteristic.value()) {
    Serial.println("ON");
    microbit.show(microbit.HEART);
  } else {
    Serial.println("OFF");
    microbit.clear();
  }
}

void blePeripheralConnectHandler(BLECentral& central) {
  // central connected event handler
  Serial.print("Connected event, central: ");
  Serial.println(central.address());

  microbit.show(microbit.YES);
}

void blePeripheralDisconnectHandler(BLECentral& central) {
  // central disconnected event handler
  Serial.print("Disconnected event, central: ");
  Serial.println(central.address());
  
  microbit.show(microbit.NO);
}
