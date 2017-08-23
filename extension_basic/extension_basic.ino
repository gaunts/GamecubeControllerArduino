#include "Nintendo.h"

bool init_done = false;
CGamecubeController controller(2);
CGamecubeConsole console(3);
Gamecube_Report_t report;

void setup() {
  report.errlatch=0; 
  report.high1=0; 
  report.errstat=0;
}

bool firstRead()
{
  if (!controller.read())
  {
    delay(100);
    return false;
  }
  if (!init_done)
  {
    init_done = true;
    report.origin = controller.getOrigin();
  }
}

void loop() {
  if (!firstRead())
    return;
  
  report = controller.getReport();
  if (!console.write(report))
  {
    delay(10);
  }
}
