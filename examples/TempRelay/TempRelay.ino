// 온도로 릴레이 제어 (i2r-03 전용)
// 온도가 30도 이상이 되면 OUT1(예: 환풍기)을 켜고, 28도 이하로 내려가면 끈다.
// 켜는 온도와 끄는 온도를 2도 떨어뜨려 두어(히스테리시스) 기준값 근처에서
// 릴레이가 계속 딸깍거리지 않게 한다.
// 온습도 센서(AHT21)는 라이브러리가 2초마다 읽으므로 따로 설치할 라이브러리가 없다.

#include <I2R_PLC.h>

I2R_PLC plc(I2R_03);  // 온습도 센서는 i2r-03에만 있습니다

const float ON_TEMP  = 30.0;  // 이 온도 이상이면 켠다
const float OFF_TEMP = 28.0;  // 이 온도 이하면 끈다

unsigned long lastPrint = 0;

void setup() {
  Serial.begin(115200);
  plc.begin();
  if (!plc.sensorOk()) {
    Serial.println("AHT21 센서를 찾지 못했습니다. 2초마다 다시 찾습니다.");
  }
  Serial.println("온도 제어 시작: 30도 이상 켜짐, 28도 이하 꺼짐");
}

void loop() {
  plc.update();
  if (!plc.sensorOk()) {
    plc.output(1, OFF);  // 센서가 없으면 안전하게 꺼 둔다
    return;
  }

  float t = plc.temperature();
  if (!plc.outputState(1) && t >= ON_TEMP) {
    plc.output(1, ON);
    Serial.println("30도 이상 → OUT1 켜짐");
  } else if (plc.outputState(1) && t <= OFF_TEMP) {
    plc.output(1, OFF);
    Serial.println("28도 이하 → OUT1 꺼짐");
  }

  if (millis() - lastPrint >= 2000) {  // 2초마다 보여 준다
    lastPrint = millis();
    Serial.print("온도 ");
    Serial.print(t, 1);
    Serial.print("°C  습도 ");
    Serial.print(plc.humidity(), 1);
    Serial.print("%  OUT1 ");
    Serial.println(plc.outputState(1) ? "켜짐" : "꺼짐");
  }
}
