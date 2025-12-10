#define MOTOR_PIN1 D3
#define MOTOR_PIN2 D4   

enum MotorState {
  MOTOR_STOPPED,
  MOTOR_OPENING,
  MOTOR_CLOSING,
  MOTOR_OPEN,
  MOTOR_CLOSED
};

enum systemVars : size_t {   
  open_cmd,
  close_cmd,
  speed,
};

bool commandOpen = false, commandClose = false;
MotorState motorState = MOTOR_STOPPED;
int motorSpeed = 255;

void stop() {
  analogWrite(MOTOR_PIN1, 0);
  analogWrite(MOTOR_PIN2, 0);
}

void open() {
  analogWrite(MOTOR_PIN1, motorSpeed);
  analogWrite(MOTOR_PIN2, 0);
}

void close() {
  analogWrite(MOTOR_PIN1, 0);
  analogWrite(MOTOR_PIN2, motorSpeed);
}