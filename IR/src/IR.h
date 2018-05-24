#pragma once

#include <string>
#include <vector>
#include <tuple>
#include <iostream>
#include <unordered_map>
#include <utils.h>


using namespace std;

namespace IR{

    struct IR_Item {
        virtual ~IR_Item(){}
        string name;
    };

    struct StringItem : IR_Item {
    };

    struct Type : IR_Item {};

    struct Void : Type {};
    struct Int64 : Type {};
    struct Tuple : Type {};
    struct Code : Type {};

    struct Variable : StringItem {};
    struct Label : StringItem {};
    struct Runtime_Function : StringItem {};

    struct Number : IR_Item {
        int value;
    };

    struct Operator : StringItem {};

    
    typedef std::unordered_map< string, Type* > Type_Map;

    struct Instruction {
        vector<IR_Item*> args;
        virtual ~Instruction(){}
        virtual string to_L3();
    };

    struct Branch_I : Instruction {
        string to_L3() override;
    };
    struct Conditional_Branch_I : Instruction {
        string to_L3() override;
    };
    struct Return_I : Instruction {
        string to_L3() override;
    };
    struct Return_Value_I : Instruction {
        string to_L3() override;
    };
    struct Assign_I : Instruction {
        string to_L3() override;
    };
    struct Assign_Op_I : Instruction {
        string to_L3() override;
    };
    struct Call_I : Instruction {
        string to_L3() override;
    };
    struct Assign_Call_I : Instruction {
        string to_L3() override;
    };
    struct Length_I : Instruction {
        string to_L3() override;
    };
    struct New_Array_I : Instruction {
        string to_L3() override;
    };
    struct New_Tuple_I : Instruction {
        string to_L3() override;
    };
    
    struct Array_Load_I : Instruction {
        string to_L3() override;
    };

    struct Tuple_Load_I : Instruction {
        string to_L3() override;
    };
    struct Array_Store_I : Instruction {
        string to_L3() override;
    };
    struct Tuple_Store_I : Instruction {
        string to_L3() override;
    };
    
    struct BasicBlock {
        Label label;
        vector<Instruction*> instructions;
        string to_L3();        
    };

    typedef tuple< Type*, Variable > Parameter;

    struct Function {
        Type returnType;
        Label name;
        vector<Parameter> parameters;
        vector<BasicBlock*> basicBlocks;        
        Type_Map type_map;
        string to_L3();
    };

    struct Program {
        vector<Function*> functions;
        string to_L3();
    };
}