#include <Bluepad32.h>
const int brakeLeftPin = NULL;
const int brakeRightPin = NULL;
const int dirLeftPin = NULL;
const int dirRightPin = NULL;
const int speedLeftPin = NULL;
const int speedRightPin = NULL;

const int MAX_CONTROLLER = 512;
const double MAX_SPEED = 255;
const double MAX_SPEED_GROWTH = MAX_SPEED/(500);

double maxSpeed = 0;
double throttle = 0;
bool isReverse = false;
bool isBrake = false;

void drive(int PWM){
  analogWrite(speedLeftPin,PWM);
  analogWrite(speedRightPin,PWM);
}

void brake(){
  throttle = 0;
  drive(0);
  maxSpeed = 0;
}

ControllerPtr myControllers[BP32_MAX_GAMEPADS];

void onConnectedController(ControllerPtr ctl) {
    bool foundEmptySlot = false;
    for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
        if (myControllers[i] == nullptr) {
            Serial.printf("CALLBACK: Controller is connected, index=%d\n", i);
            ControllerProperties properties = ctl->getProperties();
            Serial.printf("Controller model: %s, VID=0x%04x, PID=0x%04x\n", ctl->getModelName().c_str(), properties.vendor_id,properties.product_id);
            myControllers[i] = ctl;
            foundEmptySlot = true;
            break;
        }
    }
    if (!foundEmptySlot) {
        Serial.println("CALLBACK: Controller connected, but could not found empty slot");
    }
}

void onDisconnectedController(ControllerPtr ctl) {
    bool foundController = false;

    for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
        if (myControllers[i] == ctl) {
            Serial.printf("CALLBACK: Controller disconnected from index=%d\n", i);
            myControllers[i] = nullptr;
            foundController = true;
            break;
        }
    }

    if (!foundController) {
        Serial.println("CALLBACK: Controller disconnected, but not found in myControllers");
    }
    isBrake = true;
}

void processGamepad(ControllerPtr ctl) {
    //  a(), b(), x(), y(), l1(), etc...
    if (ctl->a()) {
        isBrake = true;
        if(ctl->b()){
            isBrake = false;
            ctl->setColorLED(0,0,0);
            return;
        }
        if(isBrake){
            ctl->setColorLED(255, 0, 0);
        }
    }

    if(isBrake){
        brake();
        return;
    }
    
    throttle = map(max(ctl->axisY()*-1,0), 0, MAX_CONTROLLER, 0, 1);
    double stickNorm = constrain((double)(ctl->axisRY() * -1) / MAX_CONTROLLER, -1.0, 1.0);
    double change = stickNorm * MAX_SPEED_GROWTH;
    maxSpeed = constrain(maxSpeed + change, 0.0, MAX_SPEED);

    static int led = 0;
    led = (int)round((maxSpeed / (255/4))) % 4;

    if(maxSpeed>225){
        led = 4;
    }

    ctl->setPlayerLEDs(led & 0x0f);

    /*
    //ACTUALLY POWERS THE MOTOR
    drive(throttle*maxSpeed);
    */

    //ctl->playDualRumble(0 /* delayedStartMs */, 250 /* durationMs */, 0x80 /* weakMagnitude */,
    //                         0x40 /* strongMagnitude */);

    // Another way to query controller data is by getting the buttons() function.
    // See how the different "dump*" functions dump the Controller info.
    //  Serial.printf(
//         "idx=%d, dpad: 0x%02x, buttons: 0x%04x, axis L: %4d, %4d, axis R: %4d, %4d, brake: %4d, throttle: %4d, "
//         "misc: 0x%02x, gyro x:%6d y:%6d z:%6d, accel x:%6d y:%6d z:%6d\n",
//         ctl->index(),        // Controller Index
//         ctl->dpad(),         // D-pad
//         ctl->buttons(),      // bitmask of pressed buttons
//         ctl->axisX(),        // (-511 - 512) left X Axis
//         ctl->axisY(),        // (-511 - 512) left Y axis
//         ctl->axisRX(),       // (-511 - 512) right X axis
//         ctl->axisRY(),       // (-511 - 512) right Y axis
//         ctl->brake(),        // (0 - 1023): brake button
//         ctl->throttle(),     // (0 - 1023): throttle (AKA gas) button
//         ctl->miscButtons(),  // bitmask of pressed "misc" buttons
//         ctl->gyroX(),        // Gyro X
//         ctl->gyroY(),        // Gyro Y
//         ctl->gyroZ(),        // Gyro Z
//         ctl->accelX(),       // Accelerometer X
//         ctl->accelY(),       // Accelerometer Y
//         ctl->accelZ()        // Accelerometer Z
//     );
}

void processControllers() {
    for (auto myController : myControllers) {
        if (myController && myController->isConnected() && myController->hasData()) {
            if (myController->isGamepad()) {
                processGamepad(myController);
            }
        }
    }
}

void setup() {
    Serial.begin(115200);
    Serial.printf("Firmware: %s\n", BP32.firmwareVersion());
    const uint8_t* addr = BP32.localBdAddress();
    Serial.printf("BD Addr: %2X:%2X:%2X:%2X:%2X:%2X\n", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
    BP32.setup(&onConnectedController, &onDisconnectedController);

    // "forgetBluetoothKeys()" should be called when the user performs
    // a "device factory reset", or similar.
    // Calling "forgetBluetoothKeys" in setup() just as an example.
    // Forgetting Bluetooth keys prevents "paired" gamepads to reconnect.
    // But it might also fix some connection / re-connection issues.
    BP32.forgetBluetoothKeys();

    // Enables mouse / touchpad support for gamepads that support them.
    // When enabled, controllers like DualSense and DualShock4 generate two connected devices:
    // - First one: the gamepad
    // - Second one, which is a "virtual device", is a mouse.
    // By default, it is disabled.
    BP32.enableVirtualDevice(false);
}

void loop() {
    bool dataUpdated = BP32.update();
    if (dataUpdated)
        processControllers();

    // The main loop must have some kind of "yield to lower priority task" event.
    // Otherwise, the watchdog will get triggered.
    // If your main loop doesn't have one, just add a simple `vTaskDelay(1)`.
    // Detailed info here:
    // https://stackoverflow.com/questions/66278271/task-watchdog-got-triggered-the-tasks-did-not-reset-the-watchdog-in-time

    vTaskDelay(1);
    delay(5);
}
