
// include once this file
#include "Test.h"
#include "Value.h"
#include "Log.h"
#include "Node.h"
#include "Wire.h"

using namespace Nodable;

int  Test::s_testCount        = 0;
int  Test::s_testSucceedCount = 0;

void Test::ResetCounters()
{
	LOG_MSG("---------------------------------------------------------------\n");
	LOG_MSG("   Testing software before running it\n");
	LOG_MSG("---------------------------------------------------------------\n");

	s_testCount        = 0;
	s_testSucceedCount = 0;
}

void Test::DisplayResults()
{
	LOG_MSG("---------------------------------------------------------------\n");
	if(s_testSucceedCount != s_testCount)
		LOG_MSG("   Some tests failed. Passed : %d / %d\n", s_testSucceedCount, s_testCount);
	else
		LOG_MSG("   All tests are OK : %d / %d\n", s_testSucceedCount, s_testCount);

	LOG_MSG("---------------------------------------------------------------\n");
}

bool Test::RunAll()
{
	ResetCounters();

	LOG_MSG("Running Test for Value class... \n");

	// Test 01 - Set/Get a boolean
	//----------------------------
	{
		auto v = new Nodable::Value();
		v->setValue(true);		

		if (v->getValueAsBoolean()){
			s_testSucceedCount++;
		}else{
			LOG_MSG("Test n°1a : FAILED !\n");
		}
		s_testCount++;
		
		if (v->getType() == Nodable::Type_Boolean){
			s_testSucceedCount++;
		}else{
			LOG_MSG("Test n°1b : FAILED !\n");
		}
		s_testCount++;

		v->setValue(false);
		if (!v->getValueAsBoolean()){
			s_testSucceedCount++;
		}else{
			LOG_MSG("Test n°1c : FAILED !\n");
		}
		s_testCount++;

		if (v->isSet()){
			s_testSucceedCount++;
		}else{
			LOG_MSG("Test n°1d : FAILED !\n");
		}
		s_testCount++;

		delete v;
	}

	// Test 02 - Set/Get a string
	//---------------------------
	{
		auto v = new Nodable::Value();
		v->setValue("Hello world !");
		std::string str = "Hello world !";
		if (v->getValueAsString() == str){
			s_testSucceedCount++;
		}else{
			LOG_MSG("Test n°2a : FAILED !\n");
		}
		s_testCount++;

		if (v->getValueAsBoolean() == true){
			s_testSucceedCount++;
		}else{
			LOG_MSG("Test n°2b : FAILED !\n");
		}
		s_testCount++;

		if (v->getType() == Nodable::Type_String){
			s_testSucceedCount++;
		}else{
			LOG_MSG("Test n°2c : FAILED !\n");
		}
		s_testCount++;

		if (v->getValueAsNumber() == str.length() ){
			s_testSucceedCount++;
		}else{
			LOG_MSG("Test n°2d : FAILED !\n");
		}
		s_testCount++;

		if (v->isSet()){
			s_testSucceedCount++;
		}else{
			LOG_MSG("Test n°2e : FAILED !\n");
		}
		s_testCount++;

		delete v;
	}

	// Test 03 - Set/Get a number (double)
	//------------------------------------
	{
		auto v = new Nodable::Value();
		v->setValue(50.0F);

		if (v->getValueAsNumber() == 50.0F){
			s_testSucceedCount++;
		}else{
			LOG_MSG("Test n°3a : FAILED !\n");
		}
		s_testCount++;

		if (v->getType() == Nodable::Type_Number){
			s_testSucceedCount++;
		}else{
			LOG_MSG("Test n°3b : FAILED !\n");
		}
		s_testCount++;

		if (v->isSet()){
			s_testSucceedCount++;
		}else{
			LOG_MSG("Test n°3c : FAILED !\n");
		}
		s_testCount++;

		delete v;
	}

	LOG_MSG("Running Test for Node class...\n");

	{
		// Test 4 : set/get a node member :
		//---------------------------------

		auto a = new Nodable::Node();
		a->addMember("v");
		a->setMember("v", double(100));
		
		if( a->getMember("v")->getValueAsNumber()  == double(100))
			s_testSucceedCount++;
		else
			LOG_MSG("Test n°4 : FAILED !\n");
		s_testCount++;
		delete a;
	}

	LOG_MSG("Running integration Tests for Wire and Node class...\n");

	{
		// Test 5a : connect two nodes (creates a wire)
		//---------------------------------------------

		auto a = new Nodable::Node();
		a->addMember("output");

		auto b = new Nodable::Node();
		b->addMember("input");
		
		auto wire = new Nodable::Wire();
		Nodable::Node::Connect(wire, a->getMember("output"), b->getMember("input"));

		if ( 	wire->getSource() 		== a->getMember("output") && 
				wire->getTarget() 		== b->getMember("input")
			)
			s_testSucceedCount++;
		else
			LOG_MSG("Test n°5a : FAILED !\n");
		s_testCount++;

		// Test 5b : disconnect a wire
		//----------------------------

		Nodable::Node::Disconnect(wire);
		if(wire->getSource() == nullptr && wire->getTarget() == nullptr )
			s_testSucceedCount++;
		else
			LOG_MSG("Test n°5b : FAILED !\n");
		s_testCount++;

		delete wire;
		delete a;
		delete b;
	}

	DisplayResults();

	return s_testSucceedCount == s_testCount;
}
