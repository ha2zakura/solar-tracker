#include "Arduino.h"
#include "Wire.h"
#include "Servo.h"
#include "ST7032.h"

#include <math.h>

#define PTR_EE  A1
#define PTR_NW  A0
#define PTR_TP  A3
#define PTR_SW  A2

#define SV_TH 3
#define SV_PH 2

#define SIN_000 ( 0.0000)
#define COS_000 ( 1.0000)
#define SIN_060 ( 0.8660)
#define COS_060 (-0.5000)
#define SIN_120 (-0.8660)
#define COS_120 (-0.5000)

#define SIN_TH  ( 0.7071)
#define COS_TH  ( 0.7071)

#define SQRT_2  ( 1.4141)

#define RAD2DEG(x)  (x * 57.296)

ST7032 lcd;

Servo theta;
Servo phi;

char row_one[9] = "";
char row_two[9] = "";
char buf[9] = "";

int val_ee = 0;
int val_nw = 0;
int val_tp = 0;
int val_sw = 0;

double temp_mu = 0;
double temp_sg = 0;
double temp_cos = 0;
double temp_sin = 0;
double temp_r = 0;
double temp_theta = 0;
double temp_phi = 0;

void setServo(int th, int ph) {
  // th: [0,90], ph: [0,360)
  if (th < 0) th = -th;
  if (th > 90) th = 90;
  if (ph >= 0 && ph < 90) {
    phi.write(ph + 90);
    theta.write(90 - th);
  } else if (ph >= 90 && ph < 270) {
    phi.write(ph - 90);
    theta.write(th + 90);
  } else if (ph >= 270 && ph < 360) {
    phi.write(ph - 270);
    theta.write(90 - th);
  }

  sprintf(row_one, "T%03dP%03d", th, ph);
  lcd.setCursor(0, 0);
  lcd.print(row_one);
}

void setup()
{
  lcd.begin(8,2);
  lcd.setContrast(30);

  theta.attach(SV_TH);
  phi.attach(SV_PH);

  analogReadResolution(12);

  setServo(0, 0);
  delay(1000);
}

void loop()
{
  val_tp = analogRead(PTR_TP);
  val_ee = analogRead(PTR_EE);
  val_nw = analogRead(PTR_NW);
  val_sw = analogRead(PTR_SW);

  ////
  temp_mu = (val_ee + val_nw + val_sw) / 3;
  temp_sg = sqrt(
      (val_ee - temp_mu) * (val_ee - temp_mu) +
      (val_nw - temp_mu) * (val_nw - temp_mu) +
      (val_sw - temp_mu) * (val_sw - temp_mu)
    );
  temp_cos = (val_tp - temp_mu) / (1 - COS_TH);
  temp_sin = (SQRT_2 * temp_sg) / SIN_TH;
  temp_theta = RAD2DEG(atan2(temp_sin, temp_cos));

  temp_cos = val_ee * COS_000 + val_nw * COS_060 + val_sw * COS_120;
  temp_sin = val_ee * SIN_000 + val_nw * SIN_060 + val_sw * SIN_120;
  temp_phi = RAD2DEG(atan2(temp_sin, temp_cos));
  if (temp_phi < 0) temp_phi += 360;
  ////

  // dtostrf(temp_sg, 6, 0, buf);
  sprintf(row_two, "% 4d% 4d", (int)temp_theta, (int)temp_phi);
  lcd.setCursor(0, 1);
  lcd.print(row_two);

  setServo(temp_theta, temp_phi);

  delay(100);
}