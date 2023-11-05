#include <TimerThree.h>

#include <TimerOne.h>
#include <Wire.h>
#include <LiquidCrystal.h>
#include <AccelStepper.h>
#define ENC_HALFSTEP 1  
#define ENC_DECODER ENC_FLAKY
#include <ClickEncoder.h>

#define BLINKPIN 13
#define MAX_SPEED 1200.0
#define STEP_MODE 8.0
#define TURNS 300
#define STEPS 200.0

 #define LCD_PINS_RS 16
 #define LCD_PINS_ENABLE 17
 #define LCD_PINS_D4 23
 #define LCD_PINS_D5 25
 #define LCD_PINS_D6 27
 #define LCD_PINS_D7 29

AccelStepper stepperX = AccelStepper(AccelStepper::DRIVER,54,55);
LiquidCrystal lcd(LCD_PINS_RS, LCD_PINS_ENABLE, LCD_PINS_D4, LCD_PINS_D5, LCD_PINS_D6, LCD_PINS_D7);
ClickEncoder *encoder;

int16_t last, value;

void displayAccelerationStatus() {
  lcd.setCursor(0, 2);  
  lcd.print("Acceleration ");
  lcd.print(encoder->getAccelerationEnabled() ? "on " : "off");
}
void encoderDriver()
{
 encoder->service();
}

void stepperDriver()
{
  stepperX.runSpeed(); 
}

void displayAt(int x, int y, const char* string)
{
  lcd.setCursor(x,y);
  lcd.print(string);
}
void displayStepper(long value)
{
  displayAt(0,0,"Speed: ");
  lcd.print(value);
   lcd.print(" Hz   "); 
}
void displayTurns(int16_t value)
{
  displayAt(0,1,"RPM: ");
  lcd.print(value);
  lcd.print("    ");  
}
void displayButton(ClickEncoder::Button b)
{
  lcd.setCursor(0,2);
  char bstate = ' ';
  switch(b){
    case ClickEncoder::Open:
      bstate ='O'; break;
    case ClickEncoder::Clicked:
      bstate = 'C'; break;
    case ClickEncoder::Pressed:
      bstate = 'P'; break;
    case ClickEncoder::Held:
      bstate = 'H'; break;
    case ClickEncoder::Released:
       bstate= 'R'; break;
    case ClickEncoder::DoubleClicked:
       bstate = 'D'; break; 
  }
  lcd.print(bstate);
}

void setup() {
  // setup serial
  Serial.begin(57600);

  // setup LED
  pinMode(BLINKPIN, OUTPUT);

  // setup LCD
  lcd.begin(20,4);
  lcd.clear();
  lcd.setCursor(0,0);
  
  // setup encoder
  encoder = new ClickEncoder(31, 33, 35);  
  encoder->setAccelerationEnabled(true);
  // setup stepper stepper
  stepperX.setPinsInverted(false,false,true);
  stepperX.setEnablePin(38);
  stepperX.setMaxSpeed(MAX_SPEED*STEP_MODE);
  stepperX.setAcceleration(10000*STEP_MODE);
  stepperX.setSpeed(0);
//  unsigned long dest = TURNS * STEPS * STEP_MODE;
//  stepperX.move(dest);
  
  // setup Stepper driver timer
  Timer3.initialize(20);
  Timer3.attachInterrupt(stepperDriver);

  Timer1.initialize(1000);
  Timer1.attachInterrupt(encoderDriver);
  Serial.print("Starting :");
  lcd.print("Starting: ");
//  Serial.println(dest);
//  lcd.print(dest);
  
}

void loop() {
  static unsigned long dest = 0;//TURNS * STEPS * STEP_MODE;
  static unsigned long lastRunning = 0;
  static bool isRunning = true;
  static unsigned long lastBlink;
  static bool LEDstate = false;
  // put your main code here, to run repeatedly:
  displayStepper(stepperX.speed());
  
  if(stepperX.distanceToGo()==0){
    if(isRunning){
      isRunning = false;
      lastRunning = millis();
    }else{
      if( (millis() - lastRunning) > 1000){
        isRunning = true;
        stepperX.move(dest);
        dest *=-1;        
      } 
    }

  }

  if( (millis()-lastBlink)>500){
    digitalWrite(BLINKPIN,LEDstate);
    LEDstate = !LEDstate;
    lastBlink = millis();
  }

  value += encoder->getValue();
  if (value != last) {
    last = value;
    dest = value * STEPS * STEP_MODE;
    float newSpeed = (value / 60.0) * STEPS * STEP_MODE;
    stepperX.setSpeed(newSpeed);
  }
  displayTurns(value);
  if ((0 == value) /*&& (0 == stepperX.distanceToGo())*/){
//    if(stepperX.currentPosition() != 0){
      stepperX.setSpeed(0);
//    }else{
      stepperX.disableOutputs();
//    }
  }else{
    stepperX.enableOutputs();
  }
  ClickEncoder::Button b = encoder->getButton();
  displayButton(b);
  switch(b){
    case ClickEncoder::Clicked:{
        value += 25;
        break;
    }
    case ClickEncoder::Held:
        stepperX.setSpeed(0);
        value = 0;
        break;
  }
  lcd.setCursor(0,3);
  lcd.print("newVal: ");
  lcd.print(value);

 if (b != ClickEncoder::Open) {
   lcd.setCursor(0,3);
   Serial.print("Button: ");
   #define VERBOSECASE(label) case label: lcd.print(#label); break;
   switch (b) {
     VERBOSECASE(ClickEncoder::Pressed);
     VERBOSECASE(ClickEncoder::Held)
     VERBOSECASE(ClickEncoder::Released)
     VERBOSECASE(ClickEncoder::Clicked)
     case ClickEncoder::DoubleClicked:
         lcd.print("ClickEncoder::DoubleClicked");
         //encoder->setAccelerationEnabled(!encoder->getAccelerationEnabled());
         //Serial.print("  Acceleration is ");
         //Serial.println((encoder->getAccelerationEnabled()) ? "enabled" : "disabled");
       break;
   }
 }    
  
  delay(200);
}
