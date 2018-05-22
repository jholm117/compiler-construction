#pragma once

#include <string>
#include <vector>
#include <tuple>
#include <iostream>
#include <utils.h>


using namespace std;

namespace IR{

    struct IR_Item {
        virtual ~IR_Item(){}
        string name;
    };

    struct StringItem : IR_Item {
    };

    struct Variable : StringItem {};
    struct Label : StringItem {};
    struct Runtime_Function : StringItem {};

    struct Number : IR_Item {
        int value;
    };

    // enum IR_OperatorType {
    //     PLUS,
    //     MINUS,
    //     STAR,
    //     AND,
    //     SHIFT_LEFT,
    //     SHIFT_RIGHT,
    //     LESS_THAN,
    //     LESS_THAN_EQ,
    //     EQ,
    //     GREATER_THAN,
    //     GREATER_THAN_EQ
    // };

    struct Operator : StringItem {};

    struct Type : IR_Item {};

    struct Void : Type {};
    struct Int64 : Type {
        int dimension;
    };
    struct Tuple : Type {};
    struct Code : Type {};

    struct Instruction {
        vector<IR_Item*> args;
        virtual ~Instruction(){}
    };

    struct Branch_I : Instruction {};
    struct Conditional_Branch_I : Instruction {};
    struct Return_I : Instruction {};
    struct Return_Value_I : Instruction {};
    struct Type_Var_I : Instruction {};
    struct Assign_I : Instruction {};
    struct Assign_Op_I : Instruction {};
    struct Array_Load_I : Instruction {};
    struct Array_Store_I : Instruction {};
    struct Length_I : Instruction {};
    struct Call_I : Instruction {};
    struct Assign_Call_I : Instruction {};
    struct New_Array_I : Instruction {};
    struct New_Tuple_I : Instruction {};

    
    struct BasicBlock {
        Label label;
        vector<Instruction*> instructions;        
    };

    typedef tuple< Type*, Variable > Parameter;

    struct Function {
        Type returnType;
        Label name;
        vector<Parameter> parameters;
        vector<BasicBlock*> basicBlocks;
    };

    struct Program {
        vector<Function*> functions;
    };
}