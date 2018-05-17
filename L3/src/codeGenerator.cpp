#include <codeGenerator.h>
#include <fstream>

using namespace std;

namespace L3{
    const int ARGUMENT_REGISTERS = 6;

    bool isFunctionName(Label* label, vector<Function*> & functions){
        for (auto f : functions){
            if(label->equals(f->name)) return true;
        }
        return false;
    }

    void transformLabels(Program & p){
        if(p.longestLabel.size()>0){
            for (auto f : p.functions){
                string LLF = p.longestLabel.substr(1) + "_" + f->name->toString().substr(1) + "_";

                for (auto label : f->labels){
                    if(!isFunctionName((Label*)label, p.functions)){
                        label->name.insert(1, LLF);
                    }
                }
            }
        }
    }

    void generateCode(Program & p){
        std::ofstream outputFile;
        outputFile.open("prog.L2");
        outputFile << "(:main\n";
        for (auto f : p.functions){
            outputFile << f->toString();
        }
        outputFile << ")" << endl;
    }

     string L3_Item::toString(){
        return "ehh";
    }

    string Number::toString(){
        return std::to_string(this->value);
    }

    string StringItem::toString(){
        return this->name;
    }

    string Operator::toString(){
        switch(this->op){
            case PLUS: return "+=";
            case MINUS: return "-=";
            case STAR: return "*=";
            case AND: return "&=";
            case SHIFT_LEFT: return "<<=";
            case SHIFT_RIGHT: return ">>=";
            case LESS_THAN: return "<";
            case LESS_THAN_EQ: return "<=";
            case EQ: return "=";
            default: return "error";
        }
    }
 
    string Instruction::toString(){
        return i_line("instruction");
    }

    string Assign_I::toString(){
        auto dst = this->args.front()->toString();
        auto src = this->args.back()->toString();
        return i_line(dst + " <- " + src);
    }

    string Load_I::toString(){
        auto dst = this->args.front()->toString();
        auto addr = this->args.back()->toString();
        return i_line(dst + " <- mem " + addr + " 0");
    }

    string Store_I::toString(){
        auto addr = this->args.front()->toString();
        auto src = this->args.back()->toString();
        return i_line("mem " + addr + " 0 <- " + src);
    }

    string normalAssignOpI(string dst, string src, Operator* op, string src2){
        string s;
        s += i_line( dst + " <- " + src );
        s += i_line( dst + " " + op->toString() + " " + src2 );
        return s;
    }

    string Assign_Op_I::toString(){
        auto op = (Operator*)this->args[2];
        auto dst = this->args.front()->toString();
        auto src = this->args[1]->toString();
        auto src2 = this->args.back()->toString();
        switch(op->op){
            case GREATER_THAN:
                return i_line( dst + " <- " + src2 + " < " + src );
            case GREATER_THAN_EQ:
                return i_line( dst + " <- " + src2 + " <= " + src );
            case LESS_THAN:
            case LESS_THAN_EQ:
            case EQ:
                return i_line( dst + " <- " + src + " " + op->toString() + " " + src2 );
                // bug here
            // case SHIFT_LEFT:
            // case SHIFT_RIGHT:
            //     if(dst == src){
            //         return i_line(dst + " " + op->toString() + " " + src2);
            //     } else if(dst == src2){
            //         string s;
            //         s+= i_line(dst + " *= -1");
            //         s+= i_line(dst + " += " + src);
            //         return s;
            //     } else {
            //         return normalAssignOpI(dst, src, op, src2);
            //     }
            case MINUS:
                if(dst == src){
                    return i_line(dst + " " + op->toString() + " " + src2);
                } else if(dst == src2){
                    string s;
                    s+= i_line(dst + " *= -1");
                    s+= i_line(dst + " += " + src);
                    return s;
                } else {
                    return normalAssignOpI(dst, src, op, src2);
                }
            default:
                if(dst == src){
                    return i_line(dst + " " + op->toString() + " " + src2);
                } else if(dst == src2) {
                    return i_line(dst + " " + op->toString() + " " + src);
                } else{
                    return normalAssignOpI(dst, src, op, src2);
                }
        }
    }

    string Branch_I::toString(){
        return i_line("goto " + this->args.front()->toString());
    }

    string Conditional_Branch_I::toString(){
        string var = this->args.front()->toString();
        string true_label = this->args.back()->toString();
        string false_label = this->args[1]->toString();
        return i_line("cjump 0 = " + var + " " + true_label + " " + false_label);
    }

    string Label_I::toString(){
        return i_line(this->args.front()->toString());
    }

    string Return_I::toString(){
        return i_line("return");
    }

    string Return_Value_I::toString(){
        auto s = i_line( "rax <- " + this->args.front()->toString() );
        s += i_line("return");
        return s;
    }

    string argumentsToRegisters(vector<L3_Item*>& args){
        string s = "";
        for (int i =0; i < args.size(); ++i){
            if(i < ARGUMENT_REGISTERS){
                s += i_line( argumentRegisters[i] + " <- " + args[i]->toString());
            } else {
                auto x = (i - 4) * 8;
                s += i_line( "mem rsp -" + to_string(x) + " <- " + args[i]->toString() ); 
            }
        }
        return s;
    }

    string calleeCC(vector<Variable*>& params){
        string s = "";
        for (int i =0; i < params.size() ; ++i){
            if( i < ARGUMENT_REGISTERS ){
                s += i_line( params[i]->toString() + " <- " + argumentRegisters[i]);
            } else {
                auto M = 8 * (params.size() - i - 1);
                s += i_line( params[i]->toString() + "<- stack-arg " + to_string(M) );
            }
        }
        return s;
    }

    string callerCC(vector<L3_Item*>& callArgs, int count){
        auto addr = callArgs.front();
        auto functionName = addr->toString();
        auto returnLabel = functionName + "_ret_" + to_string(count);
        if(dynamic_cast<Variable*>(addr)){
            returnLabel.insert(0, ":");
        }
        auto isCallRuntime = dynamic_cast<Runtime_Function*>(callArgs.front());
        auto functionArguments = callArgs;
        functionArguments.erase(functionArguments.begin());
        
        auto s = isCallRuntime ? "" : i_line( "mem rsp -8 <- " + returnLabel);
        s += argumentsToRegisters(functionArguments);
        s += i_line("call " + functionName + " " + to_string(functionArguments.size()));
        s += isCallRuntime ? "" : i_line(returnLabel);
        return s;
    }

    string Call_I::toString(){
        return callerCC(this->args, this->count);
    }

    string Assign_Call_I::toString(){
        auto callArgs = this->args;
        callArgs.erase(callArgs.begin());
        auto s = callerCC(callArgs, this->count);
        s += i_line( this->args.front()->toString() + " <- rax");
        return s;
    }

    string Function::toString(){
        auto s = f_line("(" + this->name->toString());
        // vector<Contextual_I*> context;

        s += i_line(to_string(this->arguments.size()) + " 0");
        s += calleeCC(this->arguments);
        for (auto i : this->instructions){
            // cout << i->toString();
            if (dynamic_cast<Contextual_I*>(i)){
                // context.push_back((Contextual_I*)i);
                
                s += i->toString();
            }
            if (dynamic_cast<CallingC_I*>(i)){
                // s += transformContext(context);
                // context.clear();
                s += i->toString();
            }            
        }
        s += f_line(")");
        return s;
    }
}