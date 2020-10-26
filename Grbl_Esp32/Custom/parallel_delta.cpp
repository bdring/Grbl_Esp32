
/*
  parallel_delta.cpp - 
  
  Copyright (c) 2019    Barton Dring @buildlog, 
                        Jason Huggins, Tapster Robotics

  Kinematics for a parallel delta robot.

  Note: You must do a clean before compiling whenever this file is altered!

      
  ==================== How it Works ====================================

  On a delta machine, Grbl axis units are in radians
  The kinematics converts the cartesian moves in gcode into
  the radians to move the arms. The Grbl motion planner never sees
  the actual cartesian values.

  To make the moves straight and smooth on a delta, the cartesian moves
  are broken into small segments where the non linearity will not be noticed.
  This is similar to how Grgl draws arcs.

  If you request MPos status it will tell you the position in
  arm angles. The MPos will report in cartesian values using forward kinematics. 

  The arm 0 values (angle) are the arms at horizontal.
  Positive angles are below horizontal.
  The machine's Z zero point in the kinematics is parallel to the arm axes.
  The offset of the Z distance from the arm axes to the end effector joints 
  at arm angle zero will be printed at startup on the serial port.

  Feedrate in gcode is in the cartesian units. This must be converted to the
  angles. This is done by calculating the segment move distance and the angle 
  move distance and applying that ration to the feedrate. 

  TODO Cleanup
  Update so extra axes get delt with ... passed through properly
  Have MPos use kinematics too
  
  ============================================================================

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

#include "../src/Settings.h"

enum class KinematicError : uint8_t {
    NONE               = 0,
    OUT_OF_RANGE       = 1,
    ANGLE_TOO_NEGATIVE = 2,
    ANGLE_TOO_POSITIVE = 3,
};

// Create custom run time $ settings
FloatSetting* kinematic_segment_len;
FloatSetting* delta_crank_len;
FloatSetting* delta_link_len;
FloatSetting* delta_crank_side_len;
FloatSetting* delta_effector_side_len;

// trigonometric constants to speed up calculations
const float sqrt3  = 1.732050807;
const float dtr    = M_PI / (float)180.0;  // degrees to radians
const float sin120 = sqrt3 / 2.0;
const float cos120 = -0.5;
const float tan60  = sqrt3;
const float sin30  = 0.5;
const float tan30  = 1.0 / sqrt3;

// the geometry of the delta
float rf;  // radius of the fixed side (length of motor cranks)
float re;  // radius of end effector side (length of linkages)
float f;   // sized of fixed side triangel
float e;   // size of end effector side triangle

static float last_angle[3]          = { 0.0, 0.0, 0.0 };  // A place to save the previous motor angles for distance/feed rate calcs
static float last_cartesian[N_AXIS] = {
    0.0, 0.0, 0.0
};  // A place to save the previous motor angles for distance/feed rate calcs                             // Z offset of the effector from the arm centers

// prototypes for helper functions
int            calc_forward_kinematics(float* angles, float* cartesian);
KinematicError delta_calcInverse(float* cartesian, float* angles);
KinematicError delta_calcAngleYZ(float x0, float y0, float z0, float& theta);
float          three_axis_dist(float* point1, float* point2);
void           read_settings();

void machine_init() {
    float angles[N_AXIS]    = { 0.0, 0.0, 0.0 };
    float cartesian[N_AXIS] = { 0.0, 0.0, 0.0 };

    // Custom $ settings
    kinematic_segment_len   = new FloatSetting(EXTENDED, WG, NULL, "Kinematics/SegmentLength", KINEMATIC_SEGMENT_LENGTH, 0.2, 1000.0);
    delta_crank_len         = new FloatSetting(EXTENDED, WG, NULL, "Delta/CrankLength", RADIUS_FIXED, 50.0, 500.0);
    delta_link_len          = new FloatSetting(EXTENDED, WG, NULL, "Delta/LinkLength", RADIUS_EFF, 50.0, 500.0);
    delta_crank_side_len    = new FloatSetting(EXTENDED, WG, NULL, "Delta/CrankSideLength", LENGTH_FIXED_SIDE, 20.0, 500.0);
    delta_effector_side_len = new FloatSetting(EXTENDED, WG, NULL, "Delta/EffectorSideLength", LENGTH_EFF_SIDE, 20.0, 500.0);

    read_settings();

    // Calculate the Z offset at the arm zero angles ...
    // Z offset is the z distance from the motor axes to the end effector axes at zero angle
    calc_forward_kinematics(angles, cartesian);  // Sets the cartesian values
    // print a startup message to show the kinematics are enabled. Print the offset for reference
    grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "Delta Kinematics Init: %s Z Offset:%4.3f", MACHINE_NAME, cartesian[Z_AXIS]);

    grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "Delta Angle Range %3.3f, %3.3f", MAX_NEGATIVE_ANGLE, MAX_POSITIVE_ANGLE);

    //     grbl_msg_sendf(CLIENT_SERIAL,
    //                    MsgLevel::Info,
    //                    "DXL_COUNT_MIN %4.0f CENTER %d MAX %4.0f PER_RAD %d",
    //                    DXL_COUNT_MIN,
    //                    DXL_CENTER,
    //                    DXL_COUNT_MAX,
    //                    DXL_COUNT_PER_RADIAN);
}
bool user_defined_homing(uint8_t cycle_mask) {  // true = do not continue with normal Grbl homing
#ifdef USE_CUSTOM_HOMING
    return true;
#else
    return false;
#endif
}

// This function is used by Grbl
void inverse_kinematics(float* position) {
    float motor_angles[3];

    read_settings();
    delta_calcInverse(position, motor_angles);
    position[0] = motor_angles[0];
    position[1] = motor_angles[1];
    position[2] = motor_angles[2];
}

// This function is used by Grbl
void inverse_kinematics(float* target, plan_line_data_t* pl_data, float* position)  //The target and position are provided in MPos
{
    float dx, dy, dz;  // distances in each cartesian axis
    float motor_angles[3];

    float seg_target[3];                  // The target of the current segment
    float feed_rate  = pl_data->feed_rate;  // save original feed rate
    bool  show_error = true;                // shows error once

    KinematicError status;

    read_settings();

    // grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "Start %3.3f %3.3f %3.3f", position[0], position[1], position[2]);
    // grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "Target %3.3f %3.3f %3.3f", target[0], target[1], target[2]);

    status = delta_calcInverse(position, motor_angles);
    if (status == KinematicError::OUT_OF_RANGE) {
        //grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "Start position error %3.3f %3.3f %3.3f", position[0], position[1], position[2]);
        //start_position_error = true;
    }

    // Check the destination to see if it is in work area
    status = delta_calcInverse(target, motor_angles);
    if (status == KinematicError::OUT_OF_RANGE) {
        grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "Target unreachable  error %3.3f %3.3f %3.3f", target[0], target[1], target[2]);
    }

    position[X_AXIS] += gc_state.coord_offset[X_AXIS];
    position[Y_AXIS] += gc_state.coord_offset[Y_AXIS];
    position[Z_AXIS] += gc_state.coord_offset[Z_AXIS];

    // calculate cartesian move distance for each axis
    dx         = target[X_AXIS] - position[X_AXIS];
    dy         = target[Y_AXIS] - position[Y_AXIS];
    dz         = target[Z_AXIS] - position[Z_AXIS];
    float dist = sqrt((dx * dx) + (dy * dy) + (dz * dz));

    // determine the number of segments we need	... round up so there is at least 1 (except when dist is 0)
    uint32_t segment_count = ceil(dist / kinematic_segment_len->get());

    float segment_dist = dist / ((float)segment_count);  // distance of each segment...will be used for feedrate conversion

    for (uint32_t segment = 1; segment <= segment_count; segment++) {
        // determine this segment's target
        seg_target[X_AXIS] = position[X_AXIS] + (dx / float(segment_count) * segment);
        seg_target[Y_AXIS] = position[Y_AXIS] + (dy / float(segment_count) * segment);
        seg_target[Z_AXIS] = position[Z_AXIS] + (dz / float(segment_count) * segment);

        // calculate the delta motor angles
        KinematicError status = delta_calcInverse(seg_target, motor_angles);

        if (status == KinematicError ::NONE) {
            float delta_distance = three_axis_dist(motor_angles, last_angle);

            // save angles for next distance calc
            memcpy(last_angle, motor_angles, sizeof(motor_angles));

            if (pl_data->motion.rapidMotion) {
                pl_data->feed_rate = feed_rate;
            } else {
                pl_data->feed_rate = (feed_rate * delta_distance / segment_dist);
            }

            mc_line(motor_angles, pl_data);

        } else {
            if (show_error) {
                // grbl_msg_sendf(CLIENT_SERIAL,
                //                MsgLevel::Info,
                //                "Error:%d, Angs X:%4.3f Y:%4.3f Z:%4.3f",
                //                status,
                //                motor_angles[0],
                //                motor_angles[1],
                //                motor_angles[2]);
                show_error = false;
            }
        }
    }
}

// this is used used by Grbl soft limits to see if the range of the machine is exceeded.
uint8_t kinematic_limits_check(float* target) {
    float motor_angles[3];

    read_settings();

    grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "Kin Soft Check %3.3f, %3.3f, %3.3f", target[0], target[1], target[2]);

    KinematicError status = delta_calcInverse(target, motor_angles);

    switch (status) {
        case KinematicError::OUT_OF_RANGE:
            grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "Kin target out of range");
            break;
        case KinematicError::ANGLE_TOO_NEGATIVE:
            grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "Kin target max negative");
            break;
        case KinematicError::ANGLE_TOO_POSITIVE:
            grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "Kin target max positive");
            break;
        case KinematicError::NONE:
            break;
    }

    return (status == KinematicError::NONE);
}

// inverse kinematics: cartesian -> angles
// returned status: 0=OK, -1=non-existing position
KinematicError delta_calcInverse(float* cartesian, float* angles) {
    angles[0] = angles[1] = angles[2] = 0;
    KinematicError status             = KinematicError::NONE;

    status = delta_calcAngleYZ(cartesian[X_AXIS], cartesian[Y_AXIS], cartesian[Z_AXIS], angles[0]);
    if (status != KinematicError ::NONE) {
        return status;
    }

    status = delta_calcAngleYZ(cartesian[X_AXIS] * cos120 + cartesian[Y_AXIS] * sin120,
                               cartesian[Y_AXIS] * cos120 - cartesian[X_AXIS] * sin120,
                               cartesian[Z_AXIS],
                               angles[1]);  // rotate coords to +120 deg
    if (status != KinematicError ::NONE) {
        return status;
    }

    status = delta_calcAngleYZ(cartesian[X_AXIS] * cos120 - cartesian[Y_AXIS] * sin120,
                               cartesian[Y_AXIS] * cos120 + cartesian[X_AXIS] * sin120,
                               cartesian[Z_AXIS],
                               angles[2]);  // rotate coords to -120 deg
    if (status != KinematicError ::NONE) {
        return status;
    }

    //grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "xyx (%4.3f,%4.3f,%4.3f) ang (%4.3f,%4.3f,%4.3f)", x0, y0, z0, theta1, theta2, theta3);
    return status;
}

// inverse kinematics: angles -> cartesian
int calc_forward_kinematics(float* angles, float* catesian) {
    float t = (f - e) * tan30 / 2;

    float y1 = -(t + rf * cos(angles[0]));
    float z1 = -rf * sin(angles[0]);

    float y2 = (t + rf * cos(angles[1])) * sin30;
    float x2 = y2 * tan60;
    float z2 = -rf * sin(angles[1]);

    float y3 = (t + rf * cos(angles[2])) * sin30;
    float x3 = -y3 * tan60;
    float z3 = -rf * sin(angles[2]);

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

    catesian[Z_AXIS] = -(float)0.5 * (b + sqrt(d)) / a;
    catesian[X_AXIS] = (a1 * catesian[Z_AXIS] + b1) / dnm;
    catesian[Y_AXIS] = (a2 * catesian[Z_AXIS] + b2) / dnm;
    return 0;
}
// helper functions, calculates angle theta1 (for YZ-pane)
KinematicError delta_calcAngleYZ(float x0, float y0, float z0, float& theta) {
    float y1 = -0.5 * 0.57735 * f;  // f/2 * tg 30
    y0 -= 0.5 * 0.57735 * e;        // shift center to edge
    // z = a + b*y
    float a = (x0 * x0 + y0 * y0 + z0 * z0 + rf * rf - re * re - y1 * y1) / (2 * z0);
    float b = (y1 - y0) / z0;
    // discriminant
    float d = -(a + b * y1) * (a + b * y1) + rf * (b * b * rf + rf);
    if (d < 0)
        return KinematicError::OUT_OF_RANGE;          // non-existing point
    float yj = (y1 - a * b - sqrt(d)) / (b * b + 1);  // choosing outer point
    float zj = a + b * yj;
    //theta    = 180.0 * atan(-zj / (y1 - yj)) / M_PI + ((yj > y1) ? 180.0 : 0.0);
    theta = atan(-zj / (y1 - yj)) + ((yj > y1) ? M_PI : 0.0);

    if (theta < MAX_NEGATIVE_ANGLE) {
        return KinematicError::ANGLE_TOO_NEGATIVE;
    }

    if (theta > MAX_POSITIVE_ANGLE) {
        return KinematicError::ANGLE_TOO_POSITIVE;
    }

    return KinematicError::NONE;
}

// Determine the unit distance between (2) 3D points
float three_axis_dist(float* point1, float* point2) {
    return sqrt(((point1[0] - point2[0]) * (point1[0] - point2[0])) + ((point1[1] - point2[1]) * (point1[1] - point2[1])) +
                ((point1[2] - point2[2]) * (point1[2] - point2[2])));
}
// called by reporting for WPos status
void forward_kinematics(float* position) {
    float calc_fwd[N_AXIS];
    int   status;

    read_settings();

    // convert the system position in steps to radians
    float   position_radians[N_AXIS];
    int32_t position_steps[N_AXIS];  // Copy current state of the system position variable
    memcpy(position_steps, sys_position, sizeof(sys_position));
    system_convert_array_steps_to_mpos(position_radians, position_steps);

    // grbl_msg_sendf(
    //     CLIENT_SERIAL, MsgLevel::Info, "Fwd Kin Angs %1.3f, %1.3f, %1.3f ", position_radians[0], position_radians[1], position_radians[2]);

    // detmine the position of the end effector joint center.
    status = calc_forward_kinematics(position_radians, calc_fwd);

    if (status == 0) {
        // apply offsets and set the returned values
        position[X_AXIS] = calc_fwd[X_AXIS] - gc_state.coord_system[X_AXIS] + gc_state.coord_offset[X_AXIS];
        position[Y_AXIS] = calc_fwd[Y_AXIS] - gc_state.coord_system[Y_AXIS] + gc_state.coord_offset[Y_AXIS];
        position[Z_AXIS] = calc_fwd[Z_AXIS] - gc_state.coord_system[Z_AXIS] + gc_state.coord_offset[Z_AXIS];
    } else {
        grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "MSG:Fwd Kin Error");
    }
}

bool kinematics_pre_homing(uint8_t cycle_mask) {  // true = do not continue with normal Grbl homing
#ifdef USE_CUSTOM_HOMING
    return true;
#else
    //grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "kinematics_pre_homing");
    return false;
#endif
}

void kinematics_post_homing() {
#ifdef USE_CUSTOM_HOMING

#else
    last_angle[X_AXIS] = sys_position[X_AXIS] / axis_settings[X_AXIS]->steps_per_mm->get();
    last_angle[Y_AXIS] = sys_position[Y_AXIS] / axis_settings[Y_AXIS]->steps_per_mm->get();
    last_angle[Z_AXIS] = sys_position[Z_AXIS] / axis_settings[Z_AXIS]->steps_per_mm->get();

    read_settings();

    calc_forward_kinematics(last_angle, last_cartesian);

    // grbl_msg_sendf(CLIENT_SERIAL,
    //                MsgLevel::Info,
    //                "kinematics_post_homing Angles: %3.3f, %3.3f, %3.3f",
    //                last_angle[X_AXIS],
    //                last_angle[Y_AXIS],
    //                last_angle[Z_AXIS]);

    // grbl_msg_sendf(CLIENT_SERIAL,
    //                MsgLevel::Info,
    //                "kinematics_post_homing Cartesian: %3.3f, %3.3f, %3.3f",
    //                last_cartesian[X_AXIS],
    //                last_cartesian[Y_AXIS],
    //                last_cartesian[Z_AXIS]);

    gc_state.position[X_AXIS] = last_cartesian[X_AXIS];
    gc_state.position[Y_AXIS] = last_cartesian[Y_AXIS];
    gc_state.position[Z_AXIS] = last_cartesian[Z_AXIS];

#endif
#ifdef USE_POST_HOMING_DELAY
    delay(1000);  // give time for servo type homing
#endif
}

void user_m30() {}

void read_settings() {
    rf = delta_crank_len->get();          // radius of the fixed side (length of motor cranks)
    re = delta_link_len->get();           // radius of end effector side (length of linkages)
    f  = delta_crank_side_len->get();     // sized of fixed side triangel
    e  = delta_effector_side_len->get();  // size of end effector side triangle
}
