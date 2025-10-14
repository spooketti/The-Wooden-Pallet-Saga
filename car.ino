
void setup() {
 Serial.begin(9600);
}

void loop()
{
    if(Serial.available() > 0)
    {
        String data = Serial.readStringUntil('\n');
        switch(data)
        {
            case "KILL":
            break;

            case "BREAK":
            break;

            case "UNBREAK":
            break;

            default:
            break;
        }
    }
}