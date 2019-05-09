#pragma once
#include "Nodable.h"
#include "Container.h"
#include "Wire.h"
#include "WireView.h"
#include <vector>

namespace Nodable
{
	class Cmd;

	class History
	{
	public:
		History(Container* _container){};
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

	private:
		Container*          container;
		std::vector<Cmd*>	history;		/* Command history */
		size_t           	historyCursor;	/* Command history cursor (zero based index) */
	};

	class Cmd
	{
	public:
		Cmd(){};
		virtual ~Cmd()=0;
	private:
		/* Call this to execute the command instance */
		virtual void execute()=0;

		/* Call this to undo the execution of the command instance */
		virtual void undo()=0;
		
		bool done = false;	/* if set to true after do() has been called */		
	};

	class Cmd_AddWire : public Cmd
	{
	public:
		Cmd_AddWire(Container* _container, Value* _source, Value* _target)
		{
			this->container  = _container;
			this->source     = _source;
			this->target     = _target;
		};

		~Cmd_AddWire(){};

		void execute()
		{
			wire = new Wire;
			wire->addComponent("view", new WireView);	
			wire->setSource(source);
			wire->setTarget(target);
			container->addEntity(wire);
		}

		void undo()
		{

		}

	private:
		Wire*      wire          = nullptr;
		Value*     source        = nullptr;
		Value*     target        = nullptr;
		Container* container     = nullptr;
	};

}