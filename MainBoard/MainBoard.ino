
// NOTAS:
/*
Alarms have set by default being re set to the next day, until it explicitly it's told to clear by the program.
Timers always repeat, theres no one option, it needs to be explicitly told to be cleared for it to stop.

*/


#include <Servo.h>
#include <RTClib.h>
#include <Wire.h>

RTC_DS3231 rtc;

// Dates
DateTime initDate;
DateTime currDate;

DateTime blankDate;

// Debugging function
void printDate(DateTime date){
  Serial.print(date.day());
  Serial.print("/");
  Serial.print(date.month());
  Serial.print("/");
  Serial.print(date.year());
  Serial.print(" ");
  Serial.print(date.hour());
  Serial.print(":");
  Serial.print(date.minute());
  Serial.print(":");
  Serial.println(date.second());
}

class Bin {
  public:
    String id;
    int id_int;
    int delaySec = 2; // Delay in seconds *SET MANUALLY*
    int servoDelaySec = 1; // Delay in seconds for servo to retract *SET AS DEFAULT*

    void servoInit(int pin){
      servo.attach(pin);
      servo.write(0);
    };

    void servoWrite(int degrees){
      servo.write(degrees);
    };
    void setAlarm(DateTime alarmTime){
      if(mode == 2){
        Serial.println("[ERROR] Timer already configured, please use clear() before setting the alarm!");
      }
      else{
        dueDate = alarmTime;
        mode = 1;
      }
    };
    // Preferebly use the timer *FOR NOW*
    void setTimer(int days, int hours, int minutes, int seconds){
      if(mode == 1){
        Serial.println("[ERROR] Alarm already configured, please use clear() before setting the timer!");
      }
      else{
        dueDate = DateTime(
          initDate.year(),
          initDate.month(),
          initDate.day() + days,
          initDate.hour() + hours,
          initDate.minute() + minutes,
          initDate.second() + seconds
        );
        timerParams[0] = days;
        timerParams[1] = hours;
        timerParams[2] = minutes;
        timerParams[3] = seconds;        
        mode = 2;
      }
    };
    bool hasFired(){
      if(mode != 0){
        if(currDate.unixtime() >= dueDate.unixtime()){
          if(!isTriggered){
            // Single trigger zone, only is called once till' next timer/alarm
            delayDate = DateTime(currDate.unixtime() + delaySec);
            servoDelayDate = DateTime(currDate.unixtime() + servoDelaySec);
            if(mode == 1){ handleAlarmTrigger(); }
            else if(mode == 2) { handleTimerTrigger(); }

            servo.write(180);
            hasServoRetracted = false;
            Wire.beginTransmission(1);
            Wire.write(id_int);
            Wire.endTransmission();

            Serial.print("BIN TRIGGERED! ID: ");
            Serial.print(id);
            Serial.print(" INT: ");
            Serial.println(id_int);
          }
          if(!hasServoRetracted && (currDate.unixtime() >= servoDelayDate.unixtime())){
            servo.write(0);
            hasServoRetracted = true;
            Serial.print("SERVO RETRACTED! ID: ");
            Serial.println(id);
            Wire.beginTransmission(1);
            Wire.write(id_int + 10);
            Wire.endTransmission();
          }
          if(currDate.unixtime() <= delayDate.unixtime()){
            isTriggered = true;
            return true;
          }
          else{
            dueDate = newDueDate;
          }
        }
      }
      isTriggered = false;
      return false;
    }
    void clear(){
      dueDate = blankDate;
      newDueDate = blankDate;
      mode = 0;
      timerParams[0] = 0; 
      timerParams[1] = 0;
      timerParams[2] = 0;
      timerParams[3] = 0;
      isTriggered = false;
    }
  private:
    DateTime dueDate; // Expected dueDate
    DateTime newDueDate; // Temp storage of expected new dueDate
    DateTime delayDate; // Full date for delay
    DateTime servoDelayDate; // Full date for servo delay
    bool hasServoRetracted = false;
    int mode = 0; // 1: Alarm, 2: Timer
    int timerParams[4] = {
      0, // Days
      0, // Hours
      0, // Minutes
      0  // Seconds
    };
    bool isTriggered = false;
    Servo servo;
    void handleAlarmTrigger(){
      newDueDate = DateTime(
        dueDate.year(),
        dueDate.month(),
        dueDate.day() + 1,
        dueDate.hour(),
        dueDate.minute(),
        dueDate.second()
      );
    }
    void handleTimerTrigger(){
      newDueDate = DateTime(
        dueDate.year(),
        dueDate.month(),
        dueDate.day() + timerParams[0],
        dueDate.hour() + timerParams[1],
        dueDate.minute() + timerParams[2],
        dueDate.second() + timerParams[3]
      );
    }

}; 

const int totalBins = 9;
Bin bin[totalBins];
String binIds[9] = { "A1", "A2", "A3", "B1", "B2", "B3", "C1", "C2", "C3" };

void initializeServos(int minRange, int maxRange){
  for(int i = minRange; i <= maxRange; i++){
    bin[i - minRange].id = binIds[i - minRange];
    bin[i - minRange].id_int = i - minRange;
    bin[i - minRange].servoInit(i);
    bin[i - minRange].setTimer(0,0,0,6 + (2*(i-minRange)) );
  }
}

void initializeBinaryCount(int minRange, int maxRange){
  // for(int i=minRange; i<=maxRange; i++){
  //   Serial.println(i);
  //   pinMode(i, OUTPUT);
  //   digitalWrite(i, HIGH);
  // }
  // pinMode(11, OUTPUT);
  // analogWrite(11, 0);
}


void setup() {
  // El proposito del proyecto es realizar un pastillero el cual se pueda configurar en dos modos
  // Temporizador (cada ciertas horas), o en alarma (a una hora en especifico)

  // Configurar los servos
  // initializeServos();

  // Hora interna de la computadora
  initDate = DateTime(F(__DATE__), F(__TIME__));

  // Setup clock, serial port and wire library
  rtc.begin();
  rtc.adjust(initDate);
  Serial.begin(9600);
  Wire.begin();
  initializeServos(2, 10);
  initializeBinaryCount(11, 13);
  Serial.println("----------- BOOT -----------");

  // Work Zone

  int binary = 0b111;
  Serial.println(binary);

  // bin[0].setTimer(0, 0, 0, 10);
  // bin[1].setTimer(0, 0, 0, 8); 
  // bin[2].setTimer(0, 0, 0, 6);


};

void loop() {
  // put your main code here, to run repeatedly:
  currDate = rtc.now();

  for(int i=0; i<totalBins; i++){
    if(bin[i].hasFired()){
      // Constant trigger while delay check ends

    };
  }


};
