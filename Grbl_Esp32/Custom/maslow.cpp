/*
	custom_code_template.cpp (copy and use your machine name)
	Part of Grbl_ESP32

	copyright (c) 2020 -	Bart Dring. This file was intended for use on the ESP32

  ...add your date and name here.

	CPU. Do not use this with Grbl for atMega328P

	Grbl_ESP32 is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	Grbl_ESP32 is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Grbl.  If not, see <http://www.gnu.org/licenses/>.

	=======================================================================

This is a template for user-defined C++ code functions.  Grbl can be
configured to call some optional functions, enabled by #define statements
in the machine definition .h file.  Implement the functions thus enabled
herein.  The possible functions are listed and described below.

To use this file, copy it to a name of your own choosing, and also copy
Machines/template.h to a similar name.

Example:
Machines/my_machine.h
Custom/my_machine.cpp

Edit machine.h to include your Machines/my_machine.h file

Edit Machines/my_machine.h according to the instructions therein.

Fill in the function definitions below for the functions that you have
enabled with USE_ defines in Machines/my_machine.h

===============================================================================

*/


TLC59711 tlc(NUM_TLC59711, TLC_CLOCK, TLC_DATA);

esp_adc_cal_characteristics_t *adc_1_characterisitics = (esp_adc_cal_characteristics_t*) calloc(1, sizeof(esp_adc_cal_characteristics_t));

MotorUnit axis1(&tlc, MOTOR_1_FORWARD, MOTOR_1_BACKWARD, MOTOR_1_ADC, RSENSE, adc_1_characterisitics, MOTOR_1_CS, DC_TOP_LEFT_MM_PER_REV, 1);
MotorUnit axis2(&tlc, MOTOR_2_FORWARD, MOTOR_2_BACKWARD, MOTOR_2_ADC, RSENSE, adc_1_characterisitics, MOTOR_2_CS, DC_TOP_LEFT_MM_PER_REV, 1);
MotorUnit axis3(&tlc, MOTOR_3_FORWARD, MOTOR_3_BACKWARD, MOTOR_3_ADC, RSENSE, adc_1_characterisitics, MOTOR_3_CS, DC_TOP_LEFT_MM_PER_REV, 1);
MotorUnit axis4(&tlc, MOTOR_4_FORWARD, MOTOR_4_BACKWARD, MOTOR_4_ADC, RSENSE, adc_1_characterisitics, MOTOR_4_CS, DC_TOP_LEFT_MM_PER_REV, 1);

float maslowWidth;
float maslowHeight;
float beltEndExtension;
float armLength;

bool axis1Homed;
bool axis2Homed;
bool axis3Homed;
bool axis4Homed;
bool calibrationInProgress;  //Used to turn off regular movements during calibration


#ifdef USE_MACHINE_INIT
/*
machine_init() is called when Grbl_ESP32 first starts. You can use it to do any
special things your machine needs at startup.
*/
void machine_init()
{
    tlc.begin();
    
    Serial.println("Testing reading from encoders: ");
    axis1.testEncoder();
    axis2.testEncoder();
    axis3.testEncoder();
    axis4.testEncoder();
    
    axis1.zero();
    axis2.zero();
    axis3.zero();
    axis4.zero();
    
    axis1Homed = false;
    axis2Homed = false;
    axis3Homed = false;
    axis4Homed = false;
    
    maslowWidth = 2900.0;
    maslowHeight = 1780.0;
    beltEndExtension = 30;
    armLength = 114;
}
#endif

void recomputePID(){
    //Stop everything but keep track of the encoder positions if we are idle or alarm. Unless doing calibration.
    if((sys.state == State::Idle || sys.state == State::Alarm) && !calibrationInProgress){
        axis1.stop();
        axis1.updateEncoderPosition();
        axis2.stop();
        axis2.updateEncoderPosition();
        axis3.stop();
        axis3.updateEncoderPosition();
        axis4.stop();
        axis4.updateEncoderPosition();
    }
    else{  //Position the axis
        axis1.recomputePID();
        axis2.recomputePID();
        axis3.recomputePID();
        axis4.recomputePID();
    }
}

//Upper left belt
float computeL1(float x, float y){
    x = x + maslowWidth/2.0;
    y = (maslowHeight/2.0) - y;
    return sqrt(x*x+y*y) - (beltEndExtension+armLength);
}

//Upper right belt
float computeL2(float x, float y){
    x = x + maslowWidth/2.0;
    y = maslowHeight/2.0 - y;
    return sqrt((maslowWidth-x)*(maslowWidth-x)+y*y) - (beltEndExtension+armLength);
}

//Lower right belt
float computeL3(float x, float y){
    x = x + maslowWidth/2.0;
    y = maslowHeight/2.0 - y;
    return sqrt((maslowWidth-x)*(maslowWidth-x)+(maslowHeight-y)*(maslowHeight-y)) - (beltEndExtension+armLength);
}

//Lower left belt
float computeL4(float x, float y){
    x = x + maslowWidth/2.0;
    y = maslowHeight/2.0 - y;
    return sqrt((maslowHeight-y)*(maslowHeight-y)+x*x) - (beltEndExtension+armLength);
}

void setTargets(float xTarget, float yTarget, float zTarget){
    
    if(!calibrationInProgress){
        axis1.setTarget(computeL1(xTarget, yTarget));
        axis2.setTarget(computeL2(xTarget, yTarget));
        axis3.setTarget(computeL3(xTarget, yTarget));
        axis4.setTarget(computeL4(xTarget, yTarget));
    }
}


//Runs the calibration sequence to determine the machine's dimensions
void runCalibration(){
    
    grbl_sendf(CLIENT_ALL, "Begining calibration\n");
    
    calibrationInProgress = true;
    
    //This is a bit of a hack, but it undoes any calls by the system to move these to (0,0)
    axis1.setTarget(computeL4(0, -200));
    axis2.setTarget(computeL3(0, -200));
    axis3.setTarget(computeL2(0, -400));
    axis4.setTarget(computeL1(0, -400));
    
    
    float lengths1[4];
    takeMeasurement(lengths1);
    
    grbl_sendf(CLIENT_ALL, "First measurement:\n%f \n%f \n%f \n%f \n",lengths1[0], lengths1[1], lengths1[2], lengths1[3]);
    
    moveUp(computeL1(0, 400));
    
    float lengths2[4];
    takeMeasurement(lengths2);
    
    grbl_sendf(CLIENT_ALL, "Second measurement:\n%f \n%f \n%f \n%f \n",lengths2[0], lengths2[1], lengths2[2], lengths2[3]);
    
    float machineDimensions [2];
    
    computeFrameDimensions(lengths1, lengths2, machineDimensions);
    
    calibrationInProgress = false;
    grbl_sendf(CLIENT_ALL, "Calibration finished\n");
    
    axis1.stop();
    axis2.stop();
    axis3.stop();
    axis4.stop();
}

//Retract the lower belts until they pull tight and take a measurement
void takeMeasurement(float lengths[]){
    
    bool axis1Done = false;
    bool axis2Done = false;
    
    while(!axis1Done || !axis2Done){  //As long as one axis is still pulling
        
        
        //If any of the current values are over the threshold then stop and exit, otherwise pull each axis a little bit tighter by incrementing the target position
        int currentThreshold = 2;
        
        if(axis1.getCurrent() > currentThreshold || axis1Done){
            axis1Done = true;
            //grbl_sendf(CLIENT_ALL, "Axis 1 declaring stopping");
        }
        else{
            axis1.setTarget(axis1.getTarget() - .2);
        }
        
        if(axis2.getCurrent() > currentThreshold || axis2Done){
            axis2Done = true;
            //grbl_sendf(CLIENT_ALL, "Axis 2 declaring stopping");
        }
        else{
            axis2.setTarget(axis2.getTarget() - .2);
        }
        
        // Delay without blocking
        unsigned long time = millis();
        unsigned long elapsedTime = millis()-time;
        while(elapsedTime < 50){
            elapsedTime = millis()-time;
            recomputePID();  //This recomputes the PID four all four servos
        }
    }
    
    axis1.setTarget(axis1.getPosition());
    axis2.setTarget(axis2.getPosition());
    axis3.setTarget(axis3.getPosition());
    axis4.setTarget(axis4.getPosition());
    
    axis1.stop();
    axis2.stop();
    axis3.stop();
    axis4.stop();
    
    lengths[0] = axis1.getPosition()+beltEndExtension+armLength;
    lengths[1] = axis2.getPosition()+beltEndExtension+armLength;
    lengths[2] = axis3.getPosition()+beltEndExtension+armLength;
    lengths[3] = axis4.getPosition()+beltEndExtension+armLength;
    
    return;
}

//Reposition the sled higher without knowing the machine dimensions
void moveUp(float targetLength){
    
    //The distance we need to move is the current position minus the target position
    double distToRetract = axis3.getPosition() - targetLength;
    
    //Make the lower arms compliant and move retract the other two until we get to the target distance
    
    unsigned long timeLastMoved1 = millis();
    unsigned long timeLastMoved2 = millis();
    double lastPosition1 = axis1.angleSensor->getRotation();
    double lastPosition2 = axis2.angleSensor->getRotation();
    double amtToMove1 = 0.1;
    double amtToMove2 = 0.1;
    
    while(distToRetract > 0){
        
        //Set the lower axis to be compliant. PID is recomputed in comply()
        axis1.comply(&timeLastMoved1, &lastPosition1, &amtToMove1);
        axis2.comply(&timeLastMoved2, &lastPosition2, &amtToMove2);
        
        //Pull in on the upper axis
        axis3.setTarget(axis3.getTarget() - .05);
        axis4.setTarget(axis4.getTarget() - .05);
        axis3.recomputePID();
        axis4.recomputePID();
        distToRetract = distToRetract - .05;
        
        // Delay without blocking
        unsigned long time = millis();
        unsigned long elapsedTime = millis()-time;
        while(elapsedTime < 10){
            elapsedTime = millis()-time;
        }
    }
    
    axis1.setTarget(axis1.getPosition());
    axis2.setTarget(axis2.getPosition());
    axis3.setTarget(axis3.getPosition());
    axis4.setTarget(axis4.getPosition());
    
    axis1.stop();
    axis2.stop();
    axis3.stop();
    axis4.stop();
}

float computeVertical(float firstUpper, float firstLower, float secondUpper, float secondLower){
    //Derivation at https://math.stackexchange.com/questions/4090346/solving-for-triangle-side-length-with-limited-information
    
    float b = secondUpper;   //upper, second
    float c = secondLower; //lower, second
    float d = firstUpper; //upper, first
    float e = firstLower;  //lower, first

    float aSquared = (((b*b)-(c*c))*((b*b)-(c*c))-((d*d)-(e*e))*((d*d)-(e*e)))/(2*(b*b+c*c-d*d-e*e));

    float a = sqrt(aSquared);

    Serial.println("Measured vertical: ");
    Serial.println(a);
    
    return a;
}

void computeFrameDimensions(float lengthsSet1[], float lengthsSet2[], float machineDimensions[]){
    Serial.println("Computing frame dimensions");
    //Call compute verticals from each side
    
    float leftHeight = computeVertical(lengthsSet1[3],lengthsSet1[0], lengthsSet2[3], lengthsSet2[0]);
    float rightHeight = computeVertical(lengthsSet1[2],lengthsSet1[1], lengthsSet2[2], lengthsSet2[1]);
    
    grbl_sendf(CLIENT_ALL, "Computed vertical measurements:\n%f \n%f \n%f \n",leftHeight, rightHeight, (leftHeight+rightHeight)/2.0);
}

#ifdef USE_CUSTOM_HOMING
/*
  user_defined_homing() is called at the begining of the normal Grbl_ESP32 homing
  sequence.  If user_defined_homing() returns false, the rest of normal Grbl_ESP32
  homing is skipped if it returns false, other normal homing continues.  For
  example, if you need to manually prep the machine for homing, you could implement
  user_defined_homing() to wait for some button to be pressed, then return true.
*/
bool user_defined_homing(uint8_t cycle_mask)
{
  
  grbl_sendf(CLIENT_ALL, "User defined calibration");
  
  if(cycle_mask == 1){  //Upper left
    axis4.testEncoder();
    axis4Homed = axis4.retract(computeL1(0, -400));
  }
  else if(cycle_mask == 2){  //Upper right
    axis3.testEncoder();
    axis3Homed = axis3.retract(computeL2(0, -400));
    grbl_sendf(CLIENT_ALL, "Extending to: %f", computeL2(0, 0));
  }
  else if(cycle_mask == 4){ //Lower right
    axis2.testEncoder();
    if(axis1Homed && axis2Homed && axis3Homed && axis4Homed){
        runCalibration();
    }
      else{
        axis2Homed = axis2.retract(computeL3(0, -200));
    }
    grbl_sendf(CLIENT_ALL, "Extending to: %f", computeL2(0, 0));
  }
  else if(cycle_mask == 0){  //Lower left
    axis1.testEncoder();
    axis1Homed = axis1.retract(computeL4(0, -200));
  }
  
  return true;
}
#endif

#ifdef USE_KINEMATICS
/*
  Inverse Kinematics converts X,Y,Z cartesian coordinate to the steps
  on your "joint" motors.  It requires the following three functions:
*/

/*
  inverse_kinematics() looks at incoming move commands and modifies
  them before Grbl_ESP32 puts them in the motion planner.

  Grbl_ESP32 processes arcs by converting them into tiny little line segments.
  Kinematics in Grbl_ESP32 works the same way. Search for this function across
  Grbl_ESP32 for examples. You are basically converting cartesian X,Y,Z... targets to

    target = an N_AXIS array of target positions (where the move is supposed to go)
    pl_data = planner data (see the definition of this type to see what it is)
    position = an N_AXIS array of where the machine is starting from for this move
*/
void inverse_kinematics(float *target, plan_line_data_t *pl_data, float *position)
{
  // this simply moves to the target. Replace with your kinematics.
  mc_line(target, pl_data);
}

/*
  kinematics_pre_homing() is called before normal homing
  You can use it to do special homing or just to set stuff up

  cycle_mask is a bit mask of the axes being homed this time.
*/
bool kinematics_pre_homing(uint8_t cycle_mask))
{
  return false; // finish normal homing cycle
}

/*
  kinematics_post_homing() is called at the end of normal homing
*/
void kinematics_post_homing()
{
}
#endif

#ifdef USE_FWD_KINEMATIC
/*
  The status command uses forward_kinematics() to convert
  your motor positions to cartesian X,Y,Z... coordinates.

  Convert the N_AXIS array of motor positions to cartesian in your code.
*/
void forward_kinematics(float *position)
{
  // position[X_AXIS] =
  // position[Y_AXIS] =
}
#endif

#ifdef USE_TOOL_CHANGE
/*
  user_tool_change() is called when tool change gcode is received,
  to perform appropriate actions for your machine.
*/
void user_tool_change(uint8_t new_tool)
{
}
#endif

#if defined(MACRO_BUTTON_0_PIN) || defined(MACRO_BUTTON_1_PIN) || defined(MACRO_BUTTON_2_PIN)
/*
  options.  user_defined_macro() is called with the button number to
  perform whatever actions you choose.
*/
void user_defined_macro(uint8_t index)
{
}
#endif

#ifdef USE_M30
/*
  user_m30() is called when an M30 gcode signals the end of a gcode file.
*/
void user_m30()
{
}
#endif

#ifdef USE_MACHINE_TRINAMIC_INIT
/*
  machine_triaminic_setup() replaces the normal setup of trinamic
  drivers with your own code.  For example, you could setup StallGuard
*/
void machine_trinamic_setup()
{
}
#endif

// If you add any additional functions specific to your machine that
// require calls from common code, guard their calls in the common code with
// #ifdef USE_WHATEVER and add function prototypes (also guarded) to grbl.h
