#pragma once

#include "Nodable.h"
#include "ComputeBase.h"

namespace Nodable
{
	/**
	  * ComputeFunction is a class able to eval a function (using its prototype and a language)
	  */
	class ComputeFunction : public ComputeBase {
	public:
		ComputeFunction(const Function* _function, const Language* _language);
		~ComputeFunction() {};

		void setArg(size_t _index, Member* _value) { args[_index] = _value; };
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
