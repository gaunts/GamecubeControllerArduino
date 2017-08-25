#include "Nintendo.h"

bool init_done = false;
CGamecubeController controller(2);
CGamecubeConsole console(3);
Gamecube_Report_t report;

bool off = false;
byte analog_x_abs, analog_y_abs, cstick_x_abs, cstick_y_abs, cycles;
char analog_x, analog_y, cstick_x, cstick_y, buf;

unsigned long dleft_count = 0;

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

void initAxes()
{
  //offsets from nuetral position of analog and c stick
  analog_x = report.xAxis -128; 
  analog_y = report.yAxis -128;
  cstick_x = report.cxAxis - 128;
  cstick_y = report.cyAxis - 128;
  
  //get abs values for easier magnitude comparison
  analog_x_abs = abs(analog_x);
  analog_y_abs = abs(analog_y);
  cstick_x_abs = abs(cstick_x);
  cstick_y_abs = abs(cstick_y);
}

void executeFixes()
{
  dashBack();
}

void dashBack(){
  if (analog_y_abs >= 23)
    return;

  if (analog_x_abs < 21)
  {
    buf = cycles;
    return;
  }
  
  if (buf > 0)
  {
    if (analog_x_abs < 64) // if > 64, no need to edit the value
      report.xAxis = 128;
    buf--;
  }
}

// Turns features off, sets dolphin mode or calibrates shield drop notches
void checkInputs()
{
  // off
  if (report.dleft)
  {
    if (dleft_count == 0)
      dleft_count = millis();
    else if (millis() - dleft_count > 2000)
      off = !off;
  }
  else
    dleft_count = 0;

  // dolphin
  cycle = 3; // 8 if dolphin
}

void loop() 
{
  if (!firstRead())
    return;

  report = controller.getReport();
  initAxes();

  if (!off)
    executeFixes();
  
  if (!console.write(report))
  {
    delay(10);
  }
}
