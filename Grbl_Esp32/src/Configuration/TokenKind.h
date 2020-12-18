#pragma once

namespace Configuration {

    enum struct TokenKind {
        Section,
        Boolean,
        String,
        IntegerValue,
        FloatingPoint,
        Eof,
    };
}
