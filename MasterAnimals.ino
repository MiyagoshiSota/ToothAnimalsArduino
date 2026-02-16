// 設定値
const int SENSOR_COUNT = 1;     // センサーの数
const int MAX_VALUE    = 200;   // 最大値
const int HEAL_DIVISOR = 25;    // 回復速度の除数
const int LOOP_DELAY   = 25;    // ループ遅延(ms)

// 状態管理用の構造体
struct Tooth {
  int  pinSensor;     // センサーの入力ピン
  int  pinPwmDecay;   // 虫歯(進行)表示用LEDピン
  int  pinPwmHeal;    // 治療(完了)表示用LEDピン
  
  int  progress;      // 進行度
  bool isDecayed;     // 虫歯かどう

  Tooth(int sensor, int pwmDecay, int pwmHeal) 
    : pinSensor(sensor), pinPwmDecay(pwmDecay), pinPwmHeal(pwmHeal), 
      progress(MAX_VALUE), isDecayed(false) {}
};

// センサーオブジェクトの配列作成
Tooth teeth[SENSOR_COUNT] = {
  Tooth(A0, 2, 3) 
};

void setup() {
  Serial.begin(9600);

  // ピンモードの設定
  for (int k = 0; k < SENSOR_COUNT; k++) {
    pinMode(teeth[k].pinPwmDecay, OUTPUT);
    pinMode(teeth[k].pinPwmHeal, OUTPUT);
    pinMode(teeth[k].pinSensor, INPUT);
    
    // 初期状態の出力（健康状態）
    updateOutput(k); 
  }
}

void loop() {
  // シリアル通信の処理（虫歯発生コマンド等の受信）
  processSerialInput();

  bool allHealthy = true;

  // 各センサーの状態更新と出力
  for (int k = 0; k < SENSOR_COUNT; k++) {
    if (teeth[k].isDecayed) {
      allHealthy = false;
      updateToothState(k); // センサー値を読んで進行度を更新
    }
    // LED等の出力反映
    updateOutput(k);
  }

  // 全て健康な場合のシリアル出力
  if (allHealthy) {
    Serial.print(-1);
    Serial.print(",");
    Serial.println(0);
  }

  delay(LOOP_DELAY);
}

// ---------------------------------------------------------
// ヘルパー関数群
// ---------------------------------------------------------

// シリアル入力の処理
void processSerialInput() {
  if (Serial.available() <= 0) return;

  String line = Serial.readStringUntil('\n');
  line.trim(); // 改行コード除去

  if (line.length() == 0) return;

  long targetId = line.toInt();

  // 受信したIDが有効範囲なら「虫歯」にする
  if (targetId >= 0 && targetId < SENSOR_COUNT) {
    teeth[targetId].isDecayed = true;
    teeth[targetId].progress  = 0; // 0からスタート
  } 
  // 範囲外の数値が来たら「全リセット（治療完了）」とみなす
  else {
    for (int k = 0; k < SENSOR_COUNT; k++) {
      teeth[k].isDecayed = false;
      teeth[k].progress  = MAX_VALUE;
    }
  }
}

// センサー値を読み取って進行度を更新する
void updateToothState(int index) {
  Tooth &t = teeth[index]; // 参照で取得

  int sensorVal = analogRead(t.pinSensor);

  // センサー反応があれば回復
  if (sensorVal > 0) {
    t.progress += (sensorVal / HEAL_DIVISOR);
  }

  // 最大値キャップ
  if (t.progress >= MAX_VALUE) {
    t.progress = MAX_VALUE;
  }

  // 完了判定
  if (t.progress >= MAX_VALUE) {
    t.isDecayed = false;
    Serial.print(index);
    Serial.print(",");
    Serial.println(MAX_VALUE);
  } else {
    // 途中経過出力
    Serial.print(index);
    Serial.print(",");
    Serial.println(t.progress);
  }
}

// LEDへの出力（PWM）
void updateOutput(int index) {
  if (index == 0) {
    int valI = teeth[index].progress;
    int valJ = MAX_VALUE - valI; // j は i の逆数として計算
    
    // 負にならないようクランプ
    if (valJ < 0) valJ = 0;

    analogWrite(teeth[index].pinPwmDecay, valJ);
    analogWrite(teeth[index].pinPwmHeal,  valI);
  }
}
