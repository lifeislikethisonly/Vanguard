//
// Created by Jon Stephens on 9/3/22.
//

#ifndef VANGUARD_TOP_H
#define VANGUARD_TOP_H

#include <unordered_set>
#include <string>
#include "InstructionClassVisitor.h"
#include <utility>

namespace vanguard {
    template <typename Domain>
    class Top : public Domain {
    public:
        class Function;
        class Block;
        class Instruction;

        class Contract : public Domain::Contract {
        public:
            template<typename ...Args>
            explicit Contract(std::vector<Function *> fns, Args&&... args) : fns(fns), Domain::Contract(std::forward<Args>(args)...) {
                for(auto *fn : fns) {
                    fn->setContract(this);
                }
            };

            const std::vector<Function *> &functions() const {
                return fns;
            }

        protected:
            std::vector<Function *> fns;
        };

        class CompilationUnit : public Domain::CompilationUnit {
        public:
            template<typename ...Args>
            explicit CompilationUnit(Args&&... args) : domain(nullptr), Domain::CompilationUnit(std::forward<Args>(args)...) {};

            virtual Function* findFunction(std::string name)  {
                return dynamic_cast<Function *>(this->findFn(name));
            }

            virtual std::vector<Function*> functions() const {
                std::vector<Function *> fns;
                for(auto fn : this->fns()) {
                    fns.push_back(dynamic_cast<Function *>(fn));
                }

                return fns;
            }

            Top<Domain> *universe() {
                return domain;
            }

            void setUniverse(Top<Domain> &universe) {
                if(domain == nullptr) {
                    domain = &universe;
                }
            }

        protected:
            Top<Domain> *domain;
        };

        class Function : public Domain::Function{
        public:
            template<typename ...Args>
            explicit Function(Args&&... args) : Domain::Function(std::forward<Args>(args)...) {};

            virtual CompilationUnit *unit() const {
                return dynamic_cast<CompilationUnit *>(this->compilationUnit());
            }

            virtual Block* body() const {
                return dynamic_cast<Block *>(this->head());
            }

            virtual std::vector<Instruction*> instructions() const {
                std::vector<Instruction *> ins;
                for(auto i : this->insts()) {
                    ins.push_back(dynamic_cast<Instruction *>(i));
                }

                return ins;
            }

            virtual std::vector<Block *> blocks() const {
                std::vector<Block *> blks;
                for(auto blk : this->blks()) {
                    blks.push_back(dynamic_cast<Block *>(blk));
                }

                return blks;
            }

        protected:

        };

        class Block : public Domain::Block {
        public:
            template<typename ...Args>
            explicit Block(Args&&... args) : Domain::Block(std::forward<Args>(args)...) {};


            virtual std::unordered_set<Block*> successors() const {
                std::unordered_set<Block *> blks;
                for(auto blk : this->succs()) {
                    blks.insert(dynamic_cast<Block *>(blk));
                }

                return blks;
            }

            virtual std::vector<Instruction*> instructions() const {
                std::vector<Instruction *> ins;
                for(auto i : this->insts()) {
                    ins.push_back(dynamic_cast<Instruction *>(i));
                }

                return ins;
            }

            virtual Function* function() const {
                return dynamic_cast<Function *>(this->fn());
            }

        protected:

        };

        class Instruction : public Domain::Instruction {
        public:
            template<typename ...Args>
            explicit Instruction(Args&&... args) : Domain::Instruction(std::forward<Args>(args)...) {};
            virtual void accept(InstructionClassVisitor<Top<Domain>> &v) const = 0;

            virtual Block* block() const {
                return dynamic_cast<Block *>(this->blk());
            }
        protected:
        };

        template<typename ...Args>
        explicit Top(Args&&... args) : Domain(std::forward<Args>(args)...) {
            for(auto unit : this->units()) {
                auto dUnit = dynamic_cast<CompilationUnit *>(unit);
                dUnit->setUniverse(*this);
            }
        };
    };
}

#endif //VANGUARD_TOP_H
