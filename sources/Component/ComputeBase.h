#pragma once

#include "Nodable.h"
#include "Component.h"

namespace Nodable
{
	class ComputeBase : public Component {
	public:
	    ComputeBase():Component(){}
		ComputeBase(const Language* _language) :language(_language) {};
		virtual ~ComputeBase() {};
		virtual void updateResultSourceExpression() const = 0;
		void         setResult(Member* _value) { result = _value; };
		void setLanguage(const Language* _language){ language = _language; };
	protected:
		const Language* language;
		Member* result = nullptr;
		MIRROR_CLASS(ComputeBase)(
			MIRROR_PARENT(Component)
			);
	};
}