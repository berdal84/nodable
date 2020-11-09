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
		ComputeFunction(std::shared_ptr<const Function> _function, std::shared_ptr<const Language> _language);
		~ComputeFunction() {};
		void setFunction(std::shared_ptr<const Function> _function);
		void setArg(size_t _index, std::shared_ptr<Member> _value);
		bool update()override;
		void updateResultSourceExpression() const override;

	protected:
		std::vector<std::shared_ptr<Member>> args;
        std::shared_ptr<const Function> function;

		MIRROR_CLASS(ComputeFunction)(
			MIRROR_PARENT(ComputeBase)
			);
	};
}
