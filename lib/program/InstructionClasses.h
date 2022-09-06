//
// Created by Jon Stephens on 8/26/22.
//

#ifndef VANGUARD_INSTRUCTIONCLASSES_H
#define VANGUARD_INSTRUCTIONCLASSES_H

#include "Value.h"
#include "Universe.h"
#include "InstructionClassVisitor.h"


namespace vanguard {
    enum BinOp{Add, Sub, Mul, Div, Mod, Shl, Shr, And, Or, Xor, IFCmpInst};

    enum UnOp{Neg, Not};

    // Branch Instruction
    template<typename Base>
    class Branch : public Base::Instruction {
    public:
        template<typename ...Args>
        explicit Branch(Args&&... args) : Base::Instruction(std::forward<Args>(args)...) {};

        static inline bool classof(const Branch &) { return true; }
        static inline bool classof(const Branch *) { return true; }
        static inline bool classof(const Universe::Instruction *inst) { return classof(*inst); }
        static inline bool classof(const Universe::Instruction &inst) {
            if (inst.instructionClass() == BRANCH){ return true; }
            return false;
        }

        virtual bool isConditional() const = 0;
        virtual Value* condition() const = 0;
        virtual std::list<typename Base::Block*> targets() const = 0;

        InstructionClassEnum instructionClass() const override {
            return BRANCH;
        }

        void accept(InstructionClassVisitor<Base> &v) const override {
            v.visit(*this);
        }
    };

    //Return Instruction
    template<typename Base>
    class Return : public Base::Instruction {
    public:
        template<typename ...Args>
        explicit Return(Args&&... args) : Base::Instruction(std::forward<Args>(args)...) {};

        static inline bool classof(const Return &) { return true; }
        static inline bool classof(const Return *) { return true; }
        static inline bool classof(const Universe::Instruction *inst) { return classof(*inst); }
        static inline bool cslassof(const Universe::Instruction &inst) {
            if (inst.instructionClass() == RETURN){ return true; }
            return false;
        }

        virtual bool returnsValue() const = 0;
        virtual Value* value() const = 0;

        InstructionClassEnum instructionClass() const override {
            return RETURN;
        }

        void accept(InstructionClassVisitor<Base> &v) const override {
            v.visit(*this);
        }
    };

    template<typename Base>
    class Error : public Base::Instruction {
    public:
        template<typename ...Args>
        explicit Error(Args&&... args) : Base::Instruction(std::forward<Args>(args)...) {};

        static inline bool classof(const Error &) { return true; }
        static inline bool classof(const Error *) { return true; }
        static inline bool classof(const typename Universe::Instruction *inst) { return classof(*inst); }
        static inline bool classof(const typename Universe::Instruction &inst) {
            if (inst.instructionClass() == ERROR){ return true; }
            return false;
        }

        virtual std::string msg() const = 0;

        InstructionClassEnum instructionClass() const override {
            return ERROR;
        }

        void accept(InstructionClassVisitor<Base> &v) const override {
            v.visit(*this);
        }
    };

    template<typename Base>
    class Expression : public Base::Instruction {
    public:
        template<typename ...Args>
        explicit Expression(Args&&... args) : Base::Instruction(std::forward<Args>(args)...) {};

        static inline bool classof(const Expression &) { return true; }
        static inline bool classof(const Expression *) { return true; }
        static inline bool classof(const Universe::Instruction *inst) { return classof(*inst); }
        static inline bool classof(const typename Universe::Instruction &inst) {
            if (inst.instructionClass() >= EXPRESSION_BEGIN && inst.instructionClass() <= EXPRESSION_END){ return true; }
            return false;
        }

        virtual Value* result() const {
            auto* insVar = this->factory.createVal(this->ins);
            return insVar;
        }
    };

    // Assign Instruction
    template<typename Base>
    class Assignment : public Expression<Base> {
    public:
        template<typename ...Args>
        explicit Assignment(Args&&... args) : Expression<Base>(std::forward<Args>(args)...) {};

        static inline bool classof(const Assignment &) { return true; }
        static inline bool classof(const Assignment *) { return true; }
        static inline bool classof(const typename Universe::Instruction *inst) { return classof(*inst); }
        static inline bool classof(const typename Universe::Instruction &inst) {
            if (inst.instructionClass() == ASSIGNMENT){ return true; }
            return false;
        }

        InstructionClassEnum instructionClass() const override {
            return ASSIGNMENT;
        }

        void accept(InstructionClassVisitor<Base> &v) const override {
            v.visit(*this);
        }
    };

    //BinaryOpInstruction
    template<typename Base>
    class BinaryOpExpr : public Expression<Base> {
    public:
        template<typename ...Args>
        explicit BinaryOpExpr(Args&&... args) : Expression<Base>(std::forward<Args>(args)...) {};
        static inline bool classof(const BinaryOpExpr &) { return true; }
        static inline bool classof(const BinaryOpExpr *) { return true; }
        static inline bool classof(const typename Universe::Instruction *inst) { return classof(*inst); }
        static inline bool classof(const typename Universe::Instruction &inst) {
            if (inst.instructionClass() == BIN_OP){ return true; }
            return false;
        }

        virtual BinOp op() const  = 0;

        InstructionClassEnum instructionClass() const override {
            return BIN_OP;
        }

        void accept(InstructionClassVisitor<Base> &v) const override {
            v.visit(*this);
        }
    };

    //Unary Operation Instruction
    template<typename Base>
    class  UnaryOpExpr : public Expression<Base> {
    public:
        template<typename ...Args>
        explicit UnaryOpExpr(Args&&... args) : Expression<Base>(std::forward<Args>(args)...) {};

        static inline bool classof(const UnaryOpExpr &) { return true; }
        static inline bool classof(const UnaryOpExpr *) { return true; }
        static inline bool classof(const typename Universe::Instruction *inst) { return classof(*inst); }
        static inline bool classof(const typename Universe::Instruction &inst) {
            if (inst.instructionClass() == UN_OP){ return true; }
            return false;
        }

        virtual UnOp op() const = 0;
        virtual Value* operand() const = 0;

        InstructionClassEnum instructionClass() const override {
            return UN_OP;
        }

        void accept(InstructionClassVisitor<Base> &v) const override {
            v.visit(*this);
        }
    };

    template<typename Base>
    class CallExpr : public Expression<Base> {
    public:
        template<typename ...Args>
        explicit CallExpr(Args&&... args) : Expression<Base>(std::forward<Args>(args)...) {};

        static inline bool classof(const CallExpr &) { return true; }
        static inline bool classof(const CallExpr *) { return true; }
        static inline bool classof(const typename Universe::Instruction *inst) { return classof(*inst); }
        static inline bool classof(const typename Universe::Instruction &inst) {
            if (inst.instructionClass() == CALL){ return true; }
            return false;
        }

        virtual bool hasReturn() const = 0;
        virtual std::vector<typename Base::Function *> targets() const = 0;
        virtual std::list<Value*> args() const = 0;

        InstructionClassEnum instructionClass() const override {
            return CALL;
        }

        void accept(InstructionClassVisitor<Base> &v) const override {
            v.visit(*this);
        }
    };

    template<typename Base>
    class CastExpr : public Expression<Base> {
    public:
        template<typename ...Args>
        explicit CastExpr(Args&&... args) : Expression<Base>(std::forward<Args>(args)...) {};

        static inline bool classof(const CastExpr &) { return true; }
        static inline bool classof(const CastExpr *) { return true; }
        static inline bool classof(const typename Universe::Instruction *inst) { return classof(*inst); }
        static inline bool classof(const typename Universe::Instruction &inst) {
            if (inst.instructionClass() == CAST){ return true; }
            return false;
        }

        virtual Type *castTo() const = 0;

        InstructionClassEnum instructionClass() const override {
            return CAST;
        }

        void accept(InstructionClassVisitor<Base> &v) const override {
            v.visit(*this);
        }
    };

    template<typename Base>
    class TernaryExpr : public Expression<Base> {
    public:
        template<typename ...Args>
        explicit TernaryExpr(Args&&... args) : Expression<Base>(std::forward<Args>(args)...) {};

        static inline bool classof(const TernaryExpr &) { return true; }
        static inline bool classof(const TernaryExpr *) { return true; }
        static inline bool classof(const typename Universe::Instruction *inst) { return classof(*inst); }
        static inline bool classof(const typename Universe::Instruction &inst) {
            if (inst.instructionClass() == TERNARY){ return true; }
            return false;
        }

        virtual Value *condition() const = 0;
        virtual Value *trueValue() const = 0;
        virtual Value *falseValue() const = 0;

        InstructionClassEnum instructionClass() const override {
            return TERNARY;
        }

        void accept(InstructionClassVisitor<Base> &v) const override {
            v.visit(*this);
        }
    };

    template<typename Base>
    class UnknownExpr : public Expression<Base> {
    public:
        template<typename ...Args>
        explicit UnknownExpr(Args&&... args) : Expression<Base>(std::forward<Args>(args)...) {};

        static inline bool classof(const UnknownExpr &) { return true; }
        static inline bool classof(const UnknownExpr *) { return true; }
        static inline bool classof(const typename Universe::Instruction *inst) { return classof(*inst); }
        static inline bool classof(const typename Universe::Instruction &inst) {
            if (inst.instructionClass() == UNKNOWN){ return true; }
            return false;
        }

        InstructionClassEnum instructionClass() const override {
            return UNKNOWN;
        }

        void accept(InstructionClassVisitor<Base> &v) const override {
            v.visit(*this);
        }
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

#endif //VANGUARD_INSTRUCTIONCLASSES_H
