#pragma once

#include "HandlerType.h"
#include "../Pin.h"
#include "../StringRange.h"

namespace Configuration
{
    class Configurable;

    template <typename BaseType>
    class GenericFactory;

    class HandlerBase
    {
    protected:
        virtual void handleDetail(const char* name, Configurable* value) = 0;
        virtual bool matchesUninitialized(const char* name) = 0;

        template <typename BaseType>
        friend class GenericFactory;

    public:
        virtual void handle(const char* name, bool& value) = 0;
        virtual void handle(const char* name, int& value) = 0;
        virtual void handle(const char* name, double& value) = 0;
        virtual void handle(const char* name, float& value) = 0;
        virtual void handle(const char* name, StringRange value) = 0;
        virtual void handle(const char* name, Pin& value) = 0;

        virtual HandlerType handlerType() = 0;

        template <typename T>
        void handle(const char* name, T*& value) {
            if (handlerType() == HandlerType::Parser)
            {
                if (value == nullptr && matchesUninitialized(name))
                {
                    value = new T();
                    handleDetail(name, value);
                }
            }
            else {
                if (value != nullptr) {
                    handleDetail(name, value);
                }
            }
        }

        template <typename T>
        void handle(const char* name, T& value) { handleDetail(name, &value); }
    };
}
