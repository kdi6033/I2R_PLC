// 보드 점검
// 1초마다 입력 상태, 출력 상태, 온습도(i2r-03)를 시리얼에 보여 준다.
// 보드의 공장 초기화 버튼을 누르고 있으면 LED가 켜지고 모든 출력을 끈다.
// 배선이 맞는지 확인할 때 쓴다.

#include <I2R_PLC.h>

I2R_PLC plc(I2R_03);  // 내 보드에 맞게 I2R_02, I2R_03, I2R_04 중 하나로 바꾸세요

unsigned long lastPrint = 0;

void setup() {
  Serial.begin(115200);
  plc.begin();
  Serial.print("입력 ");
  Serial.print(plc.inputCount());
  Serial.print("개, 출력 ");
  Serial.print(plc.outputCount());
  Serial.println("개");
}

void loop() {
  plc.update();

  bool pressed = plc.button();
  plc.led(pressed);
  if (pressed) plc.allOff();

  if (millis() - lastPrint < 1000) return;
  lastPrint = millis();

  Serial.print("IN ");
  for (int n = 1; n <= plc.inputCount(); n++) Serial.print(plc.input(n) ? "1" : "0");
  Serial.print("  OUT ");
  for (int n = 1; n <= plc.outputCount(); n++) Serial.print(plc.outputState(n) ? "1" : "0");
  Serial.print("  버튼 ");
  Serial.print(pressed ? "눌림" : "-");
  if (plc.board() == I2R_03) {
    Serial.print("  온도 ");
    Serial.print(plc.temperature(), 1);
    Serial.print("°C 습도 ");
    Serial.print(plc.humidity(), 1);
    Serial.print("%");
  }
  Serial.println();
}
