#include <code_generator.h>

using namespace std;
using namespace utils;

#define ARG(i) (this->args.at(i)->name)
#define BRACKET(s) ("["+s+"]")

namespace LA {

    void printParsedItems(Program & p){
        for (auto f : p.functions){
            cout << f->name.name;
            for (auto p : f->parameters){
                cout << SPACE << utils::pair_first(p)->name << SPACE << utils::pair_second(p).name;
            }
            cout << " {" << endl;
                for (auto i : f->instructions){
                    for(auto a : i->args){
                        cout << a->name << SPACE;
                    }
                    cout << endl;
                }
            
            cout << "}" << endl << endl;
        }
    }


    string Instruction::gen_IR(){
        return "";
    }

    string Branch_I::gen_IR(){
        return NEW_LINE_TAB + BR + ARG(0);
    }

    string Label_I::gen_IR(){
        return NEW_LINE_TAB + ARG(0);
    }

    string Length_I::gen_IR(){
        return NEW_LINE_TAB + ARG(0) + ARROW + LENGTH + ARG(1) + SPACE + ARG(2);
    }

    string New_Array_I::gen_IR(){
        // cout << "yo" << endl;
        auto s = NEW_LINE_TAB + ARG(0) + ARROW + "new Array(" + ARG(1); 
        for(int i = 2; i < this->args.size(); ++i){
            s += COMMA + ARG(i);
        }
        
        s += ")";
        // string s;
        // cout << s << endl;
        // cout << "uh suh" << endl;
        return s;
    }

    string Type_Var_I::gen_IR(){
        string s;
        s += NEW_LINE_TAB + ARG(0) + SPACE + ARG(1);
        auto type = this->args[0];

        if(dynamic_cast<Int64_Array*>(type) || dynamic_cast<Tuple*>(type)){
            s += NEW_LINE_TAB + ARG(1) + ARROW + ZERO; 
        }
        return s;
    }

    string Array_Load_I::gen_IR(){
        auto dims = utils::subvector(this->args, 2, this->args.size());
        string s;
        // s += check_allocation(ARG(1));
        // s += check_in_range(ARG(1), dims);
        s += NEW_LINE_TAB + ARG(0) + ARROW + ARG(1);
        for(auto dim : dims){
            s += BRACKET(dim->name);
        }
        return s;
    }

    string Tuple_Load_I::gen_IR(){
        auto dims = utils::subvector(this->args, 2, 3);
        string s;
        // s += check_allocation(ARG(1));
        // s += check_in_range(ARG(1), dims);
        s += NEW_LINE_TAB + ARG(0) + ARROW + ARG(1) + "[" + ARG(2) + "]";
        return s;
    }

    string Array_Store_I::gen_IR(){
        auto dims = utils::subvector(this->args, 1, this->args.size()-1);
        string s;
        // s += check_allocation(ARG(0));
        // s += check_in_range(ARG(0), dims);
        s += NEW_LINE_TAB + ARG(0);
        for(auto dim : dims){
            s += BRACKET(dim->name);
        }
        s += ARROW + ARG(this->args.size()-1);
        return s;
    }

    string Tuple_Store_I::gen_IR(){
        auto dims = utils::subvector(this->args, 1, 2);
        string s;
        // s += check_allocation(ARG(0));
        // s += check_in_range(ARG(0), dims);
        s += NEW_LINE_TAB + ARG(0) + BRACKET(ARG(1)) + ARROW + ARG(2);
        return s;
    }

    string New_Tuple_I::gen_IR(){
        return NEW_LINE_TAB + ARG(0) + ARROW + "new Tuple(" + ARG(1) + ")";
    }

    string Conditional_Branch_I::gen_IR(){
        // cout << "in heeyar"<< endl;
        return NEW_LINE_TAB + BR + ARG(0) + SPACE + ARG(1) + SPACE + ARG(2);
    }
    
    string Return_I::gen_IR(){
        return NEW_LINE_TAB + RETURN;
    }

    string Return_Value_I::gen_IR(){
        return NEW_LINE_TAB + RETURN + ARG(0);
    }

    string Assign_I::gen_IR(){
        return NEW_LINE_TAB + ARG(0) + ARROW + ARG(1);
    }

    string Assign_Op_I::gen_IR(){
        return NEW_LINE_TAB + ARG(0) + ARROW + ARG(1) + SPACE + ARG(2) + SPACE + ARG(3);
    }

    string translateCall(string & callee, vector<LA_Item*> & args){
        string s = CALL + callee + OPEN_PAREN;
        int count=0;
        for(auto arg : args){
            s += count++ > 0 ? COMMA + arg->name : arg->name;
        }
        s += CLOSED_PAREN;
        return s;
    }

    string Call_I::gen_IR(){
        auto call_args = utils::subvector(this->args, 1, this->args.size());
        return NEW_LINE_TAB + translateCall( ARG(0), call_args );
    }

    string Assign_Call_I::gen_IR(){
        auto call_args = utils::subvector(this->args, 2, this->args.size());
        return NEW_LINE_TAB + ARG(0) + ARROW + translateCall( ARG(1), call_args );
    }

    string Function::gen_IR(){
        string f;
        f += DEFINE + this->returnType->name + SPACE + COLON + this->name.name + OPEN_PAREN;
        int count = 0;
        for(auto p : this->parameters){
            auto var_name = utils::pair_second(p).name;
            auto type_name = utils::pair_first(p)->name;
            auto param = type_name + SPACE + var_name;
            f += count++ > 0 ? COMMA + param : param;
        }
        f += CLOSED_PAREN + OPEN_BRACE;
        int hola = 0;
        for (auto i : this->instructions){
            f += i->gen_IR();
            // cout << endl << hola++;
            // cout << i->gen_IR();
        }
        
        f += NEW_LINE + CLOSED_BRACE + NEW_LINE + NEW_LINE;
        return f;
    }

    string Program::gen_IR(){
        string p;
        for (auto f : this->functions){
            p += f->gen_IR();
        }
        return p;
    }

    string generate_code(Program & p){
        // printParsedItems(p);
        
        return p.gen_IR();
    }

}