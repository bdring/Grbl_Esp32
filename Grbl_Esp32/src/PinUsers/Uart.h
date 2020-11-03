#pragma once

#include "../Pin.h"
#include <WString.h>
#include "../Assert.h"

// TODO FIXME: Uart also suffers from the way settings works: What you would want is 2 phases,
// so (1) validation and preparation, and (2) building the actual uart. Now, it's all combined
// in the single constructor, which works, but could throw an exception.

namespace PinUsers {
    class UartDetail {
    public:
        virtual int write(const uint8_t* ptr, int bytes)           = 0;
        virtual int read(uint8_t* ptr, int bytes, int ticksToWait) = 0;

        virtual ~UartDetail() {}
    };

    // Pin user for UART.
    class Uart {
        Pin _tx;
        Pin _rx;
        Pin _rts;

        UartDetail* _detail;

    public:
        Uart() : _tx(Pin::UNDEFINED), _rx(Pin::UNDEFINED), _rts(Pin::UNDEFINED), _detail(nullptr) {}
        Uart(Pin tx, Pin rx, Pin rts, String config, String userConfig);

        // TODO FIXME: If _detail is null is currently asserted. We can also just 'do nothing', which might
        // be easier in the application.

        // Writes a buffer to the uart. Returns the number of _bytes_ written
        template <typename T>
        int writePartial(const T& buffer, int byteOffset = 0) {
            return writePartial(&buffer, 1, byteOffset);
        }

        // Writes a buffer to the uart. Returns the number of _bytes_ written
        template <typename T>
        int writePartial(const T* bufferArray, int numberElements, int byteOffset = 0) {
            Assert(_detail != nullptr, "Uart is not initialized; cannot write to it.");
            auto byteBuf = reinterpret_cast<const uint8_t*>(bufferArray);
            return _detail->write(byteBuf + byteOffset, sizeof(T) * numberElements - byteOffset);
        }

        // Writes a buffer to the uart. This continues while it can write. If writing succeeded, true is
        // returned, otherwise false.
        template <typename T>
        bool write(const T& buffer) {
            return write(&buffer, 1);
        }

        // Writes a buffer to the uart. This continues while it can write. If writing succeeded, true is
        // returned, otherwise false.
        template <typename T>
        bool write(const T* buffer, int numberElements) {
            auto index = 0;
            auto limit = sizeof(T) * numberElements;
            while (index < limit) {
                int size = writePartial(buffer, numberElements, index);
                index += size;

                if (size == 0) {
                    return false;
                }
            }
            return true;
        }

        // Reads a buffer from the uart. If ticks elapsed, stops reading. Returns the number of bytes
        // that was read.
        template <typename T>
        int readPartial(T* bufferArray, int numberElements, int offset = 0, int ticksToWait = 0) {
            Assert(_detail != nullptr, "Uart is not initialized; cannot write to it.");

            auto byteArray = reinterpret_cast<uint8_t*>(bufferArray) + offset;
            auto bytes     = sizeof(T) * numberElements - offset;
            return _detail->read(byteArray, bytes, ticksToWait);
        }

        // Reads a buffer from the uart. If ticks elapsed, stops reading. Returns the number of bytes
        // that was read.
        template <typename T>
        int readPartial(T& buffer, int offset = 0, int ticksToWait = 0) {
            return readPartial(&buffer, 1, offset, ticksToWait);
        }

        // Reads a buffer from the uart within the given time. The return value is 'true' if this succeeded
        // and 'false' if it failed or gave a partial result.
        template <typename T>
        bool readOnce(T* bufferArray, int numberElements, int ticksToWait = 0) {
            auto bytesRead = readPartial(bufferArray, numberElements, 0, ticksToWait);
            return bytesRead == (numberElements * sizeof(T));
        }

        // Reads a buffer from the uart within the given time. The return value is 'true' if this succeeded
        // and 'false' if it failed or gave a partial result.
        template <typename T>
        bool readOnce(T& buffer, int ticksToWait = 0) {
            return readOnce(&buffer, 1, ticksToWait);
        }

        // Reads a buffer from the uart. While data is returned, this keeps on reading until the buffer is
        // full. If no data is returned anymore, or if the buffer is full, this returns 'true' if successul,
        // or 'false' otherwise
        template <typename T>
        bool readBlocking(T* bufferArray, int numberElements, int ticksToWait = 0) {
            int offset = 0;
            int limit  = sizeof(T) * numberElements;
            while (offset < limit) {
                auto bytesRead = readPartial(bufferArray, numberElements, offset, ticksToWait);
                if (bytesRead == 0) {
                    break;
                }
                offset += bytesRead;
            }
            return offset == limit;
        }

        // Reads a buffer from the uart within the given time. The return value is 'true' if this succeeded
        // and 'false' if it failed or gave a partial result.
        template <typename T>
        bool readBlocking(T& buffer, int offset = 0, int ticksToWait = 0) {
            return readBlocking(&buffer, 1, ticksToWait);
        }

        inline ~Uart() { delete _detail; }
    };
}
