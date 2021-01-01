#pragma once

#include <string>
#include <vector>

namespace Configuration {
    template <typename BaseType>
    class GenericFactory
    {
        static GenericFactory& instance() {
            static GenericFactory instance_;
            return instance_;
        }

        GenericFactory() = default;

        GenericFactory(const GenericFactory&) = delete;
        GenericFactory& operator=(const GenericFactory&) = delete;

        class BuilderBase {
            const char* name_;

        public:
            BuilderBase(const char* name) : name_(name) {}

            BuilderBase(const BuilderBase& o) = delete;
            BuilderBase& operator=(const BuilderBase& o) = delete;

            virtual BaseType* create(Configuration::Parser& parser) const = 0;

            inline bool matches(const char* name) {
                return !strcmp(name, name_);
            }

            virtual ~BuilderBase() = default;
        };

        std::vector<BuilderBase*> builders_;

        inline static void registerBuilder(BuilderBase* builder)
        {
            instance().builders_.push_back(builder);
        }

    public:
        template <typename DerivedType>
        class InstanceBuilder : public BuilderBase
        {
        public:
            InstanceBuilder(const char* name) : BuilderBase(name) {
                instance().registerBuilder(this);
            }

            BaseType* create(Configuration::Parser& parser) const override
            {
                return new DerivedType(parser);
            }
        };

        static const BuilderBase* find(const char* name) {
            for (auto it : instance().builders_)
            {
                if (it->matches(name))
                {
                    return it;
                }
            }
            return nullptr;
        }

        inline static const BuilderBase* find(const std::string& name) {
            return find(name.c_str());
        }
    };
}