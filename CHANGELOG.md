
Nodable Change Log :
====================

Author: Bérenger Dalle-Cort, 2020-2017


0.6:
	- Result node constraint to stay on visible rect after updating.
	- Build using CMake.
	- Parser: functions rewrote, parenthesis, detailed logs.
	- Language class.	

0.5:
	- fix(NodeView): Variable node drawing crash.
	- First draft version of an UNDO/REDO system.

0.4:
	- A brand new logo !
	- Nodable is now able to open existing files and to save them.
	- The UI has a tab system to switch between multiple loaded files.
	- Bug fixes

0.3:
	- New Node_Assign : '=' can be used to assign a value to a symbol (ex: a = 10)
	- Now Able to perform binary operations on symbols (ex: c = a + b).
	- Node_Context : is now used as a factory.
	- Node : each node can get its contexts with Entity::getParent()
	- Added a change log.
	- Added version number into the header file (NODABLE_VERSION_MAJOR, NODABLE_VERSION_MINOR, NODABLE_VERSION)

0.2:
	- New Binary Operations : Node_Substract, Node_Multiply, Node_Divide
	- Lexer : nos supports operator precedence.

0.1:
	- Node_Add : to add two Node_Numbers
	- Lexer : first version able to evaluate additions.