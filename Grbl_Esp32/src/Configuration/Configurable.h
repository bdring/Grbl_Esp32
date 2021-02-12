#pragma once

#include "Generator.h"
#include "Parser.h"

namespace Configuration
{
    class HandlerBase;

    class Configurable
    {
        Configurable(const Configurable&) = delete;
        Configurable(Configurable&&) = default;

        Configurable& operator=(const Configurable&) = delete;
        Configurable& operator=(Configurable&&) = default;

    public:
        Configurable() = default;

        virtual void validate() const = 0;
        virtual void handle(HandlerBase& handler) = 0;
        // virtual const char* name() const = 0;

        virtual ~Configurable() {}
    };
}