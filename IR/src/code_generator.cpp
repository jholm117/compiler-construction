#include <code_generator.h>

using namespace std;

#define ARG(i) (this->args[i]->name)

namespace IR {

    int64_t VARIABLE_COUNT = 0;
    const string ARROW = " <- ";
    const string BR = "br ";
    const string RETURN = "return ";
    const string NEW_LINE = "\n";
    const string SPACE = " ";
    const string CALL = "call ";
    const string OPEN_PAREN = " (";
    const string CLOSED_PAREN = ") ";
    const string COMMA = ", ";
    const string DEFINE = "define ";
    const string OPEN_BRACE = "{";
    const string CLOSED_BRACE = "}";
    const string TAB = "\t";

    void printParsedItems(Program & p){
        for (auto f : p.functions){
            cout << f->name.name;
            for (auto p : f->parameters){
                cout << SPACE << utils::pair_first(p)->name << SPACE << utils::pair_second(p).name;
            }
            cout << " {" << endl;
            for (auto bb : f->basicBlocks){
                cout << bb->label.name << endl;
                for (auto i : bb->instructions){
                    for(auto a : i->args){
                        cout << a->name << SPACE;
                    }
                    cout << endl;
                }
            }
            cout << "}" << endl << endl;
        }
    }

    string new_var_name(){
        // make this robust somehow
        return "jeff_theo_simone" + std::to_string(VARIABLE_COUNT++);
    }

    string Instruction::to_L3(){
        return "";
    }

    string Branch_I::to_L3(){
        return BR + this->args[0]->name;
    }

    string Conditional_Branch_I::to_L3(){
        auto cmpItem = this->args[0];
        string ret_str;
        string cmp_var;
        if(dynamic_cast<Number*>(cmpItem)){
            cmp_var = new_var_name();
            ret_str += cmp_var + ARROW + cmpItem->name + NEW_LINE + TAB;
        } else {
            cmp_var = cmpItem->name;
        }
        ret_str +=  BR + cmp_var + SPACE + this->args[1]->name + SPACE + this->args[2]->name ;
        return ret_str;
    }
    
    string Return_I::to_L3(){
        return RETURN;
    }

    string Return_Value_I::to_L3(){
        return RETURN + this->args[0]->name;
    }

    string Assign_I::to_L3(){
        return this->args[0]->name + ARROW + this->args[1]->name;
    }

    string Assign_Op_I::to_L3(){
        return ARG(0) + ARROW + ARG(1) + SPACE + ARG(2) + SPACE + ARG(3);
    }

    string translateCall(string callee, vector<IR_Item*> & args){
        string s = CALL + callee + OPEN_PAREN;
        int count=0;
        for(auto arg : args){
            s += count++ > 0 ? COMMA + arg->name : arg->name;
        }
        s += CLOSED_PAREN;
        return s;
    }

    string Call_I::to_L3(){
        auto call_args = utils::subvector(this->args, 1, this->args.size());
        return translateCall( ARG(0), call_args );
    }

    string Assign_Call_I::to_L3(){
        auto call_args = utils::subvector(this->args, 2, this->args.size());
        auto s = ARG(0) + ARROW;
        s += translateCall( ARG(1), call_args );
        return s;
    }

    string BasicBlock::to_L3(){
        string bb = TAB + this->label.name + NEW_LINE;
        for(auto i : this->instructions){
            bb += TAB + i->to_L3() + NEW_LINE;
        }
        return bb;
    }

    string Function::to_L3(){
        string f;
        f += DEFINE + this->name.name + OPEN_PAREN;
        int count = 0;
        for(auto p : this->parameters){
            auto var_name = utils::pair_second(p).name;
            f += count++ > 0 ? COMMA + var_name : var_name;
        }
        f += CLOSED_PAREN + OPEN_BRACE + NEW_LINE;
        for (auto bb : this->basicBlocks){
            f += bb->to_L3();
        }
        
        f += CLOSED_BRACE + NEW_LINE + NEW_LINE;
        return f;
    }

    string Program::to_L3(){
        string p;
        for (auto f : this->functions){
            p += f->to_L3();
        }
        return p;
    }

    string generate_code(Program & p){
        // printParsedItems(p);
        
        return p.to_L3();
    }

}