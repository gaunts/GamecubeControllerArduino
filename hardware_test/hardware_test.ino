CGamecubeController controller(2);
CGamecubeConsole console(3);
Gamecube_Report_t report;

void setup() {
  // put your setup code here, to run once:

}

void loop() {
  if (!controller.read())
  {
    delay(100);
    return;
  }
  report = controller.getReport();
  if (!GamecubeConsole1.write(d))
  {
    delay(1);
  }
}
