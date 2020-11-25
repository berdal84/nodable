#pragma once

#include "Nodable.h"
#include "ComputeBase.h"

namespace Nodable
{
    class Function;
    class Language;

	/**
	  * ComputeFunction is a class able to eval a function (using its prototype and a language)
	  */
	class ComputeFunction : public ComputeBase {
	public:
		ComputeFunction(const Function* _function, const Language* _language);
		~ComputeFunction() {};

		void setArg(size_t _index, Member* _value) { args[_index] = _value; }
		Member* getArg(size_t _index)const  { return args[_index]; }
		const std::vector<Member*>& getArgs()const { return args; }
		const Function* getFunction()const { return function; }
		bool update() override;
		void updateResultSourceExpression() const override;
	protected:
		std::vector<Member*> args;
		const Function* function;

		MIRROR_CLASS(ComputeFunction)(
			MIRROR_PARENT(ComputeBase)
			);
	};
}
