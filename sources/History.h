#pragma once
#include "Component.h" // base class
#include "Nodable.h"
#include "Container.h"
#include "Wire.h"
#include "WireView.h"
#include "Member.h"
#include <vector>

namespace Nodable
{
	class Cmd;

	class History : public Component {
	public:
		COMPONENT_CONSTRUCTOR(History);
		~History();

		/* Execute a command and add it to the history.
		If there are other commands after they will be erased from the history */
		void addAndExecute(Cmd*);

		/* Undo the current (in the history) command  */
		void undo();

		/* Redo the next (in the history) command */
		void redo();
		
		// Future: For command groups (ex: 5 commands that are atomic)
		// static BeginGroup();
		// static EndGroup()

		static History*     global;
	private:
		Container*          container;
		std::vector<Cmd*>	commands;		/* Command history */
		size_t           	commandsCursor = 0;	/* Command history cursor (zero based index) */
	};

	/* Base class for all commands */
	class Cmd
	{
	public:
		Cmd(){};
		virtual ~Cmd(){};
		/* Call this to execute the command instance */
		virtual void execute()=0;

		/* Call this to undo the execution of the command instance */
		virtual void undo()=0;
		
	private:
		bool done = false;	/* if set to true after do() has been called */		
	};

	/* Command to add a wire between two Members */
	class Cmd_ConnectWire : public Cmd
	{
	public:
		Cmd_ConnectWire(Wire* _wire, Member* _source, Member* _target)
		{
			this->wire		 = _wire;
			this->source     = _source;
			this->target     = _target;
		};

		~Cmd_ConnectWire(){};

		void execute()
		{
			// Link Members
			wire->setSource(source);
			wire->setTarget(target);

			// Add the wire pointer to the Entity instance to speed up drawing process.
			target->getOwner()->getAs<Entity*>()->addWire(wire);
		}

		void undo()
		{
			// Link Members
			wire->setSource(nullptr);
			wire->setTarget(nullptr);

			// Add the wire pointer to the Entity instance to speed up drawing process.
			target->getOwner()->getAs<Entity*>()->removeWire(wire);
		}

	private:
		Wire*      wire          = nullptr;
		Member*    source        = nullptr;
		Member*    target        = nullptr;
	};

}