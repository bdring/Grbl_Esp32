/*
    Part of Grbl_ESP32
    2021 -  Stefan de Bruijn

    Grbl_ESP32 is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Grbl_ESP32 is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Grbl_ESP32.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "ErrorPinDetail.h"
#include "../Assert.h"

namespace Pins {
    ErrorPinDetail::ErrorPinDetail(const PinOptionsParser& options) : PinDetail(0) {}

    PinCapabilities ErrorPinDetail::capabilities() const { return PinCapabilities::Error; }

    void ErrorPinDetail::write(int high) { Assert(false, "Cannot write to an error pin."); }
    int  ErrorPinDetail::read() { Assert(false, "Cannot read from an error pin."); }
    void ErrorPinDetail::setAttr(PinAttributes value) { /* Fine, this won't get you anywhere. */
    }
    PinAttributes ErrorPinDetail::getAttr() const { return PinAttributes::None; }

    String ErrorPinDetail::toString() const { return "ERROR_PIN"; }
}
