#pragma once

#include "Nodable.h"   // for constants and forward declarations
#include "mirror.h"
#include "ComputeFunction.h"
#include "Function.h"
#include "Operator.h"
#include <functional>

namespace Nodable
{
    // forward declarations
    class Language;
    class Operator;

	/**
	 * @brief Extends ComputeFunction to get L&R values (Member) and Operator.
	*/
	class ComputeBinaryOperation: public ComputeFunction
    {
	public:		
		ComputeBinaryOperation( const Operator* _operator): ComputeFunction( reinterpret_cast<const Function*>(_operator)) {}
        ~ComputeBinaryOperation() = default;
        inline void            setLValue(Member* _value) { m_args[0] = _value; }
        inline Member*         getLValue() { return m_args[0]; };
		inline void            setRValue( Member* _value )  { m_args[1] = _value; }
        inline Member*         getRValue() { return m_args[1]; };
        inline const Operator* getOperator() const { return reinterpret_cast<const Operator*>(m_function); };

        // reflect class using mirror
		MIRROR_CLASS(ComputeBinaryOperation)(
			MIRROR_PARENT(ComputeUnaryOperation)
		);
	};
}
