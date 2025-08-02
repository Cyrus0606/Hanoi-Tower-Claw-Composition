#include <Servo.h>

// ===== 硬件配置 =====
#define EN_PIN 8          // 使能引脚
#define X_STEP_PIN 2      // X轴步进
#define X_DIR_PIN 5       // X轴方向  
#define Y_STEP_PIN 3      // Y轴步进
#define Y_DIR_PIN 6       // Y轴方向
#define SERVO_PIN 12      // 夹爪舵机

// ===== 运动参数 =====
const int stepDelay = 1000;       // 步进延迟（控制速度）
const int servoOpen = 0;          // 夹爪全开角度
const long startHeight = 1000;       // 初始高度
const long globalSafeHeight = 1200; // 全局安全高度
const long towerSpacing = 3000;    // 塔间距（步数）
const long platePenetration = 0; // 放置时下压深度

// 31次移动的夹取角度序列
const int grabAngles[31] = {
  80, 75, 80, 55, 80, 75, 80, 50, 80, 75, 80, 55, 80, 75, 80, 45, 
  80, 75, 80, 55, 80, 75, 80, 50, 80, 75, 80, 55, 80, 75, 80
};

// 碟片高度参数（从顶层到底层）
const long diskHeights[] = {5200, 5700, 6700, 7300, 8500};

// ===== 全局变量 =====
Servo gripper;
long currentX = 0;                // 当前X位置
long currentY = startHeight;      // 当前Y位置
int diskCounts[3] = {5, 0, 0};    // 各塔碟片数
bool startFlag = false;           // 启动标志
int currentMoveIndex = 0;         // 当前移动序号

// ===== 函数声明 =====
void setGripperAngle(int angle, bool immediate = false);
void moveToAbsoluteHeight(long targetHeight);
void safeMoveHorizontal(long steps);
void safeMoveVertical(long steps, bool isUp);
//void emergencyStop();
void printTowerStatus();
void hanoiTower(int n, int from, int to, int aux);
void moveDiskSafely(int from, int to);

// ===== 初始化设置 =====
void setup() {
  // 引脚模式设置
  pinMode(EN_PIN, OUTPUT);
  digitalWrite(EN_PIN, LOW);      // 激活驱动器
  pinMode(X_DIR_PIN, OUTPUT);
  pinMode(X_STEP_PIN, OUTPUT);
  pinMode(Y_DIR_PIN, OUTPUT);
  pinMode(Y_STEP_PIN, OUTPUT);

  // 夹爪初始化
  gripper.attach(SERVO_PIN);
  setGripperAngle(servoOpen, true); // 初始化完全打开
  delay(2000);  // 确保舵机到位

  // 初始抬升到安全高度
  moveToAbsoluteHeight(globalSafeHeight);
  
  Serial.begin(9600);
  Serial.println("河内塔机械臂已就绪");
  Serial.println("输入 'Start' 开始执行");
  Serial.println("输入 'Stop' 紧急停止");
  printTowerStatus();

  // 测试运动（可选）
  // testMovement();
}

// ===== 主循环 =====
void loop() {
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim();
    
    if (input.equalsIgnoreCase("Start")) {
      startFlag = true;
      currentMoveIndex = 0;  // 重置移动计数器
      Serial.println("\n=== 开始执行河内塔 ===");
      hanoiTower(5, 0, 2, 1);  // 5层从塔A(0)到塔C(2)
      Serial.println("=== 执行完成 ===");
      startFlag = false;
    } 
    else if (input.equalsIgnoreCase("Status")) {
      printTowerStatus();
    }
  }
}

// ===== 运动测试函数 =====
void testMovement() {
  Serial.println("\n=== 开始运动测试 ===");
  
  Serial.println("测试右移1000步");
  safeMoveHorizontal(1000);
  delay(1000);
  
  Serial.println("测试左移1000步");
  safeMoveHorizontal(-1000);
  delay(1000);
  
  Serial.println("测试上升500步");
  safeMoveVertical(500, true);
  delay(1000);
  
  Serial.println("测试下降500步");
  safeMoveVertical(500, false);
  
  Serial.println("=== 运动测试完成 ===");
  moveToAbsoluteHeight(globalSafeHeight);
}

// ===== 核心递归函数 =====
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

// ===== 安全移动函数 =====
void moveDiskSafely(int from, int to) {
  int diskIndex = 5 - diskCounts[from];  // 0=底层大碟片
  long pickupHeight = diskHeights[diskIndex];
  long placeHeight = diskHeights[5 - diskCounts[to] - 1];

  Serial.print("\n>> 移动第"); Serial.print(currentMoveIndex+1); 
  Serial.print("/31: 塔"); Serial.print(from);
  Serial.print(" → 塔"); Serial.print(to);
  Serial.print(" (碟片"); Serial.print(diskIndex+1);
  Serial.print(", 角度:"); Serial.print(grabAngles[currentMoveIndex]); Serial.println("°)");

  // === 严格的安全移动序列 ===
  moveToAbsoluteHeight(globalSafeHeight);          // 1. 安全高度
  safeMoveHorizontal(from * towerSpacing - currentX); // 2. 水平移动源塔
  safeMoveVertical(pickupHeight - currentY, false);   // 3. 下降夹取高度
  setGripperAngle(0);     // 4. 夹取碟片
  delay(800);
  moveToAbsoluteHeight(globalSafeHeight);          // 5. 抬升安全高度
  safeMoveHorizontal(to * towerSpacing - currentX);   // 6. 水平移动目标塔
  safeMoveVertical(placeHeight - currentY, false);    // 7. 下降放置高度
  safeMoveVertical(platePenetration, false);         // 8. 轻微下压
  setGripperAngle(servoOpen,true);                      // 9. 释放碟片
  delay(500);
  moveToAbsoluteHeight(globalSafeHeight);          // 10. 返回安全高度

  // 更新状态
  diskCounts[from]--;
  diskCounts[to]++;
  currentMoveIndex++;
  
  printTowerStatus();
}

// ===== 改进的夹爪控制函数 =====
// ===== 改进版夹爪控制函数 =====
void setGripperAngle(int angle, bool immediate = false) {
  // 如果是初始化打开夹爪，直接执行

  if (immediate) {
    gripper.write(angle);
    Serial.print("夹爪直接设置: "); Serial.print(angle); Serial.println("°");
    return;

  }

  // 正常移动时，使用grabAngles数组中的角度
  int targetAngle = grabAngles[currentMoveIndex % 31]; // 循环使用31个预设角度
  targetAngle = constrain(targetAngle, 0, 90); // 安全限制
  
  Serial.print("夹爪调整: ");
  Serial.print(gripper.read()); Serial.print("° → "); 
  Serial.print(targetAngle); Serial.print("° (步骤");
  Serial.print(currentMoveIndex+1); Serial.println("/31)");

  // 渐进式调整角度
  while (gripper.read() != targetAngle) {
    int current = gripper.read();
    int step = (targetAngle > current) ? 1 : -1;
    gripper.write(current + step);
    delay(50); // 调整速度
  }
}


// ===== 运动控制函数 =====
void moveToAbsoluteHeight(long targetHeight) {
  long delta = targetHeight - currentY;
  if (delta == 0) return;
  
  bool isUp = delta > 0;
  safeMoveVertical(abs(delta), isUp);
}

// 修正后的水平移动（X/Y协同控制）
void safeMoveHorizontal(long steps) {
  if (steps == 0) return;
  
  bool moveRight = steps > 0;
  // 右移：双逆时针 | 左移：双顺时针
  digitalWrite(X_DIR_PIN, moveRight ? HIGH : LOW);
  digitalWrite(Y_DIR_PIN, moveRight ? HIGH : LOW);
  
  steps = abs(steps);
  Serial.print(moveRight ? "右移" : "左移"); 
  Serial.print(steps); Serial.println("步");

  for (long i = 0; i < steps; i++) {
    digitalWrite(X_STEP_PIN, HIGH);
    digitalWrite(Y_STEP_PIN, HIGH);
    delayMicroseconds(stepDelay);
    digitalWrite(X_STEP_PIN, LOW);
    digitalWrite(Y_STEP_PIN, LOW);
    delayMicroseconds(stepDelay);
  }
  currentX += moveRight ? steps : -steps;
}

// 修正后的垂直移动（X/Y协同控制）
void safeMoveVertical(long steps, bool isUp) {
  if (steps == 0) return;
  
  // 上升：X逆 Y顺 | 下降：X顺 Y逆
  digitalWrite(X_DIR_PIN, isUp ? HIGH : LOW);   // 上升=逆时针
  digitalWrite(Y_DIR_PIN, isUp ? LOW : HIGH);   // 上升=顺时针
  
  Serial.print(isUp ? "上升" : "下降"); 
  Serial.print(steps); Serial.println("步");

  for (long i = 0; i < steps; i++) {
    digitalWrite(X_STEP_PIN, HIGH);
    digitalWrite(Y_STEP_PIN, HIGH);
    delayMicroseconds(stepDelay);
    digitalWrite(X_STEP_PIN, LOW);
    digitalWrite(Y_STEP_PIN, LOW);
    delayMicroseconds(stepDelay);
  }
  currentY += isUp ? steps : -steps;
}

// ===== 辅助函数 =====
/* void emergencyStop() {
  Serial.println("\n!!! 紧急停止 !!!");
  setGripperAngle(servoOpen, true);
  moveToAbsoluteHeight(globalSafeHeight);
  startFlag = false;
  while(1) { delay(100); }  // 锁定系统
} */

void printTowerStatus() {
  Serial.println("\n[当前状态]");
  Serial.print("塔A(右): "); Serial.print(diskCounts[0]); Serial.println("个碟片");
  Serial.print("塔B(中): "); Serial.print(diskCounts[1]); Serial.println("个碟片");
  Serial.print("塔C(左): "); Serial.print(diskCounts[2]); Serial.println("个碟片");
  Serial.print("机械臂位置: X="); Serial.print(currentX);
  Serial.print(", Y="); Serial.println(currentY);
  Serial.print("当前移动: "); Serial.print(currentMoveIndex); Serial.println("/31");
  if(currentMoveIndex < 31) {
    Serial.print("下次夹取角度: "); Serial.print(grabAngles[currentMoveIndex]); Serial.println("°");
  }
}