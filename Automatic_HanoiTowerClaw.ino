#include <Servo.h>

// ===== 硬體配置 =====
#define EN_PIN 8 // 啟用引腳
#define X_STEP_PIN 2 // X軸步進
#define X_DIR_PIN 5 // X軸方向
#define Y_STEP_PIN 3 // Y軸步進
#define Y_DIR_PIN 6 // Y軸方向
#define SERVO_PIN 12 // 夾爪舵機

// ===== 運動參數 =====
const int stepDelay = 1000; // 步進延遲（控制速度）
const int servoOpen = 0; // 夾爪全開角度
const long startHeight = 1000; // 初始高度
const long globalSafeHeight = 1200; // 全域安全高度
const long towerSpacing = 3000; // 塔間距（步數）
const long platePenetration = 0; // 放置時下壓深度

// 31次移動的夾取角度序列
const int grabAngles[31] = {
 80, 75, 80, 55, 80, 75, 80, 50, 80, 75, 80, 55, 80, 75, 80, 45,
 80, 75, 80, 55, 80, 75, 80, 50, 80, 75, 80, 55, 80, 75, 80
};

// 碟片高度參數（從頂層到下層）
const long diskHeights[] = {5200, 5700, 6700, 7300, 8500};

// ===== 全域變數 =====
Servo gripper;
long currentX = 0; // 目前X位置
long currentY = startHeight; // 目前Y位置
int diskCounts[3] = {5, 0, 0}; // 各塔碟數
bool startFlag = false; // 啟動標誌
int currentMoveIndex = 0; // 目前移動序號

// ===== 函數宣告 =====
void setGripperAngle(int angle, bool immediate = false);
void moveToAbsoluteHeight(long targetHeight);
void safeMoveHorizontal(long steps);
void safeMoveVertical(long steps, bool isUp);
//void emergencyStop();
void printTowerStatus();
void hanoiTower(int n, int from, int to, int aux);
void moveDiskSafely(int from, int to);

// ===== 初始化設定 =====
void setup() {
 // 引腳模式設定
 pinMode(EN_PIN, OUTPUT);
 digitalWrite(EN_PIN, LOW); // 啟動磁碟機
 pinMode(X_DIR_PIN, OUTPUT);
 pinMode(X_STEP_PIN, OUTPUT);
 pinMode(Y_DIR_PIN, OUTPUT);
 pinMode(Y_STEP_PIN, OUTPUT);

 // 夾爪初始化
 gripper.attach(SERVO_PIN);
 setGripperAngle(servoOpen, true); // 初始化完全打開
 delay(2000); // 確保舵機到位

 // 初始抬升到安全高度
 moveToAbsoluteHeight(globalSafeHeight);

 Serial.begin(9600);
 Serial.println("河內塔機械手臂已就緒");
 Serial.println("輸入 'Start' 開始執行");
 Serial.println("輸入 'Stop' 緊急停止");
 printTowerStatus();

 // 測試運動（可選）
 // testMovement();
}

// ===== 主循環 =====
void loop() {
 if (Serial.available()) {
 String input = Serial.readStringUntil('\n');
 input.trim();

 if (input.equalsIgnoreCase("Start")) {
 startFlag = true;
 currentMoveIndex = 0; // 重置移動計數器
 Serial.println("\n=== 開始執行河內塔 ===");
 hanoiTower(5, 0, 2, 1); // 5層從塔A(0)到塔C(2)
 Serial.println("=== 執行完成 ===");
 startFlag = false;
 }
 else if (input.equalsIgnoreCase("Status")) {
 printTowerStatus();
 }
 }
}

// ===== 運動測試函數 =====
void testMovement() {
 Serial.println("\n=== 開始運動測試 ===");

 Serial.println("測試右移1000步");
 safeMoveHorizontal(1000);
 delay(1000);

 Serial.println("測試左移1000步");
 safeMoveHorizontal(-1000);
 delay(1000);

 Serial.println("測試上升500步");
 safeMoveVertical(500, true);
 delay(1000);

 Serial.println("測試下降500步");
 safeMoveVertical(500, false);

 Serial.println("=== 運動測試完成 ===");
 moveToAbsoluteHeight(globalSafeHeight);
}

// ===== 核心遞歸函數 =====
void hanoiTower(int n, int from, int to, int aux) {
 if (!startFlag) return;

 if (n == 1) {
 moveDiskSafely(from, to);
 } else {
 hanoiTower(n-1, from, aux, to);
 moveDiskSafely(from, to);
 hanoiTower(n-1, aux, to, from);
 }
}

// ===== 安全移動函數 =====
void moveDiskSafely(int from, int to) {
 int diskIndex = 5 - diskCounts[from]; // 0=底層大碟片
 長 pickupHeight = diskHeights[diskIndex];
 長 placeHeight = diskHeights[5 - diskCounts[to] - 1];

 Serial.print("\n>> 移動第"); Serial.print(currentMoveIndex+1);
 Serial.print("/31: 塔"); Serial.print(from);
 Serial.print(" → 塔"); Serial.print(to);
 Serial.print(" (碟片"); Serial.print(diskIndex+1);
 Serial.print(", 角度:"); Serial.print(grabAngles[currentMoveIndex]); Serial.println("°)");

 // === 嚴格的安全移動序列 ===
 moveToAbsoluteHeight(globalSafeHeight); // 1. 安全高度
 safeMoveHorizontal(from * towerSpacing - currentX); // 2. 水平移動源塔
 safeMoveVertical(pickupHeight - currentY, false); // 3. 下降夾取高度
 setGripperAngle(0); // 4. 夾取碟片
 delay(800);
 moveToAbsoluteHeight(globalSafeHeight); // 5. 抬升安全高度
 safeMoveHorizontal(to * towerSpacing - currentX); // 6. 水平移動目標塔
 safeMoveVertical(placeHeight - currentY, false); // 7. 下降放置高度
 safeMoveVertical(platePenetration, false); // 8. 輕微下壓
 setGripperAngle(servoOpen,true); // 9. 釋放碟片
 delay(500);
 moveToAbsoluteHeight(globalSafeHeight); // 10. 回傳安全高度

 // 更新狀態
 diskCounts[from]--;
 diskCounts[to]++;
 currentMoveIndex++;

 printTowerStatus();
}

// ===== 改進的夾爪控制函數 =====
// ===== 改進版夾爪控制函數 =====
void setGripperAngle(int angle, bool immediate = false) {
 // 如果是初始化打開夾爪，直接執行

 if (immediate) {
 gripper.write(angle);
 Serial.print("夾爪直接設定: "); Serial.print(angle); Serial.println("°");
 return;

 }

 // 正常移動時，使用grabAngles數組中的角度
 int targetAngle = grabAngles[currentMoveIndex % 31]; // 迴圈使用31個預設角度
 targetAngle = constrain(targetAngle, 0, 90); // 安全性限制

 Serial.print("夾爪調整: ");
 Serial.print(gripper.read()); Seria
