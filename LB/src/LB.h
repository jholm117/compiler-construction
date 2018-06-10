#pragma once

#include <string>
#include <vector>
#include <tuple>
#include <iostream>
#include <unordered_map>
#include <utils.h>


using namespace std;

namespace LB{

    struct LB_Item {
        string name;
        explicit LB_Item(){}
        explicit LB_Item(string str){
            this->name = str;
        }
        virtual ~LB_Item(){}
        bool operator==(const LB_Item& other) const{
            return this->name == other.name;
        }
    };

    struct Type : LB_Item {};

    struct Void : Type {};
    struct Int64 : Type {
        Int64(){
            this->name = "int64";
        }
    };
    struct Int64_Array : Type {};
    struct Tuple : Type {};
    struct Code : Type {};

    struct Variable : LB_Item {
        Variable(){}
        Variable(string str) : LB_Item(str){}
    };
    struct Label : LB_Item {
        Label(){}
        Label(string str) : LB_Item(str){}
    };
    struct Runtime_Callee : LB_Item {
        Runtime_Callee(){}
        Runtime_Callee(string str) : LB_Item(str){}
    };
    struct Function_Name : LB_Item {     
        Function_Name(){}   
        Function_Name(string str) : LB_Item(str){}
    };

    struct Number : LB_Item {
        int value;
        Number(){}
        Number(string str) : LB_Item(str){}
    };

    struct Operator : LB_Item {
        Operator(){}
        Operator(string str) : LB_Item(str){}
    };
    
    typedef std::unordered_map< string, Type* > Type_Map;
    typedef std::unordered_map< string, string > Bindings_Map;
    struct Instruction {
        vector<LB_Item*> args;
        explicit Instruction(){}
        explicit Instruction(vector<LB_Item*> & a){
            this->args = a;
        }
        virtual ~Instruction(){}
        virtual string gen_LA();
    };
    
    struct Scope {
        vector<Instruction*> instructions;
        Type_Map type_map;
        Bindings_Map flat_bindings_map;
    };

    struct Branch_I : Instruction {
        string gen_LA() override;
        using Instruction::Instruction;
    };
    struct Conditional_Branch_I : Instruction {
        Conditional_Branch_I(){}
        Conditional_Branch_I(Variable* v, Label* l1, Label * l2){
            this->args.push_back(v);
            this->args.push_back(l1);
            this->args.push_back(l2);
            
        }
        string gen_LA() override;
    };
    struct Return_I : Instruction {
        string gen_LA() override;
    };
    struct Return_Value_I : Instruction {
        string gen_LA() override;
    };
    struct Assign_I : Instruction {
        string gen_LA() override;
    };
    struct Assign_Cond_I : Instruction {
        using Instruction::Instruction;
        string gen_LA() override;
    };
    struct Call_I : Instruction {
        using Instruction::Instruction;
        string gen_LA() override;
    };
    struct Assign_Call_I : Instruction {
        string gen_LA() override;
    };
    struct Length_I : Instruction {
        using Instruction::Instruction;
        string gen_LA() override;
    };
    struct New_Array_I : Instruction {
        string gen_LA() override;
    };
    struct New_Tuple_I : Instruction {
        string gen_LA() override;
    };    
    struct Array_Load_I : Instruction {
        string gen_LA() override;
    };
    struct Array_Store_I : Instruction {
        string gen_LA() override;
    };
    struct Label_I : Instruction {
        string gen_LA() override;
        Label_I(){}
        Label_I(Label* l){
            this->args.push_back(l);
        }
    };
    
    struct Type_Var_I : Instruction {
        string gen_LA() override;
    };

    struct Scope_I : Instruction {
        Scope scope;
    };

    struct While_I : Instruction {
        string gen_LA() override;
    };
    struct If_I : Instruction {
        string gen_LA() override;
    };
    struct Continue_I : Instruction {
        string gen_LA() override;
    };
    struct Break_I : Instruction {
        string gen_LA() override;
    };

    typedef tuple< Type*, Variable > Parameter;

    struct Function {
        Type* returnType;
        Function_Name name;
        vector<Parameter> parameters;
        Scope scope;
        string gen_LA();
    };

    struct Program {
        vector<Function*> functions;
        string gen_LA();
    };
}