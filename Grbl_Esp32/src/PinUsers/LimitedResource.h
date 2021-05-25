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

#pragma once

#include <cstdint>

namespace PinUsers {
    /// <summary>
    /// Helper class that to manage 'Count' resources. Resources can be claimed and released at
    /// any time. This helper class keeps track of a bitmap that holds the resources.
    /// </summary>
    template <int Count>
    class LimitedResource {
        static_assert(Count > 0, "Resource count cannot be 0 or negative.");

        // We do +32 instead of +31 because 0 should round to 1. Worse case we loose 1 uint.
        uint32_t _claimed[(Count + 31) / 32];

    public:
        inline void forceClaim(int index) { _claimed[index / 32] |= (1 << (index % 32)); }

        inline int tryClaim() {
            for (int i = 0; i < Count; ++i) {
                auto bit = _claimed[i / 32] & (uint32_t(1) << (i % 32));
                if (bit == 0) {
                    _claimed[i / 32] |= (uint32_t(1) << (i % 32));
                    return bit;
                }
            }
            return -1;
        }

        inline void release(int index) { _claimed[index / 32] &= ~(uint32_t(1) << (index % 32)); }
    };
}
