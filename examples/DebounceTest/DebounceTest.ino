// 떨림 제거 시간(setDebounce) 확인
// 출력(릴레이)은 건드리지 않고 입력만 본다. 릴레이에 장치가 연결돼 있어도 안전하다.
//
// [떨림(채터링)이란]
// 스위치나 릴레이 접점은 눌리는 순간 금속이 몇 ms 동안 튀면서
// 붙었다 떨어졌다를 반복한다. 그대로 읽으면 한 번 눌렀는데 여러 번 누른 것이 된다.
//
// [라이브러리가 거르는 방법]
// plc.update()가 입력 값을 읽어, 값이 바뀐 뒤 정한 시간(ms) 동안 그대로 있으면
// 그때 진짜로 바뀐 것으로 인정한다. 이 시간을 plc.setDebounce(ms)로 정한다. 기본은 30ms.
// - 모든 입력에 똑같이 적용되고, 부르는 즉시 바뀐다.
// - input(), rising(), falling(), onDelay(), offDelay()가 모두 이 값을 따른다.
// - 크게 하면 떨림은 잘 걸러지지만 반응이 그만큼 늦고, 그보다 짧은 신호는 무시된다.
// - 0이면 거르지 않는다.
//
// [시험 방법]
// 1) 업로드하고 시리얼 모니터를 115200, "새 줄(Newline)"로 연다.
// 2) IN1을 스위치로 한 번씩 눌러 본다. 한 번에 1번째, 2번째...로 1씩 늘면 정상이다.
// 3) 시리얼 모니터에 0을 입력하고 Enter → 거르기 끔. 다시 눌러 본다.
//    기계식 스위치면 한 번 눌렀는데 횟수가 2 이상 뛰는 때가 보인다. (떨림)
// 4) 30, 50, 100 등을 입력해 바꿔 가며 비교한다. 다시 업로드할 필요가 없다.
// 5) 마음에 드는 값을 찾았으면 내 스케치의 setup()에 plc.setDebounce(그 값); 을 넣는다.
//
// 배선: i2r-02, i2r-04는 입력 단자와 COM을 스위치로 연결만 한다 (전압을 넣으면 고장 난다).
//       i2r-03은 COM을 쓰는 전압 입력이다. 보드 문서의 입력 배선을 따른다.

#include <I2R_PLC.h>

I2R_PLC plc(I2R_03);  // 내 보드에 맞게 I2R_02, I2R_03, I2R_04 중 하나로 바꾸세요

unsigned long debounceMs = 30;  // 지금 떨림 제거 시간 (라이브러리 기본값과 같게 시작)
int count[8] = {0};             // 입력마다 들어온 횟수 (i2r-04는 8개)
unsigned long pressTime[8] = {0};  // 들어온 시각 (얼마나 눌렀는지 보려고)
String line;                    // 시리얼로 받은 글자를 모으는 곳

void setup() {
  Serial.begin(115200);
  plc.begin();
  plc.setDebounce(debounceMs);  // 떨림 제거 시간 정하기 (begin() 다음에)

  Serial.print("보드: ");
  Serial.println(plc.type());
  Serial.print("떨림 제거 시간: ");
  Serial.print(debounceMs);
  Serial.println("ms");
  Serial.println("입력을 눌러 보세요. 숫자(ms)를 입력하고 Enter를 치면 시간이 바뀝니다. (0 = 거르지 않음)");
}

void loop() {
  plc.update();  // 입력을 읽고 떨림을 거른다 (loop 맨 앞에서 매번, delay() 없이)

  // 시리얼로 숫자를 받으면 떨림 제거 시간을 바꾼다 (기다리지 않고 한 글자씩 받는다)
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      line.trim();
      if (line.length() > 0) {
        debounceMs = line.toInt();
        plc.setDebounce(debounceMs);  // 부르는 즉시 바뀐다
        for (int i = 0; i < 8; i++) count[i] = 0;  // 비교하기 쉽게 횟수를 0부터 다시 센다
        Serial.print("떨림 제거 시간을 ");
        Serial.print(debounceMs);
        Serial.println("ms로 바꿨습니다. 횟수를 0부터 다시 셉니다.");
      }
      line = "";
    } else if (isDigit(c)) {
      line += c;
    }
  }

  for (int n = 1; n <= plc.inputCount(); n++) {
    if (plc.rising(n)) {            // 걸러진 뒤 막 들어온 순간에 한 번만
      count[n - 1]++;
      pressTime[n - 1] = millis();
      Serial.print("[");
      Serial.print(debounceMs);
      Serial.print("ms] IN");
      Serial.print(n);
      Serial.print(" 들어옴 ");
      Serial.print(count[n - 1]);
      Serial.println("번째");
    }
    if (plc.falling(n)) {           // 걸러진 뒤 막 꺼진 순간에 한 번만
      Serial.print("[");
      Serial.print(debounceMs);
      Serial.print("ms] IN");
      Serial.print(n);
      Serial.print(" 꺼짐 (");
      Serial.print(millis() - pressTime[n - 1]);
      Serial.println("ms 동안 들어와 있었음)");
    }
  }
}
