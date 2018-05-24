#include <code_generator.h>

using namespace std;

#define ARG(i) (this->args[i]->name)

namespace IR {

    int64_t VARIABLE_COUNT = 0;
    const string BASE_VARIABLE_NAME = "hola_";

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
    const string STAR = " * ";
    const string EIGHT = "8";
    const string SIXTEEN = "16";
    const string PLUS = " + ";
    const string LOAD = "load ";
    const string STORE = "store ";
    const string NEW_LINE_TAB = "\n\t";
    const string ONE = "1";
    const string ALLOCATE = "allocate";
    const string CALL_ALLOCATE = "call allocate(";
    const string SHIFT_RIGHT = " >> ";
    const string SHIFT_LEFT = " << ";
    const string ZERO = "0";

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
        return BASE_VARIABLE_NAME + std::to_string(VARIABLE_COUNT++);
    }

    string Instruction::to_L3(){
        return "";
    }

    string Branch_I::to_L3(){
        return NEW_LINE_TAB + BR + this->args[0]->name;
    }

    string Length_I::to_L3(){
        auto v0 = new_var_name();
        auto v1 = new_var_name();
        auto v2 = new_var_name();

        string s;
        s += NEW_LINE_TAB + v0 + ARROW + ARG(2) + STAR + EIGHT;
        s += NEW_LINE_TAB + v1 + ARROW + v0 + PLUS + SIXTEEN;
        s += NEW_LINE_TAB + v2 + ARROW + ARG(1) + PLUS + v1;
        s += NEW_LINE_TAB + ARG(0) + ARROW + LOAD + v2;
        return s;
    }

    string mapToVarName(IR_Item* i){
        return new_var_name();
    }

    string multiply_dimensions(string & accumulator, vector<IR_Item*> & dimensions){
        vector<string> dimension_vars;
        dimension_vars = utils::map(dimensions, dimension_vars, mapToVarName);

        string s;
        for (int i=0; i < dimensions.size(); ++i){
            s += NEW_LINE_TAB + dimension_vars[i] + ARROW + dimensions[i]->name + SHIFT_RIGHT + ONE;
        }

        s += NEW_LINE_TAB + accumulator + ARROW + dimension_vars[0];
        for (int i=1; i < dimension_vars.size(); ++i){
            s += NEW_LINE_TAB + accumulator + ARROW + accumulator + STAR + dimension_vars[i];
        }
        return s;
    }

    string store_dimensions(string & address_var, vector<IR_Item*> & dimensions){
        string s;
        int offset = 16;
        for(auto dim : dimensions){
            auto vx = new_var_name();
            s += NEW_LINE_TAB + vx + ARROW + address_var + PLUS + to_string(offset);
            s += NEW_LINE_TAB + STORE + vx + ARROW + dim->name;
            offset += 8;
        }
        return s;
    }

    string New_Array_I::to_L3(){

        auto dimensions = utils::subvector(this->args, 1, this->args.size());
        string s;
        auto v0 = new_var_name();
        auto v1 = new_var_name();
        auto overhead = to_string(dimensions.size() + 1);
        auto encoded_dim = to_string(dimensions.size() * 2 + 1);
        s += multiply_dimensions(v0, dimensions);
        s += NEW_LINE_TAB + v0 + ARROW + v0 + PLUS + overhead;
        s += NEW_LINE_TAB + v0 + ARROW + v0 + SHIFT_LEFT + ONE;
        s += NEW_LINE_TAB + v0 + ARROW + v0 + PLUS + ONE;
        s += NEW_LINE_TAB + ARG(0) + ARROW + CALL_ALLOCATE + v0 + COMMA + ONE + CLOSED_PAREN;
        s += NEW_LINE_TAB + v1 + ARROW + ARG(0) + PLUS + EIGHT;
        s += NEW_LINE_TAB + STORE + v1 + ARROW + encoded_dim;
        s += store_dimensions( ARG(0), dimensions );
        return s;
    }

    string calculateArrayOffset(string & arr_name, string & offset, vector<IR_Item*> & indices){
        auto B = 16 + indices.size() * 8;
        string s;
        int dimension_offset = 24;
        vector<string> dimension_lengths;
        auto it = indices.begin();
        for(++it; it != indices.end(); ++it){
            auto ADDR_M = new_var_name();
            auto M_ = new_var_name();
            auto M = new_var_name();
            dimension_lengths.push_back(M);
            s += NEW_LINE_TAB + ADDR_M + ARROW + arr_name + PLUS + to_string(dimension_offset);
            s += NEW_LINE_TAB + M_ + ARROW + LOAD + ADDR_M;
            s += NEW_LINE_TAB + M + ARROW + M_ + SHIFT_RIGHT + ONE;   
            dimension_offset += 8;
        }

        auto L3_index = new_var_name();
        s += NEW_LINE_TAB + L3_index + ARROW + ZERO;
        for (int i =0; i < indices.size(); ++i){
            auto index = indices[i];
            auto accum = new_var_name();
            s += NEW_LINE_TAB + accum + ARROW + index->name;
            for(int j =i; j < dimension_lengths.size(); ++j){
                s += NEW_LINE_TAB + accum + ARROW + accum + STAR + dimension_lengths[j];
            }
            s += NEW_LINE_TAB + L3_index + ARROW + L3_index + PLUS + accum;
        }
        auto offsetAfterB = new_var_name();
        s += NEW_LINE_TAB + offsetAfterB + ARROW + L3_index + STAR + EIGHT;
        s += NEW_LINE_TAB + offset + ARROW + offsetAfterB + PLUS + to_string(B);
        return s;
    }

    string Array_Load_I::to_L3(){
        auto offset = new_var_name();
        auto addr = new_var_name();
        auto indices = utils::subvector(this->args, 2, this->args.size());
        string s;
        s += calculateArrayOffset(ARG(1), offset, indices);
        s += NEW_LINE_TAB + addr + ARROW + ARG(1) + PLUS + offset;
        s += NEW_LINE_TAB + ARG(0) + ARROW + LOAD + addr; 
        return s;
    }

    string tuple_offset(string & index){
        return to_string( 8 + (8 * std::stoi(index)) );
    }

    string Tuple_Load_I::to_L3(){
        auto v1 = new_var_name();
        auto offset = tuple_offset(ARG(2));
        string s;
        s += NEW_LINE_TAB + v1 + ARROW + ARG(1) + PLUS + offset;
        s += NEW_LINE_TAB + ARG(0) + ARROW + LOAD + v1;
        return s;
    }

    string Array_Store_I::to_L3(){
        auto offset = new_var_name();
        auto addr = new_var_name();
        auto src_index = this->args.size()-1;
        auto indices = utils::subvector(this->args, 1, src_index);
        string s;
        s += calculateArrayOffset(ARG(0), offset, indices);
        s += NEW_LINE_TAB + addr + ARROW + ARG(0) + PLUS + offset;
        s += NEW_LINE_TAB + STORE + addr + ARROW + ARG(src_index); 
        return s;
    }

    string Tuple_Store_I::to_L3(){
        auto v0 = new_var_name();
        auto offset = tuple_offset(ARG(1));
        string s;
        s += NEW_LINE_TAB + v0 + ARROW + ARG(0) + PLUS + offset;
        s += NEW_LINE_TAB + STORE + v0 + ARROW + ARG(2);
        return s;
    }

    string New_Tuple_I::to_L3(){
        return NEW_LINE_TAB + ARG(0) + ARROW + CALL + SPACE + ALLOCATE + OPEN_PAREN + ARG(1) + COMMA + ONE + CLOSED_PAREN;
    }

    string Conditional_Branch_I::to_L3(){
        auto cmpItem = this->args[0];
        string ret_str;
        string cmp_var;
        if(dynamic_cast<Number*>(cmpItem)){
            cmp_var = new_var_name();
            ret_str += NEW_LINE_TAB + cmp_var + ARROW + cmpItem->name;
        } else {
            cmp_var = cmpItem->name;
        }
        ret_str += NEW_LINE_TAB + BR + cmp_var + SPACE + this->args[1]->name + SPACE + this->args[2]->name ;
        return ret_str;
    }
    
    string Return_I::to_L3(){
        return NEW_LINE_TAB + RETURN;
    }

    string Return_Value_I::to_L3(){
        return NEW_LINE_TAB + RETURN + ARG(0);
    }

    string Assign_I::to_L3(){
        return NEW_LINE_TAB + ARG(0) + ARROW + ARG(1);
    }

    string Assign_Op_I::to_L3(){
        return NEW_LINE_TAB + ARG(0) + ARROW + ARG(1) + SPACE + ARG(2) + SPACE + ARG(3);
    }

    string translateCall(string & callee, vector<IR_Item*> & args){
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
        return NEW_LINE_TAB + translateCall( ARG(0), call_args );
    }

    string Assign_Call_I::to_L3(){
        auto call_args = utils::subvector(this->args, 2, this->args.size());
        return NEW_LINE_TAB + ARG(0) + ARROW + translateCall( ARG(1), call_args );
    }

    string BasicBlock::to_L3(){
        string bb = NEW_LINE_TAB + this->label.name;
        for(auto i : this->instructions){
            // cout << i->to_L3() << endl;
            bb += i->to_L3();
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
        f += CLOSED_PAREN + OPEN_BRACE;
        for (auto bb : this->basicBlocks){
            f += bb->to_L3();
        }
        
        f += NEW_LINE + CLOSED_BRACE + NEW_LINE + NEW_LINE;
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