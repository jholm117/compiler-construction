#pragma once

#include <vector>
#include <string>

#include <utils.h>
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

    struct Runtime_Function : StringItem {};

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
        Operator( L3_OperatorType o );
        string toString() override;
    };

    struct SOAP : Operator {
        SOAP (L3_OperatorType o ) : Operator (o){}
    };

    struct CMP : Operator {
        CMP (L3_OperatorType o) : Operator(o){}
    };

    struct Instruction {
        vector<L3_Item*> args;
        virtual ~Instruction(){}
    };

    struct Contextual_I : Instruction {};

    struct CallingC_I : Instruction {
        virtual string toString()=0;
    };

    struct Assign_I : Contextual_I {};

    struct Assign_Op_I : Contextual_I {};

    struct Assign_Cmp_I : Contextual_I{};

    struct Load_I : Contextual_I {};

    struct Store_I : Contextual_I {};

    struct Label_I : CallingC_I {
        string toString() override;
    };

    struct Branch_I : CallingC_I {
        string toString() override;
    };

    struct Conditional_Branch_I : CallingC_I {
        string toString() override;
    };

    struct Call : CallingC_I {
        int count;
    };
    
    struct Call_I : Call {
        string toString() override;
    };

    struct Assign_Call_I : Call {
        string toString() override;
    };

    struct Return_I : CallingC_I {
        string toString() override;
    };

    struct Return_Value_I : CallingC_I {
        string toString() override;
    };

    struct Function {
        Label* name;
        vector<Variable*> arguments;
        vector<Instruction*> instructions;
        vector<Label*> labels;
        std::string toString();
    };

    struct Program {
        vector<Function*> functions;
        string longestLabel;
    };
}