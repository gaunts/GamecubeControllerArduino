#include "Nintendo.h"

CGamecubeController controller(2);
CGamecubeConsole console(3);
Gamecube_Report_t report;

unsigned long buf = 0;

void setup() {
  // put your setup code here, to run once:
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  if (!controller.read())
  {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
    return;
  }
  buf++;
  if ((buf % 60) == 0)
  {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(17);
    digitalWrite(LED_BUILTIN, LOW);
  }
  report = controller.getReport();
  if (!console.write(report))
  {
    delay(10);
  }
}
