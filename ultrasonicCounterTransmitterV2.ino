#include <Ultrasonic.h>
#include <VirtualWire.h>


const String ERR_NEG_NUM_PPL = "Error: negative number of people.";


const int TRIG_PIN_1 = 2; // trig pin of ultrasonic 1 (outside sensor)
const int ECHO_PIN_1 = 3; // echo pin of ultrasonic 1
const int TRIG_PIN_2 = 4; // trig pin of ultrasonic 2 (inside sensor)
const int ECHO_PIN_2 = 5; // echo pin of ultrasonic 2
const int SETUP_DELAY = 10000; // delay after setup to allow for mounting w/o reading people


const int INDICATOR_LED = 13;     // pin of LED that flashes to show wireless transmission
const int TX_PIN = 12;            // tx (data) pin of wireless transmitter
const int VW_BITS_PER_SEC = 2000; // bits per second of wireless transmitter
const int NUM_TIMES_SENT = 10;    // number of times data is sent (to prevent data loss)


enum Movement {
  MOVE_NONE = 0,   // neither side triggered
  MOVE_START = 1,  // only starting side triggered
  MOVE_CROSS = 2,  // both sides triggered
  MOVE_FINISH = 3, // side opposite of starting side triggered
  MOVE_DONE = 4    // only other side triggered and then no side triggered
};

// if object within MAX_TRIP_DISTANCE of sensor, sensor will be "triggered"
// found by taking distance which sensors could reliably detect objects
const int MAX_TRIP_DISTANCE = 33; 


Ultrasonic ultrasonic1( TRIG_PIN_1, ECHO_PIN_1 ); // outside sensor
Ultrasonic ultrasonic2( TRIG_PIN_2, ECHO_PIN_2 ); // inside sensor
long ultrasonic1Timing;  // time in ms for ultrasonic 1 (outside) to ping
long ultrasonic2Timing;  // time in ms for ultrasonic 2 (outside) to ping
bool ultrasonic1Tripped; // whether ultrasonic 1 (outside) detected a person
bool ultrasonic2Tripped; // whether ultrasonic 2 (outside) detected a person


Movement enteringStep; // stage of movement of entering object is at
Movement exitingStep;  // stage of movement of exiting object is at
int numPeople;         // num of people who have crossed to "inside" side

void setup() {

  // init Serial
  Serial.begin( 9600 );

  // init VirtualWire
  vw_set_ptt_inverted( true );
  vw_setup( VW_BITS_PER_SEC );
  vw_set_tx_pin( TX_PIN );
  

  // init variables
  enteringStep = MOVE_NONE;
  exitingStep = MOVE_NONE;
  numPeople = 0;

  // delay (to allow mounting/etc.)
  Serial.println( "post-setup() delay (to allow mounting/etc.)" );
  delay( SETUP_DELAY );
  
  Serial.println( "setup() done" );
} // end setup()


void loop() {

  // have each ultrasonic ping to determine the distance to the nearest object
  ultrasonic1Timing = ultrasonic1.timing();
  ultrasonic2Timing = ultrasonic2.timing();
  // from the ping times, check if each ultrasonic detects an object in front of it that isn't
  // noise
  ultrasonic1Tripped = ifTripped( ultrasonic1Timing, ultrasonic1 );
  ultrasonic2Tripped = ifTripped( ultrasonic2Timing, ultrasonic2 );


  if( enteringStep ) { // found object that started entering
    // continue to track object as if entering and mark if done entering
    enteringStep = toEnteringMovement( ultrasonic1Tripped, ultrasonic2Tripped, enteringStep );
    if( enteringStep == MOVE_DONE ) {
      enteringStep = MOVE_NONE;
      transmit( ++numPeople );
    }
    
  } else if( exitingStep ) { // found object that started exiting
    // continue to track object as if exiting and mark if done exiting
    exitingStep = toExitingMovement( ultrasonic1Tripped, ultrasonic2Tripped, exitingStep );
    if( exitingStep == MOVE_DONE ) {
      exitingStep = MOVE_NONE;
      if( numPeople > 0 ) { // note: number of people in room cannot be below 0
        transmit( --numPeople );
      } else {
        Serial.println( ERR_NEG_NUM_PPL );
      }
    }
    
  } else { // no object (entering or exiting) detected; look for one

    // check if object begins on outside (check if object crosses to inside aka entering)
    if( ultrasonic1Tripped && !ultrasonic2Tripped ) {
      enteringStep = MOVE_START;
      
      // check if object begins on inside (check if object crosses to outside aka exiting)
    } else if( !ultrasonic1Tripped && ultrasonic2Tripped ) {
      exitingStep = MOVE_START;
    }
  }
} // end loop()


// method that takes the time for an ultrasonic to ping and determines if what it pinged off of was
// close enough to be certain that there was, indeed, an object and not just noise
bool ifTripped( long timing, Ultrasonic ultrasonic ) {
  
  if( ultrasonic.CalcDistance( ultrasonic.timing(), Ultrasonic::CM ) < MAX_TRIP_DISTANCE ) {
    return true;
    
  } // else
  return false;
  
} // end function ifTripped


// function that describes entering as a Movement based on which sensors are tripped and the 
// previous Movement
Movement toEnteringMovement( bool outsideTripped, bool insideTripped, Movement previousMovement ) {
  if( outsideTripped ) {
    if( insideTripped ) { 
      return MOVE_CROSS; // outsideTripped and insideTripped
    }
    return MOVE_START; // outsideTripped and !insideTripped
  }
  if( insideTripped ) { 
    return MOVE_FINISH; // not outsideTripped and insideTripped
  }
  if( previousMovement == MOVE_FINISH ) {
    return MOVE_DONE; // not outsideTripped and not insideTripped and finishing entering
  }  
  return MOVE_NONE; // not outsideTripped and not insideTripped and finishing entering
  
} // end function toEnteringMovement


// function that describes exiting as a Movement based on which sensors are tripped and the 
// previous Movement
Movement toExitingMovement( bool outsideTripped, bool insideTripped, Movement previousMovement ) {
  return toEnteringMovement( insideTripped, outsideTripped, previousMovement );
}


// function that transmits a given integer using the VirtualWire library
void transmit( int integer ) {
  int integerAsArray[ 1 ] = { integer }; // VirtualWire is designed to use arrays
  
  for( int i = 0; i < NUM_TIMES_SENT; i++ ) {
    digitalWrite( INDICATOR_LED, HIGH ); // flash the indicator LED as data is sent
    vw_send( (uint8_t *)integerAsArray, sizeof( integerAsArray ) );
    vw_wait_tx();
    digitalWrite( INDICATOR_LED, LOW ); // flash the indicator LED as data is sent
    Serial.println( integer ); // display the number of people to debug
  }
} // end function transmit
