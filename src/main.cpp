/*----------------------------------------------------------------------------*/
/*                                                                            */
/*    Module:       main.cpp                                                  */
/*    Author:       C:\Users\Jess                                             */
/*    Created:      Mon Jul 12 2021                                           */
/*    Description:  V5 project                                                */
/*                                                                            */
/*----------------------------------------------------------------------------*/

// ---- START VEXCODE CONFIGURED DEVICES ----
// Robot Configuration:
// [Name]               [Type]        [Port(s)]
// rf                   motor         16              
// rb                   motor         17              
// lf                   motor         18              
// lb                   motor         19              
// Controller1          controller                    
// tilter               motor         11              
// mogo                 motor         15              
// lift                 motor         6               
// ---- END VEXCODE CONFIGURED DEVICES ----

#include "vex.h"
using namespace vex;

// A global instance of competition
competition Competition;

///
// Globals
//  - it's good practice to name globals in all caps
//  - we also try to avoid "hard coding" number.  use variables wherever
//  - you can, so you can change things when you eventually need to
///
const int  RESET_TIMEOUT     = 3000; // Time for resetting sensors to give up
bool       WAS_RESET_SUCCESS = false; // Flag for if resetting worked

const float SCALE      =  12000/127; // Makes input out of 127 instead of 12000
const int   THRESH     =  5;         // When joystick is within this, the motors will set to 0.  This is the deadzone
const int   DELAY_TIME =  10;        // Delay time for loops (ms)
const int   MOGO_OUT   =  490;       // Out position for the mogo lift
const int   MOGO_IN    =  0;       // In position for mogo lift
const int   TILTER_OUT = -600;       // Out position for the tilter
const int   TILTER_IN  =  -220;         // In position for tilter

// Lift constants
/*
const int NUM_OF_POS = 4; // Amount of heights
const int LIFT_HEIGHTS[NUM_OF_POS] = {0, 620, 1000, 1800}; // Lift positions, from lowest to highest
const int UP_SPEED   = 100;
const int DOWN_SPEED = 100;
*/
const int NUM_OF_POS = 3; // Amount of heights
const int LIFT_HEIGHTS[NUM_OF_POS] = {0, 620, 1000}; // Lift positions, from lowest to highest
const int UP_SPEED   = 100;
const int DOWN_SPEED = 100;


///
// Set Motor Functions
//  - this sets motors between -12000 and 12000.  i'm used to
//  - -127 to 127, so the "scale" variable lets me use that as
//  - inputs and scales it to -12000 to 12000
///

// Set voltage
void set_tank(int l, int r) {
  lf.spin(fwd, l*SCALE, voltageUnits::mV);
  lb.spin(fwd, l*SCALE, voltageUnits::mV);
  rb.spin(fwd, r*SCALE, voltageUnits::mV);
  rf.spin(fwd, r*SCALE, voltageUnits::mV);
}
void brake_drive() {
  lf.setStopping(hold);
  lb.setStopping(hold);
  rf.setStopping(hold);
  rb.setStopping(hold);
}
void coast_drive() {
  lf.setStopping(coast);
  lb.setStopping(coast);
  rf.setStopping(coast);
  rb.setStopping(coast);
}
void set_mogo  (int input) { mogo.  spin(fwd, input*SCALE, voltageUnits::mV); }
void set_tilter(int input) { tilter.spin(fwd, input*SCALE, voltageUnits::mV); }
void set_lift  (int input) { lift.  spin(fwd, input*SCALE, voltageUnits::mV); }

// Set position
void set_mogo_position  (int pos, int speed) { mogo.  startRotateTo(pos, rotationUnits::deg, speed, velocityUnits::pct); }
void set_tilter_position(int pos, int speed) { tilter.startRotateTo(pos, rotationUnits::deg, speed, velocityUnits::pct); }
void set_lift_position  (int pos, int speed) { lift.  startRotateTo(pos, rotationUnits::deg, speed, velocityUnits::pct); 

lift.rotateFor(fwd, 2, deg, speed, velocityUnits::pct, false);
}



///
// Reset Sensors
//  - we need all mechanisms start in a consistent place so we can move relatively off of
//  - the starting position.  this function moves all mechanisms into their hardstops, and 
//  - stops once the velocity of the mechanism is 0 (the mechanism has hit the hardstop)
///
void zero_sensors() {
  bool run = true;
  bool tilter_zero = false;
  bool mogo_zero   = false;
  bool lift_zero   = false;

  bool last_tilter = false;
  bool last_mogo   = false;
  bool last_lift   = false;

  int timeout_timer = 0;

  set_tilter(50);
  set_mogo(0);
  set_lift(-40);

  wait(200, msec);
  while (run) {
    last_tilter = tilter_zero;
    last_mogo   = mogo_zero;
    last_lift   = lift_zero;

    // Bring tilter back until velocity is 0
    if (tilter.velocity(percentUnits::pct) == 0) {
      set_tilter(0);
      tilter_zero = true;
      tilter.resetPosition();
    }

    // Bring mogo back until velocity is 0
    if (mogo.velocity(percentUnits::pct) == 0) {
      set_mogo(0);
      mogo_zero = true;
      mogo.resetPosition();
    }

    // Bring lift down until velocity is 0
    if (lift.velocity(percentUnits::pct) == 0) {
      set_lift(0);
      lift_zero = true;
      lift.resetPosition();
    }

    // Timeout 
    if (tilter_zero && mogo_zero && lift_zero) {
      run = false;
    }

    // Check if any of the subsystems have reset, and if they have, reset timeout_timer to 0
    if (last_tilter!=tilter_zero || last_mogo!=mogo_zero || last_lift!=lift_zero) {
      timeout_timer = 0;
    } else {
      // Once timeout_timer is greater then RESET_TIMER, exit this loop
      timeout_timer+=DELAY_TIME;
      if (timeout_timer>RESET_TIMEOUT) {
        run = false;
      }
    }

    wait(DELAY_TIME, msec); // Don't hog the CPU!
  }
  // Make sure no motors are running coming out of this loop
  set_tilter(0);
  set_mogo  (0);
  set_lift  (0);

  // If a mechanism didn't zero, reset it here
  if (!tilter_zero) tilter.resetPosition();
  if (!mogo_zero)   mogo.  resetPosition();
  if (!lift_zero)   lift.  resetPosition();

  //tell the rest of the code that pre_auton finished
  WAS_RESET_SUCCESS = true;
}

///
//
///
void
auton() {
  set_tilter_position(TILTER_OUT, 100);
  set_tank(127, 127);
  wait(800, msec);
  set_tank(0, 0);
}

/*---------------------------------------------------------------------------*/
/*                          Pre-Autonomous Functions                         */
/*                                                                           */
/*  You may want to perform some actions before the competition starts.      */
/*  Do them in the following function.  You must return from this function   */
/*  or the autonomous and usercontrol tasks will not be started.  This       */
/*  function is only called once after the V5 has been powered on and        */
/*  not every time that the robot is disabled.                               */
/*---------------------------------------------------------------------------*/

void pre_auton(void) {
  // Initializing Robot Configuration. DO NOT REMOVE!
  vexcodeInit();

  // All activities that occur before the competition starts
  // Example: clearing encoders, setting servo positions, ...
  zero_sensors();
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*                              Autonomous Task                              */
/*                                                                           */
/*  This task is used to control your robot during the autonomous phase of   */
/*  a VEX Competition.                                                       */
/*                                                                           */
/*  You must modify the code to add your own robot specific commands here.   */
/*---------------------------------------------------------------------------*/

void autonomous(void) {
  // ..........................................................................
  // Insert autonomous user code here.
  // ..........................................................................
   //checks if pre_auton ran and if did not run pre_auto
  if(WAS_RESET_SUCCESS == false){
    zero_sensors();
  }
  else{ 
    //nothing needed but makes code look cleaner 
  }
  brake_drive();
  auton();
  wait(500, msec);
  coast_drive();
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*                              User Control Task                            */
/*                                                                           */
/*  This task is used to control your robot during the user control phase of */
/*  a VEX Competition.                                                       */
/*                                                                           */
/*  You must modify the code to add your own robot specific commands here.   */
/*---------------------------------------------------------------------------*/

void usercontrol(void) {
  // User control code here, inside the loop
  // Parameters for user control
  bool tilter_up;
  int tilter_lock = 0;
  int tilter_timer = 0;
  bool down = false;

  bool mogo_up  = true;
  int mogo_lock = 0;
  bool is_up = true;

  int up_lock    = 0;
  int down_lock  = 0;
  int lift_state = 0;
  int lift_speed = UP_SPEED;

  tilter_up = true;

  //checks if pre_auton ran and if did not run pre_auto
  if(WAS_RESET_SUCCESS == false){
    zero_sensors();
  }
  else{ 
    //nothing needed but makes code look cleaner 
  }
  

  mogo.resetPosition();
  while (1) {
    // This is the main execution loop for the user control program.
    // Each time through the loop your program should update motor + servo
    // values based on feedback from the joysticks.

    ///
    // Joysticks
    ///

    // Ternary operator (one line if statement) for controller deadzone
    // int x = some condition   ?   x is this if condition was true   :   x is this if condition was false;
    int l_joy = abs(Controller1.Axis3.value())>THRESH ? Controller1.Axis3.value() : 0;
    int r_joy = abs(Controller1.Axis2.value())>THRESH ? Controller1.Axis2.value() : 0;
    set_tank(l_joy, r_joy);



    ///
    // Tilter
    //  - tilter has two positions, pressing L2 toggles it between them
    ///
  // Flip boolean when button is pressed
    if (Controller1.ButtonL2.pressing() && tilter_lock==0) {
      if (down)
        tilter_up = false;
      else
        tilter_up = !tilter_up;
      tilter_lock = 1;
      down=false;

      // tilter_lock makes it so this if statement only runs once when the button is pressed
      // if this lock didn't exist, while the button is pressed, tilter_up would spam between
      // true and false, causing the mechanism to jitter
    } 
    else if (Controller1.ButtonL2.pressing()) {
      tilter_timer+=DELAY_TIME;
      if (tilter_timer>300) {
        down=true;
      }
    }
    else if (!Controller1.ButtonL2.pressing()) {
      tilter_lock = 0;
      tilter_timer=0;
    }

    // Have the motor go to a position depending on boolean state
    if (down)
      set_tilter_position(-940, 100);
    else if (tilter_up) 
      set_tilter_position(TILTER_IN, 100);
    else  //tilter_up == false
      set_tilter_position(TILTER_OUT, 100);
    

    ///
    // Mogo
    //  - mogo has two positions, pressing L1 toggles it between them
    ///

    // Flip boolean when button is pressed
    if (Controller1.ButtonL1.pressing() && mogo_lock==0) {
      mogo_up = !mogo_up;
      mogo_lock = 1;
      // mogo_lock makes it so this if statement only runs once when the button is pressed
      // if this lock didn't exist, while the button is pressed, mogo_up would spam between
      // true and false, causing the mechanism to jitter
    } 
    else if (!Controller1.ButtonL1.pressing()) {
      mogo_lock = 0;
    }

    // Have the motor go to a position depending on boolean state.
    // This runs the motor at full power until the velocity of the motor is 0.
    // when the velocity is 0, we know the subsystem has reached a hardstop.
    // then it sets the motor to a low amount of power to stop it from overheating
    if (mogo_up) {
      if (mogo.rotation(deg)<150) {
        if (mogo.velocity(pct)==0) {
          is_up = true;
          set_mogo(0);
        }
        else {
          set_mogo(is_up?0:-30);
        }
      }
      else {
        is_up = false;
        set_mogo(-127);
      }
    }
    else {
      if (mogo.rotation(deg)>MOGO_OUT-100) {
        if (mogo.velocity(pct)==0) 
          set_mogo(0);
        else 
          set_mogo(30);
      }
      else {
        set_mogo(127);
      }
    }

    /*
    // Have the motor go to a position depending on boolean state.
    // This runs the motor at full power until the velocity of the motor is 0.
    // when the velocity is 0, we know the subsystem has reached a hardstop.
    // then it sets the motor to a low amount of power to stop it from overheating
    if (mogo_up) {
      if (mogo.rotation(deg)<150) {
        if (mogo.velocity(pct)==0) {
          is_up = true;
          set_mogo(0);
        }
        else {
          set_mogo(is_up?0:-50);
        }
      }
      else {
        is_up = false;
        set_mogo(-127);
      }
    }
    else {
      if (mogo.rotation(deg)>MOGO_OUT-100) {
        if (mogo.velocity(pct)==0) 
          set_mogo(0);
        else 
          set_mogo(30);
      }
      else {
        set_mogo(127);
      }
    }
    */



    ///
    // Lift
    //  - the lift has multiple positions, R1 brings the lift up in the ladder,
    //  - R2 brings it down in the ladder.
    ///
    
    // Ladder up, R1
    if (Controller1.ButtonR1.pressing() && up_lock==0) {
      // If the ladder is at the top, bring it down
      if (lift_state==NUM_OF_POS-1)  {
        lift_state = 0;
        lift_speed = DOWN_SPEED;
      }
      // Otherwise, bring ladder up
      else {
        lift_state++;
        lift_speed = UP_SPEED;
      }
      up_lock = 1;
    }
    else if (!Controller1.ButtonR1.pressing()) {
      up_lock = 0;
    }

    // Ladder down, R2
    if (Controller1.ButtonR2.pressing() && down_lock==0) {
      // If the ladder is at the bottom, bring it up
      if (lift_state==0) {
        lift_state = NUM_OF_POS-1;
        lift_speed = UP_SPEED;
      }
      // Otherwise, bring it down
      else {
        lift_state--;
        lift_speed = DOWN_SPEED;
      }
      down_lock = 1;
    }
    else if (!Controller1.ButtonR2.pressing()) {
      down_lock = 0;
    }
    // Set the lift position to the height array defined at the top of the code
    set_lift_position(LIFT_HEIGHTS[lift_state], lift_speed);
    

    wait(DELAY_TIME, msec); // Don't hog the CPU!
  }
}

//
// Main will set up the competition functions and callbacks.
//
int main() {
  // Set up callbacks for autonomous and driver control periods.
  Competition.autonomous(autonomous);
  Competition.drivercontrol(usercontrol);

  // Run the pre-autonomous function.
  pre_auton();

  // Prevent main from exiting with an infinite loop.
  while (true) {
    wait(100, msec);
  }
}