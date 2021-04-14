/*
  Error.cpp - Error names
  Part of Grbl
  Copyright (c) 2014-2016 Sungeun K. Jeon for Gnea Research LLC

	2018 -	Bart Dring This file was modifed for use on the ESP32
					CPU. Do not use this with Grbl for atMega328P
        2020 - Mitch Bradley

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
*/

#include "Error.h"

std::map<Error, const char*> ErrorNames = {
    { Error::Ok, "No error" },
    { Error::ExpectedCommandLetter, "Expected GCodecommand letter" },
    { Error::BadNumberFormat, "Bad GCode number format" },
    { Error::InvalidStatement, "Invalid $ statement" },
    { Error::NegativeValue, "Negative value" },
    { Error::SettingDisabled, "Setting disabled" },
    { Error::SettingStepPulseMin, "Step pulse too short" },
    { Error::SettingReadFail, "Failed to read settings" },
    { Error::IdleError, "Command requires idle state" },
    { Error::SystemGcLock, "GCode cannot be executed in lock or alarm state" },
    { Error::SoftLimitError, "Soft limit error" },
    { Error::Overflow, "Line too long" },
    { Error::MaxStepRateExceeded, "Max step rate exceeded" },
    { Error::CheckDoor, "Check door" },
    { Error::LineLengthExceeded, "Startup line too long" },
    { Error::TravelExceeded, "Max travel exceeded during jog" },
    { Error::InvalidJogCommand, "Invalid jog command" },
    { Error::SettingDisabledLaser, "Laser mode requires PWM output" },
    { Error::HomingNoCycles, "No Homing/Cycle defined in settings" },
    { Error::GcodeUnsupportedCommand, "Unsupported GCode command" },
    { Error::GcodeModalGroupViolation, "Gcode modal group violation" },
    { Error::GcodeUndefinedFeedRate, "Gcode undefined feed rate" },
    { Error::GcodeCommandValueNotInteger, "Gcode command value not integer" },
    { Error::GcodeAxisCommandConflict, "Gcode axis command conflict" },
    { Error::GcodeWordRepeated, "Gcode word repeated" },
    { Error::GcodeNoAxisWords, "Gcode no axis words" },
    { Error::GcodeInvalidLineNumber, "Gcode invalid line number" },
    { Error::GcodeValueWordMissing, "Gcode value word missing" },
    { Error::GcodeUnsupportedCoordSys, "Gcode unsupported coordinate system" },
    { Error::GcodeG53InvalidMotionMode, "Gcode G53 invalid motion mode" },
    { Error::GcodeAxisWordsExist, "Gcode extra axis words" },
    { Error::GcodeNoAxisWordsInPlane, "Gcode no axis words in plane" },
    { Error::GcodeInvalidTarget, "Gcode invalid target" },
    { Error::GcodeArcRadiusError, "Gcode arc radius error" },
    { Error::GcodeNoOffsetsInPlane, "Gcode no offsets in plane" },
    { Error::GcodeUnusedWords, "Gcode unused words" },
    { Error::GcodeG43DynamicAxisError, "Gcode G43 dynamic axis error" },
    { Error::GcodeMaxValueExceeded, "Gcode max value exceeded" },
    { Error::PParamMaxExceeded, "P param max exceeded" },
    { Error::FsFailedMount, "Failed to mount device" },
    { Error::FsFailedRead, "Failed to read" },
    { Error::FsFailedOpenDir, "Failed to open directory" },
    { Error::FsDirNotFound, "Directory not found" },
    { Error::FsFileEmpty, "File empty" },
    { Error::FsFileNotFound, "File not found" },
    { Error::FsFailedOpenFile, "Failed to open file" },
    { Error::FsFailedBusy, "Device is busy" },
    { Error::FsFailedDelDir, "Failed to delete directory" },
    { Error::FsFailedDelFile, "Failed to delete file" },
    { Error::BtFailBegin, "Bluetooth failed to start" },
    { Error::WifiFailBegin, "WiFi failed to start" },
    { Error::NumberRange, "Number out of range for setting" },
    { Error::InvalidValue, "Invalid value for setting" },
    { Error::MessageFailed, "Failed to send message" },
    { Error::NvsSetFailed, "Failed to store setting" },
    { Error::NvsGetStatsFailed, "Failed to get setting status" },
    { Error::AuthenticationFailed, "Authentication failed!" },
    { Error::AnotherInterfaceBusy, "Another interface is busy" },
};
