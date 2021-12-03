#pragma once

#include <functional>
#include <tuple>
#include <stdarg.h>     /* va_list, va_start, va_arg, va_end */

#include <nodable/Nodable.h>
#include <nodable/TokenType.h>
#include <nodable/Member.h>

namespace Nodable {

	/*
	 * Type of a callable function.
	 * Require an integer as output (works like an error code, 0: OK, 1 >= : error)
	 * An output member and some arguments
     *
	 * TODO: try to replace Member* by Variant*
	 */
	typedef std::function <int (Member*, const std::vector<Member*>&)> FunctionImplem;

	/*
	 * Simple object to store a function argument (token, name)
	 */
	class FunctionArg {
	public:
		FunctionArg(TokenType, std::string);
		TokenType type;
		std::string name;
	};

	/*
	 * Class to store a function signature.
	 * We can check if two function signature are matching using this->match(other)
	 */
	class FunctionSignature {
	public:
		FunctionSignature(std::string _identifier, TokenType _type, std::string _label = "");
		~FunctionSignature() {};
		void                           pushArg(TokenType _type, std::string _name = "");

		template <typename... TokenType>
		void pushArgs(TokenType&&... args) {
			int dummy[] = { 0, ((void)pushArg(std::forward<TokenType>(args)),0)... };
		}

        bool                           hasAtLeastOneArgOfType(TokenType type)const;
		bool                           match(const FunctionSignature* _other)const;
		const std::string&             getIdentifier()const;
		std::vector<FunctionArg>       getArgs() const;
		size_t                         getArgCount() const { return args.size(); }
		TokenType                      getType() const;
		std::string                    getLabel() const;

	private:
		std::string label;
		std::string identifier;
		std::vector<FunctionArg> args;
		TokenType type;

	public:
		template<typename R = TokenType, typename... TokenType>
		static FunctionSignature Create(R _type, std::string _identifier, TokenType&& ..._args) {
			FunctionSignature signature(_identifier, _type);
			signature.pushArgs(_args...);
			return signature;
		}
    };


	/*
	 * WIP work to facilitate native function wrapping inside Nodable
	 */

    /** Push Arg helpers */

    template<class Tuple, std::size_t N> // push N+1 arguments
    struct arg_pusher
    {
        static void push_into(FunctionSignature *_signature)
        {
            arg_pusher<Tuple, N - 1>::push_into(_signature);

            using t = std::tuple_element_t<N-1, Tuple>;
            TokenType tokenType = ToTokenType<t>::type;
            _signature->pushArg( tokenType );
        }
    };

    template<class Tuple>  // push 1 arguments
    struct arg_pusher<Tuple, 1>
    {
        static void push_into(FunctionSignature *_signature)
        {
            using t = std::tuple_element_t<0, Tuple>;
            TokenType tokenType = ToTokenType<t>::type;
            _signature->pushArg( tokenType );
        };
    };

    // create and argument_pusher and push arguments into signature
    template<typename... Args, std::enable_if_t<std::tuple_size_v<Args...> != 0, int> = 0>
    void push_args(FunctionSignature* _signature)
    {
        arg_pusher<Args..., std::tuple_size_v<Args...>>::push_into(_signature);
    }

    // empty function when pushing an empty arguments
    template<typename... Args, std::enable_if_t<std::tuple_size_v<Args...> == 0, int> = 0>
    void push_args(FunctionSignature* _signature){}

    /** Helpers to call a function (need serious work here) */

    /** 0 arg function */
    template<typename R, typename F = R()>
    void call(F *_function, Member *_result, const std::vector<Member *> &_args)
    {
        _result->set( _function() );
    }

    /** 1 arg function */
    template<typename R, typename A0, typename F = R(A0)>
    void call(F *_function, Member *_result, const std::vector<Member *> &_args)
    {
        _result->set( _function( (A0) *_args[0] ) );
    }

    /** 2 arg function */
    template<typename R, typename A0, typename A1, typename F = R(A0, A1)>
    void call(F *_function, Member *_result, const std::vector<Member *> &_args)
    {
        _result->set( _function( (A0) *_args[0], (A1) *_args[1] ) );
    }

    /** 3 arg function */
    template<typename R, typename A0, typename A1, typename A2, typename F = R(A0, A1, A2)>
    void call(F *_function, Member *_result, const std::vector<Member *> &_args)
    {
        _result->set( _function( (A0) *_args[0], (A1) *_args[1], (A2) *_args[2] ) );
    }

    /** 4 arg function */
    template<typename R, typename A0, typename A1, typename A2, typename A3, typename F = R(A0, A1, A2, A3)>
    void call(F *_function, Member *_result, const std::vector<Member *> &_args)
    {
        _result->set( _function( (A0) *_args[0], (A1) *_args[1], (A2) *_args[2], (A3) *_args[3] ) );
    }

    /** 5 arg function */
    template<typename R, typename A0, typename A1, typename A2, typename A3, typename A4, typename F = R(A0, A1, A2, A3, A4)>
    void call(F *_function, Member *_result, const std::vector<Member *> &_args)
    {
        _result->set( _function( (A0) *_args[0], (A1) *_args[1], (A2) *_args[2], (A3) *_args[3], (A4) *_args[4] ) );
    }



    /**
     * Interface to wrap any invokable function/operator
     */
    class Invokable
    {
    public:
        virtual const FunctionSignature* getSignature() const = 0;
        virtual void invoke(Member *_result, const std::vector<Member *> &_args) const = 0;
    };


    template<typename T>
    class InvokableFunction;

    /** Generic Invokable Function */
    template<typename R, typename... Args>
    class InvokableFunction<R(Args...)> : public Invokable
    {
    public:
        using   FunctionType = R(Args...);
        using   Tuple        = std::tuple<Args...>;

        InvokableFunction(FunctionType* _function, const char* _identifier)
        {
            m_function  = _function;
            m_signature = new FunctionSignature(_identifier, ToTokenType<R>::type , _identifier);
            push_args<Tuple>(m_signature);
        }

        inline void invoke(Member *_result, const std::vector<Member *> &_args) const
        {
            call<R, Args...>(m_function, _result, _args);
        }

        inline const FunctionSignature* getSignature() const { return m_signature; };

    private:
        FunctionType*      m_function;
        FunctionSignature* m_signature;
    };

}