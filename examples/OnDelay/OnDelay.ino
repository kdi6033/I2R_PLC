// 켜짐 지연 타이머 (TON)
// IN1이 3초 동안 계속 들어와 있어야 OUT1을 켠다. IN1이 꺼지면 바로 끈다.
// 잠깐 스치는 입력(오동작)은 무시하고 싶을 때 쓴다.

#include <I2R_PLC.h>

I2R_PLC plc(I2R_03);  // 내 보드에 맞게 I2R_02, I2R_03, I2R_04 중 하나로 바꾸세요

const unsigned long ON_DELAY_MS = 3000;  // 이 시간 동안 계속 들어와 있어야 켠다 (3초)

void setup() {
  Serial.begin(115200);
  plc.begin();
  Serial.println("켜짐 지연 시작: IN1을 3초 넣고 있으면 OUT1이 켜집니다");
}

void loop() {
  plc.update();

  bool on = plc.onDelay(1, ON_DELAY_MS);
  if (on != plc.outputState(1)) {
    plc.output(1, on);
    Serial.println(on ? "3초 지남 → OUT1 켜짐" : "IN1 꺼짐 → OUT1 꺼짐");
  }
}
