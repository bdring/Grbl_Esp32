#include "../TestFramework.h"

#include <string>

#include <src/Configuration/Tokenizer.h>
#include <src/Configuration/Parser.h>
#include <src/Configuration/ParserHandler.h>
#include <src/Configuration/Configurable.h>

namespace Configuration {
    class TestBasic : public Configurable {
    public:
        String a;
        String b;
        String c;

        void validate() const {}
        void handle(HandlerBase& handler) {
            handler.handle("a", a);
            handler.handle("b", b);
            handler.handle("c", c);
        }
    };

    class TestBasic2 : public Configurable {
    public:
        String aap;
        int    banaan;

        void validate() const {}
        void handle(HandlerBase& handler) {
            handler.handle("aap", aap);
            handler.handle("banaan", banaan);
        }
    };

    class TestHierarchical : public Configurable {
    public:
        TestBasic*  n1  = nullptr;
        TestBasic2* n2  = nullptr;
        int         foo = 0;

        void validate() const {}
        void handle(HandlerBase& handler) {
            handler.handle("n1", n1);
            handler.handle("n2", n2);
            handler.handle("foo", foo);
        }
    };

    struct Helper {
        template <typename T>
        static inline void Parse(const char* config, T& test) {
            Parser        p(config, config + strlen(config));
            ParserHandler handler(p);

            test.handle(handler);
            for (; !p.isEndSection(); handler.moveNext()) {
                test.handle(handler);
            }
        }
    };

    Test(YamlTreeBuilder, BasicProperties) {
        const char* config = "a: aap\n"
                             "b: banaan\n"
                             "\n"
                             "c: chocolade";

        TestBasic test;
        Helper::Parse(config, test);

        Assert(test.a == "aap");
        Assert(test.b == "banaan");
        Assert(test.c == "chocolade");
    }

    Test(YamlTreeBuilder, BasicPropertiesInvert) {
        const char* config = "c: chocolade\n"
                             "b: banaan\n"
                             "a: aap\n";

        TestBasic test;
        Helper::Parse(config, test);

        Assert(test.a == "aap");
        Assert(test.b == "banaan");
        Assert(test.c == "chocolade");
    }

    Test(YamlTreeBuilder, BasicProperties2) {
        const char* config = "aap: aap\n"
                             "banaan: 2\n";

        TestBasic2 test;
        Helper::Parse(config, test);

        Assert(test.aap == "aap");
        Assert(test.banaan == 2);
    }

    Test(YamlTreeBuilder, BasicPropertiesInvert2) {
        const char* config = "banaan: 2\n"
                             "aap: aap\n";

        TestBasic2 test;
        Helper::Parse(config, test);

        Assert(test.aap == "aap");
        Assert(test.banaan == 2);
    }

    Test(YamlTreeBuilder, Hierarchical1) {
        const char* config = "n1:\n"
                             "  a: aap\n"
                             "  b: banaan\n"
                             "  \n"
                             "  c: chocolade\n"
                             "n2:\n"
                             "  banaan: 2\n"
                             "  aap: aap\n"
                             "foo: 2\n";

        TestHierarchical test;
        Helper::Parse(config, test);

        {
            Assert(test.n1 != nullptr);
            Assert(test.n1->a == "aap");
            Assert(test.n1->b == "banaan");
            Assert(test.n1->c == "chocolade");
        }

        {
            Assert(test.n2 != nullptr);
            Assert(test.n2->banaan == 2);
            Assert(test.n2->aap == "aap");
        }
        Assert(test.foo == 2);
    }

    Test(YamlTreeBuilder, Hierarchical2) {
        const char* config = "n2:\n"
                             "  banaan: 2\n"
                             "  aap: aap\n"
                             "n1:\n"
                             "  a: aap\n"
                             "  b: banaan\n"
                             "  \n"
                             "  c: chocolade\n"
                             "foo: 2\n";

        TestHierarchical test;
        Helper::Parse(config, test);

        {
            Assert(test.n1 != nullptr);
            Assert(test.n1->a == "aap");
            Assert(test.n1->b == "banaan");
            Assert(test.n1->c == "chocolade");
        }

        {
            Assert(test.n2 != nullptr);
            Assert(test.n2->banaan == 2);
            Assert(test.n2->aap == "aap");
        }
        Assert(test.foo == 2);
    }

    Test(YamlTreeBuilder, Hierarchical3) {
        const char* config = "foo: 2\n"
                             "n2:\n"
                             "  banaan: 2\n"
                             "  aap: aap\n"
                             "n1:\n"
                             "  a: aap\n"
                             "  b: banaan\n"
                             "  \n"
                             "  c: chocolade\n";

        TestHierarchical test;
        Helper::Parse(config, test);

        {
            Assert(test.n1 != nullptr);
            Assert(test.n1->a == "aap");
            Assert(test.n1->b == "banaan");
            Assert(test.n1->c == "chocolade");
        }

        {
            Assert(test.n2 != nullptr);
            Assert(test.n2->banaan == 2);
            Assert(test.n2->aap == "aap");
        }
        Assert(test.foo == 2);
    }
}
