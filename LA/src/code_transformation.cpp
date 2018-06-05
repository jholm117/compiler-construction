#include <code_transformation.h>

using namespace std;
using namespace utils;

namespace LA {
    void generate_basic_blocks(Program & p){
        for(auto f : p.functions){
            auto inst = f->instructions.front();
            vector<Instruction*> new_Insts;
            bool startBB = true;
            for (auto inst : f->instructions){
                if(startBB){
                    if(!dynamic_cast<Label_I*>(inst)){
                        auto L = new Label();
                        L->name = utils::new_label_name();
                        auto label_i = new Label_I();
                        label_i->args.push_back(L);
                        new_Insts.push_back(label_i);
                    }
                   startBB = false;
                } else if(dynamic_cast<Label_I*>(inst)){
                    auto g = new Branch_I();
                    g->args.push_back(inst->args[0]);
                    new_Insts.push_back(g);
                }
                new_Insts.push_back(inst);
                if(dynamic_cast<Terminator*>(inst)) startBB = true;
            }
            f->instructions = new_Insts;


        }
    }

    LA::Number * One(){
        auto one = new Number();
        one->name = "1";
        one->value = 1;
        return one;
    }

    vector<int> Instruction::toDecode(){
        return vector<int>();
    }

    vector<int> Instruction::toEncode(){
        return vector<int>();
    }

    vector<int> Assign_Op_I::toEncode(){
        return vector<int>{0};
    }

    vector<int> Assign_Op_I::toDecode(){
        return vector<int>{1, 3};
    }
    
    vector<int> Conditional_Branch_I::toDecode(){
        return vector<int>{0};
    }

    vector<int> Length_I::toDecode(){
        auto last_arg = this->args.back();
        if(dynamic_cast<Variable*>(last_arg)){
            return vector<int>{2};
        } else {
            return vector<int>();
        }
    }

    vector<int> arr_tup_load(int start, int end, vector<LA_Item*> & args){
        vector<int> v;
        for(int i=start; i < end; ++i){
            auto arg = args.at(i);
            if(dynamic_cast<Variable*>(arg)){
                v.push_back(i);
            }
        }
        return v;
    }

    vector<int> Array_Load_I::toDecode(){
        return arr_tup_load(2, this->args.size(), this->args);
    }

    vector<int> Array_Store_I::toDecode(){
        return arr_tup_load(1, this->args.size()-1, this->args);
    }

    vector<int> Tuple_Load_I::toDecode(){
        return arr_tup_load(2, 3, this->args);
    }

    vector<int> Tuple_Store_I::toDecode(){
        return arr_tup_load(1, 2, this->args);
    }


    void decode_args(Instruction* inst, vector<Instruction*> & new_Insts){
        for (auto tee_index : inst->toDecode()){
            auto tee = inst->args.at(tee_index);
            auto v_prime = utils::new_var_name();
            auto var = new Variable();
            var->name = v_prime;
            vector<LA_Item*> v1 = {var, tee};
            auto assign = new Assign_I();
            assign->args = v1;
            new_Insts.push_back(assign);
            auto decode = new Assign_Op_I();
            auto shift = new Operator();
            auto one = new Number();
            one->name = "1";
            shift->name = ">>";
            vector<LA_Item*> v = {var, var, shift, one};
            decode->args = v;
            new_Insts.push_back(decode);

            inst->args.at(tee_index) = var;

            // for(int i =0; i < inst->args.size(); ++i){
            //     auto arg = inst->args.at(i);
            //     if (*arg == *tee) {
            //         delete arg;
            //         arg = tee;
            //     }
            // }
        }
        return;
    }

    void encode_args(Instruction* inst, vector<Instruction*> & new_Insts){
        for (auto var_index : inst->toEncode()){
            auto var = inst->args.at(var_index);
            auto shift_i = new Assign_Op_I();
            auto shift_op = new Operator(); 
            auto one = One();
            shift_op->name = "<<";
            vector<LA_Item*> shift_args = {var, var, shift_op, one};
            shift_i->args = shift_args;
            auto plus_i = new Assign_Op_I();
            auto plus_op = new Operator();
            auto one2 = One();
            plus_op->name = "+";
            vector<LA_Item*> plus_args = {var, var, plus_op, one2};
            plus_i->args = plus_args;
            new_Insts.push_back(shift_i);
            new_Insts.push_back(plus_i);
        }
        return;

    }

    void encode_decode(Program & p){
        for (auto f : p.functions){
            vector<Instruction*> new_Insts;
            for (auto i : f->instructions){
                decode_args(i, new_Insts);
                new_Insts.push_back(i);
                encode_args(i, new_Insts);
                
            }
            f->instructions = new_Insts;
        }
    }

    void check_allocation(Variable* arr, vector<Instruction*> & new_Insts){
        // s += NEW_LINE_TAB + BR + arr_tup + SPACE + continue_label + SPACE + error_label;
        auto continue_label = new Label( new_label_name() );
        auto error_label = new Label( new_label_name() );
        // vector<LA_Item*> branch_args = {arr, continue_label, error_label};
        auto branch_i = new Conditional_Branch_I( arr, continue_label, error_label );
        // cout << "hee: " << branch_i->args.size() << endl;
        // s += NEW_LINE_TAB + error_label;
        auto error_label_i = new Label_I(error_label);
        // s += NEW_LINE_TAB + CALL_ARRAY_ERROR + "(0, 0)";
        auto array_error = new Runtime_Callee("array-error");
        auto n1 = new Number("0");
        auto n2 = new Number("0");
        vector<LA_Item*> call_args = {array_error, n1, n2};
        auto call_array_error = new Call_I(call_args);
        // s += NEW_LINE_TAB + continue_label;
        auto continue_label_i = new Label_I(continue_label);
        new_Insts.push_back(branch_i);
        new_Insts.push_back(error_label_i);
        new_Insts.push_back(call_array_error);
        new_Insts.push_back(continue_label_i);
    }

    void check_in_range(Variable* arr, vector<LA_Item*> & indices, vector<Instruction*> & new_Insts){
        int dim = 0;
        for (auto index : indices){
            auto l_i = new Variable( new_var_name() );
            auto cmp = new Variable( new_var_name() );
            auto continue_label = new Label( new_label_name() );
            auto error_label = new Label( new_label_name() );
            // s += NEW_LINE_TAB + INT64 + l_i;
            auto int_64 = new Int64();
            vector<LA_Item*> args1 = {int_64, l_i};
            auto declare_l_i = new Type_Var_I(args1);
            new_Insts.push_back(declare_l_i);
            
            // s += NEW_LINE_TAB + INT64 + cmp;
            vector<LA_Item*> args2 = {int_64, cmp};
            auto declare_cmp = new Type_Var_I(args2);
            new_Insts.push_back(declare_cmp);

            // s += NEW_LINE_TAB + l_i + ARROW + LENGTH + arr_tup + SPACE + std::to_string(dim++);
            auto curr_dim = new Number(to_string(dim++));
            vector<LA_Item*> args3 = {l_i, arr, curr_dim};
            auto length_i = new Length_I(args3);
            new_Insts.push_back(length_i);

            auto shift_right = new Operator(">>");
            auto one = One();
            vector<LA_Item*> args8 = {l_i, l_i, shift_right, one};
            auto decode_length = new Assign_Op_I(args8);
            new_Insts.push_back(decode_length);


            // s += NEW_LINE_TAB + cmp + ARROW + index->name + LESS_THAN + l_i;
            auto less_than = new Operator("<");
            vector<LA_Item*> args4 = {cmp, index, less_than, l_i };
            auto compare_i = new Assign_Op_I(args4);
            new_Insts.push_back(compare_i);

            // s += NEW_LINE_TAB + BR + cmp + SPACE + continue_label + SPACE + error_label;
            // vector<LA_Item*> args5 = { cmp, continue_label, error_label };
            auto cond_branch_i = new Conditional_Branch_I(cmp, continue_label, error_label);
            new_Insts.push_back(cond_branch_i);

            // s += NEW_LINE_TAB + error_label;
            auto error_label_i = new Label_I(error_label);
            new_Insts.push_back(error_label_i);

            //encode
            auto encoded_index = new Variable( new_var_name() );
            vector<LA_Item*> args9 = {new Int64(), encoded_index};
            auto declare_temp = new Type_Var_I(args9);
            new_Insts.push_back(declare_temp);
            auto shift_left = new Operator("<<");
            vector<LA_Item*> args10 = {encoded_index, index, shift_left, one};
            auto shift_left_i = new Assign_Op_I(args10);
            new_Insts.push_back(shift_left_i);
            auto plus = new Operator("+");
            vector<LA_Item*> args11 = {encoded_index, encoded_index, plus, one};
            auto plus_one_i = new Assign_Op_I(args11);
            new_Insts.push_back(plus_one_i);
            


            // s += NEW_LINE_TAB + CALL_ARRAY_ERROR + "(" + arr_tup + COMMA + index->name + ")";
            auto arr_error = new Runtime_Callee("array-error");
            vector<LA_Item*> args6 = {arr_error, arr, encoded_index};
            auto call_array_error = new Call_I(args6);
            new_Insts.push_back(call_array_error);


            // s += NEW_LINE_TAB + continue_label;
            new_Insts.push_back(new Label_I(continue_label));


        }
    }

    void check_array_access(Program & p){
        for(auto f : p.functions){
            vector<Instruction*> new_Insts;
            for(auto i : f->instructions){
                // cout << "1" << endl;
                if(dynamic_cast<Array_Load_I*>(i) || dynamic_cast<Tuple_Load_I*>(i)){
                    // cout << "2" << endl;
                    auto dims = utils::subvector(i->args, 2, i->args.size());
                    auto arr = dynamic_cast<Variable*>(i->args.at(1));
                    check_allocation(arr, new_Insts);
                    check_in_range(arr, dims, new_Insts);
                } else if (dynamic_cast<Array_Store_I*>(i) || dynamic_cast<Tuple_Store_I*>(i)){
                    // cout << "3" << endl;
                    auto dims = utils::subvector(i->args, 1, i->args.size()-1);
                    auto arr = dynamic_cast<Variable*>(i->args.at(0));
                    check_allocation(arr, new_Insts);
                    check_in_range(arr, dims, new_Insts);
                }
                // cout << "4" << endl;
                new_Insts.push_back(i);
            }
            f->instructions = new_Insts;
        }
    }
}