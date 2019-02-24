#include <SigFox.h>
#include <ArduinoLowPower.h>

#define WAKE_UP_INTERVAL 86400000     // 24 hours
#define SUB_WAKE_UP_INTERVAL 14400000 //4 hours
#define WAKE_CYCLES (WAKE_UP_INTERVAL / SUB_WAKE_UP_INTERVAL)

float readBatteryLevel();
void alarmInterruption();
void reboot();
void sendMessage();

const char KEEP_ALIVE = '2';
const char DOOR_OPEN = '1';
const char DOOR_CLOSED = '0';

char alarmCode = KEEP_ALIVE;

int wake_reps = 1;

void setup()
{

  if (!SigFox.begin())
  {
    //something is really wrong, try rebooting
    reboot();
  }

  //Send module to standby until we need to send a message
  SigFox.end();

  SigFox.debug();

  // attach pin 0 and 1 to a switch and enable the interrupt on voltage falling event
  pinMode(0, INPUT_PULLDOWN);
  LowPower.attachInterruptWakeup(0, alarmInterruption, CHANGE);
}

void loop()
{
  // Sleep until an event is recognized
  LowPower.deepSleep(SUB_WAKE_UP_INTERVAL);

  switch (alarmCode)
  {

  case KEEP_ALIVE:
    if (wake_reps < WAKE_CYCLES)
    {
      wake_reps++;
      break;
    }

  default:
    wake_reps = 1;

    sendMessage();

    alarmCode = KEEP_ALIVE;
  }
}

void sendMessage()
{
  SigFox.begin();

  delay(100);

  // 2 bytes (int alarmCode) + 4 bytes (float batt. level) < 12 bytes
  String to_be_sent = String(alarmCode) + String(readBatteryLevel());

  SigFox.beginPacket();
  SigFox.print(to_be_sent);
  SigFox.endPacket();

  // shut down module, back to standby
  SigFox.end();
}

void alarmInterruption()
{
  int state = digitalRead(0);

  if (state == HIGH)
    alarmCode = DOOR_OPEN;
  else
    alarmCode = DOOR_CLOSED;
}

void reboot()
{
  NVIC_SystemReset();
  while (1)
    ;
}

float readBatteryLevel()
{
  analogReadResolution(10);
  analogReference(AR_INTERNAL1V0); //AR_DEFAULT: the default analog reference of 3.3V // AR_INTERNAL1V0: a built-in 1.0V reference

  // read the input on analog pin 0:
  int sensorValue = analogRead(ADC_BATTERY);
  // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 4.3V):
  float voltage = sensorValue * (3.25 / 1023.0);

  return voltage;
}