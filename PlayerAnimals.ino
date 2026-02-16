#define SENSOR_MAX 4
#define MAX_VOLT 200

// ピン設定
// {DecayLED(汚れ), CleanLED(きれい)} のペア
const int ledPins[SENSOR_MAX][2] = {
  {2, 3}, // Sensor 0対応
  {4, 5}, // Sensor 1対応
  {6, 7}, // Sensor 2対応
  {8, 9}  // Sensor 3対応
};

const int sensorPins[SENSOR_MAX] = { A0, A1, A2, A3 };

// ステータス変数
// 名前をわかりやすく変更
int levelClean[SENSOR_MAX];
int levelDecay[SENSOR_MAX];
bool isDecayed[SENSOR_MAX];

// 一時変数
bool nonDecayState = true;

void setup() {
  Serial.begin(9600);

  // ピンモード設定をループ化
  for (int k = 0; k < SENSOR_MAX; k++) {
    pinMode(ledPins[k][0], OUTPUT);
    pinMode(ledPins[k][1], OUTPUT);
    pinMode(sensorPins[k], INPUT);
    
    // 初期化
    levelClean[k] = MAX_VOLT;
    levelDecay[k] = 0;
    isDecayed[k] = false;
  }
}

void loop() {
  if (Serial.available() > 0) {
    int targetTooth = Serial.parseInt(); 

    // 改行コードなどが残っている場合のためにバッファをクリア
    while(Serial.available()) { Serial.read(); }

    if (targetTooth >= 0 && targetTooth < SENSOR_MAX) {
      for (int k = 0; k < SENSOR_MAX; k++) {
        if (k == targetTooth) {
          // ターゲットを虫歯にする
          levelClean[k] = 0;
          levelDecay[k] = MAX_VOLT;
          isDecayed[k] = true;
        } else {
          // 他をリセット
          levelClean[k] = MAX_VOLT;
          levelDecay[k] = 0;
          isDecayed[k] = false;
        }
      }
    }
  }

  // センサー読み取りと状態更新
  nonDecayState = true; // とりあえず「虫歯なし」と仮定して、ループ内でチェック

  for (int k = 0; k < SENSOR_MAX; k++) {
    if (isDecayed[k]) {
      nonDecayState = false; // 虫歯が見つかった

      int sensorVal = analogRead(sensorPins[k]);
      int pressure = sensorVal / 35; // スケーリング

      // 圧力が一定以上なら治療進行
      if (sensorVal != 0 && pressure > 2) {
        levelClean[k] += pressure;
        levelDecay[k] -= pressure;
      }

      // 範囲制限
      if (levelClean[k] > MAX_VOLT) levelClean[k] = MAX_VOLT;
      if (levelDecay[k] < 0)       levelDecay[k] = 0;

      // 完治判定
      if (levelDecay[k] == 0 && levelClean[k] == MAX_VOLT) {
        isDecayed[k] = false;
        // 完治の瞬間だけここでリセット判定はできないので（他が虫歯かもしれない）、次のループから反映
        Serial.print(k);
        Serial.print(",");
        Serial.println(MAX_VOLT);
      } else {
        // 治療中
        Serial.print(k);
        Serial.print(",");
        Serial.println(levelClean[k]);
      }
    }
  }

  // 誰も虫歯でない場合
  if (nonDecayState) {
    Serial.print(-1);
    Serial.print(",");
    Serial.println(0);
  }

  // LED出力
  for (int k = 0; k < SENSOR_MAX; k++) {
    analogWrite(ledPins[k][0], levelDecay[k]); // 赤（汚れ）
    analogWrite(ledPins[k][1], levelClean[k]); // 青/白（きれい）
  }

  delay(25);
}
