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

MotorUnit axisBL(&tlc, MOTOR_1_FORWARD, MOTOR_1_BACKWARD, MOTOR_1_ADC, RSENSE, adc_1_characterisitics, MOTOR_1_CS, DC_TOP_LEFT_MM_PER_REV, 1);
MotorUnit axisBR(&tlc, MOTOR_2_FORWARD, MOTOR_2_BACKWARD, MOTOR_2_ADC, RSENSE, adc_1_characterisitics, MOTOR_2_CS, DC_TOP_LEFT_MM_PER_REV, 1);
MotorUnit axisTR(&tlc, MOTOR_3_FORWARD, MOTOR_3_BACKWARD, MOTOR_3_ADC, RSENSE, adc_1_characterisitics, MOTOR_3_CS, DC_TOP_LEFT_MM_PER_REV, 1);
MotorUnit axisTL(&tlc, MOTOR_4_FORWARD, MOTOR_4_BACKWARD, MOTOR_4_ADC, RSENSE, adc_1_characterisitics, MOTOR_4_CS, DC_TOP_LEFT_MM_PER_REV, 1);

//The xy cordinates of each of the anchor points
float tlX;
float tlY;
float trX;
float trY;
float blX;
float blY;
float brX;
float brY;
float centerX;
float centerY;

float beltEndExtension;
float armLength;

bool axisBLHomed;
bool axisBRHomed;
bool axisTRHomed;
bool axisTLHomed;
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
    axisBL.testEncoder();
    axisBR.testEncoder();
    axisTR.testEncoder();
    axisTL.testEncoder();
    
    axisBL.zero();
    axisBR.zero();
    axisTR.zero();
    axisTL.zero();
    
    axisBLHomed = false;
    axisBRHomed = false;
    axisTRHomed = false;
    axisTLHomed = false;
    
    tlX = 0;
    tlY = 1800;
    trX = 3000;
    trY = 1800;
    blX = 0;
    blY = 0;
    brX = 3000;
    brY = 0;
    
    double A = (trY - blY)/(trX-blX);
    double B = (brY-tlY)/(brX-tlX);
    
    centerX = (brY-(B*brX)+(A*trX)-trY)/(A-B);
    centerY = A*(centerX - trX) + trY;
    
    beltEndExtension = 30;
    armLength = 114;
}
#endif

void printToWeb (double precision){
    grbl_sendf(CLIENT_ALL, "Calibration Precision: %fmm\n", precision);
}

void recomputePID(){
    //Stop everything but keep track of the encoder positions if we are idle or alarm. Unless doing calibration.
    
    if((sys.state == State::Idle || sys.state == State::Alarm) && !calibrationInProgress){
        axisBL.stop();
        axisBL.updateEncoderPosition();
        axisBR.stop();
        axisBR.updateEncoderPosition();
        axisTR.stop();
        axisTR.updateEncoderPosition();
        axisTL.stop();
        axisTL.updateEncoderPosition();
    }
    else{  //Position the axis
        axisBL.recomputePID();
        axisBR.recomputePID();
        axisTR.recomputePID();
        axisTL.recomputePID();
    }
}

//Upper left belt
float computeBL(float x, float y){
    //Move from lower left corner cordinates to centered cordinates
    x = x + centerX;
    y = centerY - y;
    float a = tlX - x;
    float b = tlY - y;
    return sqrt(a*a+b*b) - (beltEndExtension+armLength);
}

//Upper right belt
float computeBR(float x, float y){
    //Move from lower left corner cordinates to centered cordinates
    x = x + centerX;
    y = centerY - y;
    float a = trX - x;
    float b = trY - y;
    return sqrt(a*a+b*b) - (beltEndExtension+armLength);
}

//Lower right belt
float computeTR(float x, float y){
    //Move from lower left corner cordinates to centered cordinates
    x = x + centerX;
    y = centerY - y;
    float a = brX - x;
    float b = brY - y;
    return sqrt(a*a+b*b) - (beltEndExtension+armLength);
}

//Lower left belt
float computeTL(float x, float y){
    //Move from lower left corner cordinates to centered cordinates
    x = x + centerX;
    y = centerY - y;
    float a = blX - x;
    float b = blY - y;
    return sqrt(a*a+b*b) - (beltEndExtension+armLength);
}

void setTargets(float xTarget, float yTarget, float zTarget){
    
    if(!calibrationInProgress){
        axisBL.setTarget(computeBL(xTarget, yTarget));
        axisBR.setTarget(computeBR(xTarget, yTarget));
        axisTR.setTarget(computeTR(xTarget, yTarget));
        axisTL.setTarget(computeTL(xTarget, yTarget));
    }
}


//Runs the calibration sequence to determine the machine's dimensions
void runCalibration(){
    
    grbl_sendf(CLIENT_ALL, "Beginning calibration\n");
    
    calibrationInProgress = true;
    
    //This is a hack and should be fixed, but it undoes any calls by the system to move these to (0,0)
    axisBL.setTarget(computeBL(0, -300));
    axisBR.setTarget(computeBR(0, -300));
    axisTR.setTarget(computeTR(0, -500));
    axisTL.setTarget(computeTL(0, -500));
    
    float lengths1[4];
    float lengths2[4];
    float lengths3[4];
    
    //------------------------------------------------------Take measurements
    
    takeMeasurementAvg(lengths1);//Repeat and throw away the first one
    takeMeasurementAvg(lengths1);
    
    moveUp(computeTL(0, 0));
    
    takeMeasurementAvg(lengths2);
    
    moveUp(computeTL(0, 500));
    
    takeMeasurementAvg(lengths3);
    
    axisBL.stop();
    axisBR.stop();
    axisTR.stop();
    axisTL.stop();
    
    //----------------------------------------------------------Do the computation
    
    printMeasurements(lengths1);
    printMeasurements(lengths2);
    printMeasurements(lengths3);
    
    double measurements[][4] = {
        //TL              TR           BL           BR
        {lengths1[3], lengths1[2], lengths1[0], lengths1[1]},
        {lengths2[3], lengths2[2], lengths2[0], lengths2[1]},
        {lengths3[3], lengths3[2], lengths3[0], lengths3[1]}
    };
    
    double results[9] = {0,0,0,0,0,0,0,0,0};
    
    computeCalibration(measurements, results, printToWeb);
    
    grbl_sendf(CLIENT_ALL, "After computing calibration %f\n", results[8]);
    
    if(results[8] < 1){
        grbl_sendf(CLIENT_ALL, "Calibration sucessfull with precision %f\n", results[8]);
        tlX = results[0];
        tlY = results[1];
        trX = results[2];
        trY = results[3];
        blX = 0;
        blY = 0;
        brX = results[4];
        brY = results[5];
        grbl_sendf(CLIENT_ALL, "tlx: %f tly: %f trx: %f try: %f blx: %f bly: %f brx: %f bry: %f \n", tlX, tlY, trX, trY, blX, blY, brX, brY);
    }
    else{
        grbl_sendf(CLIENT_ALL, "Calibration failed: %f\n", results[8]);
    }
    
    //---------------------------------------------------------Finish
    
    
    //Move back to center
    moveDown(computeTL(0, 0));
    
    calibrationInProgress = false;
    grbl_sendf(CLIENT_ALL, "Calibration finished\n");
    
}

void printMeasurements(float lengths[]){
    grbl_sendf(CLIENT_ALL, "BL:%f   BR:%f   TR:%f   TL:%f\n", lengths[0], lengths[1], lengths[2], lengths[3]);
}

void lowerBeltsGoSlack(){
    unsigned long timeLastMoved1 = millis();
    unsigned long timeLastMoved2 = millis();
    double lastPosition1 = axisBL.angleSensor->getRotation();
    double lastPosition2 = axisBR.angleSensor->getRotation();
    double amtToMove1 = 0.1;
    double amtToMove2 = 0.1;
    
    unsigned long startTime = millis();
    
    while(millis()- startTime < 5000){
        //Set the lower axis to be compliant. PID is recomputed in comply()
        axisBL.comply(&timeLastMoved1, &lastPosition1, &amtToMove1, 3);
        axisBR.comply(&timeLastMoved2, &lastPosition2, &amtToMove2, 3);
        
        //The other axis hold position
        axisTR.recomputePID();
        axisTL.recomputePID();
        
        // Delay without blocking
        unsigned long time = millis();
        unsigned long elapsedTime = millis()-time;
        while(elapsedTime < 10){
            elapsedTime = millis()-time;
        }
    }
}

//Takes 5 measurements and computes the average of them
void takeMeasurementAvg(float lengths[]){
    
    //Where our five measurements will be stored
    float lengths1[4];
    float lengths2[4];
    float lengths3[4];
    float lengths4[4];
    float lengths5[4];
    
    takeMeasurement(lengths1);
    lowerBeltsGoSlack();
    takeMeasurement(lengths1);  //Repeat the first measurement to discard the one before everything was pulled taught
    lowerBeltsGoSlack();
    takeMeasurement(lengths2);
    lowerBeltsGoSlack();
    takeMeasurement(lengths3);
    lowerBeltsGoSlack();
    takeMeasurement(lengths4);
    lowerBeltsGoSlack();
    takeMeasurement(lengths5);
    
    lengths[0] = (lengths1[0]+lengths2[0]+lengths3[0]+lengths4[0]+lengths5[0])/5.0;
    lengths[1] = (lengths1[1]+lengths2[1]+lengths3[1]+lengths4[1]+lengths5[1])/5.0;
    lengths[2] = (lengths1[2]+lengths2[2]+lengths3[2]+lengths4[2]+lengths5[2])/5.0;
    lengths[3] = (lengths1[3]+lengths2[3]+lengths3[3]+lengths4[3]+lengths5[3])/5.0;
}

//Retract the lower belts until they pull tight and take a measurement
void takeMeasurement(float lengths[]){
    
    bool axisBLDone = false;
    bool axisBRDone = false;
    
    while(!axisBLDone || !axisBRDone){  //As long as one axis is still pulling
        
        //If any of the current values are over the threshold then stop and exit, otherwise pull each axis a little bit tighter by incrementing the target position
        int currentThreshold = 2;
        
        if(axisBL.getCurrent() > currentThreshold || axisBLDone){
            axisBLDone = true;
            //grbl_sendf(CLIENT_ALL, "Axis 1 declaring stopping");
        }
        else{
            axisBL.setTarget(axisBL.getTarget() - .2);
        }
        
        if(axisBR.getCurrent() > currentThreshold || axisBRDone){
            axisBRDone = true;
            //grbl_sendf(CLIENT_ALL, "Axis 2 declaring stopping");
        }
        else{
            axisBR.setTarget(axisBR.getTarget() - .2);
        }
        
        // Delay without blocking
        unsigned long time = millis();
        unsigned long elapsedTime = millis()-time;
        while(elapsedTime < 50){
            elapsedTime = millis()-time;
            recomputePID();  //This recomputes the PID four all four servos
        }
    }
    
    axisBL.setTarget(axisBL.getPosition());
    axisBR.setTarget(axisBR.getPosition());
    //axisTR.setTarget(axisTR.getPosition());
    //axisTL.setTarget(axisTL.getPosition());
    
    axisBL.stop();
    axisBR.stop();
    axisTR.stop();
    axisTL.stop();
    
    lengths[0] = axisBL.getPosition()+beltEndExtension+armLength;
    lengths[1] = axisBR.getPosition()+beltEndExtension+armLength;
    lengths[2] = axisTR.getPosition()+beltEndExtension+armLength;
    lengths[3] = axisTL.getPosition()+beltEndExtension+armLength;
    
    //grbl_sendf(CLIENT_ALL, "Measured:\n%f \n%f \n%f \n%f \n",lengths[0], lengths[1], lengths[2], lengths[3]);
    
    return;
}

//Reposition the sled higher without knowing the machine dimensions
void moveUp(float targetLength){
    
    //The distance we need to move is the current position minus the target position
    double distToRetract = axisTR.getPosition() - targetLength;
    
    //Make the lower arms compliant and move retract the other two until we get to the target distance
    
    unsigned long timeLastMoved1 = millis();
    unsigned long timeLastMoved2 = millis();
    double lastPosition1 = axisBL.getPosition();
    double lastPosition2 = axisBR.getPosition();
    double amtToMove1 = 0.1;
    double amtToMove2 = 0.1;
    
    while(distToRetract > 0){
        
        //Set the lower axis to be compliant. PID is recomputed in comply()
        axisBL.comply(&timeLastMoved1, &lastPosition1, &amtToMove1, 1.5);
        axisBR.comply(&timeLastMoved2, &lastPosition2, &amtToMove2, 1.5);
        
        //Syncronize the two pulling axis
        double axisTRBoost = (axisTR.getTarget() - axisTL.getTarget())/20.0;
        double axisTLBoost = (axisTL.getTarget() - axisTR.getTarget())/20.0;
        
        //Pull in on the upper axis
        axisTR.setTarget((axisTR.getTarget() - .05) - axisTRBoost);
        axisTL.setTarget((axisTL.getTarget() - .05) - axisTLBoost);
        axisTR.recomputePID();
        axisTL.recomputePID();
        distToRetract = distToRetract - .05;
        
        // Delay without blocking
        unsigned long time = millis();
        unsigned long elapsedTime = millis()-time;
        while(elapsedTime < 10){
            elapsedTime = millis()-time;
        }
        
        //Force extend if the belt is getting taught
        if(axisTR.getCurrent() > 3){
           lastPosition2 = lastPosition2 - 2.0;
           // grbl_sendf(CLIENT_ALL, "Axis 3 taught %f\n",axisTR.getCurrent());
        }
        if(axisTL.getCurrent() > 3){
           lastPosition1 = lastPosition1 - 2.0;
           // grbl_sendf(CLIENT_ALL, "Axis 4 taught %f\n",axisTL.getCurrent());
        }
    }
    
    axisBL.setTarget(axisBL.getPosition());
    axisBR.setTarget(axisBR.getPosition());
    axisTR.setTarget(axisTR.getPosition());
    axisTL.setTarget(axisTL.getPosition());
    
    axisBL.stop();
    axisBR.stop();
    axisTR.stop();
    axisTL.stop();
}

//Reposition the sled lower without knowing the machine dimensions
void moveDown(float targetLength){
    
    //The distance we need to move is the current position minus the target position
    double distToExtend = targetLength - axisTR.getPosition();
    
    grbl_sendf(CLIENT_ALL, "Dist to extend %f\n",distToExtend);
    
    while(distToExtend > 0){
        
        //Extend the upper axis
        axisTR.setTarget(axisTR.getTarget() + .15);
        axisTL.setTarget(axisTL.getTarget() + .15);
        axisTR.recomputePID();
        axisTL.recomputePID();
        distToExtend = distToExtend - .15;
        
        
        axisBL.stop();
        axisBR.stop();
        
        // Delay without blocking
        unsigned long time = millis();
        unsigned long elapsedTime = millis()-time;
        while(elapsedTime < 10){
            elapsedTime = millis()-time;
        }
    }
    
    axisBL.setTarget(axisBL.getPosition());
    axisBR.setTarget(axisBR.getPosition());
    axisTR.setTarget(axisTR.getPosition());
    axisTL.setTarget(axisTL.getPosition());
    
    axisBL.stop();
    axisBR.stop();
    axisTR.stop();
    axisTL.stop();
    
    //Take some fake measurements to retract the lower arms fully
    float lengths[4];
    takeMeasurementAvg(lengths);
}

float computeVertical(float firstUpper, float firstLower, float secondUpper, float secondLower){
    //Derivation at https://math.stackexchange.com/questions/4090346/solving-for-triangle-side-length-with-limited-information
    
    float b = secondUpper;   //upper, second
    float c = secondLower; //lower, second
    float d = firstUpper; //upper, first
    float e = firstLower;  //lower, first

    float aSquared = (((b*b)-(c*c))*((b*b)-(c*c))-((d*d)-(e*e))*((d*d)-(e*e)))/(2*(b*b+c*c-d*d-e*e));

    float a = sqrt(aSquared);
    
    return a;
}

void computeFrameDimensions(float lengthsSet1[], float lengthsSet2[], float machineDimensions[]){
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
  grbl_sendf(CLIENT_ALL, "Extending\n");
  
  Serial.println("In user homing");
  
  grbl_sendf(CLIENT_ALL, "Begin test\n");
    double measurements[][4] = {
        //TL              TR           BL           BR
        {2172.76,2327.74,1513.27,1523.18},
        {1786.16,1900.65,1708.83,1700},
        {1513.80,1617.55,2044.52,2066.49},
        {1632.48,3145.29,451.77,2618.43},
        {1053.42,2860.24,898.99,2742.35},
        {523.45,2645.69,1497.23,2972.13},
        {2975.71,1726.96,2617.25,508.03},
        {2849.37,1212.69,2817.45,862.62},
        {2607.83,682.42,2971.27,1475.26}
    };
    
    double results[6] = {0,0,0,0,0,0};
    
    computeCalibration(measurements, results, printToWeb);
    
    grbl_sendf(CLIENT_ALL, "After computing calibration %f\n", results[5]);
    
    grbl_sendf(CLIENT_ALL, "tlx: %f tly: %f trx: %f try: %f brx: %f \n", results[0] - 30, results[1] - 1800, results[2] - 3000, results[3] - 1980, results[4] - 3010);
    
    return true;
  
  if(cycle_mask == 1){  //Top left
    axisTL.testEncoder();
    axisTLHomed = axisTL.retract(computeTL(0, -500));
  }
  else if(cycle_mask == 2){  //Top right
    axisTR.testEncoder();
    axisTRHomed = axisTR.retract(computeTR(0, -500));
  }
  else if(cycle_mask == 4){ //Bottom right
    axisBR.testEncoder();
    if(axisBLHomed && axisBRHomed && axisTRHomed && axisTLHomed){
        runCalibration();
    }
      else{
        axisBRHomed = axisBR.retract(computeBR(0, -300));
    }
  }
  else if(cycle_mask == 0){  //Bottom left
    axisBL.testEncoder();
    axisBLHomed = axisBL.retract(computeBL(0, -300));
  }
  
  if(axisBLHomed && axisBRHomed && axisTRHomed && axisTLHomed){
      grbl_sendf(CLIENT_ALL, "All axis ready.\n");
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
