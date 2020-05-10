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

#define TEST(name, content) \
	{ \
		LOG_MESSAGE("Testing %s...\n", name);\
		status.push( TestState() ); \
		{ \
			content \
		} \
		if( ! status.top().allPassed() ) { \
			LOG_ERROR("Test: %s failed, %i/%i passed.\n", name, status.top().m_successCount,  status.top().m_count); \
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

	TEST("Member Connection",

		TEST("Member 1: Connection_In",

			std::unique_ptr<Member> m(new Member);
			m->setConnectionFlags(Connection_In);

			EXPECT(m->allows(Connection_Out)	, false)
			EXPECT(m->allows(Connection_InOut)	, false)
			EXPECT(m->allows(Connection_In)		, true)
			EXPECT(m->allows(Connection_None)	, true)
		)


		TEST("Member 2: Connection_Out",

			std::unique_ptr<Member> m(new Member);
			m->setConnectionFlags(Connection_Out);

			EXPECT(m->allows(Connection_Out)	, true)
			EXPECT(m->allows(Connection_InOut)	, false)
			EXPECT(m->allows(Connection_In)		, false)
			EXPECT(m->allows(Connection_None)	, true)
		)

		TEST("Member 3: Connection_None",

			std::unique_ptr<Member> m(new Member);
			m->setConnectionFlags(Connection_Out);

			EXPECT(m->allows(Connection_Out)	, true)
			EXPECT(m->allows(Connection_InOut)	, false)
			EXPECT(m->allows(Connection_In)		, false)
			EXPECT(m->allows(Connection_None)	, true)
		)

		TEST("Member 4: Connection_InOut",

			std::unique_ptr<Member> m(new Member);
			m->setConnectionFlags(Connection_InOut);

			EXPECT(m->allows(Connection_Out)	, true)
			EXPECT(m->allows(Connection_InOut)	, true)
			EXPECT(m->allows(Connection_In)		, true)
			EXPECT(m->allows(Connection_None)	, true)
		)
	)

	return s_lastGroupTestPassed;
}

bool Member_AsBoolean_Tests() {

	TEST("Member: Booleans",

		std::unique_ptr<Member> m(new Member);

		m->setValue(true);
		EXPECT(m->getValueAsBoolean(), true)
		EXPECT(m->getType(), Type_Boolean)

		m->setValue(false);
		EXPECT(m->getValueAsBoolean(), false)
		EXPECT(m->isSet(), true)
	)

	return s_lastGroupTestPassed;
}

bool Member_AsString_Tests() {

	TEST("Member: String",

		std::unique_ptr<Member> m(new Member);
		m->setValue("Hello world !");
		const std::string str = "Hello world !";

		EXPECT(m->getValueAsString(), str)
		EXPECT(m->getValueAsBoolean(), true)
		EXPECT(m->getType(), Type_String)
		EXPECT(m->getValueAsNumber(), str.length())
		EXPECT(m->isSet(), true)
	)

	return s_lastGroupTestPassed;
}

bool Member_AsNumber_Tests() {

	TEST("Member: Number",
				
		std::unique_ptr<Member> m(new Member);
		m->setValue(50.0F);

		EXPECT(m->getValueAsNumber(), 50.0F)
		EXPECT(m->getType(), Type_Number)
		EXPECT(m->isSet(), true)				
	)

	return s_lastGroupTestPassed;

}

std::string Parser_Test(const std::string& expression) {

	auto container(new Container);
	auto expressionVariable(container->newVariable("expression"));
	expressionVariable->setValue(expression);

	auto parser(container->newParser(expressionVariable));

	parser->eval();

	auto resultVariable = container->getResultVariable();
	resultVariable->update();

	auto result = resultVariable->getValueAsString();

	delete container;

	return result;
}

bool Parser_Tests() {

	TEST("Parser",

		TEST("Simple",

			EXPECT(Parser_Test("-5")         , "-5")
			EXPECT(Parser_Test("2+3")        , "5")
			EXPECT(Parser_Test("-5+4")       , "-1")
			EXPECT(Parser_Test("-1+2*5-3/6") , "8.5")
		)

		TEST("Simple parenthesis",

			EXPECT(Parser_Test("-1*20")    , "-20")
			EXPECT(Parser_Test("(1+4)")    , "5")
			EXPECT(Parser_Test("(1)+(2)")  , "3")
			EXPECT(Parser_Test("(1+2)*3")  , "9")
			EXPECT(Parser_Test("2*(5+3)")  , "16")
		)

		TEST("Complex parenthesis",

			EXPECT(Parser_Test("2+(5*3)")                   , "17")
			EXPECT(Parser_Test("2*(5+3)+2")                 , "18")
			EXPECT(Parser_Test("(2-(5+3))-2+(1+1)")         , "-6")
			EXPECT(Parser_Test("(2 -(5+3 )-2)+9/(1- 0.54)") , "11.565217")
			EXPECT(Parser_Test("1/3")                       , "0.333333")
		)
		

		TEST("Function call",

			EXPECT(Parser_Test("nothing(5)"), "5")
			EXPECT(Parser_Test("nothing(1)"), "1")
		)
	)

	return s_lastGroupTestPassed;
}

bool Node_Tests() {

	TEST("Node",

		TEST("Node (add member Type_Number)",

			std::unique_ptr<Node> node(new Node);
			node->add("val");
			node->setMember("val", double(100));

			EXPECT(node->get("val")->getValueAsNumber(), double(100))
			EXPECT(node->get("val")->getValueAsString(), "100")
			EXPECT(node->get("val")->getValueAsBoolean(), true)
		)
	)

	return s_lastGroupTestPassed;
}

bool WireAndNode_Tests() {
	TEST("Wire and Node",

		TEST("Connect two nodes with a wire",

			std::unique_ptr<Node> a(new Node);
			a->add("output");

			std::unique_ptr<Node> b(new Node);
			b->add("input");

			std::unique_ptr<Wire> wire(new Wire);
			Node::Connect(wire.get(), a->get("output"), b->get("input"));

			EXPECT(wire->getSource() , a->get("output"))
			EXPECT(wire->getTarget() , b->get("input"))
		)

		TEST("Disconnect a wire",

			std::unique_ptr<Node> a(new Node);
			a->add("output");

			std::unique_ptr<Node> b(new Node);
			b->add("input");

			std::unique_ptr<Wire> wire(new Wire);
			Node::Connect(wire.get(), a->get("output"), b->get("input"));
			Node::Disconnect(wire.get());

			EXPECT(wire->getSource(), nullptr)
			EXPECT(wire->getTarget(), nullptr)
		)
	)

	return s_lastGroupTestPassed;
}

bool RunAllTests()
{
	GLOBAL_TEST_BEGIN

	{
		TEST("Member",
			EXPECT(Member_Connections_Tests(), true)
			EXPECT(Member_AsBoolean_Tests(), true)
			EXPECT(Member_AsString_Tests(), true)
			EXPECT(Member_AsNumber_Tests(), true)
		)

		TEST("Parser",
			EXPECT(Parser_Tests(), true)
		)

		TEST("Node",
			EXPECT(Node_Tests(), true)
		)

		TEST("Wire & Node",
			EXPECT(WireAndNode_Tests(), true)
		)

		//TEST("Biology (DNA to Phenylalanine)",
		//	EXPECT(Parser_Test("TTT"), "F")
		//	EXPECT(Parser_Test("TTC"), "F")
		//)
	}

	GLOBAL_TEST_END

	return GLOBAL_TEST_RESULT;
}
