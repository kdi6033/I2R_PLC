// 입력을 출력으로
// IN1~INn에 입력이 들어오면 같은 번호의 OUT을 켜고, 입력이 꺼지면 끈다.
// 입력이 바뀌는 순간에는 시리얼에 상태를 보여 준다.

#include <I2R_PLC.h>

I2R_PLC plc(I2R_03);  // 내 보드에 맞게 I2R_02, I2R_03, I2R_04 중 하나로 바꾸세요

void setup() {
  Serial.begin(115200);
  plc.begin();
  Serial.println("입력 → 출력 시작");
}

void loop() {
  plc.update();  // loop 맨 앞에서 입력을 읽는다

  for (int n = 1; n <= plc.inputCount(); n++) {
    plc.output(n, plc.input(n));  // 입력과 같은 번호의 출력

    if (plc.rising(n)) {
      Serial.print("IN");
      Serial.print(n);
      Serial.println(" 들어옴");
    }
    if (plc.falling(n)) {
      Serial.print("IN");
      Serial.print(n);
      Serial.println(" 꺼짐");
    }
  }
}
