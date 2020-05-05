#pragma once

namespace Nodable
{
	/*
		Static class for testing purposes.
	*/
	class Test
	{
	public:
		/* Run all tests and return true if they are all successfully passed. 
		Side effect: display console logs while running tests.*/
		static bool RunAll();
	private:
		/* Display a log */
		static void DisplayResults();

		/* Reset the counters */
		static void ResetCounters();

		static int  s_testCount;
		static int  s_testSucceedCount;
	};
}