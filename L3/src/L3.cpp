#include <L3.h>

using namespace std;

namespace L3 {
    
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

    Operator::Operator(L3_OperatorType o){
        this->op = o;
    }
} 
