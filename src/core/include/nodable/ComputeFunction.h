#pragma once

#include <nodable/Nodable.h>
#include <nodable/ComputeBase.h>

namespace Nodable
{
    // forward declarations
    class Language;
    class Invokable;

	/**
	  * @brief ComputeFunction extends Compute base to provide a Component that represents a Function.
	  *        This function has some arguments.
	  */
	class ComputeFunction : public ComputeBase
    {
	public:
		ComputeFunction(const Invokable* _function);
		~ComputeFunction() = default;

		inline void                        setArg(size_t _index, Member* _value) { m_args[_index] = _value; }
		inline Member*                     getArg(size_t _index)const  { return m_args[_index]; }
		inline const std::vector<Member*>& getArgs()const { return m_args; }
		inline const Invokable*             getFunction()const { return m_function; }
		bool                               update() override;
	protected:
		std::vector<Member*> m_args;
		const Invokable* m_function;

		REFLECT_DERIVED(ComputeFunction)
        REFLECT_EXTENDS(ComputeBase)
        REFLECT_END
	};
}
