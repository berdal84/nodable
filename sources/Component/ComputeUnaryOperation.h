#pragma once

#include "Nodable.h"   // for constants and forward declarations
#include "mirror.h"
#include "ComputeFunction.h"
#include <functional>

namespace Nodable {

    class Operator;
    class Language;

	/* BinaryOperationComponent is an interface for all binary operations
	*
	* Note for later: This class and all derivate should be destroyed and replaced by an "OperatorComponent"
	*                 using prototypes but with different serialization and parsing methods.
	*/
	class ComputeUnaryOperation : public ComputeFunction {
	public:
		ComputeUnaryOperation(const Operator*, const Language*);
		~ComputeUnaryOperation() {};
		void                setLValue(Member* _value);
		void                updateResultSourceExpression() const override;
		const Operator* ope;

	protected:
		MIRROR_CLASS(ComputeUnaryOperation)(
			MIRROR_PARENT(ComputeFunction)
			);
	};
}
