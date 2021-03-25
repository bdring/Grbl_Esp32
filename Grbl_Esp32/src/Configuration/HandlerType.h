#pragma once

namespace Configuration
{
    enum struct HandlerType
    {
        Parser,
        AfterParse,
        Runtime,
        Generator,
        Validator
    };
}
