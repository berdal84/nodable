#pragma once

#include <functional>
#include <mirror.h>

#include <nodable/Nodable.h> // for constants and forward declarations
#include <nodable/ComputeFunction.h>
#include <nodable/Function.h>
#include <nodable/Operator.h>

namespace Nodable
{
	/**
	 * @brief Extends ComputeFunction to work with UnaryOperations expose a L value and an operator.
	*/
	class ComputeUnaryOperation : public ComputeFunction
    {
	public:
		ComputeUnaryOperation( const Operator* _operator) : ComputeFunction( reinterpret_cast<const Function*>(_operator)) {}
		~ComputeUnaryOperation() = default;
		void            setLValue(Member* _value) { m_args[0] = _value; }
        Member*         getLValue() { return m_args[0]; };
        const Operator* getOperator() const { return reinterpret_cast<const Operator*>(m_function); };

		MIRROR_CLASS(ComputeUnaryOperation)(
			MIRROR_PARENT(ComputeFunction)
		);
    };
}
