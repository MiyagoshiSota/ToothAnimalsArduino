#define SENSER_MAX 1

String line;   // 受信文字列
int line_len;  // 受信文字列の長さ
long num;      // 受信整数
int i[SENSER_MAX], j[SENSER_MAX];
bool tooth_array[SENSER_MAX];
bool nonDecay;
const int MAXVOLT = 200;
int sensorPins[SENSER_MAX] = { A0 };  // センサーピンの配列

void setup() {
  Serial.begin(9600);
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(A0, INPUT);

  nonDecay = true;

  for (int k = 0; k < SENSER_MAX; k++) {
    i[k] = 0;
    j[k] = MAXVOLT;
    tooth_array[k] = true;
  }
}

void loop() {
  //虫歯に変更
  if (Serial.available() > 0) {
    // シリアル通信で1行（改行コードまで）読み込む
    line = Serial.readStringUntil('\n');
    // 文字列の長さを取得する
    line_len = line.length();
    // 文字列の長さが1文字以上の場合
    if (line_len > 0) {
      // 文字列を整数に変換する
      num = line.toInt();

      // numの範囲チェック
      if (num >= 0 && num < SENSER_MAX) {
        tooth_array[num] = true;

        //0個目は0,1 1個目は2,3
        i[num] = 0;
        j[num] = MAXVOLT;
      }
      //初期化処理
      else {
        for (int k = 0; k < SENSER_MAX; k++) {
          i[k] = MAXVOLT;
          j[k] = 0;
          tooth_array[k] = false;
        }
      }
    }
  }

  nonDecay = true;  // ループの前に初期化

  //以下で圧力センサーの値から消えるまで計算
  for (int k = 0; k < SENSER_MAX; k++) {
    if (tooth_array[k] == true) {
      nonDecay = false;

      num = analogRead(sensorPins[k]);  // 正しいアナログピンを読む
      if (num != 0) {
        i[k] += (num / 25);
        j[k] -= (num / 25);
      }

      if (i[k] >= MAXVOLT) {
        i[k] = MAXVOLT;
      }

      if (j[k] <= 0) {
        j[k] = 0;
      }

      if (j[k] == 0 && i[k] == MAXVOLT) {
        tooth_array[k] = false;
        nonDecay = true;

        Serial.print(k);
        Serial.print(",");
        Serial.println(MAXVOLT);
      } else {
        Serial.print(k);
        Serial.print(",");
        Serial.println(i[k]);
      }
    }
  }

  if (nonDecay) {
    Serial.print(-1);
    Serial.print(",");
    Serial.println(0);
  }

  analogWrite(2, j[0]);
  analogWrite(3, i[0]);
  
  delay(25);
}