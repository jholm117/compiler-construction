#pragma once
#include <vector>
#include <string>
#include <set>
#include <map>
#include <my_utils.h>
#include <iostream>

using namespace std;

namespace L2 {
    struct L2_Item {
        itemType item_type;
        virtual std::string toString()=0;// { return "my boy"; };
        bool equals(L2_Item* rhs){
            return this->toString() == rhs->toString(); 
        }
    };

    // bool OUR_SET::operator== (OUR_SET lhs, L2_Item* rhs) {
    //     cout << "uh suhh" <<endl;
    //     return lhs->toString() == rhs->toString();
    // }

    struct L2_Compare {
        bool operator()( L2_Item* lhs, L2_Item* rhs) {
            // cout << "L2 COMPARE" << endl;
            // cout << lhs ->toString() << "  " << rhs->toString() << endl;
            // cout << "result:   "  << to_string(lhs->toString() < rhs->toString()) << endl;
            return lhs->toString() < rhs->toString();
        }
    };

    typedef std::set<L2_Item*, L2_Compare> OUR_SET;

    struct VarExp : L2_Item {
        VarExp() {item_type = VAREXP;}
    };
    
    struct Constant : L2_Item {
        Constant() {item_type = CONSTANT;}
    };

    struct Register : VarExp {
        regType name;
        Register(regType n) {
            name = n;
        }
        std::string toString() override {
            return chooseRegisterName(name);
        };
    };

    struct Number : Constant {
        int64_t num;
        Number(int64_t n){
            num = n;
        }
        std::string toString(){
            return std::to_string(num);
        };
    };

    struct Variable : VarExp {
        std::string name;
        Variable(std::string n){
            name = n;
        }
        std::string toString() override {
            return name;
        };
    };
    struct Label : Constant {
        std::string name;
        Label(std::string n){
            name = n;
        }
        std::string toString() {
            return name;
        };
    };

    struct Operator : Constant {
        operatorType op;
        Operator(operatorType o){
            op = o;
        }
        std::string toString() {
            return opTypeToString(op);
        }
    };

    struct Mem : L2_Item {
        L2_Item* address;
        L2_Item* num;
        Mem(L2_Item* a, L2_Item* n){
            item_type = MEMORY;
            address = a;
            num = n;
        }
        std::string toString() {
            return "mem " + address->toString() +  " " + num->toString();
        }
    };

    struct StackArg : L2_Item {
        L2_Item* num;
        StackArg(L2_Item* n) {
            item_type = STACKARG;
            num = n;
        }
        std::string toString(){
            return "mem rsp ";
        }
    };

    struct RuntimeFunction : Constant {
        std::string name;
        std::string toString(){ return name; }
    };

    struct Instruction {
        std::vector<L2_Item*> args;
        instructionType type;
        std::string toString(int locals){
            std::string s = chooseInstructionPrefix(type); 
            for ( auto item : args ) {
                if (item->item_type == STACKARG) {
                    int n = ((Number*)((StackArg*)item)->num)->num;
                    s += item->toString() + std::to_string(n+locals*8); 
                } else {
                    s += item->toString() + (type == INC_DEC ? "" : " ");
                }
            }
            return type == INC_DEC || STACKARG ? s : s.substr(0, s.size()-1); 
        }
    };

    struct Function{
        std::string name;
        int64_t arguments;
        int64_t locals;
        std::vector<L2::Instruction *> instructions;

        std::string toString() {
            std::string str = "(" + name + "\n";
            str += "\t" + std::to_string(arguments) + " " + std::to_string(locals) + "\n";
            for( auto i : instructions ) {
                str += "\t" + i->toString(locals) + "\n";
            }
            str += ")";
            return str;
        }
    };

    struct Spiller{
        Variable* varToSpill;
        std::string prefixSpilledVars;
    };

    struct Program {
        std::string entryPointLabel;
        std::vector<L2::Function *> functions;
        Spiller spill;
    };

    struct DataFlowResult {
        std::vector<OUR_SET> inSets;
        std::vector<OUR_SET> outSets;
        std::vector<OUR_SET> genSets;
        std::vector<OUR_SET> killSets;
        
        DataFlowResult(std::vector<OUR_SET> i, std::vector<OUR_SET> o, std::vector<OUR_SET> g, std::vector<OUR_SET> k) {
            genSets = g;
            killSets = k;
            inSets = i;
            outSets = o;
        }
        std::string toString();
    };

    std::vector<L2_Item*> mapToRegisters(std::vector<regType> * regs);

    //Program parse_spill_file(char **argv);
    //void REG_spill(Program p, Function* f, char **argv);

    void SPILLER_spill(Program p, Function* f, Variable* var, std::string prefix );

    typedef std::map<L2::L2_Item*, OUR_SET, L2_Compare> InterferenceGraph;
    typedef std::map<L2::L2_Item*, regType, L2_Compare> ColoredGraph;

    OUR_SET ComputeKill(Instruction* i);
    OUR_SET ComputeGen(Instruction* i);
    std::string SetToString(OUR_SET s);
    DataFlowResult* computeLivenessAnalysis( Function* f);
    InterferenceGraph* computeInterferenceGraph(Function* f, DataFlowResult* dfr);
    ColoredGraph* computeColoredGraph(InterferenceGraph* i);
    void printIGraph(InterferenceGraph* i);
}