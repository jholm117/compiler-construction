#include <code_transformation.h>
#include <utils.h>
#include <set>

namespace LB{

    void rename_var_definitons(Scope & s){
        for (auto i : s.instructions){
            
            auto scope_i = dynamic_cast<Scope_I*>(i);
            if(scope_i){
                rename_var_definitons(scope_i->scope);
            } else if(dynamic_cast<Type_Var_I*>(i)){
                for(auto it = i->args.begin() + 1; it != i->args.end(); ++it){
                    auto new_name = utils::new_var_name();
                    s.flat_bindings_map.emplace( (*it)->name, new_name);
                    (*it)->name = new_name;
                }
            }
        }
    }

    string lookup_newname(string & old_name, vector<Bindings_Map> & bindings ){
        for(auto it = bindings.rbegin(); it != bindings.rend(); ++it){
            auto curr_map = (*it);
            auto new_name = curr_map.find(old_name);
            if(new_name != curr_map.end()) return (*new_name).second;
        }
        return old_name;
    }

    void rename_var_uses(Scope & s, vector<Bindings_Map> bindings ){
        for(auto i : s.instructions){
            auto scope_i = dynamic_cast<Scope_I*>(i);
            bindings.push_back(s.flat_bindings_map);
            if(scope_i){
                rename_var_uses(scope_i->scope, bindings);
            } else if (!dynamic_cast<Type_Var_I*>(i)){
                for(auto item : i->args){
                    if(dynamic_cast<Variable*>(item)){
                        item->name = lookup_newname(item->name, bindings);
                    }
                }
            }
        }
    }

    void flatten_scope(Scope & s){
        vector<Instruction*> new_Insts;
        for(auto i : s.instructions){
            auto scope_i = dynamic_cast<Scope_I*>(i);
            if(scope_i){
                flatten_scope(scope_i->scope);
                new_Insts.insert(new_Insts.end(), scope_i->scope.instructions.begin(), scope_i->scope.instructions.end());
            } else{
                new_Insts.push_back(i);
            }
        }
        s.instructions = new_Insts;
    }    

    void flatten_scopes(Program & p){
        for(auto f : p.functions){
            for(auto i : f->scope.instructions){
                auto scope_i = dynamic_cast<Scope_I*>(i);
                if(scope_i){
                    rename_var_definitons(scope_i->scope);
                    rename_var_uses(scope_i->scope, vector<Bindings_Map>());
                }
            }
            flatten_scope(f->scope);
            
        }
    }

    Label_I* find_label(Label target, Function * f){
        for(auto i : f->scope.instructions){
            auto label_i = dynamic_cast<Label_I*>(i);
            if(label_i && *(i->args[0]) == target){
                return label_i;
            }
        }

        cout << "couldn't find label" << endl;
        return nullptr;
    }
    typedef unordered_map<While_I*, Label_I*> While_Label_Map;

    vector<Instruction*> init_maps_and_cond_labels(Function * f, While_Label_Map & begin_while, While_Label_Map & end_while, While_Label_Map & cond_labels ){
        vector<Instruction*> new_Insts;
            for(auto i : f->scope.instructions){
                auto while_i = dynamic_cast<While_I*>(i);
                if(while_i){
                    auto l1 = *dynamic_cast<Label*>(while_i->args[3]);
                    auto l2 = *dynamic_cast<Label*>(while_i->args[4]);
                    
                    auto begin = find_label(l1, f);
                    auto end = find_label(l2, f);
                    begin_while.emplace(while_i, begin);
                    end_while.emplace(while_i, end);

                    auto cond_label = new Label( utils::new_label_name() );
                    auto cond_label_i = new Label_I();
                    cond_label_i->args.push_back(cond_label);
                    new_Insts.push_back(cond_label_i);
                    cond_labels.emplace(while_i, cond_label_i);
                }
                new_Insts.push_back(i);
            }
        return new_Insts;
    }

    While_Label_Map::iterator scan_begin_while(Label & target_label, While_Label_Map & begin_while){
        for(auto it = begin_while.begin(); it != begin_while.end(); ++it){
            if( *(it->second->args[0]) == target_label ){
                return it;
            }
        }
        return begin_while.end();
    }

    While_Label_Map::iterator scan_end_while(Label & target_label, While_Label_Map & end_while){
        for(auto it = end_while.begin(); it != end_while.end(); ++it){
            if( *(it->second->args[0]) == target_label ){
                return it;
            }
        }
        return end_while.end();
    }

    unordered_map<Instruction*, While_I*> map_instructions_to_loops(vector<Instruction*> & new_Insts, While_Label_Map & begin_while, While_Label_Map & end_while){
        vector<While_I*> loop_stack;
        unordered_map<Instruction*, While_I*> instruction_loop_map;
        set<While_I*> while_seen;
        for(auto i : new_Insts){
            // cout << i->gen_LA() <<endl;
            // cout << "active loops: " << loop_stack.size() << endl;
            if(!loop_stack.empty()){
                instruction_loop_map.emplace(i, loop_stack.back());
            }
            auto while_i = dynamic_cast<While_I*>(i);
            if(while_i && while_seen.find(while_i) == while_seen.end()){
                loop_stack.push_back(while_i);
                while_seen.insert(while_i);
                continue;
            }
            auto label_i = dynamic_cast<Label_I*>(i);
            if(label_i){
                auto target_label = *dynamic_cast<Label*>(label_i->args[0]);
                auto w = scan_begin_while(target_label, begin_while);
                if(w != begin_while.end() && while_seen.find(w->first) == while_seen.end()){
                    loop_stack.push_back(w->first);
                    continue;
                }
                auto w2 = scan_end_while(target_label, end_while);
                if(w2 != end_while.end()){
                    loop_stack.pop_back();
                }
            }
        }
        return instruction_loop_map;
    }

    void translate_control_structures(Program & p){
        for(auto f : p.functions){
            While_Label_Map begin_while;
            While_Label_Map end_while;
            While_Label_Map cond_labels;

            auto new_Insts = init_maps_and_cond_labels(f, begin_while, end_while, cond_labels);
            // cout << "yo " <<new_Insts.size() << endl;
            auto instruction_loop_map = map_instructions_to_loops(new_Insts, begin_while, end_while);
            // cout << "asd " << instruction_loop_map.size() << endl;
            // for(auto kvpair : instruction_loop_map){
            //     cout << kvpair.first->gen_LA() << endl;
            // }
            // cout << "hola" << endl;
            for(auto i : new_Insts){
                if(dynamic_cast<Continue_I*>(i)){
                    auto w = instruction_loop_map.at(i);
                    // cout << instruction_loop_map.size() << endl;
                    // cout << w->gen_LA() << endl;
                    // cout << cond_labels.size() << endl;
                    auto l_cond = cond_labels.at(w)->args[0];
                    // cout << "not gonna print" << endl;
                    i->args.push_back(l_cond);
                }
                if(dynamic_cast<Break_I*>(i)){
                    auto w = instruction_loop_map.at(i);
                    auto l_exit = cond_labels.at(w)->args[0];
                    i->args.push_back(l_exit);
                }
            }
            // cout << "over here" << endl;
            f->scope.instructions = new_Insts;
        }
    }
}