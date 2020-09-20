#pragma once
#include "Nodable.h"
#include "Component.h"

namespace Nodable {
	
	class Compound {

	public:
		Compound(){}
		~Compound(){}

		/* Add a component to this Node
		   note: User must check be sure this Node has no other Component of the same type (cf. hasComponent(...))*/
		template<typename T>
		void addComponent(T* _component)
		{
			static_assert(std::is_base_of<Component, T>::value, "T must inherit from Component");
			std::string name(T::GetClass()->getName());
			components.emplace(std::make_pair(name, _component));
			_component->setOwner(reinterpret_cast<Node*>(this) );
		}

		/* Return true if this node has the component specified by it's type T.
		   note: T must be a derived class of Component
		 */
		template<typename T>
		bool hasComponent()const
		{
			static_assert(std::is_base_of<Component, T>::value, "T must inherit from Component");
			return getComponent<T>() != nullptr;
		}

		/* Get all components of this Node */
		const Components& getComponents()const;

		/* Delete a component of this node by specifying it's type T.
		   note: T must be Component derived. */
		template<typename T>
		void deleteComponent() {
			static_assert(std::is_base_of<Component, T>::value, "T must inherit from Component");
			auto name = T::GetClass()->getName();
			auto component = getComponent<T>();
			components.erase(name);
			delete component;
		}

		/* Get a component of this Node by specifying it's type T.
		   note: T must be Component derived.*/
		template<typename T>
		T* getComponent()const {
			static_assert(std::is_base_of<Component, T>::value, "T must inherit from Component");
			auto c    = T::GetClass();
			auto name = c->getName();

			// Search with class name
			{
				auto it = components.find(name);
				if (it != components.end()) {
					return reinterpret_cast<T*>(it->second);
				}
			}

			// Search for a derived class
			for (auto it = components.begin(); it != components.end(); it++) {
				Component* component = it->second;
				if (component->getClass()->isChildOf(c, false)) {
					return reinterpret_cast<T*>(component);
				}
			}

			return nullptr;
		};

	protected:
		Components components;
	};
}