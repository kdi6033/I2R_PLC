// 깜빡이기: IN1이 들어오면 OUT1을 0.5초 켜고 0.5초 끄기를 되풀이, IN2가 들어오면 멈춤
//
// plc.blink(n, 켜짐ms, 꺼짐ms)  출력 n을 계속 깜빡인다 (켜진 상태에서 시작)
// 멈추기: plc.output(n, OFF), 또는 다른 output(), toggle(), allOff()
//
// 예) 경광등, 경고 부저. delay() 없이 update()가 알아서 켜고 끈다.
//     그래서 깜빡이는 동안에도 다른 입력을 바로 처리할 수 있다.
//
// 배선: i2r-02, i2r-04는 입력 단자와 COM을 스위치로 연결만 한다 (전압을 넣으면 고장 난다).
//       i2r-03은 COM을 쓰는 전압 입력이다. 보드 문서의 입력 배선을 따른다.

#include <I2R_PLC.h>

I2R_PLC plc(I2R_03);  // 내 보드에 맞게 I2R_02, I2R_03, I2R_04 중 하나로 바꾸세요

void setup() {
  Serial.begin(115200);
  plc.begin();
  Serial.println("IN1 = 깜빡이기 시작, IN2 = 멈춤");
}

void loop() {
  plc.update();  // 깜빡임은 여기서 처리된다 (loop 맨 앞에서 매번, delay() 없이)

  if (plc.rising(1)) {
    plc.blink(1, 500, 500);  // 0.5초 켜고 0.5초 끄기
    Serial.println("OUT1 깜빡이기 시작");
  }
  if (plc.rising(2)) {
    plc.output(1, OFF);      // 깜빡임 멈추고 끄기
    Serial.println("OUT1 멈춤");
  }
}
