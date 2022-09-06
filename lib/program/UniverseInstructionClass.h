
//
// Created by Jon Stephens on 8/26/22.
//

#ifndef VANGUARD_UNIVERSEINSTRUCTIONCLASSES_H
#define VANGUARD_UNIVERSEINSTRUCTIONCLASSES_H

#include "Universe.h"
#include "InstructionClasses.h"
#include "llvm/IR/Instructions.h"

namespace vanguard {

    // Branch Instruction
    template<typename Base, typename Wrap>
    class Universe::Branch : public vanguard::Branch<Base> {
    public:
        template<typename ...Args>
        explicit Branch(const Wrap *ins, Args&&... args) : wrapped(ins), vanguard::Branch<Base>(std::forward<Args>(args)..., ins) {};

        virtual bool isConditional() const {
            return wrapped->isConditional();
        }

        virtual Value* condition() const {
            return this->factory.createVal(wrapped->getCondition());
        }

        virtual std::list<typename Base::Block*> targets() const {
            std::list<typename Base::Block*> successors = {};
            unsigned int n = wrapped->getNumSuccessors();
            for(unsigned int i = 0; i < n; i++){
                successors.push_back(dynamic_cast<typename Base::Block*>(this->factory.createBlk(wrapped->getSuccessor(i))));
            }
            return successors;
        }
    protected:
        const Wrap *wrapped;
    };

    template<typename Base>
    class Universe::Branch<Base, llvm::SwitchInst>: public vanguard::Branch<Base> {
    public:
        template<typename ...Args>
        explicit Branch(const llvm::SwitchInst *ins, Args&&... args) : wrapped(ins), vanguard::Branch<Base>(std::forward<Args>(args)..., ins) {};

        virtual bool isConditional() const override {
            return true;
        }
        virtual Value* condition() const override {
            return this->factory.createVal(wrapped->getCondition());
        }
        virtual std::list<typename Base::Block*> targets() const override {
            std::list<typename Base::Block*> successors = {};
            unsigned int n = wrapped->getNumSuccessors();
            for(unsigned int i = 0; i < n; i++){
                successors.push_back(dynamic_cast<typename Base::Block*>(this->factory.createBlk(wrapped->getSuccessor(i))));
            }
            return successors;
        }

    protected:
        const llvm::SwitchInst *wrapped;
    };

    template<typename Base>
    class Universe::Branch<Base, llvm::IndirectBrInst>: public vanguard::Branch<Base> {
    public:
        template<typename ...Args>
        explicit Branch(const llvm::IndirectBrInst *ins, Args&&... args) : wrapped(ins), vanguard::Branch<Base>(std::forward<Args>(args)..., ins) {};

        virtual bool isConditional() const override {
            return false;
        }
        virtual Value* condition() const override {
            throw std::runtime_error("Indirect Branch Instruction does not have a condition.");
        }
        virtual std::list<typename Base::Block*> targets() const override {
            std::list<typename Base::Block*> successors = {};
            unsigned int n = wrapped->getNumSuccessors();
            for(unsigned int i = 0; i < n; i++){
                successors.push_back(dynamic_cast<typename Base::Block*>(this->factory.createBlk(wrapped->getSuccessor(i))));
            }
            return successors;
        }

    protected:
        const llvm::IndirectBrInst *wrapped;
    };

    //Return Instruction
    template<typename Base, typename Wrap>
    class Universe::Return : public vanguard::Return<Base> {
    public:
        template<typename ...Args>
        explicit Return(const Wrap *ins, Args&&... args) : wrapped(ins), vanguard::Return<Base>(std::forward<Args>(args)..., ins) {};

        bool returnsValue() const override {
            return wrapped->getReturnValue() != nullptr;
        }
        Value* value() const override {
            return this->factory.createVal(wrapped->getReturnValue());
        }

    protected:
        const Wrap *wrapped;
    };

    template<typename Base, typename Wrap>
    class Universe::Error : public vanguard::Error<Base> {
    public:
        template<typename ...Args>
        explicit Error(const Wrap *ins, Args&&... args) : wrapped(ins), vanguard::Error<Base>(std::forward<Args>(args)..., ins) {};


        std::string msg() const override{
            return "This instruction is unreachable.";
        }

    protected:
        const Wrap *wrapped;
    };

    /*template<typename Base, typename Wrap>
    class Universe::Expression : public vanguard::Expression<Base> {
    public:
        template<typename ...Args>
        explicit Expression(const Wrap *ins, Args&&... args) : wrapped(ins), vanguard::Expression<Base>(std::forward<Args>(args)..., ins) {};

        Value* result() const override {
            auto* insVar = this->factory.createVal(wrapped);
            return insVar;
        }

    protected:
        const Wrap *wrapped;
    };*/

    // Assign Instruction
    template<typename Base, typename Wrap>
    class Universe::Assignment : public vanguard::Assignment<Base> {
    public:
        template<typename ...Args>
        explicit Assignment(const Wrap *ins, Args&&... args) : wrapped(ins), vanguard::Assignment<Base>(std::forward<Args>(args)..., ins) {};


    protected:
        const Wrap *wrapped;
    };

    //BinaryOpInstruction
    template<typename Base, typename Wrap>
    class Universe::BinaryOpExpr : public vanguard::BinaryOpExpr<Base> {
    public:
        template<typename ...Args>
        explicit BinaryOpExpr(const Wrap *ins, Args&&... args) : wrapped(ins), vanguard::BinaryOpExpr<Base>(std::forward<Args>(args)..., ins) {};

        BinOp op() const override {
            throw std::runtime_error("Unknown op");
        }

    protected:
        const Wrap *wrapped;
    };

    template<typename Base>
    class Universe::BinaryOpExpr<Base, llvm::CmpInst> : public vanguard::BinaryOpExpr<Base> {
    public:
        template<typename ...Args>
        explicit BinaryOpExpr(const llvm::CmpInst *ins, Args&&... args) : wrapped(ins), vanguard::BinaryOpExpr<Base>(std::forward<Args>(args)..., ins) {};

        BinOp op() const override {
            unsigned opcode = wrapped->getOpcode();
            BinOp binaryOpClass;
            if (opcode == 53 || opcode == 54) binaryOpClass = IFCmpInst;
            else {
                throw std::runtime_error("The opcode "+ std::string(wrapped->getOpcodeName()) + " is not a cmp operator class in vanguard.");
            }
            return  binaryOpClass;
        }

    protected:
        const llvm::CmpInst *wrapped;
    };

    template<typename Base>
    class Universe::BinaryOpExpr<Base, llvm::BinaryOperator> : public vanguard::BinaryOpExpr<Base> {
    public:
        template<typename ...Args>
        explicit BinaryOpExpr(const llvm::BinaryOperator *ins, Args&&... args) : wrapped(ins), vanguard::BinaryOpExpr<Base>(std::forward<Args>(args)..., ins) {};

        BinOp op() const override {
            unsigned opcode = wrapped->getOpcode();
            BinOp binaryOpClass;
            if (opcode == 13 || opcode == 14) binaryOpClass = Add;
            else if(opcode == 15 || opcode == 16) binaryOpClass = Sub;
            else if(opcode == 17 || opcode == 18) binaryOpClass = Mul;
            else if (opcode == 19 || opcode == 20 || opcode == 21) binaryOpClass = Div;
            else if (opcode == 22 || opcode == 23 || opcode == 24) binaryOpClass = Mod;
            else if (opcode == 25)  binaryOpClass = Shl;
            else if (opcode == 26 || opcode == 27) binaryOpClass = Shr;
            else if (opcode == 28) binaryOpClass = And;
            else if (opcode == 29) binaryOpClass = Or;
            else if (opcode == 30) binaryOpClass = Xor;
            else{
                throw std::runtime_error("The opcode "+ std::string(wrapped->getOpcodeName()) + " is not a binary operator class in vanguard.");
            }
            return binaryOpClass;
        }

    protected:
        const llvm::BinaryOperator *wrapped;
    };

    //Unary Operation Instruction
    template<typename Base, typename Wrap>
    class  Universe::UnaryOpExpr : public vanguard::UnaryOpExpr<Base> {
    public:
        template<typename ...Args>
        explicit UnaryOpExpr(const Wrap *ins, Args&&... args) : wrapped(ins), vanguard::UnaryOpExpr<Base>(std::forward<Args>(args)..., ins) {};

        virtual UnOp op() const override {
            unsigned opcode = wrapped->getOpcode();
            UnOp unaryOpClass;
            if (opcode == 12) unaryOpClass = Neg;
            else{
                throw std::runtime_error("The opcode "+ std::string(wrapped->getOpcodeName())+ " is not a unary operator class in Vanguard.");
            }
            return unaryOpClass;
        }

        Value* operand() const override {
            return this->factory.createVal(wrapped->getOperand(0));
        }

    protected:
        const Wrap *wrapped;
    };

    template<typename Base, typename Wrap>
    class Universe::CallExpr : public vanguard::CallExpr<Base> {
    public:
        template<typename ...Args>
        explicit CallExpr(const Wrap *ins, Args&&... args) : wrapped(ins), vanguard::CallExpr<Base>(std::forward<Args>(args)..., ins) {};

        bool hasReturn() const override {
            return !wrapped->doesNotReturn();
        }

        std::vector<typename Base::Function*> targets() const override {
            return {dynamic_cast<typename Base::Function*>(this->factory.createFn(wrapped->getCalledFunction()))};
        }

        std::list<Value*> args() const override{
            std::list<Value*> args = {};
            for(auto itr = wrapped->arg_begin(); itr != wrapped->arg_end(); itr++){
                args.push_back(this->factory.createVal(*itr));
            }
            return args;
        }

    protected:
        const Wrap *wrapped;
    };

    template<typename Base, typename Wrap>
    class Universe::CastExpr : public vanguard::CastExpr<Base> {
    public:
        template<typename ...Args>
        explicit CastExpr(const Wrap *ins, Args&&... args) : wrapped(ins), vanguard::CastExpr<Base>(std::forward<Args>(args)..., ins) {};

        Type *castTo() const override {
            return this->factory.createType(wrapped->getDestTy());
        }


    protected:
        const Wrap *wrapped;
    };

    template<typename Base, typename Wrap>
    class Universe::TernaryExpr : public vanguard::TernaryExpr<Base> {
    public:
        template<typename ...Args>
        explicit TernaryExpr(const Wrap *ins, Args&&... args) : wrapped(ins), vanguard::TernaryExpr<Base>(std::forward<Args>(args)..., ins) {};

        Value *condition() const override {
            return this->factory.createVal(wrapped->getCondition());
        }

        Value *trueValue() const override {
            return this->factory.createVal(wrapped->getTrueValue());
        }

        Value *falseValue() const override {
            return this->factory.createVal(wrapped->getFalseValue());
        }

    protected:
        const Wrap *wrapped;
    };

    template<typename Base, typename Wrap>
    class Universe::UnknownExpr : public vanguard::UnknownExpr<Base> {
    public:
        template<typename ...Args>
        explicit UnknownExpr(const Wrap *ins, Args&&... args) : wrapped(ins), vanguard::UnknownExpr<Base>(std::forward<Args>(args)..., ins) {};


    protected:
        const Wrap *wrapped;
    };

    /*class MemoryReadInstruction: public AssignInstruction{
    public:
        virtual InstructionVariable* getLHS() const = 0;
    };

    class MemoryWriteInstruction: public AssignInstruction{
    public:
        virtual MemoryRegion* getMemoryAddress() const = 0;
    };

    class AssignInst: public AssignInstruction{
    public:
        virtual InstructionVariable* getLHS() const = 0;
    };*/
}


#endif //VANGUARD_UNIVERSEINSTRUCTIONCLASSES_H
