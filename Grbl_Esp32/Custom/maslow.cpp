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

MotorUnit axisBR(&tlc, MOTOR_1_FORWARD, MOTOR_1_BACKWARD, MOTOR_1_ADC, RSENSE, adc_1_characterisitics, MOTOR_1_CS, DC_TOP_LEFT_MM_PER_REV, 1, printStall);
MotorUnit axisTR(&tlc, MOTOR_2_FORWARD, MOTOR_2_BACKWARD, MOTOR_2_ADC, RSENSE, adc_1_characterisitics, MOTOR_2_CS, DC_TOP_LEFT_MM_PER_REV, 1, printStall);
MotorUnit axisTL(&tlc, MOTOR_3_FORWARD, MOTOR_3_BACKWARD, MOTOR_3_ADC, RSENSE, adc_1_characterisitics, MOTOR_3_CS, DC_TOP_LEFT_MM_PER_REV, 1, printStall);
MotorUnit axisBL(&tlc, MOTOR_4_FORWARD, MOTOR_4_BACKWARD, MOTOR_4_ADC, RSENSE, adc_1_characterisitics, MOTOR_4_CS, DC_TOP_LEFT_MM_PER_REV, 1, printStall);

//The xy coordinates of each of the anchor points
float tlX;
float tlY;
float tlZ;
float trX;
float trY;
float trZ;
float blX;
float blY;
float blZ;
float brX;
float brY;
float brZ;
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
    
    tlX = -8.339;
    tlY = 1828.17;
    tlZ = 0;
    trX = 2870.62;
    trY = 1829.05;
    trZ = 0;
    blX = 0;
    blY = 0;
    blZ = 0;
    brX = 2891.36;
    brY = 0;
    brZ = 0;
    
    //Recompute the center XY
    updateCenterXY();
    
    beltEndExtension = 30;
    armLength = 114;
}
#endif

void printToWeb (double precision){
    grbl_sendf(CLIENT_ALL, "Calibration Precision: %fmm\n", precision);
}

void printStall (double variable){
    grbl_sendf(CLIENT_ALL, "Motor stalled at: %f\n", variable);
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

//Updates where the center x and y positions are
void updateCenterXY(){
    
    double A = (trY - blY)/(trX-blX);
    double B = (brY-tlY)/(brX-tlX);
    centerX = (brY-(B*brX)+(A*trX)-trY)/(A-B);
    centerY = A*(centerX - trX) + trY;
    
}

//Upper left belt
float computeBL(float x, float y){
    //Move from lower left corner coordinates to centered coordinates
    x = x + centerX;
    y = y + centerY;
    float a = blX - x;
    float b = blY - y;
    float c = 0.0 - blZ;
    return sqrt(a*a+b*b+c*c) - (beltEndExtension+armLength);
}

//Upper right belt
float computeBR(float x, float y){
    //Move from lower left corner coordinates to centered coordinates
    x = x + centerX;
    y = y + centerY;
    float a = brX - x;
    float b = brY - y;
    float c = 0.0 - brZ;
    return sqrt(a*a+b*b+c*c) - (beltEndExtension+armLength);
}

//Lower right belt
float computeTR(float x, float y){
    //Move from lower left corner coordinates to centered coordinates
    x = x + centerX;
    y = y + centerY;
    float a = trX - x;
    float b = trY - y;
    float c = 0.0 - trZ;
    return sqrt(a*a+b*b+c*c) - (beltEndExtension+armLength);
}

//Lower left belt
float computeTL(float x, float y){
    //Move from lower left corner coordinates to centered coordinates
    x = x + centerX;
    y = y + centerY;
    float a = tlX - x;
    float b = tlY - y;
    float c = 0.0 - tlZ;
    return sqrt(a*a+b*b+c*c) - (beltEndExtension+armLength);
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
    float lengths4[4];
    float lengths5[4];
    float lengths6[4];
    float lengths7[4];
    float lengths8[4];
    float lengths9[4];
    
    //------------------------------------------------------Take measurements
    
    takeMeasurementAvg(lengths1);//Repeat and throw away the first one
    takeMeasurementAvg(lengths1);
    
    moveWithSlack(0, 0);
    
    takeMeasurementAvg(lengths2);
    
    moveWithSlack(0, 300);
    
    takeMeasurementAvg(lengths3);
    
    moveWithSlack(-450, -200);
    
    takeMeasurementAvg(lengths4);
    
    moveWithSlack(-450, 0);
    
    takeMeasurementAvg(lengths5);
    
    moveWithSlack(-450, 200);
    
    takeMeasurementAvg(lengths6);
    
    moveWithSlack(450, -200);
    
    takeMeasurementAvg(lengths7);
    
    moveWithSlack(450, 0);
    
    takeMeasurementAvg(lengths8);
    
    moveWithSlack(450, 200);
    
    takeMeasurementAvg(lengths9);
    
    moveWithSlack(0, 0);  //Go back to the center. This will pull the lower belts tight too
    
    grbl_sendf(CLIENT_ALL, "Stopping Motors\n");
    axisBL.stop();
    axisBR.stop();
    axisTR.stop();
    axisTL.stop();
    
    //----------------------------------------------------------Do the computation
    
    printMeasurements(lengths1);
    printMeasurements(lengths2);
    printMeasurements(lengths3);
    printMeasurements(lengths4);
    printMeasurements(lengths5);
    printMeasurements(lengths6);
    printMeasurements(lengths7);
    printMeasurements(lengths8);
    printMeasurements(lengths9);
    
    double measurements[][4] = {
        //TL              TR           BL           BR
        {lengths1[3], lengths1[2], lengths1[0], lengths1[1]},
        {lengths2[3], lengths2[2], lengths2[0], lengths2[1]},
        {lengths3[3], lengths3[2], lengths3[0], lengths3[1]},
        {lengths4[3], lengths4[2], lengths4[0], lengths4[1]},
        {lengths5[3], lengths5[2], lengths5[0], lengths5[1]},
        {lengths6[3], lengths6[2], lengths6[0], lengths6[1]},
        {lengths7[3], lengths7[2], lengths7[0], lengths7[1]},
        {lengths8[3], lengths8[2], lengths8[0], lengths8[1]},
        {lengths9[3], lengths9[2], lengths9[0], lengths9[1]},
    };
    double results[6] = {0,0,0,0,0,0};
    computeCalibration(measurements, results, printToWeb, tlX, tlY, trX, trY, brX, tlZ, trZ, blZ, brZ);
    
    grbl_sendf(CLIENT_ALL, "After computing calibration %f\n", results[5]);
    
    if(results[5] < 2){
        grbl_sendf(CLIENT_ALL, "Calibration successful with precision %f\n", results[5]);
        tlX = results[0];
        tlY = results[1];
        trX = results[2];
        trY = results[3];
        blX = 0;
        blY = 0;
        brX = results[4];
        brY = 0;
        updateCenterXY();
        grbl_sendf(CLIENT_ALL, "tlx: %f tly: %f trx: %f try: %f blx: %f bly: %f brx: %f bry: %f \n", tlX, tlY, trX, trY, blX, blY, brX, brY);
    }
    else{
        grbl_sendf(CLIENT_ALL, "Calibration failed: %f\n", results[5]);
    }
    
    //---------------------------------------------------------Finish
    
    
    //Move back to center after the results are applied
    moveWithSlack(0, 0);
    
    //For safety we should pull tight here and verify that the results are basically what we expect before handing things over to the controller.
    takeMeasurementAvg(lengths1);
    
    double blError = (lengths1[0]-(beltEndExtension+armLength))-computeBL(0,0);
    double brError = (lengths1[1]-(beltEndExtension+armLength))-computeBR(0,0);
    
    grbl_sendf(CLIENT_ALL, "Lower belt length mismatch: bl: %f, br: %f\n", blError, brError);
    
    calibrationInProgress = false;
    grbl_sendf(CLIENT_ALL, "Calibration finished\n");
    
}

void printMeasurements(float lengths[]){
    grbl_sendf(CLIENT_ALL, "{bl:%f,   br:%f,   tr:%f,   tl:%f},\n", lengths[0], lengths[1], lengths[2], lengths[3]);
}

void lowerBeltsGoSlack(){
    
    unsigned long time = millis();
    unsigned long elapsedTime = millis()-time;
    while(elapsedTime < 500){
        elapsedTime = millis()-time;
        axisBL.fullOut();
        axisBL.updateEncoderPosition();
        axisBR.fullOut();
        axisBR.updateEncoderPosition();
        
        //The other axis hold position
        axisTR.recomputePID();
        axisTL.recomputePID();
    }
    
    //reset where we should be
    axisBL.setTarget(axisBL.getPosition());
    axisBR.setTarget(axisBR.getPosition());
    
    axisBL.stop();
    axisBR.stop();
    
    unsigned long timeLastMoved1 = millis();
    unsigned long timeLastMoved2 = millis();
    double lastPosition1 = axisBL.angleSensor->getRotation();
    double lastPosition2 = axisBR.angleSensor->getRotation();
    double amtToMove1 = 0.1;
    double amtToMove2 = 0.1;
    
    unsigned long startTime = millis();
    
    while(millis()- startTime < 2500){
        //Set the lower axis to be compliant. PID is recomputed in comply()
        axisBL.comply(&timeLastMoved1, &lastPosition1, &amtToMove1, 300);
        axisBR.comply(&timeLastMoved2, &lastPosition2, &amtToMove2, 300);
        
        //The other axis hold position
        axisTR.recomputePID();
        axisTL.recomputePID();
        
        // Delay without blocking
        unsigned long time2 = millis();
        unsigned long elapsedTime2 = millis()-time2;
        while(elapsedTime2 < 50){
            elapsedTime2 = millis()-time2;
        }
    }
}

void printMeasurementMetrics(double avg, double m1, double m2, double m3, double m4, double m5){
    
    
    double m1Variation = myAbs(avg - m1);
    double m2Variation = myAbs(avg - m1);
    double m3Variation = myAbs(avg - m1);
    double m4Variation = myAbs(avg - m1);
    double m5Variation = myAbs(avg - m1);
    
    double maxDeviation = max(max(max(m1Variation, m2Variation), max(m3Variation, m4Variation)), m5Variation);
    
    grbl_sendf(CLIENT_ALL, "Max deviation: %f\n", maxDeviation);
}

//Takes 5 measurements and computes the average of them
void takeMeasurementAvg(float lengths[]){
    
    grbl_sendf(CLIENT_ALL, "Beginning to take a measurement\n");
    
    //Where our five measurements will be stored
    float lengths1[4];
    float lengths2[4];
    float lengths3[4];
    float lengths4[4];
    float lengths5[4];
    
    takeMeasurement(lengths1);
    lowerBeltsGoSlack();
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
    lowerBeltsGoSlack();
    
    lengths[0] = (lengths1[0]+lengths2[0]+lengths3[0]+lengths4[0]+lengths5[0])/5.0;
    lengths[1] = (lengths1[1]+lengths2[1]+lengths3[1]+lengths4[1]+lengths5[1])/5.0;
    lengths[2] = (lengths1[2]+lengths2[2]+lengths3[2]+lengths4[2]+lengths5[2])/5.0;
    lengths[3] = (lengths1[3]+lengths2[3]+lengths3[3]+lengths4[3]+lengths5[3])/5.0;
    
    grbl_sendf(CLIENT_ALL, "Measurement taken.\n");
    printMeasurementMetrics(lengths[0], lengths1[0], lengths2[0], lengths3[0], lengths4[0], lengths5[0]);
    printMeasurementMetrics(lengths[1], lengths1[1], lengths2[1], lengths3[1], lengths4[1], lengths5[1]);
    printMeasurementMetrics(lengths[2], lengths1[2], lengths2[2], lengths3[2], lengths4[2], lengths5[2]);
    printMeasurementMetrics(lengths[3], lengths1[3], lengths2[3], lengths3[3], lengths4[3], lengths5[3]);
}

//Retract the lower belts until they pull tight and take a measurement
void takeMeasurement(float lengths[]){
    
    grbl_sendf(CLIENT_ALL, "Taking a measurement\n");
    
    bool axisBLDone = false;
    bool axisBRDone = false;
    
    while(!axisBLDone || !axisBRDone){  //As long as one axis is still pulling
        
        //If any of the current values are over the threshold then stop and exit, otherwise pull each axis a little bit tighter by incrementing the target position
        int currentThreshold = 12;
        
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
        while(elapsedTime < 25){
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

//Reposition the sled without knowing the machine dimensions
void moveWithSlack(float x, float y){
    
    grbl_sendf(CLIENT_ALL, "Moving with slack to %f , %f \n", x, y);
    
    //The distance we need to move is the current position minus the target position
    double TLDist = axisTL.getPosition() - computeTL(x,y);
    double TRDist = axisTR.getPosition() - computeTR(x,y);
    
    //Record which direction to move
    double TLDir  = constrain(TLDist, -1, 1);
    double TRDir  = constrain(TRDist, -1, 1);
    
    double stepSize = .5;
    
    //Only use positive dist for incrementing counter (float int conversion issue?)
    TLDist = abs(TLDist);
    TRDist = abs(TRDist);
    
    //Make the lower arms compliant and move retract the other two until we get to the target distance
    
    unsigned long timeLastMoved1 = millis();
    unsigned long timeLastMoved2 = millis();
    double lastPosition1 = axisBL.getPosition();
    double lastPosition2 = axisBR.getPosition();
    double amtToMove1 = 0.1;
    double amtToMove2 = 0.1;
    
    while(TLDist > 0 || TRDist > 0){
        
        //Set the lower axis to be compliant. PID is recomputed in comply()
        axisBL.comply(&timeLastMoved1, &lastPosition1, &amtToMove1, 300);
        axisBR.comply(&timeLastMoved2, &lastPosition2, &amtToMove2, 300);
        
        //grbl_sendf(CLIENT_ALL, "BRPos: %f, BRamt: %f, BRtime: %lu\n", lastPosition2, amtToMove2, timeLastMoved2);
        
        //Move the upper axis one step
        if(TLDist > 0){
            TLDist = TLDist - stepSize;
            axisTL.setTarget((axisTL.getTarget() - (stepSize*TLDir)));
        }
        if(TRDist > 0){
            TRDist = TRDist - stepSize;
            axisTR.setTarget((axisTR.getTarget() - (stepSize*TRDir)));
        }
        axisTR.recomputePID();
        axisTL.recomputePID();
        
        // Delay without blocking
        unsigned long time = millis();
        unsigned long elapsedTime = millis()-time;
        while(elapsedTime < 50){
            elapsedTime = millis()-time;
        }
        
        //Force extend if the belt is getting taught
        // if(axisTR.getCurrent() > 3){
           // lastPosition2 = lastPosition2 - 2.0;
           // grbl_sendf(CLIENT_ALL, "Axis 3 taught %f\n",axisTR.getCurrent());
        // }
        // if(axisTL.getCurrent() > 3){
           // lastPosition1 = lastPosition1 - 2.0;
           // grbl_sendf(CLIENT_ALL, "Axis 4 taught %f\n",axisTL.getCurrent());
        // }
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
