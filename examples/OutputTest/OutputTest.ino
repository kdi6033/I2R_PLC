// 출력 함수 확인
// 1) 보드 이름(type)과 출력 개수를 시리얼에 보여 준다.
// 2) OUT1을 2초 켰다가 끈다.
// 3) OUT2를 "켜고 3초 뒤 자동으로 끄기"로 켠다.       plc.output(2, ON, 3);
// 4) OUT3을 "3초 뒤 켜고, 3초 동작한 뒤 끄기"로 예약한다. plc.output(3, ON, 3, 3);
//    예약된 켜기와 끄기는 loop()의 update()가 맡는다.
// 5) 없는 포트 번호(9)를 넣어 본다. 아무 출력도 켜지지 않고 시리얼에 알림이 나온다.

#include <I2R_PLC.h>

I2R_PLC plc(I2R_03);  // 내 보드에 맞게 I2R_02, I2R_03, I2R_04 중 하나로 바꾸세요

bool last2 = false;   // 바로 전 OUT2 상태 (바뀌는 순간을 알려고)
bool last3 = false;   // 바로 전 OUT3 상태

void setup() {
  Serial.begin(115200);
  plc.begin();  // 모든 출력을 끈 상태에서 시작

  Serial.print("보드: ");
  Serial.print(plc.type());
  Serial.print(", 출력 ");
  Serial.print(plc.outputCount());
  Serial.println("개");

  plc.output(1, ON);
  Serial.println("OUT1 켜짐");
  delay(2000);  // 시험용이라 잠깐 기다린다 (실제 프로그램에서는 millis()를 쓴다)
  plc.output(1, OFF);
  Serial.println("OUT1 꺼짐");

  plc.output(2, ON, 3);      // 켜고 3초 뒤 자동으로 끄기
  last2 = true;
  Serial.println("OUT2 켜짐 (3초 뒤 자동으로 꺼짐)");

  plc.output(3, ON, 3, 3);   // 3초 뒤 켜고, 3초 동작한 뒤 끄기
  Serial.println("OUT3 예약 (3초 뒤 켜고 3초 동작)");

  if (!plc.output(9, ON)) {  // 없는 번호: false를 돌려준다
    Serial.println("OUT9는 켜지 못했습니다");
  }
}

void loop() {
  plc.update();  // 예약된 켜기, 끄기를 여기서 처리한다

  bool now2 = plc.outputState(2);
  if (now2 != last2) {
    last2 = now2;
    Serial.println(now2 ? "OUT2 켜짐" : "OUT2 자동으로 꺼짐");
  }

  bool now3 = plc.outputState(3);
  if (now3 != last3) {
    last3 = now3;
    Serial.println(now3 ? "3초 지남 → OUT3 켜짐" : "3초 동작 → OUT3 꺼짐");
  }
}
