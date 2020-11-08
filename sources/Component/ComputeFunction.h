#pragma once
#include "ComputeBase.h"

namespace Nodable
{
    class Function;

	/**
	  * ComputeFunction is a class able to eval a function (using its prototype and a language)
	  */
	class ComputeFunction : public ComputeBase
    {
	public:
	    ComputeFunction(): ComputeBase(){};
		ComputeFunction(const Function* _function, const Language* _language);
		~ComputeFunction() {};
		void setFunction(const Function* _function);
		void setArg(size_t _index, Member* _value);
		bool update()override;
		void updateResultSourceExpression() const override;

	protected:
		std::vector<Member*> args;
		const Function* function;

		MIRROR_CLASS(ComputeFunction)(
			MIRROR_PARENT(ComputeBase)
			);
	};
}
