float sinVal;
int toneVal;
int brightness = 0;
int fadeAmount = 5;

int x = 0;

void foFun(int zummer_pin, int led_pin) {
  sinVal = (sin(x*(3.1412/180)));
  toneVal = 2000+(int(sinVal*1000));
  tone(zummer_pin, toneVal);
  delay(2);
  analogWrite(led_pin, brightness);
  brightness = brightness + fadeAmount;
  if (brightness <= 0 || brightness >= 255) {
    fadeAmount = -fadeAmount;
  }

  x++;
  if (x >= 180) x = 0;
}

void foFunOff(int zummer_pin, int led_pin) {
  noTone(zummer_pin);
  analogWrite(led_pin, 0);
}