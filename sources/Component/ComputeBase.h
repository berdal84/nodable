#pragma once
#include <utility>

#include "Component.h"

namespace Nodable
{
    class Language;

	class ComputeBase : public Component {
	public:
	    ComputeBase():Component(){}
		ComputeBase(std::shared_ptr<const Language> _language): language(std::move(_language)) {};
		virtual ~ComputeBase() {};
		virtual void updateResultSourceExpression() const = 0;
		void setResult(std::shared_ptr<Member> _value) { result = std::move(_value); };
		void setLanguage(std::shared_ptr<const Language> _language){ language = std::move(_language); };
	protected:
        std::shared_ptr<const Language> language;
        std::shared_ptr<Member> result;
		MIRROR_CLASS(ComputeBase)(
			MIRROR_PARENT(Component)
			);
	};
}