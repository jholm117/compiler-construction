#include <L2.h>
#include <string>
#include <vector>
#include <iostream>
#include <set>
#include <iterator>
#include <algorithm>

using namespace std;
namespace L2{
    void PrintVec(std::vector<L2_Item*> v){
      for (L2_Item* a : v) {
          std::cout << a->toString() << std::endl;
      }
    }
    std::string SetToString(OUR_SET s){
        string r = "(";
        // for(OUR_SET::iterator it = s.begin(); it != s.end(); ++it){
        for(L2_Item* it : s){
            r += it->toString() + " ";
        }
        r += ")\n";
        return r;
    }

    std::string DataFlowResult::toString(){ 
        // std::string r = "gen set\n";
        // for(auto s: genSets) r += SetToString(s);
        // r+= "\nkill set\n";
        // for(auto s: killSets) r += SetToString(s);
        // r+="\n";
        std::string r = "(\n(in\n"; 
        for (OUR_SET s : inSets) {
            r += SetToString(s);
        }
        r += ")\n\n(out\n";
        for (OUR_SET s : outSets) {
            r += SetToString(s);
        }
        r += ")\n\n)";
        return r;
    }


    OUR_SET ComputeSet(vector<L2_Item*> items) {
        OUR_SET v; 
        for (L2_Item* a : items){
            //doesnt work :(
            switch(a->item_type) {
                case VAREXP:
                    v.insert(a);
                    break;
                case MEMORY:
                    v.insert(((Mem*)a)->address);
                default:
                    break;
            }
        }
        v.erase(new Register(rsp));
        return v;
    }

    vector<L2_Item*> mapToRegisters(vector<regType> regs){
        vector<L2_Item*> v;
        for (regType r : regs){
            v.push_back(new Register(r));
        }
        return v;
    }
    // start to end inclusive
    vector<L2_Item*> Subvector(vector<L2_Item*>& vec, int start, int end){
        vector<L2_Item*>::iterator first = vec.begin() + start;
        vector<L2_Item*>::iterator last = vec.begin() + end;
        
        return vector<L2_Item*> (first, last);
    }

    vector<L2_Item*> GenArguments(L2_Item* address, L2_Item* numArgs){
        vector<L2_Item*> v;
        int n = ((L2::Number*)numArgs)->num;
        // just added i<6
        for(int i = 0; i < n && i < 6; ++i){
            v.push_back(new Register(arguments[i]));
        }
        v.push_back(address);
        return v;
    }

    vector<L2_Item*> KillCallerSaved(){
        return mapToRegisters(callerSaved);
    }

    vector<L2_Item*> ReturnGenRegisters(){
        vector<L2_Item*> v = mapToRegisters(calleeSaved);
        v.push_back(new Register(rax));
        return v;
    }

    OUR_SET ComputeAssignGenSet(vector<L2_Item*>& args){
        auto dest = args.front();
        if(dynamic_cast<Mem*>(dest)){
            return ComputeSet(args);
        } else {
            return ComputeSet(Subvector(args,1, args.size()));
        }
    }

    OUR_SET ComputeAssignKillSet(vector<L2_Item*>& args){
        auto dest = args.front();
        if(dynamic_cast<Mem*>(dest)){
            return OUR_SET();
        } else {
            return ComputeSet(Subvector(args,0, 1));
        }
    }

    OUR_SET ComputeGen(Instruction* i) {
        switch(i->type){
            case CALL_LOCAL:
            case CALL_RUNTIME:
                return ComputeSet(GenArguments(i->args.front(), i->args.back()));
            case CJUMP:
            case INC_DEC:
            case SOP:
            case AOP:
                return ComputeSet(i->args);
            case LEA:
            case ASSIGN_CMP:
                return ComputeSet(Subvector(i->args, 1, i->args.size()));
            case ASSIGN: 
                return ComputeAssignGenSet(i->args);
            case RETURN:
                return ComputeSet(ReturnGenRegisters());
            default:
                return OUR_SET();
        }
    }

    OUR_SET ComputeKill(Instruction* i) {
        switch(i->type){
            case ASSIGN:
            case AOP:
                return ComputeAssignKillSet(i->args); 
            case INC_DEC:
            case SOP:
            case LEA:
            case ASSIGN_CMP:
                return ComputeSet(Subvector(i->args, 0 , 1)); // hope 0 to 0 works
            case CALL_LOCAL:
            case CALL_RUNTIME:
                return ComputeSet(KillCallerSaved());
            default:
                return OUR_SET();
        }
    }

    OUR_SET ComputeInSet(OUR_SET& gen, OUR_SET& kill, OUR_SET& out){
        OUR_SET out_kill_diff;
        std::set_difference(out.begin(), out.end(), kill.begin(), kill.end(), std::inserter(out_kill_diff, out_kill_diff.begin()), L2_Compare());
        OUR_SET in_set;
        std::set_union(gen.begin(), gen.end(), out_kill_diff.begin(), out_kill_diff.end(), std::inserter(in_set, in_set.begin()), L2_Compare());
        return in_set;
    }

    int find_label_i(L2_Item* l, vector<Instruction*>& instructions){
        for (int index = 0; index < instructions.size(); ++index){
            Instruction* i = instructions[index];
            if(i->type == LABEL && i->args.back()->equals( l )) return index;
        }
        std::cout << "couldn't find label" << std::endl;
        return -1;
    }

    //make reference
    vector<OUR_SET> getSuccessors(int i_index, vector<Instruction*> & instructions, vector<OUR_SET> & in_sets){
        Instruction* i_ptr = instructions[i_index];
        int successor, first_succ, second_succ;
        switch(i_ptr->type){
            case GOTO:
                successor = find_label_i(i_ptr->args.back(), instructions);
                // cout << successor <<endl;
                return vector<OUR_SET> { in_sets[successor] }; 
                
            case CJUMP:
                first_succ = find_label_i(i_ptr->args[3], instructions);
                second_succ = find_label_i(i_ptr->args[4], instructions);
                // cout << first_succ << " " << second_succ << endl;
                return vector<OUR_SET> { in_sets[first_succ], in_sets[second_succ] };

            case RETURN:
                // cout << endl;
                return vector<OUR_SET>();

            default:
                // cout << i_index+1 << endl;
                return vector<OUR_SET> { in_sets[i_index+1] }; 
        } 
    }

    // vector<int> computeSuccessors(int i_index, vector<Instruction*> instructions){
    //     Instruction* i_ptr = instructions[i_index];
    //     vector<int> successors;
    //     switch(i_ptr->type){
    //         case GOTO:
    //             successors.push_back( find_label_i(i_ptr->args.back(), instructions) );
    //             break;
                
    //         case CJUMP:
    //             successors.push_back( find_label_i(i_ptr->args[3], instructions) );
    //             successors.push_back( find_label_i(i_ptr->args[4], instructions)  );
    //             break;

    //         case RETURN:
    //             break;

    //         default:
    //             successors.push_back( i_index +1 ); 
    //     } 
    //     return successors;
    // }

    // vector<OUR_SET> getSuccessors(int i_index, vector<Instruction*> instructions, vector<vector<int>> successors){
    //     if( successors.at(i_index).front() == -1 ) computeSuccessors(i_index, instructions)

    // }

    OUR_SET ComputeOutSet(int index, vector<Instruction*>& instructions, vector<OUR_SET>& in_sets){
        vector<OUR_SET> successor_in_sets = getSuccessors(index, instructions, in_sets);
        OUR_SET out_set;
        for(OUR_SET s : successor_in_sets){
            std::set_union(out_set.begin(),out_set.end(),s.begin(),s.end(),std::inserter(out_set, out_set.begin()));
        }
        return out_set;
    }

    bool areDifferent(OUR_SET & set1, OUR_SET & set2){
        if(set1.size() != set2.size()) return true;
        OUR_SET diff;
        std::set_symmetric_difference(set1.begin(), set1.end(), set2.begin(), set2.end(), std::inserter(diff, diff.begin()), L2_Compare());
        return !diff.empty();
    }

    DataFlowResult* computeLivenessAnalysis(Program p, Function* f) {
        vector<OUR_SET> gen_sets;
        vector<OUR_SET> kill_sets;

        for (Instruction* i : f->instructions) {
            // cout << i->toString(f->locals) << endl;
            // cout << "GEN:" << SetToString(ComputeGen(i));
            // cout << "KILL:" << SetToString(ComputeKill(i)) << endl << endl;
            
            gen_sets.push_back(ComputeGen(i));
            kill_sets.push_back(ComputeKill(i));
        }
        vector<OUR_SET> in_sets(f->instructions.size());
        vector<OUR_SET> out_sets(f->instructions.size());

        bool in_or_out_changed;
            int j =0 ;

        do{
            in_or_out_changed = false;

            for(int i = 0; i < f->instructions.size(); ++i){
            // for(int i = f->instructions.size()-1; i >= 0 ; --i){
               
                OUR_SET in_set = ComputeInSet(gen_sets[i], kill_sets[i], out_sets[i]);
                OUR_SET out_set = ComputeOutSet(i, f->instructions, in_sets);

                in_or_out_changed = in_or_out_changed || areDifferent(in_set, in_sets[i]) || areDifferent(out_set, out_sets[i]);
                in_sets[i] = in_set;
                out_sets[i] = out_set;
            }
                // return nullptr;
            // cout << endl;
        } while(in_or_out_changed);
        

        return new DataFlowResult(in_sets, out_sets, gen_sets, kill_sets);
    }

}
