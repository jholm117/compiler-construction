#pragma once

#include <vector>
#include <algorithm>

using namespace std;

namespace utils {
    
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

}