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

#ifdef ESP32
#    include "../Report.h"
#endif

namespace Pins {
    ErrorPinDetail::ErrorPinDetail(const String& descr) : PinDetail(0), _description(descr) {}

    PinCapabilities ErrorPinDetail::capabilities() const { return PinCapabilities::Error; }

#ifdef ESP32
    void ErrorPinDetail::write(int high) { info_all("Cannot write to pin %s. The config is incorrect.", _description.c_str()); }
    int  ErrorPinDetail::read() {
        info_all("Cannot read from pin %s. The config is incorrect.", _description.c_str());
        return false;
    }
    void ErrorPinDetail::setAttr(PinAttributes value) {
        info_all("Cannot set mode on pin %s. The config is incorrect.", _description.c_str());
    }

#else
    void ErrorPinDetail::write(int high) { Assert(false, "Cannot write to an error pin."); }
    int  ErrorPinDetail::read() {
        Assert(false, "Cannot read from an error pin.");
        return false;
    }
    void ErrorPinDetail::setAttr(PinAttributes value) { /* Fine, this won't get you anywhere. */
    }

#endif

    PinAttributes ErrorPinDetail::getAttr() const { return PinAttributes::None; }

    String ErrorPinDetail::toString() { return "ERROR_PIN (for " + _description + ")"; }
}
