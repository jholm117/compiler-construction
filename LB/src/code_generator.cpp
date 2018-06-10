#include <code_generator.h>

using namespace std;
using namespace utils;

#define ARG(i) (this->args.at(i)->name)
#define BRACKET(s) ("["+s+"]")

namespace LB {

    void printParsedItems(Program & p){
        for (auto f : p.functions){
            cout << f->name.name;
            for (auto p : f->parameters){
                cout << SPACE << utils::pair_first(p)->name << SPACE << utils::pair_second(p).name;
            }
            cout << " {" << endl;
                for (auto i : f->scope.instructions){
                    for(auto a : i->args){
                        cout << a->name << SPACE;
                    }
                    cout << endl;
                }
            
            cout << "}" << endl << endl;
        }
    }


    string Instruction::gen_LA(){
        return NEW_LINE_TAB + "not implemented yet";
    }

    string Branch_I::gen_LA(){
        return NEW_LINE_TAB + BR + ARG(0);
    }

    string Label_I::gen_LA(){
        return NEW_LINE_TAB + ARG(0);
    }

    string Length_I::gen_LA(){
        return NEW_LINE_TAB + ARG(0) + ARROW + LENGTH + ARG(1) + SPACE + ARG(2);
    }

    string New_Array_I::gen_LA(){
        auto s = NEW_LINE_TAB + ARG(0) + ARROW + "new Array(" + ARG(1); 
        for(int i = 2; i < this->args.size(); ++i){
            s += COMMA + ARG(i);
        }
        
        s += ")";
        return s;
    }

    string Type_Var_I::gen_LA(){
        string s;
        auto it = this->args.begin();
        for(++it; it != this->args.end(); ++it ){
            s += NEW_LINE_TAB + ARG(0) + SPACE + (*it)->name;
        }

        return s;
    }

    string Array_Load_I::gen_LA(){
        auto dims = utils::subvector(this->args, 2, this->args.size());
        string s;
        s += NEW_LINE_TAB + ARG(0) + ARROW + ARG(1);
        for(auto dim : dims){
            s += BRACKET(dim->name);
        }
        return s;
    }

    string Array_Store_I::gen_LA(){
        auto dims = utils::subvector(this->args, 1, this->args.size()-1);
        string s;
        s += NEW_LINE_TAB + ARG(0);
        for(auto dim : dims){
            s += BRACKET(dim->name);
        }
        s += ARROW + ARG(this->args.size()-1);
        return s;
    }

    string New_Tuple_I::gen_LA(){
        return NEW_LINE_TAB + ARG(0) + ARROW + "new Tuple(" + ARG(1) + ")";
    }

    string Conditional_Branch_I::gen_LA(){
        return NEW_LINE_TAB + BR + ARG(0) + SPACE + ARG(1) + SPACE + ARG(2);
    }
    
    string Return_I::gen_LA(){
        return NEW_LINE_TAB + RETURN;
    }

    string Return_Value_I::gen_LA(){
        return NEW_LINE_TAB + RETURN + ARG(0);
    }

    string Assign_I::gen_LA(){
        return NEW_LINE_TAB + ARG(0) + ARROW + ARG(1);
    }

    string Assign_Cond_I::gen_LA(){
        return NEW_LINE_TAB + ARG(0) + ARROW + ARG(1) + SPACE + ARG(2) + SPACE + ARG(3);
    }

    string translateCall(string & callee, vector<LB_Item*> & args){
        string s = CALL + callee + OPEN_PAREN;
        int count=0;
        for(auto arg : args){
            s += count++ > 0 ? COMMA + arg->name : arg->name;
        }
        s += CLOSED_PAREN;
        return s;
    }

    string Call_I::gen_LA(){
        auto call_args = utils::subvector(this->args, 1, this->args.size());
        return NEW_LINE_TAB + translateCall( ARG(0), call_args );
    }

    string Assign_Call_I::gen_LA(){
        auto call_args = utils::subvector(this->args, 2, this->args.size());
        return NEW_LINE_TAB + ARG(0) + ARROW + translateCall( ARG(1), call_args );
    }

    string If_I::gen_LA(){
        auto temp = utils::new_var_name();
        string s;
        s += NEW_LINE_TAB + INT64 + temp;
        s += NEW_LINE_TAB + temp + ARROW + ARG(0) + SPACE + ARG(1) + SPACE + ARG(2);
        s += NEW_LINE_TAB + BR + temp + SPACE + ARG(3) + SPACE + ARG(4);
        return s;
    }

    string While_I::gen_LA(){
        auto temp = utils::new_var_name();
        string s;
        s += NEW_LINE_TAB + INT64 + temp;
        s += NEW_LINE_TAB + temp + ARROW + ARG(0) + SPACE + ARG(1) + SPACE + ARG(2);
        s += NEW_LINE_TAB + BR + temp + SPACE + ARG(3) + SPACE + ARG(4);
        return s;
    }

    string Break_I::gen_LA(){
        return NEW_LINE_TAB + BR + ARG(0);
    }

    string Continue_I::gen_LA(){
        return NEW_LINE_TAB + BR + ARG(0);
    }

    string Function::gen_LA(){
        string f;
        f += this->returnType->name + SPACE + this->name.name + OPEN_PAREN;
        int count = 0;
        for(auto p : this->parameters){
            auto var_name = utils::pair_second(p).name;
            auto type_name = utils::pair_first(p)->name;
            auto param = type_name + SPACE + var_name;
            f += count++ > 0 ? COMMA + param : param;
        }
        f += CLOSED_PAREN + OPEN_BRACE;
        for (auto i : this->scope.instructions){
            f += i->gen_LA();
        }
        f += NEW_LINE + CLOSED_BRACE + NEW_LINE + NEW_LINE;
        
        return f;
    }

    string Program::gen_LA(){
        string p;
        for (auto f : this->functions){
            p += f->gen_LA();
        }
        return p;
    }

    string generate_code(Program & p){
        // printParsedItems(p);
        
        return p.gen_LA();
    }

}