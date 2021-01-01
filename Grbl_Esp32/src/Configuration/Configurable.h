#pragma once

#include "Generator.h"
#include "Parser.h"

namespace Configuration
{
    class Configurable
    {
        Configurable(const Configurable&) = delete;
        Configurable& operator=(const Configurable&) = delete;

    public:
        Configurable() = default;

        virtual void generate(Generator& generator) const = 0;
    };
}