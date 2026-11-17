#include "I2R_PLC.h"
#include <Wire.h>

// ---- 보드별 핀 ----
static const uint8_t OUT_02_03[] = {26, 27, 32, 33};
static const uint8_t IN_02_03[]  = {16, 17, 18, 19};
static const uint8_t OUT_04[] = {26, 27, 32, 33, 21, 22, 23, 25};
static const uint8_t IN_04[]  = {18, 19, 4, 5, 12, 13, 14, 15};

static const uint8_t LED_PIN = 2;
static const uint8_t SDA_PIN = 21;  // i2r-03 AHT21
static const uint8_t SCL_PIN = 22;

// ---- AHT21 ----
static const uint8_t AHT_ADDR = 0x38;
static const unsigned long AHT_MEASURE_MS = 80;    // 측정 명령 뒤 기다리는 시간
static const unsigned long AHT_INTERVAL_MS = 2000; // 2초마다 읽기

I2R_PLC::I2R_PLC(I2R_Board board) : _board(board) {
  if (board == I2R_04) {
    _numIn = 8; _numOut = 8;
    _inPins = IN_04; _outPins = OUT_04;
    _buttonPin = 36;
  } else {
    _numIn = 4; _numOut = 4;
    _inPins = IN_02_03; _outPins = OUT_02_03;
    _buttonPin = 34;
  }
  _ledPin = LED_PIN;
  _hasSensor = (board == I2R_03);
  for (uint8_t i = 0; i < MAX_CH; i++) {
    _inStable[i] = _inLastRead[i] = _inRise[i] = _inFall[i] = _inEverOn[i] = false;
    _inReadChange[i] = _inStableChange[i] = 0;
    _out[i] = false;
    _offPending[i] = false;
    _offStart[i] = _offAfterMs[i] = 0;
  }
}

const char* I2R_PLC::type() const {
  switch (_board) {
    case I2R_02: return "i2r-02";
    case I2R_03: return "i2r-03";
    case I2R_04: return "i2r-04";
  }
  return "unknown";
}

void I2R_PLC::begin() {
  for (uint8_t i = 0; i < _numOut; i++) {
    pinMode(_outPins[i], OUTPUT);
    digitalWrite(_outPins[i], LOW);  // 꺼진 상태에서 시작
    _out[i] = false;
    _offPending[i] = false;
  }
  for (uint8_t i = 0; i < _numIn; i++) {
    pinMode(_inPins[i], INPUT);
    bool now = digitalRead(_inPins[i]) == HIGH;  // 켤 때 이미 들어와 있는 입력은 rising으로 보지 않는다
    _inStable[i] = _inLastRead[i] = now;
    _inEverOn[i] = now;
    _inReadChange[i] = _inStableChange[i] = millis();
  }
  pinMode(_ledPin, OUTPUT);
  digitalWrite(_ledPin, LOW);
  pinMode(_buttonPin, INPUT);  // GPIO 34/36은 입력 전용, 보드에 풀업이 있음

  if (_hasSensor) {
    Wire.begin(SDA_PIN, SCL_PIN);
    sensorStart();
    if (_sensorOk && sensorTrigger()) {  // 첫 값은 begin()에서 바로 읽어 둔다
      delay(AHT_MEASURE_MS);
      sensorRead();
    }
    _sensorTime = millis();
  }
}

void I2R_PLC::update() {
  unsigned long now = millis();
  for (uint8_t i = 0; i < _numIn; i++) {
    _inRise[i] = _inFall[i] = false;
    bool v = digitalRead(_inPins[i]) == HIGH;
    if (v != _inLastRead[i]) {
      _inLastRead[i] = v;
      _inReadChange[i] = now;
    }
    if (now - _inReadChange[i] >= _debounceMs && v != _inStable[i]) {
      _inStable[i] = v;
      _inStableChange[i] = now;
      if (v) { _inRise[i] = true; _inEverOn[i] = true; }
      else   { _inFall[i] = true; }
    }
  }
  // 자동 끄기: 정한 시간이 지난 출력을 끈다
  for (uint8_t i = 0; i < _numOut; i++) {
    if (_offPending[i] && now - _offStart[i] >= _offAfterMs[i]) {
      output(i + 1, false);  // 여기서 _offPending도 지워진다
    }
  }

  if (_hasSensor) sensorUpdate();
}

// ---- 출력 ----
bool I2R_PLC::output(uint8_t n, bool on, uint32_t seconds) {
  if (!validOut(n)) {
    // 같은 번호는 한 번만 알린다 (loop에서 계속 불러도 시리얼이 넘치지 않게)
    uint32_t bit = (n < 32) ? (1UL << n) : 0;
    if (!bit || !(_warned & bit)) {
      _warned |= bit;
      Serial.printf("[I2R_PLC] OUT%u은(는) %s에 없습니다. 1~%u 중에서 고르세요.\n",
                    n, type(), _numOut);
    }
    return false;
  }
  uint8_t i = n - 1;
  _out[i] = on;
  digitalWrite(_outPins[i], on ? HIGH : LOW);

  // 켜면서 시간을 주면 자동 끄기 예약, 그 밖에는 예약 취소
  _offPending[i] = on && seconds > 0;
  if (_offPending[i]) {
    _offStart[i] = millis();
    _offAfterMs[i] = seconds * 1000UL;
  }
  return true;
}

void I2R_PLC::toggle(uint8_t n) {
  if (!validOut(n)) return;
  output(n, !_out[n - 1]);
}

void I2R_PLC::allOff() {
  for (uint8_t n = 1; n <= _numOut; n++) output(n, false);
}

bool I2R_PLC::outputState(uint8_t n) const {
  return validOut(n) ? _out[n - 1] : false;
}

// ---- 입력 ----
bool I2R_PLC::input(uint8_t n) const {
  return validIn(n) ? _inStable[n - 1] : false;
}

bool I2R_PLC::rising(uint8_t n) const {
  return validIn(n) ? _inRise[n - 1] : false;
}

bool I2R_PLC::falling(uint8_t n) const {
  return validIn(n) ? _inFall[n - 1] : false;
}

// ---- 타이머 ----
bool I2R_PLC::onDelay(uint8_t n, unsigned long ms) const {
  if (!validIn(n)) return false;
  uint8_t i = n - 1;
  return _inStable[i] && (millis() - _inStableChange[i] >= ms);
}

bool I2R_PLC::offDelay(uint8_t n, unsigned long ms) const {
  if (!validIn(n)) return false;
  uint8_t i = n - 1;
  if (_inStable[i]) return true;
  if (!_inEverOn[i]) return false;  // 한 번도 들어온 적이 없으면 false
  return millis() - _inStableChange[i] < ms;
}

// ---- 보드 ----
void I2R_PLC::led(bool on) {
  digitalWrite(_ledPin, on ? HIGH : LOW);
}

bool I2R_PLC::button() const {
  return digitalRead(_buttonPin) == LOW;  // 누르면 LOW
}

// ---- AHT21 (Wire로 직접 읽음, 추가 라이브러리 없음) ----
void I2R_PLC::sensorStart() {
  Wire.beginTransmission(AHT_ADDR);
  _sensorOk = (Wire.endTransmission() == 0);
  if (!_sensorOk) return;

  // 상태 바이트의 보정 비트(bit3)가 0이면 초기화 명령을 보낸다
  if (Wire.requestFrom(AHT_ADDR, (uint8_t)1) == 1) {
    uint8_t status = Wire.read();
    if (!(status & 0x08)) {
      Wire.beginTransmission(AHT_ADDR);
      Wire.write(0xBE);
      Wire.write(0x08);
      Wire.write(0x00);
      Wire.endTransmission();
      delay(10);
    }
  }
}

bool I2R_PLC::sensorTrigger() {
  Wire.beginTransmission(AHT_ADDR);
  Wire.write(0xAC);
  Wire.write(0x33);
  Wire.write(0x00);
  if (Wire.endTransmission() != 0) {
    _sensorOk = false;
    _temp = _hum = NAN;
    return false;
  }
  _measuring = true;
  return true;
}

bool I2R_PLC::sensorRead() {
  _measuring = false;
  if (Wire.requestFrom(AHT_ADDR, (uint8_t)6) != 6) {
    _sensorOk = false;
    _temp = _hum = NAN;
    return false;
  }
  uint8_t d[6];
  for (uint8_t i = 0; i < 6; i++) d[i] = Wire.read();
  if (d[0] & 0x80) return false;  // 아직 측정 중이면 이번 값은 버린다

  uint32_t rawH = ((uint32_t)d[1] << 12) | ((uint32_t)d[2] << 4) | (d[3] >> 4);
  uint32_t rawT = ((uint32_t)(d[3] & 0x0F) << 16) | ((uint32_t)d[4] << 8) | d[5];
  _hum  = rawH * 100.0f / 1048576.0f;
  _temp = rawT * 200.0f / 1048576.0f - 50.0f;
  return true;
}

void I2R_PLC::sensorUpdate() {
  unsigned long now = millis();
  if (_measuring) {
    if (now - _sensorTime >= AHT_MEASURE_MS) sensorRead();
    return;
  }
  if (now - _sensorTime < AHT_INTERVAL_MS) return;
  _sensorTime = now;
  if (!_sensorOk) {
    sensorStart();  // 센서가 없으면 2초마다 다시 찾아본다
    if (!_sensorOk) return;
  }
  sensorTrigger();
}
