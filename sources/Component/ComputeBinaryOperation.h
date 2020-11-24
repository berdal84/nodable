#pragma once

#include "Nodable.h"   // for constants and forward declarations
#include "mirror.h"
#include "ComputeFunction.h"
#include <functional>

namespace Nodable{

    class Language;
    class Operator;

	/* BinaryOperationComponent is an interface for all binary operations
	*
	* Note for later: This class and all derivate should be destroyed and replaced by an "OperatorComponent"
	*                 using prototypes but with different serialization and parsing methods.
	*/
	class ComputeBinaryOperation: public ComputeFunction {
	public:		
		ComputeBinaryOperation(const Operator*, const Language*);
		~ComputeBinaryOperation(){};
		void                setLValue(Member* _value);
		void                setRValue(Member* _value);		
		void                updateResultSourceExpression() const override;
		const Operator* ope;
	protected:		
		MIRROR_CLASS(ComputeBinaryOperation)(
			MIRROR_PARENT(ComputeFunction)
		);
	};
}
