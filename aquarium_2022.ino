
//--------------------- Work forwards on the nano
int sensorFlow=2;       // Flow sensor
int sensor1=3;          // Main tank float sensor
int sensor2=4;          // Sump tank float sensor

//------------------------ Work Backwards

int valveOn=11;         // relay for valve
int valveOff=10;        // relay for valve
int pump=9;             // relay for pump
int lowVolt=8;          // relay to turn on/off 12v transformer

//------------------------ Set the states
int sensorState1= LOW;  // reads pushbutton status 
int sensorState2= LOW;  // reads pushbutton status 
bool pumpState=false;
bool valveState=false;
bool flowState=true;

//------------------------ Other variables
unsigned int  pulseCount=0;
unsigned long oldTime=millis();
unsigned long eventTime=0;
unsigned int  deltaTime=0;
const int spinDownTime=10000;      // Number of seconds to wait until the flow sensor spins down. If still spinning, close valve


const float   flowPerPulse=2.25;   // NOTE: This is specific to the flowmeter I bought.
float         vph=0;               // Volume of water per hour
float         gph=0;               // Convert to gallons per hour (for possible future use)

void setup() 
{ 
    //------------------------
    // Set the assorted pins
    //------------------------
    Serial.begin(9600); 
    pinMode(sensorFlow, INPUT);     // The flow sensor, set to input
    pinMode(sensor1, INPUT_PULLUP); // Float Sensor - Arduino Internal Resistor 10K
    pinMode(sensor2, INPUT_PULLUP); // Float Sensor - Arduino Internal Resistor 10K

    pinMode(pump,    OUTPUT);       // pump relay 
    pinMode(valveOn, OUTPUT);       // valve on relay
    pinMode(valveOff,OUTPUT);       // valve off relay 
    pinMode(lowVolt, OUTPUT);       // 12v transformeter on/off relay 

    //--------------------------------------------------------------
    // Actions - Input sensors
    //--------------------------------------------------------------
    digitalWrite(sensorFlow, HIGH);
    digitalWrite(valveOn, HIGH);
    digitalWrite(valveOff, HIGH);

    //--------------------------------------------------------------
    // Actions - Handling relays
    //--------------------------------------------------------------
    // Serial.println("Setup - Turning valve on");
    openValve();

    // Serial.println("Setup - Turning pump on");
    turnOnPump();

    //--------------------------------------------------------------
    // Finally, attach interup to flow sensor
    //--------------------------------------------------------------
    attachInterrupt(0, pulseCounter, FALLING);  // attach routine pulseCount() to interupt 0
} 


//----------------------------------------------
// These routines should only be called by the
// routines that turn the valve on/off
//----------------------------------------------
void turnOn12V() {
    digitalWrite(lowVolt,   LOW);
    delay(1000);
}

void turnOff12V() {
    digitalWrite(lowVolt,   HIGH);
    delay(1000);
}


void turnOnPump() {
    digitalWrite(pump,  LOW);
    pumpState=true;
    eventTime=0;
}

void turnOffPump() {
    digitalWrite(pump,  HIGH);
    pumpState=false;
    eventTime=millis();
}



void openValve() {
    turnOn12V();
    digitalWrite(valveOn,   LOW);                 // .. and make sure this relay is on
    delay(10000);
    digitalWrite(valveOn,   HIGH);
    turnOff12V();
    valveState=true;
    eventTime=0;
}

void closeValve() {
    turnOn12V();
    digitalWrite(valveOff,  LOW);                 
    delay(10000);
    digitalWrite(valveOff,   HIGH);
    turnOff12V();
    valveState=false;
    eventTime=0;
}

void pulseCounter() {
  pulseCount++;
}


void loop() 
{ 
    deltaTime=millis()-oldTime;
    if( deltaTime >= 5000 )  {    // Every 'x' milliseconds
   
        detachInterrupt(0);                                                  // Stop unpredicable crap from happening while we are trying to figure out what's going on
        oldTime=millis();                                                    // What is today but yesterday's tomorrow ?
        
        sensorState1 = digitalRead(sensor1); 
        if (sensorState1 == LOW) 
        { 
            if (!pumpState) {
                // Serial.println( "WATER LEVEL on Main Tank - OK"); 
                turnOnPump();
            }
        } 
        else 
        { 
            if (pumpState) {
                // Serial.println( "WATER LEVEL on Main Tank - WARNING" ); 
                turnOffPump();
            }
        } 

        sensorState2 = digitalRead(sensor2); 
        if (sensorState2 == LOW) 
        { 
            if (!valveState) {
                // Serial.println( "WATER LEVEL on Sump - OK"); 
                openValve();
            }
        } 
        else 
        { 
            if (valveState) {
                // Serial.println( "WATER LEVEL on Sump - WARNING" ); 
                closeValve();
            }
        } 

        //-------------------------------------------------------------------------------------
        // Check the flow - pulse usually hangs around 235-237 (when doing this for 5 seconds)
        //-------------------------------------------------------------------------------------
        vph = pulseCount * flowPerPulse * 12 * 60;   // how much water (in ml) passed in 'x' seconds
        gph= vph / 3785;                             // Convert to gallons
        if (gph > 10) flowState=true;
        else          flowState=false;

  
        if (!pumpState && (millis() - eventTime > spinDownTime) ) {
            if ( flowState ) {                       // we are still flowing after 'x' seconds, then...
                 closeValve();      
            }
        }
        
        pulseCount = 0;
        //--------------------------------------------
        attachInterrupt(0, pulseCounter, FALLING);
    }    
}
