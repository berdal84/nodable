#pragma once

#include <memory> // for unique_ptr
#include <stack>
#include "Member.h"
#include "Log.h"
#include "Node.h"
#include "Wire.h"
#include "DataAccess.h"
#include "Container.h"
#include "Language.h"
#include "Parser.h"
#include "Variable.h"

struct TestState {
	TestState() {};

	void add(const TestState& _other) {
		this->m_count += _other.m_count;
		this->m_successCount += _other.m_successCount;
		this->m_failureCount += _other.m_failureCount;
	}

	void addSuccess() {
		this->m_successCount++;
		this->m_count++;
	}

	void addFailure() {
		this->m_failureCount++;
		this->m_count++;
	}

	bool allPassed()const {
		return m_count == m_successCount;
	}

	size_t m_count = 0;
	size_t m_successCount = 0;
	size_t m_failureCount = 0;
};

using namespace Nodable;

static TestState s_globalState; \
static bool s_lastGroupTestPassed; \
static std::stack<TestState> status;

#define GLOBAL_TEST_BEGIN \
	s_globalState = TestState(); \
	s_lastGroupTestPassed = false;

#define GLOBAL_TEST_END \
	if(  s_globalState.allPassed() ) { \
		LOG_DEBUG(GREEN"All tests are OK : (%d / %d passed).\n", s_globalState.m_successCount, s_globalState.m_count); \
	} else { \
		LOG_DEBUG(RED"Some tests FAILED !: (%d / %d passed).\n", s_globalState.m_successCount, s_globalState.m_count); \
	}

#define GLOBAL_TEST_RESULT s_globalState.allPassed();

#define TEST_BEGIN(name) \
	{ \
		LOG_MESSAGE("Testing %s...\n", name);\
		status.push( TestState() ); \
		{

#define TEST_END \
		} \
		if( ! status.top().allPassed() ) { \
			LOG_ERROR("Test: failed, %i/%i passed.\n", status.top().m_successCount,  status.top().m_count); \
		} \
		s_globalState.add(status.top()); \
		s_lastGroupTestPassed = status.top().allPassed(); \
		status.pop(); \
	} 

#define TEST_RESULT s_globalState.allPassed()

#define EXPECT(function, expected) \
	if (function == expected) { \
		status.top().addSuccess();\
	} else { \
		LOG_ERROR(#function" : FAILED ! expected:" #expected "\n");\
		status.top().addFailure();\
	}


bool Member_Connections_Tests() {

	TEST_BEGIN("Member Connection"){

		TEST_BEGIN("Member 1: Connection_In"){
			std::unique_ptr<Member> m(new Member);
			m->setConnectionFlags(Connection_In);

			EXPECT(m->allows(Connection_Out)	, false)
			EXPECT(m->allows(Connection_InOut)	, false)
			EXPECT(m->allows(Connection_In)		, true)
			EXPECT(m->allows(Connection_None)	, true)
		}TEST_END


		TEST_BEGIN("Member 2: Connection_Out"){
			std::unique_ptr<Member> m(new Member);
			m->setConnectionFlags(Connection_Out);

			EXPECT(m->allows(Connection_Out)	, true)
			EXPECT(m->allows(Connection_InOut)	, false)
			EXPECT(m->allows(Connection_In)		, false)
			EXPECT(m->allows(Connection_None)	, true)
		}TEST_END

		TEST_BEGIN("Member 3: Connection_None"){
			std::unique_ptr<Member> m(new Member);
			m->setConnectionFlags(Connection_Out);

			EXPECT(m->allows(Connection_Out)	, true)
			EXPECT(m->allows(Connection_InOut)	, false)
			EXPECT(m->allows(Connection_In)		, false)
			EXPECT(m->allows(Connection_None)	, true)
		}TEST_END

		TEST_BEGIN("Member 4: Connection_InOut"){
			std::unique_ptr<Member> m(new Member);
			m->setConnectionFlags(Connection_InOut);

			EXPECT(m->allows(Connection_Out)	, true)
			EXPECT(m->allows(Connection_InOut)	, true)
			EXPECT(m->allows(Connection_In)		, true)
			EXPECT(m->allows(Connection_None)	, true)
		}TEST_END

	}TEST_END

	return s_lastGroupTestPassed;
}

bool Member_AsBoolean_Tests() {

	TEST_BEGIN("Member: Booleans"){

		std::unique_ptr<Member> m(new Member);

		m->set(true);
		EXPECT(m->as<bool>(), true)
		EXPECT(m->getType(), Type_Boolean)

		m->set(false);
		EXPECT(m->as<bool>(), false)
		EXPECT(m->isSet(), true)

	}TEST_END

	return s_lastGroupTestPassed;
}

bool Member_AsString_Tests() {

	TEST_BEGIN("Member: String"){

		std::unique_ptr<Member> m(new Member);
		m->set("Hello world !");
		const std::string str = "Hello world !";

		EXPECT(m->as<std::string>(), str)
		EXPECT(m->as<bool>(), true)
		EXPECT(m->getType(), Type_String)
		EXPECT(m->as<double>(), str.length())
		EXPECT(m->isSet(), true)
	}TEST_END

	return s_lastGroupTestPassed;
}

bool Member_AsNumber_Tests() {

	TEST_BEGIN("Member: Number"){
				
		std::unique_ptr<Member> m(new Member);
		m->set(50.0F);

		EXPECT(m->as<double>(), 50.0F)
		EXPECT(m->getType(), Type_Number)
		EXPECT(m->isSet(), true)	

	}TEST_END

	return s_lastGroupTestPassed;

}

template <typename T>
bool Parser_Test(
	const std::string& expression,	
	T _expectedValue,
	const Language* _language = Language::Nodable()
){

	auto container = new Container(_language);
	auto expressionVariable(container->newVariable("expression"));
	expressionVariable->set(expression);

	auto parser = container->newParser(expressionVariable);

	parser->eval();

	auto result = container->getResultVariable();
	result->update();

	Member expectedMember;
	expectedMember.set(_expectedValue);
	auto success = result->getMember()->equals(&expectedMember);

	delete container;

	return success;
}

bool Parser_Tests() {

	TEST_BEGIN("Parser"){

		TEST_BEGIN("Simple"){
			EXPECT( Parser_Test("-5", -5), true)
			EXPECT( Parser_Test("2+3", 5), true)
			EXPECT( Parser_Test("-5+4", -1), true)
			EXPECT( Parser_Test("-1+2*5-3/6", 8.5), true)
		}TEST_END

		TEST_BEGIN("Simple parenthesis"){
			EXPECT( Parser_Test("-1*20", -20), true)
			EXPECT( Parser_Test("(1+4)", 5), true)
			EXPECT( Parser_Test("(1)+(2)", 3), true)
			EXPECT( Parser_Test("(1+2)*3", 9), true)
			EXPECT( Parser_Test("2*(5+3)", 16), true)
		}TEST_END

		TEST_BEGIN("Complex parenthesis"){

			EXPECT(Parser_Test("2+(5*3)",
			                    2+(5*3)),
								true)

			EXPECT(Parser_Test("2*(5+3)+2",
			                    2*(5+3)+2
								), true)

			EXPECT(Parser_Test("(2-(5+3))-2+(1+1)",
			                    (2-(5+3))-2+(1+1)
								), true)

			EXPECT(Parser_Test("(2 -(5+3 )-2)+9/(1- 0.54)",
			                    (2 -(5+3 )-2)+9/(1- 0.54)
								), true)

			EXPECT(Parser_Test("1/3"
			                   , 1.0F/3.0F
							   ), true)
		}TEST_END		

		TEST_BEGIN("Function call"){
			EXPECT(Parser_Test("returnNumber(5)", 5), true)
			EXPECT(Parser_Test("returnNumber(1)", 1), true)
		}TEST_END

	}TEST_END

	return s_lastGroupTestPassed;
}

bool Node_Tests() {

	TEST_BEGIN("Node"){

		TEST_BEGIN("Node (add member Type_Number)"){

			std::unique_ptr<Node> node(new Node);
			node->add("val");
			node->set("val", double(100));

			EXPECT(node->get("val")->as<double>(), double(100))
			EXPECT(node->get("val")->as<std::string>(), std::to_string(100))
			EXPECT(node->get("val")->as<bool>(), true)
		}TEST_END

	}TEST_END

	return s_lastGroupTestPassed;
}

bool WireAndNode_Tests() {

	TEST_BEGIN("Wire and Node"){

		TEST_BEGIN("Connect two nodes with a wire"){

			std::unique_ptr<Node> a(new Node);
			a->add("output");

			std::unique_ptr<Node> b(new Node);
			b->add("input");

			std::unique_ptr<Wire> wire(new Wire);
			Node::Connect(wire.get(), a->get("output"), b->get("input"));

			EXPECT(wire->getSource() , a->get("output"))
			EXPECT(wire->getTarget() , b->get("input"))
		}TEST_END

		TEST_BEGIN("Disconnect a wire"){

			std::unique_ptr<Node> a(new Node);
			a->add("output");

			std::unique_ptr<Node> b(new Node);
			b->add("input");

			std::unique_ptr<Wire> wire(new Wire);
			Node::Connect(wire.get(), a->get("output"), b->get("input"));
			Node::Disconnect(wire.get());

			EXPECT(wire->getSource(), nullptr)
			EXPECT(wire->getTarget(), nullptr)
		}TEST_END

	}TEST_END

	return s_lastGroupTestPassed;
}

bool Test_RunAll()
{
	GLOBAL_TEST_BEGIN{

		TEST_BEGIN("Member"){
			EXPECT( Member_Connections_Tests(), true)
			EXPECT( Member_AsBoolean_Tests(), true)
			EXPECT( Member_AsString_Tests(), true)
			EXPECT( Member_AsNumber_Tests(), true)
		}TEST_END
		
		TEST_BEGIN("Parser"){
			EXPECT( Parser_Tests(), true)
		}TEST_END

		TEST_BEGIN("Node"){
			EXPECT( Node_Tests(), true)
		}TEST_END
		
		TEST_BEGIN("Wire and Node"){
			EXPECT( WireAndNode_Tests(), true)
		}TEST_END

		TEST_BEGIN("Biology (DNA to Protein)") {
			EXPECT( Parser_Test("DNAtoProtein(\"TAA\")", "_"), true)
			EXPECT( Parser_Test("DNAtoProtein(\"TAG\")", "_"), true)
			EXPECT( Parser_Test("DNAtoProtein(\"TGA\")", "_"), true)
			EXPECT( Parser_Test("DNAtoProtein(\"ATG\")", "M"), true)
		}TEST_END
	}

	GLOBAL_TEST_END

	return GLOBAL_TEST_RESULT;
}
