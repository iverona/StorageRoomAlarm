#include <SigFox.h>
#include <ArduinoLowPower.h>

#define WAKE_UP_INTERVAL 259200000 //3 days

float readBatteryLevel();
void alarmInterruption();
void reboot();

const char DOOR_OPEN = '1';
const char DOOR_CLOSED = '0';
char alarmCode = DOOR_CLOSED;

void setup()
{

  if (!SigFox.begin())
  {
    //something is really wrong, try rebooting
    reboot();
  }

  //Send module to standby until we need to send a message
  SigFox.end();

  // Enable debug prints and LED indication if we are testing
  SigFox.debug();

  // attach pin 0 and 1 to a switch and enable the interrupt on voltage falling event
  pinMode(0, INPUT_PULLDOWN);
  LowPower.attachInterruptWakeup(0, alarmInterruption, CHANGE);
  //LowPower.attachInterruptWakeup(0, alarmInterruptionClose, FALLING);
}

void loop()
{
  // Sleep until an event is recognized
  LowPower.deepSleep();

  // if we get here it means that an event was received
  SigFox.begin();

  delay(100);

  // 2 bytes (int alarmCode) + 4 bytes (float batt. level) < 12 bytes
  String to_be_sent = String(alarmCode) + String(readBatteryLevel());

  SigFox.beginPacket();
  SigFox.print(to_be_sent);
  int ret = SigFox.endPacket();

  //alarmCode = 0;

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