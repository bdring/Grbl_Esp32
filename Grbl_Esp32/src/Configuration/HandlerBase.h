#pragma once

#include "HandlerType.h"
#include "../Pin.h"
#include "../StringRange.h"

namespace Configuration {
    class Configurable;

    template <typename BaseType>
    class GenericFactory;

    class HandlerBase {
    protected:
        virtual void handleDetail(const char* name, Configurable* value) = 0;
        virtual bool matchesUninitialized(const char* name)              = 0;

        template <typename BaseType>
        friend class GenericFactory;

    public:
        virtual void handle(const char* name, bool& value) = 0;
        virtual void handle(const char* name, int& value)  = 0;
        virtual void handle(const char* name, uint32_t& value) {
            int32_t v = int32_t(value);
            handle(name, v);
            value = uint32_t(v);
        }

        // TODO: This will accept values that cannot fit in the variable
        virtual void handle(const char* name, uint8_t& value) {
            int32_t v = int32_t(value);
            handle(name, v);
            value = uint8_t(v);
        }

        virtual void handle(const char* name, double& value)      = 0;
        virtual void handle(const char* name, float& value)       = 0;
        virtual void handle(const char* name, StringRange& value) = 0;
        virtual void handle(const char* name, Pin& value)         = 0;

        template <typename T>
        inline void handle(const char* name, volatile T& value) {
            T val = value;
            handle(name, val);
            value = val;
        }

        virtual void handle(const char* name, String& value) {
            StringRange range;
            handle(name, range);
            if (range.begin() != nullptr) {
                value = range.str();
            }
        }

        virtual HandlerType handlerType() = 0;

        template <typename T>
        void handle(const char* name, T*& value) {
            if (handlerType() == HandlerType::Parser) {
                if (value == nullptr && matchesUninitialized(name)) {
                    value = new T();
                    handleDetail(name, value);
                }
            } else {
                if (value != nullptr) {
                    handleDetail(name, value);
                }
            }
        }

        template <typename T>
        void handle(const char* name, T& value) {
            handleDetail(name, &value);
        }
    };
}
