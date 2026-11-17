// 출력 함수 확인
// 1) 보드 이름(type)과 출력 개수를 시리얼에 보여 준다.
// 2) OUT1을 2초 켰다가 끈다.
// 3) OUT2를 "켜고 3초 뒤 자동으로 끄기"로 켠다. 꺼지는 것은 update()가 맡는다.
// 4) 없는 포트 번호(9)를 넣어 본다. 아무 출력도 켜지지 않고 시리얼에 알림이 나온다.

#include <I2R_PLC.h>

I2R_PLC plc(I2R_03);  // 내 보드에 맞게 I2R_02, I2R_03, I2R_04 중 하나로 바꾸세요

bool wasOn = false;   // OUT2가 켜져 있었는지 (자동으로 꺼지는 순간을 알려고)

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

  plc.output(2, ON, 3);  // 켜고 3초 뒤 자동으로 끄기
  wasOn = true;
  Serial.println("OUT2 켜짐 (3초 뒤 자동으로 꺼짐)");

  if (!plc.output(9, ON)) {  // 없는 번호: false를 돌려준다
    Serial.println("OUT9는 켜지 못했습니다");
  }
}

void loop() {
  plc.update();  // 자동 끄기 시간을 여기서 확인한다

  if (wasOn && !plc.outputState(2)) {
    wasOn = false;
    Serial.println("3초 지남 → OUT2 자동으로 꺼짐");
  }
}
