/*
  Copyright (c) 2021 Brian T. Park.

  API derived from:
  EEPROM.cpp - esp8266 EEPROM emulation

  Copyright (c) 2014 Ivan Grokhotkov. All rights reserved.
  This file is part of the esp8266 core for Arduino environment.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <stdlib.h> // getenv()
#include <stdio.h> // FILE, fopen(), fwrite(), fread()
#include "EpoxyEepromEsp.h"

void EpoxyEepromEsp::begin(size_t size) {
  // round up to nearest 4-byte alignment
  size = (size + 3) & (~3);

  if (size == size_) return;
  size_ = size;

  if (data_) {
    delete[] data_;
  }
  data_ = new uint8_t[size_];
  dirty_ = false;

  const char* dataPath = getDataPath();
  FILE* fp = fopen(dataPath, "rb");
  if (fp) {
    fread(data_, 1, size_, fp);
    fclose(fp);
  }
}

bool EpoxyEepromEsp::commit() {
  if (!size_ || !dirty_ || !data_) return true;
  const char* dataPath = getDataPath();
  FILE* fp = fopen(dataPath, "wb");
  if (!fp) return false;
  fwrite(data_, 1, size_, fp);
  fclose(fp);
  return true;
}

const char* EpoxyEepromEsp::getDataPath() {
  const char* dataPath = getenv("EPOXY_EEPROM_DATA");
  if (!dataPath) {
    dataPath = "epoxyeepromdata";
  }
  return dataPath;
}

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_EEPROM)
EpoxyEepromEsp EpoxyEepromEspInstance;
#endif
