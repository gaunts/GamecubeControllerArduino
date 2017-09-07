#include "Nintendo.h"

bool init_done = false;
CGamecubeController controller(5);
CGamecubeConsole console(6);
Gamecube_Report_t report;

byte analog_x_abs, analog_y_abs, cstick_x_abs, cstick_y_abs, cycles;
char analog_x, analog_y, cstick_x, cstick_y;

long buf;
long cycle;
bool off = false;
bool dolphin = false;
bool hasCurrentInput = false;

void setup() {
  
Serial.begin(9600);
  report.errlatch=0; 
  report.high1=0; 
  report.errstat=0;
  pinMode(LED_BUILTIN, OUTPUT);
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
    report.origin = 0;
  }
  return true;
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
  {
    return;
  }

  if (analog_x_abs < 21)
  {
    //Serial.println(cycle);
    buf = cycle;
    return;
  }
  
  if (buf > 0)
  {
    if (analog_x_abs < 68)// if > 97, no need to edit the value
    {
      //Serial.println("block");
      report.xAxis = 128;
    }
    buf--;
  }
}

// Turns features off, sets dolphin mode or calibrates shield drop notches
void checkInputs()
{
  if (report.ddown == 1 && report.b == 1)
  {
    if (!hasCurrentInput)
      off = !off;
  }
  else if (report.ddown == 1 && report.a == 1)
  {
    if (!hasCurrentInput)
      dolphin = !dolphin;
  }
  else
  {
    hasCurrentInput = false;
    return;    
  }
  hasCurrentInput = true;
}

void loop() 
{
  if (!firstRead())
    return;
  
  report = controller.getReport();
  initAxes();

  checkInputs();
  cycle = dolphin ? 8 : 2;

  if (!off)
    executeFixes();
  
  if (!console.write(report))
  {
    delay(10);
  }
}
