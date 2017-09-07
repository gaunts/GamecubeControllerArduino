#include "Nintendo.h"

CGamecubeController controller(5);
CGamecubeConsole console(6);
Gamecube_Report_t report;

unsigned long buf = 0;

void setup() {
  Serial.begin(9600);

  // put your setup code here, to run once:
  pinMode(LED_BUILTIN, OUTPUT);
}

bool a = false;
unsigned long b = 0;
long ticks = 0;
void loop() {
  ticks++;
  //controller.read();
  if (b == 0)
    b = millis();
   if (millis() - b > 1000)
   {
    Serial.println(ticks);
    b = 0;
    ticks = 0;
   }
//    digitalWrite(LED_BUILTIN, HIGH);
    //delay(1000);
//    digitalWrite(LED_BUILTIN, LOW);
    //delay(1000);
//    return;
  //report = controller.getReport();
  //console.write(report);
}
