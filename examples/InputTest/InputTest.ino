// 입력 함수 확인
// 출력(릴레이)은 건드리지 않고 입력만 본다. 릴레이에 장치가 연결돼 있어도 안전하다.
//
// 1) 보드 이름과 입력 개수를 시리얼에 보여 준다.
// 2) 입력이 들어오는 순간(rising)과 꺼지는 순간(falling)을 알려 준다.
// 3) 입력마다 들어온 횟수를 센다. 스위치를 한 번 눌렀는데 2 이상 늘면
//    떨림(채터링)이 걸러지지 않은 것이다. 그때는 setDebounce()로 시간을 늘려 본다.
// 4) 2초마다 모든 입력 상태를 한 줄로 보여 준다. (1 = 들어옴, 0 = 꺼짐)
//
// 배선: i2r-02, i2r-04는 입력 단자와 COM을 스위치로 연결만 한다 (전압을 넣으면 고장 난다).
//       i2r-03은 COM을 쓰는 전압 입력이다. 보드 문서의 입력 배선을 따른다.

#include <I2R_PLC.h>

I2R_PLC plc(I2R_03);  // 내 보드에 맞게 I2R_02, I2R_03, I2R_04 중 하나로 바꾸세요

int count[8] = {0};             // 입력마다 들어온 횟수 (i2r-04는 8개)
unsigned long lastPrint = 0;

void setup() {
  Serial.begin(115200);
  plc.begin();
  // plc.setDebounce(50);       // 떨림이 남으면 30ms(기본)에서 50ms로 늘려 본다

  Serial.print("보드: ");
  Serial.print(plc.type());
  Serial.print(", 입력 ");
  Serial.print(plc.inputCount());
  Serial.println("개");
  Serial.println("입력에 스위치를 연결하고 눌러 보세요.");
}

void loop() {
  plc.update();  // 입력을 읽고 떨림을 거른다 (loop 맨 앞에서 매번)

  for (int n = 1; n <= plc.inputCount(); n++) {
    if (plc.rising(n)) {            // 막 들어온 순간에 한 번만
      count[n - 1]++;
      Serial.print("IN");
      Serial.print(n);
      Serial.print(" 들어옴 (");
      Serial.print(count[n - 1]);
      Serial.println("번째)");
    }
    if (plc.falling(n)) {           // 막 꺼진 순간에 한 번만
      Serial.print("IN");
      Serial.print(n);
      Serial.println(" 꺼짐");
    }
  }

  if (millis() - lastPrint >= 2000) {  // 2초마다 전체 상태
    lastPrint = millis();
    Serial.print("입력 상태 IN1~IN");
    Serial.print(plc.inputCount());
    Serial.print(": ");
    for (int n = 1; n <= plc.inputCount(); n++) {
      Serial.print(plc.input(n) ? "1" : "0");
    }
    Serial.println();
  }
}
