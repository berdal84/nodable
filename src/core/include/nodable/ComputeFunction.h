#pragma once

#include <nodable/Nodable.h>
#include <nodable/ComputeBase.h>

namespace Nodable::core
{
    // forward declarations
    class Function;
    class Language;

	/**
	  * @brief ComputeFunction extends Compute base to provide a Component that represents a Function.
	  *        This function has some arguments.
	  */
	class ComputeFunction : public ComputeBase
    {
	public:
		ComputeFunction(const Function* _function);
		~ComputeFunction() = default;

		inline void                        setArg(size_t _index, Member* _value) { m_args[_index] = _value; }
		inline Member*                     getArg(size_t _index)const  { return m_args[_index]; }
		inline const std::vector<Member*>& getArgs()const { return m_args; }
		inline const Function*             getFunction()const { return m_function; }
		bool                               update() override;
	protected:
		std::vector<Member*> m_args;
		const Function*      m_function;

		// reflect class using mirror
		MIRROR_CLASS(ComputeFunction)(
			MIRROR_PARENT(ComputeBase)
		);
	};
}
