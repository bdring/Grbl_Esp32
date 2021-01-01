#include "Parser.h"

#include "ParseException.h"

namespace Configuration {
    Parser::Parser(const char* start, const char* end) : Tokenizer(start, end), current_() {
        Tokenize();
        current_ = token_;
        if (current_.kind_ != TokenKind::Eof) {
            Tokenize();
        }
    }

    void Parser::parseError(const char* description) const {
        // Attempt to use the correct position in the parser:
        if (current_.keyEnd_) {
            throw ParseException(start_, current_.keyEnd_, description);
        }
        else {
            Tokenizer::ParseError(description);
        }
    }

    /// <summary>
    /// MoveNext: moves to the next entry in the current section. By default we're in the
    /// root section.
    /// </summary>
    bool Parser::moveNext() {
        // While the indent of the token is > current indent, we have to skip it. This is a
        // sub-section, that we're apparently not interested in.
        while (token_.indent_ > current_.indent_) {
            Tokenize();
        }

        // If the indent is the same, we're in the same section. Update current, move to next
        // token.
        if (token_.indent_ == current_.indent_) {
            current_ = token_;
            Tokenize();
        } else {
            // Apparently token_.indent < current_.indent_, which means we have no more items
            // in our tokenizer that are relevant.
            //
            // Note that we want to preserve current_.indent_!
            current_.kind_ = TokenKind::Eof;
        }

        return current_.kind_ != TokenKind::Eof;
    }

    void Parser::enter() {
        indentStack_.push(current_.indent_);

        // If we can enter, token_.indent_ > current_.indent_:
        if (token_.indent_ > current_.indent_) {
            current_ = token_;
            Tokenize();
        } else {
            current_         = TokenData();
            current_.indent_ = INT_MAX;
        }
        indent_ = current_.indent_;
    }

    void Parser::leave() {
        // While the indent of the tokenizer is >= current, we can ignore the contents:
        while (token_.indent_ >= current_.indent_ && token_.kind_ != TokenKind::Eof) {
            Tokenize();
        }

        // At this point, we just know the indent is smaller. We don't know if we're in
        // the *right* section tho.
        auto last = indentStack_.top();
        indent_ = last;
        indentStack_.pop();

        if (last == token_.indent_) {
            // Yes, the token continues where we left off:
            current_ = token_;
            // Tokenize(); --> No need, this is handled by MoveNext!
        } else {
            current_         = TokenData();
            current_.indent_ = last;
        }
    }

    std::string Parser::stringValue() const {
        if (current_.kind_ != TokenKind::String) {
            parseError("Expected a string value (e.g. 'foo')");
        }
        return std::string(current_.sValueStart_, current_.sValueEnd_);
    }
    
    bool Parser::boolValue() const {
        if (current_.kind_ != TokenKind::Boolean) {
            parseError("Expected a boolean value (e.g. true or value)");
        }
        return current_.bValue_;
    }
    
    int Parser::intValue() const {
        if (current_.kind_ != TokenKind::Boolean) {
            parseError("Expected an integer value (e.g. 123456)");
        }
        return current_.iValue_;
    }
    
    double Parser::floatValue() const {
        if (current_.kind_ != TokenKind::Boolean) {
            parseError("Expected a float value (e.g. 123.456)");
        }
        return current_.fValue_;
    }

    Pin Parser::pinValue() const
    {
        if (current_.kind_ != TokenKind::String) {
            parseError("Expected a string value (e.g. 'foo')");
        }
        return Pin(std::string(current_.sValueStart_, current_.sValueEnd_));
    }

}
