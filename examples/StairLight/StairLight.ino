// 계단 조명 (꺼짐 지연 타이머, TOF)
// IN1이 들어와 있는 동안 OUT1을 켜 두고, IN1이 꺼진 뒤에도 5초 더 켜 둔다.
// 5초가 지나기 전에 IN1이 다시 들어오면 5초를 처음부터 다시 센다.

#include <I2R_PLC.h>

I2R_PLC plc(I2R_03);  // 내 보드에 맞게 I2R_02, I2R_03, I2R_04 중 하나로 바꾸세요

const unsigned long OFF_DELAY_MS = 5000;  // 꺼진 뒤 더 켜 두는 시간 (5초)

void setup() {
  Serial.begin(115200);
  plc.begin();
  Serial.println("계단 조명 시작: IN1이 꺼진 뒤 5초 더 OUT1을 켜 둡니다");
}

void loop() {
  plc.update();

  bool on = plc.offDelay(1, OFF_DELAY_MS);
  if (on != plc.outputState(1)) {  // 바뀔 때만 출력하고 알려 준다
    plc.output(1, on);
    Serial.println(on ? "OUT1 켜짐" : "5초 지남 → OUT1 꺼짐");
  }
}
