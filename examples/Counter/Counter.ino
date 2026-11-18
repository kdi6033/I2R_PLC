// 카운터 (PLC식 CTU): IN1이 5번 들어오면 OUT1 켜기, IN2로 처음부터 다시
//
// plc.count(n)      입력 n이 들어온 횟수 (떨림을 걸러 낸 뒤 센다)
// plc.resetCount(n) 입력 n의 횟수를 0으로
//
// 예) 상자 5개가 센서(IN1)를 지나가면 OUT1(램프)을 켠다. IN2(리셋 버튼)를 누르면 처음부터.
//
// 배선: i2r-02, i2r-04는 입력 단자와 COM을 스위치로 연결만 한다 (전압을 넣으면 고장 난다).
//       i2r-03은 COM을 쓰는 전압 입력이다. 보드 문서의 입력 배선을 따른다.

#include <I2R_PLC.h>

I2R_PLC plc(I2R_03);  // 내 보드에 맞게 I2R_02, I2R_03, I2R_04 중 하나로 바꾸세요

const uint32_t TARGET = 5;  // 몇 번 들어오면 켤지

void setup() {
  Serial.begin(115200);
  plc.begin();
  Serial.println("IN1을 눌러 세어 보세요. 5번이면 OUT1이 켜집니다. IN2 = 처음부터");
}

void loop() {
  plc.update();  // 입력을 읽고 횟수를 센다 (loop 맨 앞에서 매번)

  if (plc.rising(1)) {
    Serial.print("IN1 횟수: ");
    Serial.println(plc.count(1));
  }

  if (plc.count(1) >= TARGET && !plc.outputState(1)) {
    plc.output(1, ON);
    Serial.println("목표 횟수 도달 → OUT1 켜짐");
  }

  if (plc.rising(2)) {  // 리셋
    plc.resetCount(1);
    plc.output(1, OFF);
    Serial.println("IN2: 횟수를 0으로, OUT1 꺼짐");
  }
}
