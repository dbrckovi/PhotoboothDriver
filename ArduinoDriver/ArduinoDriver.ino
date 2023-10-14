#define PWM_FORWARD 6
#define ON_BACKWARD 4
#define PWM_BACKWARD 5
#define ON_FORWARD 3
#define LED_PIN 7
#define FORWARD 1
#define BACKWARD 0

int set_speed = 0;          //speed at which the motor will spin when it's toggled
int acceleration_delay = 4; //how fast the motor is speeding up or slowing down
int sleep_time = 5;         //sleep time after motor is stopped (short circuit prevention)         
int current_speed = 0;      //current motor speed (0-255)
bool currently_on = false;  //is motor currnetly running 
bool going_forward = true;  //if motor is currently running, this holds the direction

void setup() {
  pinMode(PWM_FORWARD, OUTPUT);
  pinMode(ON_BACKWARD, OUTPUT);
  pinMode(PWM_BACKWARD, OUTPUT);
  pinMode(ON_FORWARD, OUTPUT);
  AllOff();
  Serial.begin(9600);
}

void loop() {

  if (Serial.available() > 0)
  {
    String command = Serial.readStringUntil(';');
    ExecuteCommand(command);
  }
}

void ExecuteCommand(String command)
{
  if (command == "onoff")
  {
    if (currently_on)
    {
      Brake();
      AllOff();
    }
    else
    {
      if (set_speed == 0) set_speed = 255;
      Go(going_forward, set_speed);
    }
  }
  else if (command == "chdir")
  {
    if (currently_on)
    {
      Go(!going_forward, current_speed);
    }
    else going_forward = !going_forward;
  }
  else if (command == "stop")
  {
    set_speed = 0;
    Brake();
    AllOff();
  }
  else if (command == "for_plus")
  {
    if (set_speed == 0) going_forward = true;
    if (going_forward) set_speed = IncrementSpeed(set_speed);
    else set_speed = DecrementSpeed(set_speed);

    if (set_speed == 0) 
    {
      Brake();
      AllOff();
    }
    else Go(going_forward, set_speed);
  }
  else if (command == "back_plus")
  {
    if (set_speed == 0) going_forward = false;
    if (!going_forward) set_speed = IncrementSpeed(set_speed);
    else set_speed = DecrementSpeed(set_speed);

    if (set_speed == 0) 
    {
      Brake();
      AllOff();
    }
    else Go(going_forward, set_speed);    
  }  
  else
  {
    Serial.print("Wrong command. Use: onoff; chdir; stop; for_plus; back_plus;");
  }
}

int IncrementSpeed(int oldSpeed)
{
  int ret = oldSpeed + 51;
  if (ret > 255) ret = 255;
  return ret;
}

int DecrementSpeed(int oldSpeed)
{
  int ret = oldSpeed - 51;
  if (ret < 0) ret = 0;
  return ret;
}

//Sets motor speed to specified diretion and speed, slowing down and stopping in between if neccesary
//Does not turn of the ON mosfets if speed is 0
void Go(bool should_go_forward, int speed)
{
  if (currently_on && going_forward != should_go_forward) Brake();    //if going in wrong direction, brake first

  currently_on = true;
  going_forward = should_go_forward;

  while (current_speed > speed)
  {
    current_speed--;
    SetMosfets();
    delay(acceleration_delay);
  }

  while (current_speed < speed)
  {
    current_speed++;
    SetMosfets();
    delay(acceleration_delay);
  }
}

//Stops the motor gradually, and turns off all mosfets 
void Brake()
{
  if (currently_on)
  {
    while (current_speed > 0)
    {
      current_speed--;
      SetMosfets();
      delay(acceleration_delay);
    }
  }
  AllOff();
}

//Sets mosfets according to variables.
//Caller must ensure a proper braking and delays in case mosfets would be reversed by this function
void SetMosfets()
{
  if (currently_on)
  {
    if (going_forward)
    {
      digitalWrite(PWM_BACKWARD, LOW);
      digitalWrite(ON_BACKWARD, LOW);
      digitalWrite(ON_FORWARD, HIGH);
      analogWrite(PWM_FORWARD, current_speed);
    }
    else
    {
      digitalWrite(PWM_FORWARD, LOW);
      digitalWrite(ON_FORWARD, LOW);
      digitalWrite(ON_BACKWARD, HIGH);
      analogWrite(PWM_BACKWARD, current_speed);
    }
  }
  else
  {
    digitalWrite(PWM_FORWARD, LOW);
    digitalWrite(ON_BACKWARD, LOW);
    digitalWrite(PWM_BACKWARD, LOW);    
    digitalWrite(ON_FORWARD, LOW);
  }
}

//Turns off all mosfets
void AllOff()
{
  currently_on = false;
  current_speed = 0;
  SetMosfets();
  delay(sleep_time);
}
