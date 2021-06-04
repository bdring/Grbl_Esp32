#include "../TestFramework.h"

#include <src/Configuration/Tokenizer.h>
#include <src/Configuration/Parser.h>

namespace Configuration {
    Test(YamlParser, BasicProperties) {
        const char* config = 
            "a: aap\n"
            "b: banaan\n"
            "\n"
            "c: chocolade";

        Parser      p(config, config + strlen(config));
        {
            Assert(p.key().equals("a"), "Expected 'a'");
            Assert(p.stringValue().equals("aap"), "Expected 'aap'");
        }

        Assert(p.moveNext(), "Move next failed.");
        {
            Assert(p.key().equals("b"), "Expected 'b'");
            Assert(p.stringValue().equals("banaan"), "Expected 'banaan'");
        }

        Assert(p.moveNext(), "Move next failed.");
        {
            Assert(p.key().equals("c"), "Expected 'c'");
            Assert(p.stringValue().equals("chocolade"), "Expected 'chocolade'");
        }
        Assert(!p.moveNext(), "Move next failed.");
    }

     Test(YamlParser, SimpleSection) {
        const char* config = "a: aap\nb: banaan\n\nc: chocolade";
        Parser      p(config, config + strlen(config));
        {
            Assert(p.key().equals("a"), "Expected 'a'");
            Assert(p.stringValue().equals("aap"), "Expected 'aap'");
        }

        Assert(p.moveNext(), "Move next failed.");
        {
            Assert(p.key().equals("b"), "Expected 'b'");
            Assert(p.stringValue().equals("banaan"), "Expected 'banaan'");
        }

        Assert(p.moveNext(), "Move next failed.");
        {
            Assert(p.key().equals("c"), "Expected 'c'");
            Assert(p.stringValue().equals("chocolade"), "Expected 'chocolade'");
        }
        Assert(!p.moveNext(), "Move next failed.");
    }

}
