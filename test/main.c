
/*intentionally left blank.*/
#include "testFuncs1.h"
#include <string.h>

main()
{
	int a;
	int b;
	int int1;
	int int2;
	blreplacement bool1;
	blreplacement bool2;
	blreplacement decision1;
	blreplacement decision2;
	int level;
	int aa;
	int bb;
	double cc;

	testFuncStatementsinmple();
	testFuncStatementComplexIf();
	testFuncStatementNotCoverage ();
	testFuncLoopFor ();
	testFuncLoopWhile ();
	testFuncContinue ();
	testFuncBreak ();
	testFuncGoto ();
	testFuncNotReturn (a, b);
	testFuncMultiLineStatement ();
	testFuncMultipleStatement ();
	testFuncMultipleStatementNot ();
	testFuncCompOpti1 ();
	testFuncCompOpti2 ();
	testFuncCompOpti3 ();
	testFuncCompOpti4 ();
	testFuncStatementDecl1 ();
	testFuncStatementInt1 (int1, int2);
	testFuncStatementbool1 (bool1 , bool2);
	testFuncStatementbool2 (bool1 , bool2);
	testFuncStatementDecision1 (decision1, decision2);
	testFuncShortCircuit (bool1, bool2);
	testFuncMCDC1 (decision1, decision2);
#if (TRUE == INLINE)
	testFuncMultiInstantiation (level);
#endif
	testFuncQMark (int1, int2);
	testFuncCallBool ();

	im_a_mlaign_func ();
	im_a_benign_func ();
	im_a_dummy ();
	im_a_minion_01 ();
	im_a_minion_02 ();
	im_a_minion_03 ();

	test3();
	test4 (aa, bb, cc);
	test5();
	test6();
	test7();
	test8();
	test10();
	test13();
	test15();
	test17();
	test19();
	test20();
	test21();
	test22();
	test23();
	test24();
	test25();
}
/*intentionally left blank.*/