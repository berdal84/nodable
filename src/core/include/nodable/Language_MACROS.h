/*
	Here, some Macros to easily create function and add them to the Language.api
*/

#define WRAP_POLYFUNCTION( function, signature ) \
    { \
        std::string function_name = #function; \
        sanitizeFunctionName( function_name ); \
        using function_type = signature; \
        Invokable* invokable = new InvokableFunction<function_type>(function, function_name.c_str()); \
        addToAPI( invokable ); \
    }

#define WRAP_FUNCTION( function ) \
    { \
        std::string function_name = #function; \
        sanitizeFunctionName( function_name ); \
        Invokable* invokable = new InvokableFunction<decltype(function)>(function, function_name.c_str()); \
        addToAPI( invokable ); \
    }

#define WRAP_OPERATOR( Function, short_identifier, precedence, label ) \
    { \
        Invokable* invokable = new InvokableFunction<decltype(Function)>(Function, "operator" short_identifier ); \
        Operator* op = new Operator( invokable, precedence, short_identifier );\
        addOperator( op );\
        addToAPI( invokable ); \
    }
