#include "../TestFramework.h"

#include <src/Configuration/Tokenizer.h>
#include <src/Configuration/Parser.h>

namespace Configuration {
    Test(YamlParser, BasicProperties) {
        const char* config = "a: aap\n"
                             "b: banaan\n"
                             "\n"
                             "c: chocolade";

        Parser p(config, config + strlen(config));
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
        const char* config = "a: aap\n"
                             "s:\n"
                             "  b: banaan\n"
                             "\n"
                             "c: chocolade";

        Parser p(config, config + strlen(config));
        {
            Assert(p.key().equals("a"), "Expected 'a'");
            Assert(p.stringValue().equals("aap"), "Expected 'aap'");
        }

        Assert(p.moveNext(), "Move next failed.");
        {
            Assert(p.key().equals("s"), "Expected 's'");
            p.enter();
            {
                Assert(p.key().equals("b"), "Expected 'b'");
                Assert(p.stringValue().equals("banaan"), "Expected 'banaan'");
            }
            Assert(!p.moveNext(), "Move next failed.");

            p.leave();
        }

        Assert(p.moveNext(), "Move next failed.");
        {
            Assert(p.key().equals("c"), "Expected 'c'");
            Assert(p.stringValue().equals("chocolade"), "Expected 'chocolade'");
        }
        Assert(!p.moveNext(), "Move next failed.");
    }

    Test(YamlParser, TwoSequentialSections) {
        const char* config = "a: aap\n"
                             "s:\n"
                             "  b: banaan\n"
                             "t:\n"
                             "  c: chocolade\n"
                             "\n"
                             "w: wipwap";

        Parser p(config, config + strlen(config));
        {
            Assert(p.key().equals("a"), "Expected 'a'");
            Assert(p.stringValue().equals("aap"), "Expected 'aap'");
        }

        Assert(p.moveNext(), "Move next failed.");
        {
            Assert(p.key().equals("s"), "Expected 's'");
            p.enter();
            {
                Assert(p.key().equals("b"), "Expected 'b'");
                Assert(p.stringValue().equals("banaan"), "Expected 'banaan'");
            }
            Assert(!p.moveNext(), "Move next failed.");

            p.leave();
        }

        Assert(p.moveNext(), "Move next failed.");
        {
            Assert(p.key().equals("t"), "Expected 't'");
            p.enter();
            {
                Assert(p.key().equals("c"), "Expected 'c'");
                Assert(p.stringValue().equals("chocolade"), "Expected 'chocolade'");
            }
            Assert(!p.moveNext(), "Move next failed.");

            p.leave();
        }

        Assert(p.moveNext(), "Move next failed.");
        {
            Assert(p.key().equals("w"), "Expected 'w'");
            Assert(p.stringValue().equals("wipwap"), "Expected 'wipwap'");
        }
        Assert(!p.moveNext(), "Move next failed.");
    }

    Test(YamlParser, TwoSequentialSectionsSkipFirst) {
        const char* config = "a: aap\n"
                             "s:\n"
                             "  b: banaan\n"
                             "t:\n"
                             "  c: chocolade\n"
                             "\n"
                             "w: wipwap";

        Parser p(config, config + strlen(config));
        {
            Assert(p.key().equals("a"), "Expected 'a'");
            Assert(p.stringValue().equals("aap"), "Expected 'aap'");
        }

        Assert(p.moveNext(), "Move next failed.");
        {
            Assert(p.key().equals("s"), "Expected 's'");
        }

        Assert(p.moveNext(), "Move next failed.");
        {
            Assert(p.key().equals("t"), "Expected 't'");
            p.enter();
            {
                Assert(p.key().equals("c"), "Expected 'c'");
                Assert(p.stringValue().equals("chocolade"), "Expected 'chocolade'");
            }
            Assert(!p.moveNext(), "Move next failed.");

            p.leave();
        }

        Assert(p.moveNext(), "Move next failed.");
        {
            Assert(p.key().equals("w"), "Expected 'w'");
            Assert(p.stringValue().equals("wipwap"), "Expected 'wipwap'");
        }
        Assert(!p.moveNext(), "Move next failed.");
    }

    
    Test(YamlParser, TwoSequentialSectionsSkipSecond) {
        const char* config = "a: aap\n"
                             "s:\n"
                             "  b: banaan\n"
                             "t:\n"
                             "  c: chocolade\n"
                             "\n"
                             "w: wipwap";

        Parser p(config, config + strlen(config));
        {
            Assert(p.key().equals("a"), "Expected 'a'");
            Assert(p.stringValue().equals("aap"), "Expected 'aap'");
        }

        Assert(p.moveNext(), "Move next failed.");
        {
            Assert(p.key().equals("s"), "Expected 's'");
            p.enter();
            {
                Assert(p.key().equals("b"), "Expected 'b'");
                Assert(p.stringValue().equals("banaan"), "Expected 'banaan'");
            }
            Assert(!p.moveNext(), "Move next failed.");

            p.leave();
        }

        Assert(p.moveNext(), "Move next failed.");
        {
            Assert(p.key().equals("t"), "Expected 't'");
        }

        Assert(p.moveNext(), "Move next failed.");
        {
            Assert(p.key().equals("w"), "Expected 'w'");
            Assert(p.stringValue().equals("wipwap"), "Expected 'wipwap'");
        }
        Assert(!p.moveNext(), "Move next failed.");
    }

    Test(YamlParser, TwoSequentialSectionsInASection) {
        const char* config = "a: aap\n"
                             "r:\n"
                             "  s:\n"
                             "    b: banaan\n"
                             "    d: dinges\n"
                             "  t:\n"
                             "    c: chocolade\n"
                             "    e: eventjes\n"
                             "\n"
                             "w: wipwap";

        Parser p(config, config + strlen(config));
        {
            Assert(p.key().equals("a"), "Expected 'a'");
            Assert(p.stringValue().equals("aap"), "Expected 'aap'");
        }

        Assert(p.moveNext(), "Move next failed.");
        {
            Assert(p.key().equals("r"), "Expected 'r'");
            p.enter();

            {
                Assert(p.key().equals("s"), "Expected 's'");
                p.enter();
                {
                    Assert(p.key().equals("b"), "Expected 'b'");
                    Assert(p.stringValue().equals("banaan"), "Expected 'banaan'");
                }
                Assert(p.moveNext(), "Move next failed.");
                {
                    Assert(p.key().equals("d"), "Expected 'd'");
                    Assert(p.stringValue().equals("dinges"), "Expected 'dinges'");
                }
                Assert(!p.moveNext(), "Move next failed.");

                p.leave();
            }

            Assert(p.moveNext(), "Move next failed.");
            {
                Assert(p.key().equals("t"), "Expected 't'");
                p.enter();
                {
                    Assert(p.key().equals("c"), "Expected 'c'");
                    Assert(p.stringValue().equals("chocolade"), "Expected 'chocolade'");
                }
                Assert(p.moveNext(), "Move next failed.");
                {
                    Assert(p.key().equals("e"), "Expected 'e'");
                    Assert(p.stringValue().equals("eventjes"), "Expected 'eventjes'");
                }
                Assert(!p.moveNext(), "Move next failed.");

                p.leave();
            }
            Assert(!p.moveNext(), "Move next failed.");
            p.leave();
        }

        Assert(p.moveNext(), "Move next failed.");
        {
            Assert(p.key().equals("w"), "Expected 'w'");
            Assert(p.stringValue().equals("wipwap"), "Expected 'wipwap'");
        }
        Assert(!p.moveNext(), "Move next failed.");
    }
}
