
/*
  dxl_tap_delta.cpp - 
  
  Copyright (c) 2019 Barton Dring @buildlog
      
  Grbl axis units are in radians
  The MPos zero angle is the arms horizontal.
  Negative angles are up from horizontal.
  The machine Z zero point in the kinematics is parallel to the arm axes.
    delta_z_offset is the distance to the end effector joints at arm angle zero.


  Grbl is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
  Grbl is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  You should have received a copy of the GNU General Public License
  along with Grbl.  If not, see <http://www.gnu.org/licenses/>.
    FYI: http://forums.trossenrobotics.com/tutorials/introduction-129/delta-robot-kinematics-3276/
    Better: http://hypertriangle.com/~alex/delta-robot-tutorial/
*/

// trigonometric constants
const float sqrt3  = 1.732050807;
const float pi     = 3.141592653;        // PI
const float dtr    = pi / (float)180.0;  // degrees to radians
const float sin120 = sqrt3 / 2.0;
const float cos120 = -0.5;
const float tan60  = sqrt3;
const float sin30  = 0.5;
const float tan30  = 1.0 / sqrt3;

// the geometry of the delta
const float rf = 70.0;   // radius of the fixed side (length of motor cranks)
const float re = 133.5;  // radius of end effector side (length of linkages)

const float f = 179.437f;  // sized of fixed side triangel
const float e = 86.6025f;  // size of end effector side triangle

static float last_angle[N_AXIS] = { 0.0, 0.0, 0.0 };  // save the previous motor angles for distance/feed rate calcs

float delta_z_offset;

// prototypes for local functions
int   delta_calcForward(float theta1, float theta2, float theta3, float& x0, float& y0, float& z0);
int   delta_calcInverse(float x0, float y0, float z0, float& theta1, float& theta2, float& theta3);
int   delta_calcAngleYZ(float x0, float y0, float z0, float& theta);
float three_axis_dist(float* pt1, float* pt2);

void machine_init() {
    grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "Machine Init: %s", MACHINE_NAME);  // print a message

    // Calculate the Z offset at the motor zero angles ...
    // Z offset is the z distance from the motor axes to the end effector axes at zero angle
    float x0, y0;  // dummy variables
    delta_calcForward(1.0, 1.0, 1.0, x0, y0, delta_z_offset);

    grbl_msg_sendf(
        CLIENT_SERIAL, MsgLevel::Info, "Delta Z Offset at 0,0,0 is:%4.3f", delta_z_offset);  // uncomment if you want to see the z offset
}

bool user_defined_homing() {  // true = do not continue with normal Grbl homing
    return true;
}

void inverse_kinematics(float* target, plan_line_data_t* pl_data, float* position)  //The target and position are provided in machine space
{
    //grbl_sendf(CLIENT_SERIAL, "[MSG:Start X:%4.3f Y:%4.3f Z:%4.3f]\r\n", position[X_AXIS], position[Y_AXIS], position[Z_AXIS]);
    //grbl_sendf(CLIENT_SERIAL, "[MSG:Target X:%4.3f Y:%4.3f Z:%4.3f]\r\n\r\n", target[X_AXIS], target[Y_AXIS], target[Z_AXIS]);

    float dx, dy, dz;              // distances in each cartesian axis
    float theta1, theta2, theta3;  // returned angles from calculations
    float motor_angles[N_AXIS];

    float dist;            // total cartesian distance
    float segment_dist;    // segment cartesian distance
    float delta_distance;  // the"distance" from prev angle to next angle

    uint32_t segment_count;  // number of segments the move will be broken in to.

    float seg_target[N_AXIS];  // The target of the current segment

    float feed_rate = pl_data->feed_rate;  // save original feed rate

    // Check the destination to see if it is in work area
    int status = delta_calcInverse(
        target[X_AXIS], target[Y_AXIS], target[Z_AXIS] + delta_z_offset, motor_angles[0], motor_angles[1], motor_angles[2]);

    if (status == KIN_ANGLE_ERROR) {
        return;
    }
    if (motor_angles[0] < MAX_NEGATIVE_ANGLE || motor_angles[1] < MAX_NEGATIVE_ANGLE || motor_angles[2] < MAX_NEGATIVE_ANGLE) {
        grbl_sendf(CLIENT_SERIAL, "[MSG:Destination unreachable. Move rejected]\r\n");
        return;
    }

    // adjust the end effector location
    //target[Z_AXIS] += delta_z_offset; // Note: For typical (fixed top) deltas, the offset is negative

    // calculate cartesian move distance for each axis

    dx = target[X_AXIS] - position[X_AXIS];
    dy = target[Y_AXIS] - position[Y_AXIS];
    dz = target[Z_AXIS] - position[Z_AXIS];

    // calculate the total X,Y,Z axis move distance
    dist = sqrt((dx * dx) + (dy * dy) + (dz * dz));

    //dist = three_axis_dist(target, position);

    segment_count =
        ceil(dist / SEGMENT_LENGTH);  // determine the number of segments we need	... round up so there is at least 1 (except when dist is 0)

    segment_dist = dist / ((float)segment_count);  // distanse of each segment...will be used for feedrate conversion

    for (uint32_t segment = 1; segment <= segment_count; segment++) {
        // determine this segment's target
        seg_target[X_AXIS] = position[X_AXIS] + (dx / float(segment_count) * segment);
        seg_target[Y_AXIS] = position[Y_AXIS] + (dy / float(segment_count) * segment);
        seg_target[Z_AXIS] = position[Z_AXIS] + (dz / float(segment_count) * segment);

        //grbl_sendf(	CLIENT_SERIAL, "[MSG:Kin Segment Target Cart X:%4.3f Y:%4.3f Z:%4.3f]\r\n",seg_target[X_AXIS], seg_target[Y_AXIS],seg_target[Z_AXIS]);

        // calculate the delta motor angles
        int status = delta_calcInverse(
            seg_target[X_AXIS], seg_target[Y_AXIS], seg_target[Z_AXIS] + delta_z_offset, motor_angles[0], motor_angles[1], motor_angles[2]);

        // check to see if we are trying to go too high
        for (int axis = 0; axis < N_AXIS; axis++) {
            if (motor_angles[axis] < MAX_NEGATIVE_ANGLE) {
                grbl_sendf(CLIENT_SERIAL, "[MSG:Axis %d angle error. Too high %4.3f]\r\n", axis, motor_angles[axis]);
            }
        }

        /*
		if (motor_angles[0] < MAX_NEGATIVE_ANGLE || motor_angles[0] < MAX_NEGATIVE_ANGLE || motor_angles[0] < MAX_NEGATIVE_ANGLE) {
			grbl_sendf(	CLIENT_SERIAL, "[MSG:Angle error. Too high]\r\n");
			status = KIN_ANGLE_ERROR;
		}
		*/

        if (status == KIN_ANGLE_CALC_OK) {
            //grbl_sendf(	CLIENT_SERIAL, "[MSG:Kin Segment Target Angs X:%4.3f Y:%4.3f Z:%4.3f]\r\n\r\n", motor_angles[0], motor_angles[1], motor_angles[2]);

            // Convert back to radians .... TODO get rid of degrees
            motor_angles[0] *= dtr;
            motor_angles[1] *= dtr;
            motor_angles[2] *= dtr;

            delta_distance = three_axis_dist(motor_angles, last_angle);

            // save angles for next distance calc
            last_angle[0] = motor_angles[0];  // TODO use an memcpy
            last_angle[1] = motor_angles[1];
            last_angle[2] = motor_angles[2];

            if (pl_data->motion.rapidMotion) {
                pl_data->feed_rate = feed_rate;
            } else {
                pl_data->feed_rate = (feed_rate * delta_distance / segment_dist);
            }

            mc_line(motor_angles, pl_data);

        } else {
            //grbl_sendf(	CLIENT_SERIAL, "[MSG:Kin Angle Error]\r\n\r\n");
        }
    }
}

// inverse kinematics: (x0, y0, z0) -> (theta1, theta2, theta3)
// returned status: 0=OK, -1=non-existing position
int delta_calcInverse(float x0, float y0, float z0, float& theta1, float& theta2, float& theta3) {
    //grbl_sendf(CLIENT_SERIAL, "[MSG:Calc X:%4.3f Y:%4.3f Z:%4.3f]\r\n\r\n", x0, y0, z0);

    theta1 = theta2 = theta3 = 0;
    int status;

    status = delta_calcAngleYZ(x0, y0, z0, theta1);

    if (status != KIN_ANGLE_CALC_OK) {
        grbl_sendf(CLIENT_SERIAL, "[MSG:theta1 Error]\r\n");
        return status;
    }

    status = delta_calcAngleYZ(x0 * cos120 + y0 * sin120, y0 * cos120 - x0 * sin120, z0, theta2);  // rotate coords to +120 deg

    if (status != KIN_ANGLE_CALC_OK) {
        grbl_sendf(CLIENT_SERIAL, "[MSG:theta2 Error]\r\n");
        return status;
    }

    status = delta_calcAngleYZ(x0 * cos120 - y0 * sin120, y0 * cos120 + x0 * sin120, z0, theta3);  // rotate coords to -120 deg

    if (status != KIN_ANGLE_CALC_OK) {
        grbl_sendf(CLIENT_SERIAL, "[MSG:theta3 Error]\r\n");
    }

    return status;
}

int delta_calcForward(float theta1, float theta2, float theta3, float& x0, float& y0, float& z0) {
    float t   = (f - e) * tan30 / 2;
    float dtr = pi / (float)180.0;

    //theta1 *= dtr;
    //theta2 *= dtr;
    //theta3 *= dtr;

    float y1 = -(t + rf * cos(theta1));
    float z1 = -rf * sin(theta1);

    float y2 = (t + rf * cos(theta2)) * sin30;
    float x2 = y2 * tan60;
    float z2 = -rf * sin(theta2);

    float y3 = (t + rf * cos(theta3)) * sin30;
    float x3 = -y3 * tan60;
    float z3 = -rf * sin(theta3);

    float dnm = (y2 - y1) * x3 - (y3 - y1) * x2;

    float w1 = y1 * y1 + z1 * z1;
    float w2 = x2 * x2 + y2 * y2 + z2 * z2;
    float w3 = x3 * x3 + y3 * y3 + z3 * z3;

    // x = (a1*z + b1)/dnm
    float a1 = (z2 - z1) * (y3 - y1) - (z3 - z1) * (y2 - y1);
    float b1 = -((w2 - w1) * (y3 - y1) - (w3 - w1) * (y2 - y1)) / 2.0;

    // y = (a2*z + b2)/dnm;
    float a2 = -(z2 - z1) * x3 + (z3 - z1) * x2;
    float b2 = ((w2 - w1) * x3 - (w3 - w1) * x2) / 2.0;

    // a*z^2 + b*z + c = 0
    float a = a1 * a1 + a2 * a2 + dnm * dnm;
    float b = 2 * (a1 * b1 + a2 * (b2 - y1 * dnm) - z1 * dnm * dnm);
    float c = (b2 - y1 * dnm) * (b2 - y1 * dnm) + b1 * b1 + dnm * dnm * (z1 * z1 - re * re);

    // discriminant
    float d = b * b - (float)4.0 * a * c;
    if (d < 0)
        return -1;  // non-existing point

    z0 = -(float)0.5 * (b + sqrt(d)) / a;
    x0 = (a1 * z0 + b1) / dnm;
    y0 = (a2 * z0 + b2) / dnm;
    return 0;
}

// helper functions, calculates angle theta1 (for YZ-pane)
int delta_calcAngleYZ(float x0, float y0, float z0, float& theta) {
    float y1 = -0.5 * 0.57735 * f;  // f/2 * tg 30
    y0 -= 0.5 * 0.57735 * e;        // shift center to edge
    // z = a + b*y
    float a = (x0 * x0 + y0 * y0 + z0 * z0 + rf * rf - re * re - y1 * y1) / (2 * z0);
    float b = (y1 - y0) / z0;
    // discriminant
    float d = -(a + b * y1) * (a + b * y1) + rf * (b * b * rf + rf);
    if (d < 0)
        return -1;                                    // non-existing point
    float yj = (y1 - a * b - sqrt(d)) / (b * b + 1);  // choosing outer point
    float zj = a + b * yj;
    theta    = 180.0 * atan(-zj / (y1 - yj)) / pi + ((yj > y1) ? 180.0 : 0.0);
    return 0;
}

// Determine the unit distance between (2) 3D points
float three_axis_dist(float* pt1, float* pt2) {
    return sqrt(((pt1[0] - pt2[0]) * (pt1[0] - pt2[0])) + ((pt1[1] - pt2[1]) * (pt1[1] - pt2[1])) + ((pt1[2] - pt2[2]) * (pt1[2] - pt2[2])));
}

void forward_kinematics(float* position) {}

bool kinematics_pre_homing(uint8_t cycle_mask) {
    return true;
}

void kinematics_post_homing() {}

void user_m30() {}
