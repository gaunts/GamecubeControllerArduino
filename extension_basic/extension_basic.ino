#include "Nintendo.h"

bool init_done = false;
CGamecubeController controller(5);
CGamecubeConsole console(6);
Gamecube_Report_t report;

byte analog_x_abs, analog_y_abs, cstick_x_abs, cstick_y_abs;
char analog_x, analog_y, cstick_x, cstick_y;
char x_offset = 0;
char y_offset = 0;

long buf;
long cycle;

bool off = false;
bool dolphin = false;
bool ucf = false;

bool hasCurrentInput = false;
bool shield = false;
bool tiltedShield = false;
float angle;

float ang(float xval, float yval){return atan(yval/xval)*57.2958;} //function to return angle in degrees when given x and y components
float mag(char  xval, char  yval){return sqrt(sq(xval)+sq(yval));} //function to return vector magnitude when given x and y components

void setup() {
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
  fixOffset();
  initAxes();
  
  maxVectors();
  if (!ucf)
  {
    dashBack();
    shieldDrop();    
  }
}

void fixOffset()
{
  report.xAxis -= x_offset;
  report.yAxis -= y_offset;
}

bool anyButtonActive()
{
  return (
    report.a == 1 ||
    report.b == 1 ||
    report.x == 1 ||
    report.y == 1 ||
    report.r == 1 ||
    report.l == 1 ||
    report.right > 74 ||
    report.left > 74 ||
    report.z == 1 ||
    cstick_x_abs > 50 ||
    cstick_y_abs > 50
    );
}

void dashBack(){
  if (anyButtonActive())
    return;
    
  if (analog_y_abs >= 23)
    return;

  if (analog_x_abs < 21)
  {
    buf = cycle;
    return;
  }
  
  if (buf > 0)
  {
    if (analog_x_abs < 68)// if > 68, no need to edit the value as we are dashing anyway
      report.xAxis = 128;
    buf--;
  }
}

void maxVectors()
{
  bool edited = false;
  if (analog_x_abs > 85 && analog_y_abs < 10)
  {
    report.xAxis = (analog_x > 0) ? 255 : 1;
    report.yAxis = 128;
    edited = true;
  }
  else if (analog_y_abs > 85 && analog_x_abs < 10)
  {
    report.yAxis  = (analog_y > 0) ? 255 : 1;
    report.xAxis  = 128;
    edited = true;
  }

  if (cstick_x_abs > 85 && cstick_y_abs < 10)
  {
    report.cxAxis = (cstick_x > 0) ? 255 : 1;
    report.cyAxis = 128;
    edited = true;
  }
  else if (cstick_y_abs > 85 && cstick_x_abs < 10)
  {
    report.cyAxis  = (cstick_y > 0) ? 255 : 1;
    report.cxAxis  = 128;
    edited = true;
  }
  if (edited == true)
    initAxes(); // could be cleaner but fuck it
}

void shieldDrop()
{
  shield = report.l || report.r || report.right > 74 || report.left > 74 || report.z;
  
  if (shield && analog_x_abs > 120)
    tiltedShield = true;
  else if (shield && tiltedShield && mag(analog_x, analog_y) > 80)
    tiltedShield = true;
  else
    tiltedShield = false;

  if (tiltedShield)
  {
    angle = ang(analog_x_abs, analog_y_abs);
    if (angle > 45 && angle < 55)
    {
      report.yAxis = 73;
      report.xAxis = (analog_x > 0 ? 183 : 73);
    }
  }
}

// Turns features off, sets dolphin or ucf mode
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
  else if (report.ddown == 1 && report.y == 1)
  {
    if (!hasCurrentInput)
      ucf = !ucf;
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
  if (!controller.read())
  {
    init_done = false;
    delay(100);
    return;
  }
  
  report = controller.getReport();
  if (!init_done)
  {
    x_offset = report.xAxis - 128;
    y_offset = report.yAxis - 128;  
    init_done = true;
  }

  checkInputs();
  cycle = dolphin ? 8 : 2;
  if (!off)
    executeFixes();

  if (!console.write(report))
    delay(10);
}
