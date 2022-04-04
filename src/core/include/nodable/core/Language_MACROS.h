/*
	Macros to easily bind cpp functions to a Nodable::Language.
    These MACROS must be used within Nodable::Language non-static scope
*/

/**
* Wrap a native function with a specific signature.
* If your function is not polymorphic you can also use BIND_FUNCTION
*
* ex: Same functions with two different signatures:
*
*  BIND_FUNCTION_T( sin, double(double) )
*  BIND_FUNCTION_T( sin, double(float) )
*/
#define BIND_FUNCTION_T( function, function_type ) \
    { \
        std::string identifier = #function; \
        sanitize_function_identifier( identifier );\
        const IInvokable* invokable_fct = new InvokableFunction<function_type>(function, identifier.c_str()); \
        add( invokable_fct );\
    }

/**
* Wrap a native function with a specific signature as an operator.
* If your function is not polymorphic you can also use BIND_OPERATOR
*
* ex: Same function name with different label and signatures:
 *
*  BIND_OPERATOR_T( add, "+", double(double, double) )
*  BIND_OPERATOR_T( add, "+", std::string(std::string, double) )
*/
#define BIND_OPERATOR_T( function, identifier, function_type ) \
    { \
        std::string function_identifier = identifier; \
        sanitize_operator_fct_identifier( function_identifier );\
        const IInvokable* invokable = new InvokableFunction<function_type>(function, function_identifier.c_str(), identifier ); \
        const Operator* op = find_operator(identifier, invokable->get_signature());\
        const InvokableOperator* invokable_op = new InvokableOperator( op, invokable );\
        add( invokable_op );\
    }

/**
* Wrap a native non-polymorphic function.
*
* ex: BIND_FUNCTION( my_unique_name_function )
*/
#define BIND_FUNCTION( function ) BIND_FUNCTION_T( function, decltype(function))

/**
* Wrap a native non-polymorphic function.
*
* ex: BIND_OPERATOR( my_unique_name_function, "+")
*/
#define BIND_OPERATOR( function, identifier ) BIND_OPERATOR_T( function, identifier, decltype(function) )