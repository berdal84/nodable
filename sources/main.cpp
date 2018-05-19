#include <stdio.h>
#include <string>
#include "Node_Application.h"
#include "ApplicationView.h"
#include "Value.h"
#include "Log.h"

bool runValueUnitTests()
{
	bool success = true;


	// Test 01 - Setting and getting a boolean
	{
		auto v = new Nodable::Value();
		v->setValue(true);

		if (v->getValueAsBoolean()){
			LOG_MSG("Test n°1a : Success\n");
			success &= true;
		}else{
			LOG_MSG("Test n°1a : FAILED !\n");
			success &= false;
		}
		
		if (v->getType() == Nodable::Type_Boolean){
			LOG_MSG("Test n°1b : Success\n");
			success &= true;
		}else{
			LOG_MSG("Test n°1b : FAILED !\n");
			success &= false;
		}

		v->setValue(false);
		if (!v->getValueAsBoolean()){
			LOG_MSG("Test n°1c : Success\n");
			success &= true;
		}else{
			LOG_MSG("Test n°1c : FAILED !\n");
			success &= false;
		}

	}

	// Test 02 - Setting and getting a string
	{
		auto v = new Nodable::Value();
		v->setValue("Hello world !");

		if (v->getValueAsString() == "Hello world !"){
			LOG_MSG("Test n°2a : Success\n");
			success &= true;
		}else{
			LOG_MSG("Test n°2a : FAILED !\n");
			success &= false;
		}

		if (v->getType() == Nodable::Type_String){
			LOG_MSG("Test n°2b : Success\n");
			success &= true;
		}else{
			LOG_MSG("Test n°2b : FAILED !\n");
			success &= false;
		}
	}
	// Test 03 - Setting and getting a number
	{
		auto v = new Nodable::Value();
		v->setValue(50.0F);

		if (v->getValueAsNumber() == 50.0F){
			LOG_MSG("Test n°3a : Success\n");
			success &= true;
		}else{
			LOG_MSG("Test n°3a : FAILED !\n");
			success &= false;
		}

		if (v->getType() == Nodable::Type_Number){
			LOG_MSG("Test n°3b : Success\n");
			success &= true;
		}else{
			LOG_MSG("Test n°3b : FAILED !\n");
			success &= false;
		}
	}

	return success;
}

int main(int, char**)
{

	// Unit tests
	if (!runValueUnitTests())
	{
		LOG_MSG("Unit test failed !\n");
		return -1;
	}

	// run the program
	Nodable::Node_Application nodable("Nodable");

	if(!nodable.init())
		return -1;

	while (nodable.update())
	{
		if( nodable.hasComponent("view"))
			((Nodable::ApplicationView*)nodable.getComponent("view"))->draw();
	}

	nodable.shutdown();

	return 0;
}
