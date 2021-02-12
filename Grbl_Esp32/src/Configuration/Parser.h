#pragma once

#include "Tokenizer.h"
#include "../Pin.h"
#include "../StringRange.h"

#include <stack>
#include <cstring>

namespace Configuration {
    class Parser : public Tokenizer {
        // Parsing here might be a bit confusing, because the state of the tokenizer is one step
        // ahead of the parser. That way we always have 2 tokens at our disposal, so we know when
        // we're entering or exiting a section.

        std::stack<int> indentStack_;
        TokenData       current_;
        int indent_ = 0;

        void parseError(const char* description) const;

    public:
        Parser(const char* start, const char* end);

        /// <summary>
        /// MoveNext: moves to the next entry in the current section. By default we're in the
        /// root section.
        /// </summary>
        bool moveNext();

        inline bool isEndSection() { return current_.kind_ == TokenKind::Eof || current_.indent_ < indent_; }

        // !!! Important !!! We cannot use a scoped variable for enter & leave, because 'leave' can throw,
        // and it could be called using stack unrolling. Destructors by definition have to be 'nothrow',
        // so forget it: it just Won't Work. In other words, if we leave the 'leave' call up to the
        // destructor, we end up what we in C++ call 'undefined behavior'.

        void enter();
        void leave();

        inline bool is(const char* expected) const {
            return current_.keyStart_ != nullptr && 
                   !strncmp(expected, current_.keyStart_, size_t(current_.keyEnd_ - current_.keyStart_));
        }

        inline StringRange key() const { return StringRange(current_.keyStart_, current_.keyEnd_); }

        StringRange stringValue() const;
        bool boolValue() const;
        int intValue() const;
        double doubleValue() const;
        Pin pinValue() const;
    };
}
