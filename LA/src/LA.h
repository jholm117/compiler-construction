#pragma once

#include <string>
#include <vector>
#include <tuple>
#include <iostream>
#include <unordered_map>
#include <utils.h>


using namespace std;

namespace LA{

    struct LA_Item {
        string name;
        explicit LA_Item(){}
        explicit LA_Item(string str){
            this->name = str;
        }
        virtual ~LA_Item(){}
        bool operator==(const LA_Item& other) const{
            return this->name == other.name;
        }
    };

    // struct StringItem : LA_Item {};

    struct Type : LA_Item {};

    struct Void : Type {};
    struct Int64 : Type {
        Int64(){
            this->name = "int64";
        }
    };
    struct Int64_Array : Type {};
    struct Tuple : Type {};
    struct Code : Type {};

    struct Variable : LA_Item {
        Variable(){}
        Variable(string str) : LA_Item(str){}
    };
    struct Label : LA_Item {
        Label(){}
        Label(string str) : LA_Item(str){}
    };
    struct Runtime_Callee : LA_Item {
        Runtime_Callee(){}
        Runtime_Callee(string str) : LA_Item(str){}
    };

    struct Number : LA_Item {
        int value;
        Number(){}
        Number(string str) : LA_Item(str){}
    };

    struct Operator : LA_Item {
        Operator(){}
        Operator(string str) : LA_Item(str){}
    };
    
    typedef std::unordered_map< string, Type* > Type_Map;

    struct Instruction {
        vector<LA_Item*> args;
        explicit Instruction(){}
        explicit Instruction(vector<LA_Item*> & a){
            this->args = a;
        }
        virtual ~Instruction(){}
        virtual string gen_IR();
        virtual vector<int> toDecode();
        virtual vector<int> toEncode();
    };

    struct Terminator : Instruction {};

    struct Branch_I : Terminator {
        string gen_IR() override;
        using Instruction::Instruction;
    };
    struct Conditional_Branch_I : Terminator {
        Conditional_Branch_I(){}
        Conditional_Branch_I(Variable* v, Label* l1, Label * l2){
            this->args.push_back(v);
            this->args.push_back(l1);
            this->args.push_back(l2);
            
        }
        string gen_IR() override;
        vector<int> toDecode() override;
    };
    struct Return_I : Terminator {
        string gen_IR() override;
    };
    struct Return_Value_I : Terminator {
        string gen_IR() override;
    };
    struct Assign_I : Instruction {
        string gen_IR() override;
    };
    struct Assign_Op_I : Instruction {
        using Instruction::Instruction;
        string gen_IR() override;
        vector<int> toEncode() override;
        vector<int> toDecode() override;
    };
    struct Call_I : Instruction {
        using Instruction::Instruction;
        string gen_IR() override;
    };
    struct Assign_Call_I : Instruction {
        string gen_IR() override;
    };
    struct Type_Var_I : Instruction {
        using Instruction::Instruction;
        string gen_IR() override;
    };
    struct Length_I : Instruction {
        using Instruction::Instruction;
        string gen_IR() override;
        vector<int> toDecode() override;
    };
    struct New_Array_I : Instruction {
        string gen_IR() override;
    };
    struct New_Tuple_I : Instruction {
        string gen_IR() override;
    };    
    struct Array_Load_I : Instruction {
        string gen_IR() override;
        vector<int> toDecode() override;
    };
    struct Tuple_Load_I : Instruction {
        string gen_IR() override;
        vector<int> toDecode() override;
    };
    struct Array_Store_I : Instruction {
        string gen_IR() override;
        vector<int> toDecode() override;
    };
    struct Tuple_Store_I : Instruction {
        string gen_IR() override;
        vector<int> toDecode() override;
    };
    struct Label_I : Instruction {
        string gen_IR() override;
        using Instruction::Instruction;
        Label_I(){}
        Label_I(Label* l){
            this->args.push_back(l);
        }
    };
    
    // struct BasicBlock {
    //     Label label;
    //     vector<Instruction*> instructions;
    //     string gen_IR();        
    // };

    typedef tuple< Type*, Variable > Parameter;

    struct Function {
        Type* returnType;
        Label name;
        vector<Parameter> parameters;
        vector<Instruction*> instructions;        
        Type_Map type_map;
        string gen_IR();
    };

    struct Program {
        vector<Function*> functions;
        string gen_IR();
    };
}