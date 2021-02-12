#pragma once

#include "HandlerBase.h"
#include "Parser.h"
#include "Configurable.h"

#include "../Logging.h"

namespace Configuration
{
    class ParserHandler : public Configuration::HandlerBase
    {
    private:
        Configuration::Parser& parser_;

    protected:
        void handleDetail(const char* name, Configuration::Configurable* value) override {
            if (value != nullptr && parser_.is(name)) {
                debug("Parsing configurable " << name);

                parser_.enter();
                for (; !parser_.isEndSection(); parser_.moveNext()) {
                    value->handle(*this);
                }
                parser_.leave();
            }
        }

        bool matchesUninitialized(const char* name) override {
            return parser_.is(name);
        }

    public:
        ParserHandler(Configuration::Parser& parser) : parser_(parser) {}

        void handle(const char* name, int& value) override {
            if (parser_.is(name)) { value = parser_.intValue(); }
        }

        void handle(const char* name, double& value) override {
            if (parser_.is(name)) { value = parser_.doubleValue(); }
        }

        void handle(const char* name, StringRange value) override {
            if (parser_.is(name)) { value = parser_.stringValue(); }
        }

        void handle(const char* name, Pin& value) override {
            if (parser_.is(name)) { value = parser_.pinValue(); }
        }

        HandlerType handlerType() override { return HandlerType::Parser; }
    };
}