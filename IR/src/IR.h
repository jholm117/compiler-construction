#pragma once

#include <string>
#include <vector>
#include <tuple>

using namespace std;

namespace IR{

    struct IR_Item {};

    struct Variable : IR_Item {};
    struct Label : IR_Item {};
    struct Number : IR_Item {};
    struct Operator : IR_Item {};
    struct Runtime_Function : IR_Item {};
    struct Type : IR_Item {};

    struct Void : Type {};
    struct Int64 : Type {};
    struct Int64_Array : Type {};
    struct Tuple : Type {};
    struct Code : Type {};

    struct Instruction {
        vector<IR_Item*> args;
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
        vector<Instruction> instructions;        
    };

    typedef tuple<Type, Variable> Parameter;

    struct Function {
        Type returnType;
        Label name;
        vector<Parameter> parameters;
        vector<BasicBlock> basicBlocks;
    };

    struct Program {
        vector<Function*> functions;
    };
}