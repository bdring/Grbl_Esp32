#ifndef ESP32

#    include "Parser.h"

#    include <string>
#    include <fstream>
#    include <iostream>
#    include <streambuf>

namespace Configuration {

    void ParseSpecificAxis(Parser& parser, int axis, int ganged) {
        const char* allAxis = "xyzabc";
        std::cout << "Parsing axis " << allAxis[axis] << ", ganged #" << ganged << std::endl;

        for (; !parser.IsEndSection(); parser.MoveNext()) {
            if (parser.Is("limit")) {
                auto limitPin = parser.StringValue();
                std::cout << "Limit pin: " << limitPin << std::endl;
            }
            // and so on...
        }
    }

    void ParseAxis(Parser& parser) {
        std::cout << "Parsing axis." << std::endl;

        const char* allAxis = "xyzabc";

        for (; !parser.IsEndSection(); parser.MoveNext()) {
            auto str = parser.Key();
            if (str.size() == 1) {
                auto idx = strchr(allAxis, str[0]);
                if (idx != nullptr) {
                    parser.Enter();
                    ParseSpecificAxis(parser, idx - allAxis, 0);
                    parser.Leave();
                }
            } else if (str.size() == 2) {
                auto idx = strchr(allAxis, str[0]);
                if (idx != nullptr && str[1] >= '1' && str[1] <= '9') {
                    int ganged = str[1] - '1';

                    parser.Enter();
                    ParseSpecificAxis(parser, idx - allAxis, ganged);
                    parser.Leave();
                }
            }
        }
    }

    void ParseBus(Parser& parser) {
        std::cout << "Parsing bus." << std::endl;

        // TODO
    }

    void ParseRoot(Parser& parser) {
        std::cout << "Parsing root." << std::endl;

        for (; !parser.IsEndSection(); parser.MoveNext()) {
            if (parser.Is("axis")) {
                parser.Enter();
                ParseAxis(parser);
                parser.Leave();
            } else if (parser.Is("bus")) {
                parser.Enter();
                ParseBus(parser);
                parser.Leave();
            }
        }
    }

    int main() {
        std::ifstream t("..\\Fiddling\\Test.yaml");
        std::string   str((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());

        const auto begin = str.c_str();
        const auto end   = begin + str.size();

        try {
            Parser parser(begin, end);
            ParseRoot(parser);
        } catch (ParseException ex) {
            std::cout << "Parse error: " << ex.What() << " @ " << ex.LineNumber() << ":" << ex.ColumnNumber() << std::endl;
        } catch (...) { std::cout << "Uncaught exception" << std::endl; }

        std::string s;
        std::getline(std::cin, s);
        return 0;
    }
}
#endif
