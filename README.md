![Nodable Logo](https://github.com/berdal84/Nodable/blob/master/data/icon.png)

[![Build Status](https://travis-ci.org/berdal84/Nodable.svg?branch=master)](https://travis-ci.org/berdal84/Nodable)

Nodable is node-able !
======================

Introduction:
-------------

This software is a command line prompt able to evaluate literal expressions by constructing an execution graph in realtime.
When user type an expression the program split it into tokens and build the execution tree. Then, each frame each node is updated only if needed. User can interact with the graph by changing values.

Examples :
==========

An expression example :

![Example1](https://github.com/berdal84/Nodable/blob/master/screenshots/2018_06_09_SimpleExpression_And_NewIcons.png)

Video n°1 - a first demo :

[![Watch on Youtube](https://img.youtube.com/vi/1TWPsUd66XY/0.jpg)](http://www.youtube.com/watch?v=1TWPsUd66XY)

Video n°2 - Automatic Layout :

[![Watch on Youtube](https://img.youtube.com/vi/-4N3Krlsr_s/0.jpg)](http://www.youtube.com/watch?v=-4N3Krlsr_s)

Vidéo n°3 - First Talk about Nodable : Past, present and future :
[![Watch on Youtube](https://img.youtube.com/vi/_9_wzS7Hme8/0.jpg)](http://www.youtube.com/watch?v=_9_wzS7Hme8)



The language :
==============

Introduction:
-------------

The syntax used is really simple and works quite like a calculator.

Operands :
----------

An operand could be :

- a Boolean (ex: true, false)
- a Number  (ex: 1, 0.5, 100.456, etc.)
- a String  (ex: "Hello", "World", etc.)
- a Symbol  (ex: a, b, myVar, etc.)

Operand -> { Boolean , Number , String , Symbol }

Operators:
----------

An operator could be only a binary operator :

- an Addition
- a Substraction
- a Multiplication
- a Division
- an Assignment

Binary Operator -> { + , - , * , / , = }

All these binary operators supports precedence.

Expressions:
------------

An expression could be :

- a single Operand
- a tuple formed by an Operand, a Binary Operator and an other Expression.

Expression -> { Operand , ( Operand , Binary Operator , Expression ) }

Unary operands are not allowed, so you can't evaluate expressions like

Expression -> { ( Unary Operator, Expression ) }  (ex ``` -10```)

Platform compatibility :
========================
Should work on all platforms but only tested under GNU/Linux Ubuntu 16.04 LTS (64bits)

How to compile ? :
==================

To compile you need a c++ 11 compatible compiler. The make file provided is configured to use g++.

Install dependencies:

```
sudo apt-get install -y --no-install-recommends libsdl2-dev gcc-4.8 g++-4.8 libglfw3-dev
```
Clone the project with submodules:
```
git clone --recurse-submodules https://github.com/berdal84/Nodable
```
Move to the new folder:
```
cd ./Nodable
```

Compile:
```
make
```

How to run the software ? :
===========================

Install dependencies:

```
sudo apt-get install -y --no-install-recommends libsdl2-dev gcc-4.8 g++-4.8 libglfw3-dev
```
Clone the project with submodules:
```
git clone --recurse-submodules https://github.com/berdal84/Nodable
```
Move to the bin folder:
```
cd <nodable folder>/bin/Linux64
```
Run the software:
```
./nodable
```

Architecture :
==============

The software is built to be dynamically reflective. The base class **Object** has members called **Value**s. Members can be added or removed at runtime.
A **Value** is a Variant class that can handle basic data types such as Booleans (bool), Numbers (double) or Strings (std::string).

Two **Values** can be linked by a **Wire**. A **Wire** is an oriented edge, so it has a *source* and a *target* Value.

The derived class **Entity** is an *Object* able to attach **Components** on it. **Component** is the abstract base class for all components. For now there are ony three components :
- **View**s : to draw the entity on screen.
- **Operation**s : to perform a computation.
- **Container** : to contain other entities.
- **DataAccessObject** : auto-save to JSON when the entity is modified.

**Variable** class is an **Entity** with a single member named "value".

**Lexer** class is an **Entity** able to convert an expression to a graph.

**Application** is a class to rule them all. This class has by default a Container component (like global scope) and an ApplicationView (using GLFw3/ImGui/SDL2/OpenGL).

![Draft UML Class Diagram](https://github.com/berdal84/Nodable/blob/master/docs/ClassDiagram_2018_05_25.png)

Road Map :
==========
- Update the expression when the user modify the graph.
- Be able to manually create a node.
- Be able to manually create a wire between two compatible Values.

Dependencies :
==============

- *Dear ImGui* developed by OmarCornut (@ocornut).
- SDL2
- OpenGL 3.x
- GLFW 3

Licence:
=========
**Nodable** is licensed under the GPL License, see LICENSE for more information.

submodules: **Dear ImGui** is licensed under the MIT License.
