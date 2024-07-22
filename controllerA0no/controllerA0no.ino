#include <ArduinoBLE.h>

// BLE 서비스와 특성 정의
BLEService customService("12345678-1234-5678-1234-56789abcdef0");
BLEStringCharacteristic customCharacteristic("87654321-4321-6789-4321-abcdef012345", BLERead | BLEWrite | BLENotify, 5);

// 순서대로 엄지, 검지, 중지, 약지, 새끼
const int sensorPins[5] = {A0, A1, A2, A3, A6}; // 사용할 핀들
const int threshold = 200;

char prevValue[6] = "00000"; // 이전 상태 저장 변수

void setup() {
  for (int i = 0; i < 5; i++) {
    pinMode(sensorPins[i], INPUT);
  }
  Serial.begin(9600);
  //while (!Serial);

  // BLE 초기화
  if (!BLE.begin()) {
    Serial.println("Starting BLE failed!");
    while (1);
  }

  // BLE 장치 이름 설정
  BLE.setLocalName("Nano33BLE");
  BLE.setAdvertisedService(customService);
  customService.addCharacteristic(customCharacteristic);
  BLE.addService(customService);

  // 초기 값 설정
  customCharacteristic.writeValue("00000");

  // BLE 광고 시작
  BLE.advertise();
  Serial.println("Bluetooth device active, waiting for connections...");
}

void loop() {
  // 중앙 장치가 연결되었는지 확인
  BLEDevice central = BLE.central();

  if (central) {
    Serial.print("Connected to central: ");
    Serial.println(central.address());

    while (central.connected()) {
      char value[6] = "00000"; // 5개의 문자와 null terminator

      for (int i = 0; i < 5; i++) {
        if (analogRead(sensorPins[i]) < threshold) {
          value[i] = '1'; // 해당 비트를 1로 설정
        } else {
          value[i] = '0';
        }
      }
      value[0] = '0';

      // 현재 상태와 이전 상태 비교
      if (strcmp(value, prevValue) != 0) {
        // 상태가 변경된 경우에만 전송
        Serial.println(value);
        customCharacteristic.writeValue(value);

        // 이전 상태를 현재 상태로 업데이트
        strcpy(prevValue, value);
      }
      delay(100);
    }

    Serial.print("Disconnected from central: ");
    Serial.println(central.address());
  }
}
