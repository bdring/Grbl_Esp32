#pragma once

#include "Tokenizer.h"

#include <stack>
#include <cstring>

namespace Configuration {
    class Parser : public Tokenizer {
        // Parsing here might be a bit confusing, because the state of the tokenizer is one step
        // ahead of the parser. That way we always have 2 tokens at our disposal, so we know when
        // we're entering or exiting a section.

        std::stack<int> indentStack_;
        TokenData       current_;

        void ParseError(const char* description) const;

    public:
        Parser(const char* start, const char* end);

        /// <summary>
        /// MoveNext: moves to the next entry in the current section. By default we're in the
        /// root section.
        /// </summary>
        bool MoveNext();

        inline bool IsEndSection() { return current_.kind_ == TokenKind::Eof || token_.indent_ < current_.indent_; }

        // !!! Important !!! We cannot use a scoped variable for enter & leave, because 'leave' can throw,
        // and it could be called using stack unrolling. Destructors by definition have to be 'nothrow',
        // so forget it: it just Won't Work. In other words, if we leave the 'leave' call up to the
        // destructor, we end up what we in C++ call 'undefined behavior'.

        void Enter();
        void Leave();

        inline bool Is(const char* expected) const {
            return !strncmp(expected, current_.keyStart_, size_t(current_.keyEnd_ - current_.keyStart_));
        }

        inline std::string Key() const { return std::string(current_.keyStart_, current_.keyEnd_); }

        std::string StringValue() const;
        bool BoolValue() const;
        int IntValue() const;
        double FloatValue() const;
    };
}
