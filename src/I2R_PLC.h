// I2R_PLC: i2r-02, i2r-03, i2r-04 IoT PLC 공통 라이브러리
// 보드마다 다른 핀 번호를 감추고, 입력 1~N / 출력 1~N 번호로만 쓰게 한다.
//
//   #include <I2R_PLC.h>
//   I2R_PLC plc(I2R_03);
//   void setup() { plc.begin(); }
//   void loop()  { plc.update(); plc.output(1, plc.input(1)); }

#ifndef I2R_PLC_H
#define I2R_PLC_H

#include <Arduino.h>

#ifndef ON
#define ON true
#endif
#ifndef OFF
#define OFF false
#endif

enum I2R_Board {
  I2R_02,  // 입력 4, 출력 4 (무전압 접점 입력)
  I2R_03,  // 입력 4, 출력 4, AHT21 온습도 센서 (전압 입력)
  I2R_04   // 입력 8, 출력 8 (무전압 접점 입력)
};

class I2R_PLC {
public:
  static const uint8_t MAX_CH = 8;

  explicit I2R_PLC(I2R_Board board);

  // ---- 기본 ----
  void begin();   // setup()에서 한 번. 핀 설정, 출력 모두 끄기, 센서 찾기
  void update();  // loop() 맨 앞에서 매번. 입력 읽기, 떨림 제거, 센서 읽기

  I2R_Board board() const { return _board; }
  const char* type() const;  // 옛 펌웨어와 같은 보드 이름 "i2r-02", "i2r-03", "i2r-04"
  uint8_t inputCount() const { return _numIn; }
  uint8_t outputCount() const { return _numOut; }

  // ---- 출력 (n = 1부터, 보드 단자의 OUT1이 1) ----
  // 출력 n을 켜거나 끈다. seconds를 주면 켠 뒤 그 시간(초)이 지나 자동으로 끈다 (update() 필요).
  // 없는 포트 번호면 아무 핀도 건드리지 않고 시리얼에 알린 뒤 false를 돌려준다.
  bool output(uint8_t n, bool on, uint32_t seconds = 0);
  void toggle(uint8_t n);           // 출력 n을 반대로
  void allOff();                    // 모든 출력 끄기
  bool outputState(uint8_t n) const;  // 출력 n이 켜져 있는지

  // ---- 입력 (n = 1부터) ----
  bool input(uint8_t n) const;    // 입력 n이 들어와 있는지 (떨림 제거됨)
  bool rising(uint8_t n) const;   // 입력 n이 막 들어온 순간 (update 한 번 동안만 true)
  bool falling(uint8_t n) const;  // 입력 n이 막 꺼진 순간 (update 한 번 동안만 true)

  // ---- 타이머 (PLC식) ----
  bool onDelay(uint8_t n, unsigned long ms) const;   // TON: 입력 n이 ms 동안 계속 들어와 있으면 true
  bool offDelay(uint8_t n, unsigned long ms) const;  // TOF: 입력 n이 들어와 있는 동안, 그리고 꺼진 뒤 ms 동안 true

  // ---- 보드 ----
  void led(bool on);  // 보드 내장 LED
  bool button() const;  // 공장 초기화 버튼이 눌려 있는지

  // ---- 온습도 (i2r-03만, AHT21) ----
  bool sensorOk() const { return _sensorOk; }
  float temperature() const { return _temp; }  // ℃, 센서가 없으면 NAN
  float humidity() const { return _hum; }      // %RH, 센서가 없으면 NAN

  // ---- 설정 ----
  void setDebounce(unsigned long ms) { _debounceMs = ms; }  // 기본 30ms

private:
  bool validIn(uint8_t n) const { return n >= 1 && n <= _numIn; }
  bool validOut(uint8_t n) const { return n >= 1 && n <= _numOut; }

  void sensorStart();
  bool sensorTrigger();
  bool sensorRead();
  void sensorUpdate();

  I2R_Board _board;
  uint8_t _numIn, _numOut;
  const uint8_t* _inPins;
  const uint8_t* _outPins;
  uint8_t _ledPin, _buttonPin;

  unsigned long _debounceMs = 30;
  bool _inStable[MAX_CH];
  bool _inLastRead[MAX_CH];
  bool _inRise[MAX_CH];
  bool _inFall[MAX_CH];
  bool _inEverOn[MAX_CH];
  unsigned long _inReadChange[MAX_CH];    // 읽은 값이 바뀐 시각 (떨림 제거용)
  unsigned long _inStableChange[MAX_CH];  // 떨림을 걸러 낸 상태가 바뀐 시각 (타이머용)
  bool _out[MAX_CH];
  bool _offPending[MAX_CH];          // 자동 끄기가 예약돼 있는지
  unsigned long _offStart[MAX_CH];   // 켠 시각
  unsigned long _offAfterMs[MAX_CH]; // 켠 뒤 끌 때까지의 시간
  uint32_t _warned = 0;              // 없는 포트 번호를 이미 알렸는지 (번호마다 한 번만)

  bool _hasSensor = false;
  bool _sensorOk = false;
  bool _measuring = false;
  unsigned long _sensorTime = 0;
  float _temp = NAN;
  float _hum = NAN;
};

#endif
