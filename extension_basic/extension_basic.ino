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

bool enableFixes = true;
bool dolphin = false;
bool ucf = false;

bool hasCurrentInput = false;
bool featureIsTurningOn = false;
bool featureIsTurningOff = false;
long feedbackLoopCount = 0;

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
  fixOffset(); // I fix offsets even when features are turned off because I feel like it is worse than without the arduino otherwise.
  if (executeFixes)
  {
    initAxes();
    maxVectors();
    if (!ucf)
    {
      dashBack();
      shieldDrop();    
    }
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

void giveUserFeedback()
{
  if (featureIsTurningOn || featureIsTurningOff)
  {
    short frameDuration = 12; 
    short frameCount = feedbackLoopCount / cycle;
   
    if (frameCount == 0 || frameCount == frameDuration * 2)
      controller.setRumble(true);
    else if (frameCount % frameDuration == 0)
    {
      controller.setRumble(false);
      if (featureIsTurningOn)
      {
        featureIsTurningOn = false;
        feedbackLoopCount = 0;
      }
      if (featureIsTurningOff && frameCount == frameDuration * 3)
      {
        featureIsTurningOff = false;
        feedbackLoopCount = 0;
      }
    }
    feedbackLoopCount++;
  }
}

void switchFeature(bool *ptr)
{
  if (!hasCurrentInput && !featureIsTurningOn && !featureIsTurningOff)
  {
    *ptr = !(*ptr);
    if (*ptr == true)
      featureIsTurningOn = true;
    else
      featureIsTurningOff = true;
  }
}

// Turns features off, sets dolphin or ucf mode
void checkInputs()
{
  if (report.ddown == 1 && report.b == 1)
  {
    switchFeature(&enableFixes);
    report.b = 0;
  }
  else if (report.ddown == 1 && report.a == 1)
  {
    switchFeature(&dolphin);
    report.a = 0;    
  }
  else if (report.ddown == 1 && report.y == 1)
  {
    switchFeature(&ucf);
    report.y = 0;    
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
    controller.getOrigin();

  checkInputs();
  giveUserFeedback();
  
  cycle = dolphin ? 8 : 2;
  if (enableFixes)
    executeFixes();

  if (!console.write(report))
    delay(10);
  
  // I feel like this works better AFTER sending data to the console at least once, but it's only a feeling. Feel free to tell me otherwise
  if (!init_done)
  {
    x_offset = report.xAxis - 128;
    y_offset = report.yAxis - 128;  
    init_done = true;
  }
}
