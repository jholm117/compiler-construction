#include <L3.h>

using namespace std;

namespace L3 {

    // string operatorToString(L3_OperatorType t){
    //     switch(){
    //         case PLUS: return "+";
    //         case MINUS: return "-";
    //         case STAR: return "*";
    //         case AND: return "&";
    //         case SHIFT_LEFT: return "<<";
    //         case SHIFT_RIGHT: return ">>";
    //         case LESS_THAN: return "";
    //         case LESS_THAN_EQ: return "";
    //         case EQ: return "";
    //         case GREATER_THAN: return "";
    //         case GREATER_THAN_EQ: return "";
    // case LOAD
    // case STORE
    //         default: return "Invalid Operator Type";
    //     }
    // }

    L3_OperatorType stringToOperatorType(string op){
        if( op == "+") return PLUS;
        if( op == "-") return MINUS;
        if( op == "*") return STAR;
        if( op == "&") return AND;
        if( op == "<<") return SHIFT_LEFT;
        if( op == ">>") return SHIFT_RIGHT;
        if( op == "<") return LESS_THAN;
        if( op == "<=") return LESS_THAN_EQ;
        if( op == "=") return EQ;
        if( op == ">") return GREATER_THAN;
        if( op == ">=") return GREATER_THAN_EQ;
        else return OP_TYPE_ERROR;
    }
} 
