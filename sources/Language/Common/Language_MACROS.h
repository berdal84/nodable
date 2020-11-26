/*
	Here, some Macros to easily create function and add them to the Language.api

	TODO: I don't like these big macros. Use function traits instead.
*/
#define RETURN_SUCCESS return 0;

#define RETURN_FAILED \
_result->setType(Type::Any); /* We intentionnaly force result type any to avoid crashing.*/ \
return 1;

#define ARG(n) (*_args[n])
#define BEGIN_IMPL\
	auto implementation = [](Member* _result, const std::vector<Member*>& _args)->int { \
	for(auto it = _args.begin(); it != _args.end(); it++) \
	{\
		if( (*it)->isType(Type::Any) ) \
		{\
			LOG_WARNING( Log::Verbosity::Verbose, "Argument %i (%s) is unknown.\n", std::distance(_args.begin(), it), (*it)->getName().c_str() );\
		}\
	}

#define RETURN( expr )\
	_result->set( expr );


#define END_IMPL RETURN_SUCCESS\
	};

#define BINARY_OP_BEGIN( _type, _identifier, _ltype, _rtype, _precedence, _label )\
{\
	auto precedence = _precedence; \
	auto identifier = std::string(_identifier); \
	FunctionSignature signature( std::string("operator") + _identifier, _type, _label ); \
	signature.pushArgs(_ltype, _rtype); \
	BEGIN_IMPL

#define UNARY_OP_BEGIN( _type, _identifier, _ltype, _precedence, _label )\
{\
	auto precedence = _precedence; \
	auto identifier = std::string(_identifier); \
	FunctionSignature signature( std::string("operator") + _identifier, _type, _label ); \
	signature.pushArgs(_ltype); \
	BEGIN_IMPL

#define FCT_BEGIN( _type, _identifier, ... ) \
{ \
	auto signature = FunctionSignature::Create( _type, _identifier, __VA_ARGS__); \
	BEGIN_IMPL

#define FCT_END \
	END_IMPL \
	addToAPI( signature, implementation );\
}

#define OPERATOR_END \
	END_IMPL \
	addOperator(identifier, precedence, signature, implementation);\
	addToAPI( signature , implementation );\
}