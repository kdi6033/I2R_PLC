// 출력 차례로 켜기
// OUT1부터 마지막 출력까지 1초씩 차례로 켰다가 끈다.
// 처음 보드를 받았을 때 릴레이가 모두 동작하는지 확인하는 예제다.

#include <I2R_PLC.h>

I2R_PLC plc(I2R_03);  // 내 보드에 맞게 I2R_02, I2R_03, I2R_04 중 하나로 바꾸세요

void setup() {
  Serial.begin(115200);
  plc.begin();
  Serial.println("출력 차례로 켜기 시작");
}

void loop() {
  for (int n = 1; n <= plc.outputCount(); n++) {
    plc.output(n, ON);
    Serial.print("OUT");
    Serial.print(n);
    Serial.println(" 켜짐");
    delay(1000);
    plc.output(n, OFF);
  }
}
