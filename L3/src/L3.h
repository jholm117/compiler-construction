#pragma once

#include <vector>
#include <string>

#include <iostream>

using namespace std;

namespace L3{

    struct L3_Item {
        virtual string toString();
        bool equals(L3_Item* rhs){
            return this->toString() == rhs->toString(); 
        }
    };

    struct Number : L3_Item {
        int value;
        string toString() override;
    };

    struct StringItem : L3_Item {
        string name;
        string toString() override;
    };

    struct Variable : StringItem {};

    struct Label : StringItem {};

    enum L3_OperatorType {
        PLUS,
        MINUS,
        STAR,
        AND,
        SHIFT_LEFT,
        SHIFT_RIGHT,
        LESS_THAN,
        LESS_THAN_EQ,
        EQ,
        GREATER_THAN,
        GREATER_THAN_EQ,
        LOAD,
        STORE,
        ARROW,
        OP_TYPE_ERROR
    };

    L3_OperatorType stringToOperatorType(string op);

    struct Operator : L3_Item {
        L3_OperatorType op;
        string toString() override;
    };

    struct Instruction {
        virtual string toString();
        vector<L3_Item*> args;
    };

    struct Label_I : Instruction {};

    struct Assign_I : Instruction {};

    struct Assign_Op_I : Instruction {};

    struct Load_I : Instruction {};

    struct Store_I : Instruction {};

    struct Branch_I : Instruction {};

    struct Conditional_Branch_I : Instruction {};
    
    struct Call_I : Instruction {
        string toString() override;
    };

    struct Assign_Call_I : Instruction {};

    struct Return_I : Instruction {
        string toString() override;
    };

    struct Return_Value_I : Return_I {
        string toString() override;
    };

    struct Function {
        Label* name;
        vector<Variable*> arguments;
        vector<Instruction*> instructions;
    };

    struct Program {
        vector<Function*> functions;
    };
}