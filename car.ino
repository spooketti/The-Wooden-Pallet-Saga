#define PI 3.1415926535897932384626433832795 // troll emoji

const int brakePort = 3;
const int directionPort = 2; //the direction the motor should spin
const int drivePort = 9;
const int speedPort = 12;
bool killSwitch = false;

// speed related
bool state = false;
bool deltaState = false; // the last known state before a state update
unsigned long lastReadTimeMS;
unsigned long elapsedTimeSec;
const unsigned long timeoutMS = 500; // if more than this amount of time w/o state change the wheel is not moving
float angularVelocityRadPerSec = 0;
const int polePairs = 90; // the weird 90 state change thing

//driving
int desiredDutyCycle = 0; //0-255 = 0-100%btw
bool direction = false; //i lowk have 0 clue how this works, will need hardware to tell

//deadman
float lastRecievedInput;
const unsigned long deadmanTimeoutMS = 1000;

void setup()
{
    float lastRecievedInput = millis();
    Serial.begin(9600);
}

void brake(int state)
{
    digitalWrite(brakePort, state);
}

void kill()
{
    killSwitch = true;
}

void wheelPeriodic()
{
    // there is a method to the choppery here
    // pin read here returns either 5V or 0V
    // this "state change" happens 90 times per rotation
    if (state != deltaState)
    {
        deltaState = state; // update state
        elapsedTimeSec = (millis() - lastReadTimeMS) / 1000.0;
        lastReadTimeMS = millis();
        int transitionsPerRev = 6 * polePairs;
        float frequency = 1.0 / (elapsedTimeSec * transitionsPerRev);
        angularVelocityRadPerSec = 2.0 * PI * frequency;
    }
    else if (millis() > timeoutMS)
    {
        angularVelocityRadPerSec = 0;
    }
}

void serialReader()
{
    if (Serial.available() > 0)
    {
        String data = Serial.readStringUntil('\n');
        switch (data)
        {
        case "KILL":
            kill();
            break;

        case "BRAKE":
            brake(HIGH);
            break;

        case "UNBREAK":
            brake(LOW);
            break;

        default:
            double joystickAmount = data.toDouble();
            desiredDutyCycle = (int)abs(joystickAmount*255);
            direction = joystickAmount > 0;
            break;
        }
        lastRecievedInput = millis();
    }
}

void drive()
{
    digitalWrite(directionPort, direction);
    analogWrite(drivePort, desiredDutyCycle);
}

void deadmanSwitch()
{
    if(millis() - lastRecievedInput > deadmanTimeoutMS) //if we haven't recieved an input assume the controls/computer/etc are dead
    {
        kill();
    }
}

void loop()
{
    if (killSwitch)
    {
        brake(HIGH);
        delay(20);
        return;
    }
    deadmanSwitch();
    serialReader();
    wheelPeriodic();
    drive();
    delay(20);
}