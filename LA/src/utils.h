#pragma once

#include <vector>
#include <string>
#include <algorithm>

using namespace std;

namespace utils {

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
    const string LENGTH = "length ";
    const string CALL_ARRAY_ERROR = "call array-error";
    const string LESS_THAN = " < ";
    const string COLON = ":";
    const string INT64 = "int64 ";
    
    template< typename TO, typename FROM, class UnaryOperator  >
    vector<TO> map(vector<FROM> & from_vec, vector<TO> & to_vec, UnaryOperator & op){
        to_vec.resize(from_vec.size());
        std::transform(from_vec.begin(), from_vec.end(), to_vec.begin(), op );
        return to_vec;
    }
    
    template< typename TYPE >
    TYPE pop_item(vector<TYPE> & vec){
        auto item = vec.back();
        vec.pop_back();
        return item;
    }

    template < typename TYPE1, typename TYPE2 >
    TYPE1 pair_first(tuple<TYPE1, TYPE2> & pair){
        return std::get<0>(pair);
    }

    template < typename TYPE1, typename TYPE2 >
    TYPE2 pair_second(tuple<TYPE1, TYPE2> & pair){
        return std::get<1>(pair);
    }

    template < typename T >
    vector<T> subvector(vector<T> & vec, int start, int end){
        auto first = vec.begin() + start;
        auto last = vec.begin() + end;
        return vector<T> (first, last);
    }

    string new_var_name();
    string new_label_name();
}