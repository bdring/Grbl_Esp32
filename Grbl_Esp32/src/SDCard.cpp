/*
  SDCard.cpp - Adds SD Card Features to Grbl_ESP32
  Part of Grbl_ESP32

  Copyright (c) 2018 Barton Dring Buildlog.net

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

#include "Config.h"

#include "SDCard.h"
#include "Machine/MachineConfig.h"
#include "Report.h"

#include <FS.h>
#include <SD.h>
#include <SPI.h>

class SDCard::FileWrap {
public:
    FileWrap() : _file(nullptr) {}
    File _file;
};

SDCard::SDCard() :
    _pImpl(new FileWrap()), _current_line_number(0), _state(State::Idle), _readyNext(false), _client(CLIENT_SERIAL),
    _auth_level(WebUI::AuthenticationLevel::LEVEL_GUEST) {}

// attempt to mount the SD card
/*bool SDCard::mount()
{
  if(!SD.begin()) {
    report_status_message(Error::FsFailedMount, _client);
    return false;
  }
  return true;
}*/

void SDCard::listDir(fs::FS& fs, const char* dirname, uint8_t levels, uint8_t client) {
    //char temp_filename[128]; // to help filter by extension	TODO: 128 needs a definition based on something
    File root = fs.open(dirname);
    if (!root) {
        report_status_message(Error::FsFailedOpenDir, client);
        return;
    }
    if (!root.isDirectory()) {
        report_status_message(Error::FsDirNotFound, client);
        return;
    }
    File file = root.openNextFile();
    while (file) {
        if (file.isDirectory()) {
            if (levels) {
                listDir(fs, file.name(), levels - 1, client);
            }
        } else {
            grbl_sendf(CLIENT_ALL, "[FILE:%s|SIZE:%d]\r\n", file.name(), file.size());
        }
        file = root.openNextFile();
    }
}

bool SDCard::openFile(fs::FS& fs, const char* path) {
    _pImpl->_file = fs.open(path);
    if (!_pImpl->_file) {
        //report_status_message(Error::FsFailedRead, CLIENT_SERIAL);
        return false;
    }
    set_state(State::BusyPrinting);
    _readyNext           = false;  // this will get set to true when Grbl issues "ok" message
    _current_line_number = 0;
    return true;
}

bool SDCard::closeFile() {
    if (!_pImpl->_file) {
        return false;
    }
    set_state(State::Idle);
    _readyNext           = false;
    _current_line_number = 0;
    _pImpl->_file.close();
    SD.end();
    return true;
}

/*
  read a line from the SD card
  strip whitespace
  strip comments per http://linuxcnc.org/docs/ja/html/gcode/overview.html#gcode:comments
  make uppercase
  return true if a line is
*/
bool SDCard::readFileLine(char* line, int maxlen) {
    if (!_pImpl->_file) {
        report_status_message(Error::FsFailedRead, _client);
        return false;
    }

    _current_line_number += 1;
    int len = 0;
    while (_pImpl->_file.available()) {
        if (len >= maxlen) {
            return false;
        }
        char c = _pImpl->_file.read();
        if (c == '\n') {
            break;
        }
        line[len++] = c;
    }
    line[len] = '\0';
    return len || _pImpl->_file.available();
}

// return a percentage complete 50.5 = 50.5%
float SDCard::report_perc_complete() {
    if (!_pImpl->_file) {
        return 0.0;
    }
    return (float)_pImpl->_file.position() / (float)_pImpl->_file.size() * 100.0f;
}

uint32_t SDCard::get_current_line_number() {
    return _current_line_number;
}

SDCard::State SDCard::get_state(bool refresh) {
    // Before we use the SD library, we *must* make sure SPI is properly initialized. Re-initialization
    // fortunately doesn't change any of the settings.
    auto spiConfig = config->_spi;

    if (spiConfig != nullptr) {
        auto csPin = spiConfig->_cs.getNative(Pin::Capabilities::Output | Pin::Capabilities::Native);

        //no need to go further if SD detect is not correct
        if (config->_sdCard->_cardDetect.defined() && !config->_sdCard->_cardDetect.read()) {
            _state = SDCard::State::NotPresent;
            return _state;
        }

        //if busy doing something return state
        if (!((_state == SDCard::State::NotPresent) || (_state == SDCard::State::Idle))) {
            return _state;
        }
        if (!refresh) {
            return _state;  //to avoid refresh=true + busy to reset SD and waste time
        }

        //SD is idle or not detected, let see if still the case
        SD.end();

        _state = SDCard::State::NotPresent;

        //refresh content if card was removed
        if (SD.begin(csPin, SPI, SPIfreq, "/sd", 2)) {
            if (SD.cardSize() > 0) {
                _state = SDCard::State::Idle;
            }
        }
        return _state;
    } else {
        return SDCard::State::NotPresent;
    }
}

SDCard::State SDCard::set_state(SDCard::State state) {
    _state = state;
    return _state;
}

void SDCard::get_current_filename(char* name) {
    if (_pImpl->_file) {
        strcpy(name, _pImpl->_file.name());
    } else {
        name[0] = 0;
    }
}

void SDCard::init() {
    static bool init_message = true;  // used to show messages only once.

    if (init_message) {
        _cardDetect.report("SD Card Detect");
        init_message = false;
    }

    _cardDetect.setAttr(Pin::Attr::Output);
}

SDCard::~SDCard() {
    delete _pImpl;
}
