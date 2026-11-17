// 누를 때마다 켜고 끄기 (토글)
// IN1에 연결한 누름 버튼을 한 번 누르면 OUT1이 켜지고, 다시 누르면 꺼진다.
// rising()은 입력이 들어오는 순간에만 true라서 버튼을 오래 눌러도 한 번만 바뀐다.

#include <I2R_PLC.h>

I2R_PLC plc(I2R_03);  // 내 보드에 맞게 I2R_02, I2R_03, I2R_04 중 하나로 바꾸세요

void setup() {
  Serial.begin(115200);
  plc.begin();
  Serial.println("토글 시작: IN1을 누를 때마다 OUT1이 켜지고 꺼집니다");
}

void loop() {
  plc.update();

  if (plc.rising(1)) {
    plc.toggle(1);
    Serial.println(plc.outputState(1) ? "OUT1 켜짐" : "OUT1 꺼짐");
  }
}
