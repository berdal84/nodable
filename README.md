![Nodable Logo](https://github.com/berdal84/Nodable/blob/master/data/icon.png)

Nodable is node-able !
======================

Introduction:
-------------

This software is **a node-able bidirectionnal expression editor**. More precisely, it means **Text-to-Node and Node-to-Text seamless edition**.

![Example1](https://github.com/berdal84/Nodable/blob/master/screenshots/2019_06_02_Nodable_demo.gif)

Video history :
==========
Few videos hosted on Youtube :
- [Video n°1 - A first step](http://www.youtube.com/watch?v=1TWPsUd66XY)
- [Video n°2 - Automatic Layout)](http://www.youtube.com/watch?v=-4N3Krlsr_s)
- [Video n°3 - First Talk about Nodable : Past, present and future](http://www.youtube.com/watch?v=_9_wzS7Hme8)

The language :
==============

Introduction:
-------------

For now the syntax used is really simple and works quite like a calculator.

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
Windows x86

How to compile ? :
==================

Open the existing Visual Studio Solution located in build folder. Then check VS help to know how to compile.

How to run the software ? :
===========================

To run the lastest debug version, open the build/Debug folder and run Nodable.exe.

Architecture :
==============

The software is built to be dynamically reflective. The base class **Object** has members called **Members**s (OMG!). Members can be added or removed at runtime.
A **Member** uses a **Variant** class that can wrap all basic data types such as Booleans (bool), Numbers (double) or Strings (std::string).

Two **Members** can be linked by a **Wire**. A **Wire** is an oriented edge, so it has a *source* and a *target* Member.

The derived class **Entity** is an *Object* able to attach **Components** on it. **Component** is the abstract base class for all components. For now there are only few components :
- **View**s : to draw the entity on screen.
- **Operation**s : to perform a computation.
- **Container** : to instantiate and contain other entities (Factory and Container).
- **DataAccessObject** : auto-save to JSON when the entity is modified.

**Variable** class is an **Entity** with a single member named "value".

**Lexer** class is an **Entity** able to convert an expression to a graph.

**Application** is a class to rule them all.

![Draft UML Class Diagram](https://github.com/berdal84/Nodable/blob/master/docs/ClassDiagram_2019_05_11.png)

Dependencies / Credits :
==============

- SDL2 : https://www.libsdl.org/
- GLFW3 : http://www.glfw.org/
- *Dear ImGui* developed by Omar Cornut: https://github.com/omarcornut/imgui
- IconFontCppHeaders by Juliette Faucaut: https://github.com/juliettef/IconFontCppHeaders
- ImGuiColorTextEdit by BalazsJako : https://github.com/BalazsJako/ImGuiColorTextEdit

Licence:
=========
**Nodable** is licensed under the GPL License, see LICENSE for more information.

Each submodule are licensed, browse */extern* folder.
