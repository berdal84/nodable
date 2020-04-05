
// include once this file
#include "Test.h"
#include "Member.h"
#include "Log.h"
#include "Entity.h"
#include "Wire.h"
#include "DataAccess.h"
#include "Container.h"
#include "Language.h"
#include "Parser.h"
#include "Variable.h"

#include <memory> // for unique_ptr
using namespace Nodable;

int  Test::s_testCount        = 0;
int  Test::s_testSucceedCount = 0;

void Test::ResetCounters()
{
	s_testCount        = 0;
	s_testSucceedCount = 0;
}

void Test::DisplayResults()
{
	LOG_MESSAGE("---------------------------------------------------------------\n");
	if(s_testSucceedCount != s_testCount)
		LOG_MESSAGE("   Some tests failed. Passed : %d / %d\n", s_testSucceedCount, s_testCount);
	else
		LOG_MESSAGE("   All tests are OK : %d / %d\n", s_testSucceedCount, s_testCount);

	LOG_MESSAGE("---------------------------------------------------------------\n");
}

bool Test::RunAll()
{
	LOG_MESSAGE("---------------------------------------------------------------\n");
	LOG_MESSAGE("--                   Testing Nodable                         --\n");
	LOG_MESSAGE("---------------------------------------------------------------\n");
	LOG_MESSAGE("-- Info: note that these tests do NOT cover all the code     --\n");
	LOG_MESSAGE("---------------------------------------------------------------\n");
	ResetCounters();

	LOG_MESSAGE("Running Test for Value class: \n");

	LOG_MESSAGE(" - Connection Flags...\n");

	{
		std::unique_ptr<Member> v(new Member);

		v->setConnectionFlags(Connection_In);

		if (!v->allows(Connection_Out)){
			s_testSucceedCount++;
		}else{
			LOG_ERROR("Test n°0-1a : FAILED !\n");
		}
		s_testCount++;

		if (!v->allows(Connection_InOut)){
			s_testSucceedCount++;
		}else{
			LOG_ERROR("Test n°0-1b : FAILED !\n");
		}
		s_testCount++;

		if (v->allows(Connection_In)){
			s_testSucceedCount++;
		}else{
			LOG_ERROR("Test n°0-1c : FAILED !\n");
		}
		s_testCount++;

		if (v->allows(Connection_None)){
			s_testSucceedCount++;
		}else{
			LOG_ERROR("Test n°0-1d : FAILED !\n");
		}
		s_testCount++;
	}


	{
		std::unique_ptr<Member> v(new Member);

		v->setConnectionFlags(Connection_Out);

		if (v->allows(Connection_Out)){
			s_testSucceedCount++;
		}else{
			LOG_ERROR("Test n°0-2a : FAILED !\n");
		}
		s_testCount++;

		if (!v->allows(Connection_InOut)){
			s_testSucceedCount++;
		}else{
			LOG_ERROR("Test n°0-2b : FAILED !\n");
		}
		s_testCount++;

		if (!v->allows(Connection_In)){
			s_testSucceedCount++;
		}else{
			LOG_ERROR("Test n°0-2c : FAILED !\n");
		}
		s_testCount++;

		if (v->allows(Connection_None)){
			s_testSucceedCount++;
		}else{
			LOG_ERROR("Test n°0-2d : FAILED !\n");
		}
		s_testCount++;
	}

	{
		std::unique_ptr<Member> v(new Member);

		v->setConnectionFlags(Connection_None);

		if (!v->allows(Connection_Out)){
			s_testSucceedCount++;
		}else{
			LOG_ERROR("Test n°0-3a : FAILED !\n");
		}
		s_testCount++;

		if (!v->allows(Connection_InOut)){
			s_testSucceedCount++;
		}else{
			LOG_ERROR("Test n°0-3b : FAILED !\n");
		}
		s_testCount++;

		if (!v->allows(Connection_In)){
			s_testSucceedCount++;
		}else{
			LOG_ERROR("Test n°0-3c : FAILED !\n");
		}
		s_testCount++;

		if (v->allows(Connection_None)){
			s_testSucceedCount++;
		}else{
			LOG_ERROR("Test n°0-3d : FAILED !\n");
		}
		s_testCount++;
	}


	{
		std::unique_ptr<Member> v(new Member);

		v->setConnectionFlags(Connection_InOut);

		if (v->allows(Connection_Out)){
			s_testSucceedCount++;
		}else{
			LOG_ERROR("Test n°0-4a : FAILED !\n");
		}
		s_testCount++;

		if (v->allows(Connection_InOut)){
			s_testSucceedCount++;
		}else{
			LOG_ERROR("Test n°0-4b : FAILED !\n");
		}
		s_testCount++;

		if (v->allows(Connection_In)){
			s_testSucceedCount++;
		}else{
			LOG_ERROR("Test n°0-4c : FAILED !\n");
		}
		s_testCount++;

		if (v->allows(Connection_None)){
			s_testSucceedCount++;
		}else{
			LOG_ERROR("Test n°0-4d : FAILED !\n");
		}
		s_testCount++;
	}


	// Test 01 - Set/Get a boolean
	//----------------------------

	LOG_MESSAGE(" - Get/Set (Boolean)...\n");

	{
		std::unique_ptr<Member> v(new Member);
		v->setValue(true);		

		if (v->getValueAsBoolean()){
			s_testSucceedCount++;
		}else{
			LOG_ERROR("Test n°1a : FAILED !\n");
		}
		s_testCount++;
		
		if (v->getType() == ::Type_Boolean){
			s_testSucceedCount++;
		}else{
			LOG_ERROR("Test n°1b : FAILED !\n");
		}
		s_testCount++;

		v->setValue(false);
		if (!v->getValueAsBoolean()){
			s_testSucceedCount++;
		}else{
			LOG_ERROR("Test n°1c : FAILED !\n");
		}
		s_testCount++;

		if (v->isSet()){
			s_testSucceedCount++;
		}else{
			LOG_ERROR("Test n°1d : FAILED !\n");
		}
		s_testCount++;
	}

	// Test 02 - Set/Get a string
	//---------------------------

	LOG_MESSAGE(" - Get/Set (String)...\n");

	{
		std::unique_ptr<Member> v(new Member);
		v->setValue("Hello world !");
		std::string str = "Hello world !";
		if (v->getValueAsString() == str){
			s_testSucceedCount++;
		}else{
			LOG_ERROR("Test n°2a : FAILED !\n");
		}
		s_testCount++;

		if (v->getValueAsBoolean() == true){
			s_testSucceedCount++;
		}else{
			LOG_ERROR("Test n°2b : FAILED !\n");
		}
		s_testCount++;

		if (v->getType() == ::Type_String){
			s_testSucceedCount++;
		}else{
			LOG_ERROR("Test n°2c : FAILED !\n");
		}
		s_testCount++;

		if (v->getValueAsNumber() == str.length() ){
			s_testSucceedCount++;
		}else{
			LOG_ERROR("Test n°2d : FAILED !\n");
		}
		s_testCount++;

		if (v->isSet()){
			s_testSucceedCount++;
		}else{
			LOG_ERROR("Test n°2e : FAILED !\n");
		}
		s_testCount++;
	}

	// Test 03 - Set/Get a number (double)
	//------------------------------------

	LOG_MESSAGE(" - Get/Set (Number)...\n");

	{
		std::unique_ptr<Member> v(new Member);
		v->setValue(50.0F);

		if (v->getValueAsNumber() == 50.0F){
			s_testSucceedCount++;
		}else{
			LOG_ERROR("Test n°3a : FAILED !\n");
		}
		s_testCount++;

		if (v->getType() == ::Type_Number){
			s_testSucceedCount++;
		}else{
			LOG_ERROR("Test n°3b : FAILED !\n");
		}
		s_testCount++;

		if (v->isSet()){
			s_testSucceedCount++;
		}else{
			LOG_ERROR("Test n°3c : FAILED !\n");
		}
		s_testCount++;
	}

	LOG_MESSAGE("Running Test for Entity class...\n");

	{
		// Test 4 : set/get a node member :
		//---------------------------------

		std::unique_ptr<Entity> a(new Entity);
		a->addMember("v");
		a->setMember("v", double(100));
		
		if( a->getMember("v")->getValueAsNumber()  == double(100))
			s_testSucceedCount++;
		else
			LOG_ERROR("Test n°4 : FAILED !\n");
		s_testCount++;

	}

	LOG_MESSAGE("Running integration Tests for Wire and Entity class...\n");

	{
		// Test 5a : connect two nodes (creates a wire)
		//---------------------------------------------

		std::unique_ptr<Entity> a(new Entity);
		a->addMember("output");

		std::unique_ptr<Entity> b(new Entity);
		b->addMember("input");
		
		std::unique_ptr<Wire> wire(new Wire);
		Entity::Connect(wire.get(), a->getMember("output"), b->getMember("input"));

		if ( 	wire->getSource() 		== a->getMember("output") && 
				wire->getTarget() 		== b->getMember("input")
			)
			s_testSucceedCount++;
		else
			LOG_ERROR("Test n°5a : FAILED !\n");
		s_testCount++;

		// Test 5b : disconnect a wire
		//----------------------------

		Entity::Disconnect(wire.get());
		if(wire->getSource() == nullptr && wire->getTarget() == nullptr )
			s_testSucceedCount++;
		else
			LOG_ERROR("Test n°5b : FAILED !\n");
		s_testCount++;

	}

	LOG_MESSAGE("Running tests for DataAccess...\n");
	{
		std::unique_ptr<Container>           container(new Container);
		Entity* entity = container->createNodeAdd();
		std::unique_ptr<DataAccess> dataAccessComponent(new DataAccess);

		entity->setMember("name", "UnitTestEntity");
		entity->addComponent("dataAccess", dataAccessComponent.get());

		entity->addMember("BOOL");
		entity->setMember("BOOL", false);

		entity->addMember("STRING");
		entity->setMember("STRING", "hello world!");

		entity->addMember("NUMBER");
		entity->setMember("NUMBER", double(3.14));

		dataAccessComponent->update();
		entity->removeComponent("dataAccess");
	}

	
	auto testParser = [](const std::string& testName, const std::string& expression, const std::string& resultExpected)->void {

		LOG_DEBUG( "Testing: %s should return %s \n", expression.c_str(), resultExpected.c_str()) ;

		auto container(new Container);
		auto expressionVariable(container->createNodeVariable("expression"));
		expressionVariable->setValue(expression);

		auto parser(container->createNodeParser(expressionVariable));

		parser->eval();

		auto resultVariable = container->getResultVariable();
		resultVariable->update();

		auto result = resultVariable->getValueAsString();

		if (result == resultExpected)
			s_testSucceedCount++;
		else{
			const std::string message = "Test " + testName + " : FAILED ! " + expression + "\n";
			LOG_ERROR(message.c_str());
		}
		s_testCount++;

		delete container;

	};

	LOG_MESSAGE("Running tests for Parser...\n");
	{
		testParser("Parser 00", "5"         , "5");
		testParser("Parser 01", "-5"         , "-5");
		testParser("Parser 02", "2+3"        , "5");
		testParser("Parser 03", "-5+4"       , "-1");
		testParser("Parser 04", "-1+2*5-3/6" , "8.5");
		testParser("Parser 05", "-1*20"      , "-20");

		testParser("Parser 06", "(1+4)", "5");
		testParser("Parser 07", "(1)+(2)", "3");
		testParser("Parser 08", "(1+2)*3", "9");
		testParser("Parser 09", "2*(5+3)", "16");
		testParser("Parser 10", "2+(5*3)", "17");
		testParser("Parser 11", "2*(5+3)+2", "18");
	}


	DisplayResults();

	return s_testSucceedCount == s_testCount;
}
