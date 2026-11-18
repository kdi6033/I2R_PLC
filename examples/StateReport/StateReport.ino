// 상태 알리기: 원격 제어(MQTT)를 붙이기 전에 시리얼로 미리 해 보는 예제
//
// plc.onOutputChange(함수)  출력이 실제로 바뀔 때마다 함수를 불러 준다.
//                           output()으로 바꾼 것, 자동 끄기, 늦게 켜기, blink 모두 알려 준다.
// plc.inputBits()           모든 입력 상태를 숫자 하나로 (bit0 = IN1). 바뀌었는지 비교할 때 쓴다.
// plc.outputBits(값)        모든 출력을 한꺼번에 바꾼다 (원격의 "전체 출력" 명령)
// plc.inputText()           "0101" (맨 왼쪽이 IN1). 보내거나 화면에 보여 줄 때 쓴다.
// plc.outputText()          "0101" (맨 왼쪽이 OUT1)
// plc.remain(n)             예약된 출력이 바뀌기까지 남은 초
//
// [해 보기]
// 1) IN1을 누르면 OUT1이 5초 켜졌다가 저절로 꺼진다. 켜질 때와 꺼질 때 모두 알림이 온다.
// 2) 켜져 있는 동안 1초마다 남은 시간이 보인다.
// 3) 입력이 바뀔 때만 입력 상태를 보여 준다 (계속 찍지 않는다).
// 4) 시리얼 모니터("새 줄")에 0과 1로 출력 상태를 입력하면 한꺼번에 바뀐다.
//    예) 1010 → OUT1, OUT3 켜기 / 0000 → 모두 끄기 (맨 왼쪽이 OUT1)
//
// 배선: i2r-02, i2r-04는 입력 단자와 COM을 스위치로 연결만 한다 (전압을 넣으면 고장 난다).
//       i2r-03은 COM을 쓰는 전압 입력이다. 보드 문서의 입력 배선을 따른다.

#include <I2R_PLC.h>

I2R_PLC plc(I2R_03);  // 내 보드에 맞게 I2R_02, I2R_03, I2R_04 중 하나로 바꾸세요

uint16_t lastIn = 0;           // 지난번 입력 상태 (바뀌었는지 비교용)
unsigned long lastRemain = 0;
String line;                   // 시리얼로 받은 글자를 모으는 곳

// 출력이 바뀔 때마다 라이브러리가 불러 주는 함수. 나중에는 여기서 MQTT로 보낸다.
void outputChanged(uint8_t n, bool on) {
  Serial.print("[알림] OUT");
  Serial.print(n);
  Serial.print(on ? " 켜짐" : " 꺼짐");
  Serial.print("  → 출력 상태 ");
  Serial.println(plc.outputText());
}

void setup() {
  Serial.begin(115200);
  plc.begin();
  plc.onOutputChange(outputChanged);  // 알림 받을 함수 정하기

  lastIn = plc.inputBits();
  Serial.print("입력 ");
  Serial.print(plc.inputText());
  Serial.print(", 출력 ");
  Serial.println(plc.outputText());
  Serial.println("IN1 = OUT1 5초 켜기, 시리얼에 1010처럼 입력 = 출력 한꺼번에 바꾸기");
}

void loop() {
  plc.update();

  // 입력이 바뀌었을 때만 알린다 (숫자 하나로 비교)
  uint16_t in = plc.inputBits();
  if (in != lastIn) {
    lastIn = in;
    Serial.print("[입력] ");
    Serial.println(plc.inputText());
  }

  if (plc.rising(1)) plc.output(1, ON, 5);  // 5초 켜고 자동 끄기 (꺼질 때도 알림이 온다)

  // 예약이 있으면 1초마다 남은 시간 보여 주기
  if (plc.remain(1) > 0 && millis() - lastRemain >= 1000) {
    lastRemain = millis();
    Serial.print("OUT1 꺼지기까지 ");
    Serial.print(plc.remain(1));
    Serial.println("초");
  }

  // 시리얼로 "1010" 같은 출력 상태를 받으면 한꺼번에 바꾼다
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      if (line.length() > 0) {
        uint16_t bits = 0;
        for (unsigned int i = 0; i < line.length() && i < plc.outputCount(); i++) {
          if (line[i] == '1') bits |= (1U << i);  // 맨 왼쪽 글자가 OUT1 (bit0)
        }
        plc.outputBits(bits);  // 바뀐 출력마다 알림이 온다
      }
      line = "";
    } else if (c == '0' || c == '1') {
      line += c;
    }
  }
}
