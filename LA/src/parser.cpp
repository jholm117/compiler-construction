#include <parser.h>
#include <algorithm>

#include <tao/pegtl.hpp>
#include <tao/pegtl/analyze.hpp>
#include <tao/pegtl/contrib/raw_string.hpp>

namespace pegtl = tao::TAO_PEGTL_NAMESPACE;

using namespace std;
using namespace pegtl;

namespace LA {

    vector<LA_Item*> parsedItems;
    
    struct comment: 
        pegtl::disable< 
            TAOCPP_PEGTL_STRING( "//" ), 
            pegtl::until< pegtl::eolf > 
        > {};

    struct seps: 
        pegtl::star< 
            pegtl::sor< 
                pegtl::ascii::space, 
                comment 
            > 
        > {};

    struct name:
        pegtl::seq<
            pegtl::plus< 
                pegtl::sor<
                pegtl::alpha,
                pegtl::one< '_' >
                >
            >,
            pegtl::star<
                pegtl::sor<
                pegtl::alpha,
                pegtl::one< '_' >,
                pegtl::digit
                >
            >
        >{};

    struct number_rule:
        pegtl::seq<
            pegtl::opt<
                pegtl::sor<
                    pegtl::one< '-' >,
                    pegtl::one< '+' >
                >
            >,
            pegtl::plus<
                pegtl::digit
            >
        >{};
    
    struct decoded_number_rule:
        number_rule {};

    struct variable_rule:
        pegtl::seq<
            pegtl::one< '%' >,
            name
        > {};

    struct label_rule:
        pegtl::seq<
            pegtl::one< ':' >,
            name
        >{};

    struct tuple_rule:
        TAOCPP_PEGTL_STRING("tuple"){};

    struct code_rule:
        TAOCPP_PEGTL_STRING("code"){};
    
    struct void_rule:
        TAOCPP_PEGTL_STRING("void"){};

    struct int64_array_rule:
        pegtl::seq<
            TAOCPP_PEGTL_STRING("int64"),
            pegtl::plus< TAOCPP_PEGTL_STRING("[]") >
        >{};


    struct int64_rule:
        TAOCPP_PEGTL_STRING("int64"){};

    struct type_rule:
        pegtl::sor<
            tuple_rule,
            code_rule,
            int64_array_rule,
            int64_rule
        >{};
        
    struct return_type_rule:
        pegtl::sor<
            tuple_rule,
            code_rule,
            void_rule,
            int64_rule
        >{};

    struct variable_or_number:
        pegtl::sor<
            variable_rule,
            number_rule
        >{};

    struct variable_or_decoded_number:
        pegtl::sor<
            variable_rule,
            decoded_number_rule
        >{};

    struct source:
        pegtl::sor<
            variable_or_number,
            label_rule
        >{};
    
    struct op_rule:
        pegtl::sor<
            TAOCPP_PEGTL_STRING("+"),
            TAOCPP_PEGTL_STRING("-"),
            TAOCPP_PEGTL_STRING("*"),
            TAOCPP_PEGTL_STRING("&"),
            TAOCPP_PEGTL_STRING("<<"),
            TAOCPP_PEGTL_STRING(">>"),
            TAOCPP_PEGTL_STRING("<="),
            TAOCPP_PEGTL_STRING("<"),
            TAOCPP_PEGTL_STRING("="),
            TAOCPP_PEGTL_STRING(">="),
            TAOCPP_PEGTL_STRING(">")
        >{};
    
    struct parameter_rule:
        pegtl::seq<
            seps,
            type_rule,
            seps,
            variable_rule,
            seps
        >{};

    struct parameters_rule:
        pegtl::seq<
            parameter_rule,
            pegtl::star<
                pegtl::seq<
                    pegtl::one< ',' >,
                    parameter_rule
                >
            >
        >{};

    struct conditional_branch_i:
        pegtl::seq<
            TAOCPP_PEGTL_STRING("br"),
            seps,
            variable_or_number,
            seps,
            label_rule,
            seps,
            label_rule
        >{};

    struct branch_i:
        pegtl::seq<
            TAOCPP_PEGTL_STRING("br"),
            seps,
            label_rule
        >{};
    
    struct return_value_i:
        pegtl::seq<
            TAOCPP_PEGTL_STRING("return"),
            seps,
            variable_or_number
        >{};
    
    struct return_i:
        TAOCPP_PEGTL_STRING("return"){};
    
    struct type_var_i:
        pegtl::seq<
            type_rule,
            seps,
            variable_rule
        >{};
    
    struct assign_i:
        pegtl::seq<
            variable_rule,
            seps,
            TAOCPP_PEGTL_STRING("<-"),
            seps,
            source
        >{};
    
    struct assign_op_i:
        pegtl::seq<
            variable_rule,
            seps,
            TAOCPP_PEGTL_STRING("<-"),
            seps,
            variable_or_number,
            seps,
            op_rule,
            seps,
            variable_or_number
        >{};

    struct array_dimension:
        pegtl::seq<
            one< '[' >,
            variable_or_decoded_number,
            one< ']' >
        >{};

    struct array_load_i:
        pegtl::seq<
            variable_rule,
            seps,
            TAOCPP_PEGTL_STRING("<-"),
            seps,
            variable_rule,
            pegtl::plus< array_dimension >
        >{};

    struct array_store_i:
        pegtl::seq<
            variable_rule,
            pegtl::plus< array_dimension >,
            seps,
            TAOCPP_PEGTL_STRING("<-"),
            seps,
            source
        >{};

    struct length_i:
        pegtl::seq<
            variable_rule,
            seps,
            TAOCPP_PEGTL_STRING("<-"),
            seps,
            TAOCPP_PEGTL_STRING("length"),
            seps,
            variable_rule,
            seps,
            variable_or_decoded_number
        >{};

    struct args_rule:
        pegtl::seq<
            variable_or_number,
            pegtl::star<
                seps,
                pegtl::one< ',' >,
                seps,
                variable_or_number
            >
        > {};


    struct runtime_callee_rule:
        pegtl::sor<
            TAOCPP_PEGTL_STRING("print"),
            TAOCPP_PEGTL_STRING("allocate"),
            TAOCPP_PEGTL_STRING("array-error")
        >{};

    struct function_callee_rule:
        name{}; 

    struct open_paren:
        pegtl::seq< 
            seps,
            one< '(' >
        >{};

    
    struct callee_rule:
        pegtl::sor<
            pegtl::seq<
                pegtl::at<
                    pegtl::seq<
                        runtime_callee_rule,
                        open_paren
                    >
                >,
                runtime_callee_rule
            >,
            pegtl::seq<
                pegtl::at<
                    pegtl::seq<
                        function_callee_rule,
                        open_paren
                    >
                >,
                function_callee_rule
            >,
            variable_rule
        >{};

    struct call_rule:
        pegtl::seq<
            TAOCPP_PEGTL_STRING("call"),
            seps,
            callee_rule,
            seps,
            one< '(' >,
            seps,
            pegtl::opt< args_rule >,
            seps,
            one< ')' >
        >{};

    struct call_i:
        call_rule {};

    struct assign_call_i:
        pegtl::seq<
            variable_rule,
            seps,
            TAOCPP_PEGTL_STRING("<-"),
            seps,
            call_rule
        >{};
    
    struct new_array_i:
        pegtl::seq<
            variable_rule,
            seps,
            TAOCPP_PEGTL_STRING("<-"),
            seps,
            TAOCPP_PEGTL_STRING("new"),
            seps,
            TAOCPP_PEGTL_STRING("Array"),
            seps,
            one< '(' >,
            seps,
            args_rule,
            seps,
            one< ')' >
        >{};
    
    struct new_tuple_i:
        pegtl::seq<
            variable_rule,
            seps,
            TAOCPP_PEGTL_STRING("<-"),
            seps,
            TAOCPP_PEGTL_STRING("new"),
            seps,
            TAOCPP_PEGTL_STRING("Tuple"),
            seps,
            one< '(' >,
            seps,
            variable_or_number,
            seps,
            one< ')' >
        >{};

    struct label_i:
        pegtl::seq< label_rule >{};

    struct instruction_rule:
        pegtl::seq<
            seps,
            pegtl::sor<
                pegtl::seq< pegtl::at< label_i >, label_i >,            
                pegtl::seq< pegtl::at< conditional_branch_i >, conditional_branch_i >,
                pegtl::seq< pegtl::at< branch_i >, branch_i >,
                pegtl::seq< pegtl::at< return_value_i >, return_value_i >,
                pegtl::seq< pegtl::at< return_i >, return_i >,
                pegtl::seq< pegtl::at< type_var_i >, type_var_i >,
                pegtl::seq< pegtl::at< assign_op_i >, assign_op_i >,
                pegtl::seq< pegtl::at< array_load_i >, array_load_i >,
                pegtl::seq< pegtl::at< array_store_i >, array_store_i >,
                pegtl::seq< pegtl::at< length_i >, length_i >,
                pegtl::seq< pegtl::at< call_i >, call_i >,
                pegtl::seq< pegtl::at< assign_call_i >, assign_call_i >,
                pegtl::seq< pegtl::at< new_array_i >, new_array_i >,
                pegtl::seq< pegtl::at< new_tuple_i >, new_tuple_i >,
                pegtl::seq< pegtl::at< assign_i >, assign_i >
            >,
            seps
        >{};

    struct function_name_rule:
        name {};

    struct function_rule:
        pegtl::seq<
            seps,
            return_type_rule,
            seps,
            function_name_rule,
            seps,
            one< '(' >,
            seps,
            pegtl::opt< parameters_rule >,
            seps,
            one< ')' >,
            seps,
            one< '{' >,
            pegtl::plus< instruction_rule >,
            one< '}' >,
            seps
        >{};
    
    struct entryPointRule:
        pegtl::plus< function_rule >{};

    struct grammar : 
        pegtl::must< entryPointRule >{};

    template< typename Rule >
    struct action : pegtl::nothing< Rule > {};

    void cacheStringItem(LA_Item* i, std::string str){
        i->name = str;
        parsedItems.push_back(i);
    }

    void addToVarMap(Variable* v, Type* t, Program & p ){
        p.functions.back()->type_map.emplace(v->name, t);
    }

    /*
    *  LA_Item Actions
    */

    template<> struct action < variable_rule > {
        template< typename Input >
        static void apply( const Input & in, LA::Program & p){
            // cout << in.string() << endl;
            cacheStringItem(new Variable(), in.string());
        }
    };

    template<> struct action < runtime_callee_rule > {
        template< typename Input >
        static void apply( const Input & in, LA::Program & p){
            // cout << in.string() << endl;
            cacheStringItem(new Runtime_Callee(), in.string());
        }
    };

    template<> struct action < function_callee_rule > {
        template< typename Input >
        static void apply( const Input & in, LA::Program & p){
            // cout << in.string() << endl;
            cacheStringItem(new Label(), ":" + in.string());
        }
    };

    template<> struct action < label_rule > {
        template< typename Input >
        static void apply( const Input & in, LA::Program & p){
            // cout << in.string() << endl;
            cacheStringItem(new Label(), in.string());
        }
    };

    template<> struct action < number_rule > {
        template< typename Input >
        static void apply( const Input & in, LA::Program & p){
            // cout << in.string() << endl;
            auto n = new Number();
            int val = stoi(in.string());
            val = val * 2 + 1; // encode
            n->value = val;
            n->name = std::to_string(val);
            parsedItems.push_back(n);
        }
    };

    template<> struct action < decoded_number_rule > {
        template< typename Input >
        static void apply( const Input & in, LA::Program & p){
            // cout << in.string() << endl;
            auto n = new Number();
            n->value = stoi(in.string());
            n->name = in.string();
            parsedItems.push_back(n);
        }
    };

    template<> struct action < tuple_rule > {
        template< typename Input >
        static void apply( const Input & in, LA::Program & p){
            // cout << in.string() << endl;
            auto t = new Tuple();
            t->name = in.string();
            parsedItems.push_back(t);
        }
    };

    template<> struct action < code_rule > {
        template< typename Input >
        static void apply( const Input & in, LA::Program & p){
            // cout << in.string() << endl;
            auto t = new Code();
            t->name = in.string();
            parsedItems.push_back(t);
        }
    };

    template<> struct action < void_rule > {
        template< typename Input >
        static void apply( const Input & in, LA::Program & p){
            // cout << in.string() << endl;
            auto t = new Void();
            t->name = in.string();
            parsedItems.push_back(t);
        }
    };

    template<> struct action < int64_rule > {
        template< typename Input >
        static void apply( const Input & in, LA::Program & p){
            // cout << in.string() << endl;
            auto t = new Int64();
            t->name = in.string();
            parsedItems.push_back(t);
        }
    };

    template<> struct action < int64_array_rule > {
        template< typename Input >
        static void apply( const Input & in, LA::Program & p){
            // cout << in.string() << endl;
            auto t = new Int64_Array();
            t->name = in.string();
            parsedItems.push_back(t);
        }
    };

    template<> struct action < op_rule > {
        template< typename Input >
        static void apply( const Input & in, LA::Program & p){
            // cout << in.string() << endl;
            cacheStringItem(new Operator(), in.string());
        }
    };

    /*
    *   Instruction Actions
    */

    void instructionAction(Instruction* inst, Program & p){
        inst->args = parsedItems;
        parsedItems.clear();
        p.functions.back()->instructions.push_back(inst);
    }

    template<> struct action < assign_i > {
        template< typename Input >
        static void apply( const Input & in, LA::Program & p){
            // cout << in.string() << endl;
            instructionAction(new Assign_I(), p);
        }
    };

    template<> struct action < label_i > {
        template< typename Input >
        static void apply( const Input & in, LA::Program & p){
            // cout << in.string() << endl;
            instructionAction(new Label_I(), p);
        }
    };

    template<> struct action < branch_i > {
        template< typename Input >
        static void apply( const Input & in, LA::Program & p){
            // cout << in.string() << endl;
            instructionAction(new Branch_I(), p);
        }
    };

    template<> struct action < conditional_branch_i > {
        template< typename Input >
        static void apply( const Input & in, LA::Program & p){
            // cout << in.string() << endl;
            instructionAction(new Conditional_Branch_I(), p);
        }
    };

    template<> struct action < return_i > {
        template< typename Input >
        static void apply( const Input & in, LA::Program & p){
            // cout << in.string() << endl;
            instructionAction(new Return_I(), p);
        }
    };

    template<> struct action < return_value_i > {
        template< typename Input >
        static void apply( const Input & in, LA::Program & p){
            // cout << in.string() << endl;
            instructionAction(new Return_Value_I(), p);
        }
    };

    template<> struct action < type_var_i > {
        template< typename Input >
        static void apply( const Input & in, LA::Program & p){
            // cout << in.string() << endl;
            Variable* var = dynamic_cast<Variable*>(utils::pop_item(parsedItems));
            Type* type = dynamic_cast<Type*>(utils::pop_item(parsedItems));
            auto i = new Type_Var_I();
            i->args.push_back(type);
            i->args.push_back(var);
            p.functions.back()->instructions.push_back(i);
            addToVarMap(var, type, p);
        }
    };

    template<> struct action < assign_op_i > {
        template< typename Input >
        static void apply( const Input & in, LA::Program & p){
            // cout << in.string() << endl;
            instructionAction(new Assign_Op_I(), p);
        }
    };

    template<> struct action < array_load_i > {
        template< typename Input >
        static void apply( const Input & in, LA::Program & p){
            // cout << in.string() << endl;
            auto var = parsedItems[1]->name;
            auto type = p.functions.back()->type_map.at(var);
            if( dynamic_cast<Tuple*>(type) ) instructionAction( new Tuple_Load_I(), p );
            else instructionAction( new Array_Load_I(), p);
        }
    };

    template<> struct action < array_store_i > {
        template< typename Input >
        static void apply( const Input & in, LA::Program & p){
            // cout << in.string() << endl;
            auto var = parsedItems[0]->name;
            auto type = p.functions.back()->type_map.at(var);
            if( dynamic_cast<Tuple*>(type) ) instructionAction( new Tuple_Store_I(), p );
            else instructionAction( new Array_Store_I(), p);
        }
    };

    template<> struct action < length_i > {
        template< typename Input >
        static void apply( const Input & in, LA::Program & p){
            // cout << in.string() << endl;
            instructionAction(new Length_I(), p);
        }
    };

    template<> struct action < call_i > {
        template< typename Input >
        static void apply( const Input & in, LA::Program & p){
            // cout << in.string() << endl;
            instructionAction(new Call_I(), p);
        }
    };

    template<> struct action < assign_call_i > {
        template< typename Input >
        static void apply( const Input & in, LA::Program & p){
            // cout << in.string() << endl;
            instructionAction(new Assign_Call_I(), p);
        }
    };

    template<> struct action < new_tuple_i > {
        template< typename Input >
        static void apply( const Input & in, LA::Program & p){
            // cout << in.string() << endl;
            instructionAction(new New_Tuple_I(), p);
        }
    };

    template<> struct action < new_array_i > {
        template< typename Input >
        static void apply( const Input & in, LA::Program & p){
            // cout << in.string() << endl;
            instructionAction(new New_Array_I(), p);
        }
    };

    /*
    *   Function-Level Actions
    */

    template<> struct action < function_name_rule > {
        template< typename Input >
        static void apply( const Input & in, LA::Program & p){
            // cout << in.string() << endl;
            Label l;
            l.name = in.string();
            Function* f = new Function();
            f->name = l;
            Type* type = dynamic_cast<Type*>(utils::pop_item(parsedItems));
            f->returnType = type; 
            p.functions.push_back(f);
        }
    };

    template<> struct action < parameter_rule > {
        template< typename Input >
        static void apply( const Input & in, LA::Program & p){
            // cout << in.string() << endl;
            Variable* var_cast = dynamic_cast<Variable*>(utils::pop_item(parsedItems));
            Type* type_cast = dynamic_cast<Type*>(utils::pop_item(parsedItems));

            addToVarMap(var_cast, type_cast, p);

            LA::Parameter param ( type_cast, *var_cast );

            p.functions.back()->parameters.push_back(param);
        }
    };

    Program parseFile(char* filename){
        pegtl::analyze< LA::grammar >();
        file_input< > fileInput(filename);
        LA::Program p;
        parse< LA::grammar, LA::action >(fileInput, p);
        return p;
    }
}