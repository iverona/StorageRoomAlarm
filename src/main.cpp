#include <SigFox.h>
#include <ArduinoLowPower.h>

void alarmIdGenerator();
void reboot();

long alarmId;

void setup()
{

  // We are using Serial1 instead than Serial because we are going in standby
  // and the USB port could get confused during wakeup. To read the debug prints,
  // connect pins 13-14 (TX-RX) to a 3.3V USB-to-serial converter

  Serial1.begin(115200);
  while (!Serial1)
  {
  }

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
  LowPower.attachInterruptWakeup(0, alarmIdGenerator, RISING);
}

void loop()
{
  // Sleep until an event is recognized
  LowPower.deepSleep();

  // if we get here it means that an event was received
  SigFox.begin();

  delay(100);

  // 1 bytes (ALM) + 4 bytes (long alarmId) < 12 bytes
  String to_be_sent = "1"; // + String(alarmId);

  SigFox.beginPacket();
  SigFox.print(to_be_sent);
  int ret = SigFox.endPacket();

  // shut down module, back to standby
  SigFox.end();
}

void alarmIdGenerator()
{
  alarmId = random(1000);
}

void reboot()
{
  NVIC_SystemReset();
  while (1)
    ;
}
