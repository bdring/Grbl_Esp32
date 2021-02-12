#pragma once

#include <vector>

#include "HandlerBase.h"

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

            virtual BaseType* create() const = 0;
            const char* name() const { return name_; }

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

            BaseType* create() const override
            {
                return new DerivedType();
            }
        };

        static void handle(Configuration::HandlerBase& handler, BaseType*& inst)
        {
            if (inst == nullptr)
            {
                for (auto it : instance().builders_) {
                    if (handler.matchesUninitialized(it->name())) {
                        inst = it->create();
                        handler.handle(it->name(), *inst);

                        return;
                    }
                }
            }
            else 
            {
                handler.handleDetail(inst->name(), inst);
            }
        }
    };
}