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
