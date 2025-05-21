#include <AccelStepper.h>

// Определение контактов
#define MOTOR_PIN1 D1
#define MOTOR_PIN2 D2
#define MOTOR_PIN3 D5
#define MOTOR_PIN4 D6
#define ENDSTOP_PIN D3  // Концевой выключатель
#define MANUAL_ZERO_PIN D4  // Кнопка ручного обнуления (опционально)

// Создание объекта stepper
AccelStepper stepper(AccelStepper::FULL4WIRE, MOTOR_PIN1, MOTOR_PIN2, MOTOR_PIN3, MOTOR_PIN4);

// Параметры системы
long maxPosition = 10000;  // Максимальная позиция (100%)
long currentPosition = 0;
long targetPosition = 0;
int moveSpeed = 500;  // Скорость по умолчанию
bool directionInverted = false;  // Флаг инверсии направления
bool isHoming = false;  // Флаг процесса обнуления

void setup() {
  Serial.begin(115200);
  
  // Настройка пинов
  pinMode(ENDSTOP_PIN, INPUT_PULLUP);
  pinMode(MANUAL_ZERO_PIN, INPUT_PULLUP);
  
  // Настройка двигателя
  stepper.setMaxSpeed(1000);
  stepper.setSpeed(moveSpeed);
  stepper.setAcceleration(500);
  
  // Приветственное сообщение
  Serial.println("\nWindow/Door/Curtain Controller - ESP8266");
  Serial.println("Available commands:");
  Serial.println("home - Start homing procedure");
  Serial.println("zero - Set current position as zero");
  Serial.println("speed=XXX - Set speed (100-1000)");
  Serial.println("invert - Invert movement direction");
  Serial.println("move=XX - Move to XX% position (0-100)");
  Serial.println("pos=XXX - Move to absolute position");
  Serial.println("status - Get current status");
  Serial.println("help - Show this help message");
}

void loop() {
  // Обработка ручного обнуления
  if (digitalRead(MANUAL_ZERO_PIN) == LOW) {
    setCurrentPositionAsZero();
    delay(500);  // Дебаунс
  }
  
  // Обработка процесса обнуления
  if (isHoming) {
    handleHoming();
  } else {
    // Обработка движения к целевой позиции
    if (stepper.distanceToGo() != 0) {
      stepper.run();
    }
  }
  
  // Обработка команд с последовательного порта
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    processCommand(command);
  }
}

// Обработка команд
void processCommand(String command) {
  if (command == "home") {
    startHoming();
  } else if (command == "zero") {
    setCurrentPositionAsZero();
  } else if (command == "invert") {
    invertDirection();
  } else if (command == "status") {
    printStatus();
  } else if (command == "help") {
    printHelp();
  } else if (command.startsWith("speed=")) {
    setSpeed(command.substring(6).toInt());
  } else if (command.startsWith("move=")) {
    moveToPercent(command.substring(5).toInt());
  } else if (command.startsWith("pos=")) {
    moveToPosition(command.substring(4).toInt());
  } else {
    Serial.println("Error: Unknown command");
    Serial.println("Type 'help' for available commands");
  }
}

// Начать процедуру обнуления
void startHoming() {
  isHoming = true;
  stepper.setSpeed(-moveSpeed);  // Двигаемся к нулю
  Serial.println("Homing started... Moving to zero position");
}

// Обработка процесса обнуления
void handleHoming() {
  if (digitalRead(ENDSTOP_PIN) == LOW) {  // Концевик не нажат
    stepper.runSpeed();
  } else {  // Концевик нажат
    stepper.setCurrentPosition(0);
    currentPosition = 0;
    targetPosition = 0;
    isHoming = false;
    stepper.stop();
    Serial.println("Homing completed. Position set to 0.");
  }
}

// Установить текущую позицию как нулевую
void setCurrentPositionAsZero() {
  stepper.setCurrentPosition(0);
  currentPosition = 0;
  targetPosition = 0;
  Serial.println("Current position set to 0.");
}

// Инвертировать направление движения
void invertDirection() {
  directionInverted = !directionInverted;
  stepper.setPinsInverted(directionInverted, false, false);
  Serial.print("Direction ");
  Serial.println(directionInverted ? "inverted" : "normal");
}

// Установить скорость
void setSpeed(int speed) {
  if (speed >= 100 && speed <= 1000) {
    moveSpeed = speed;
    stepper.setMaxSpeed(moveSpeed);
    stepper.setAcceleration(100);
    Serial.print("Speed set to ");
    Serial.println(moveSpeed);
  } else {
    Serial.println("Error: Invalid speed (100-1000)");
  }
}

// Перемещение в процентное положение
void moveToPercent(int percent) {
  if (percent >= 0 && percent <= 100) {
    long pos = map(percent, 0, 100, 0, maxPosition);
    moveToPosition(pos);
    Serial.print("Moving to ");
    Serial.print(percent);
    Serial.println("%");
  } else {
    Serial.println("Error: Invalid percentage (0-100)");
  }
}

// Перемещение в абсолютную позицию
void moveToPosition(long pos) {
  if (pos >= 0 && pos <= maxPosition) {
    targetPosition = pos;
    stepper.moveTo(targetPosition);
    currentPosition = targetPosition;
    Serial.print("Moving to position ");
    Serial.println(pos);
  } else {
    Serial.println("Error: Position out of range");
  }
}

// Вывод текущего статуса
void printStatus() {
  Serial.println("\n=== System Status ===");
  Serial.print("Current position: ");
  Serial.print(stepper.currentPosition());
  Serial.print(" (");
  Serial.print(map(stepper.currentPosition(), 0, maxPosition, 0, 100));
  Serial.println("%)");
  Serial.print("Target position: ");
  Serial.println(targetPosition);
  Serial.print("Current speed: ");
  Serial.println(moveSpeed);
  Serial.print("Direction: ");
  Serial.println(directionInverted ? "Inverted" : "Normal");
  Serial.print("Endstop state: ");
  Serial.println(digitalRead(ENDSTOP_PIN) ? "Open" : "Closed");
  Serial.print("Is homing: ");
  Serial.println(isHoming ? "Yes" : "No");
  Serial.println("====================\n");
}

// Вывод справки
void printHelp() {
  Serial.println("\nAvailable commands:");
  Serial.println("home       - Start homing procedure (find zero position)");
  Serial.println("zero       - Set current position as zero");
  Serial.println("speed=XXX  - Set movement speed (100-1000)");
  Serial.println("invert     - Invert movement direction");
  Serial.println("move=XX    - Move to XX% position (0-100)");
  Serial.println("pos=XXX    - Move to absolute position");
  Serial.println("status     - Get current system status");
  Serial.println("help       - Show this help message");
  Serial.println();
}