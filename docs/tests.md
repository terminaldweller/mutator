# mutator Testing

This document outlines the test plans for the mutator project. It includes the current measures taken to ensure a high-quality and reliability, but also future plans to improve mutator's testing process and methods.<br/>

### Disclaimers

first off lets talk about the third-party libraries mutator uses:<br/>
* First and foremost we use the LLVM libraries. mutator mostly uses the Clang Frontend, but we do also use some LLVM libraries explicitly, some other LLVM libraries are being used by the Clang frontend implicitly. The backbone of mutator is the LLVM project. The libraries mutator uses are not guaranteed to be backwards compatible since they are under development, so mutator is being developed using the Dev version of the LLVM/Clang libraries as an attempt to be able to use all the new features offered by the new versions of LLVM/Clang.<br/>

* The two other sets of libraries mutator uses are [tinyxml2](https://github.com/leethomason/tinyxml2) and [JSON](https://github.com/nlohmann/json) which are used for report generation. For the tests and the dev process of those libraries you can visit their respective repositories on Github. mutator also features a third report generation methos which is AWK-friendly and has been writen for mutator as an alternative for those users who are not comfortable with depending on third-party libraries.<br/>

* Below you can find links that point to the C.V.s of the major contributors to the mutator project as a demonstration of competence:<br/>
	* [Farzad Sadeghi](https://ir.linkedin.com/in/farzad-sadeghi-08426277)

### TDD tests

mutator uses TDD, so we already have TDD tests for all the features implemented that check for true positives, true negatives, false positives and false negetavies. In case we detect undesirable behaviour, the behaviour is documented in either the respective section in the source-code or in a separate document, later to be fixed or improved. Currently mutator uses a human oracle to determine the results of the TDD tests.<br/>
Currently TDD tests serve as mutator's way of Validation.<br/>

### Static Analysis

For static analysis, mutator is using [Coverity](https://scan.coverity.com/projects/bloodstalker-mutator). The coverity analysis is automated via [CI Travis](https://travis-ci.org/bloodstalker/mutator) and happens on every commit. For our current code-base size, we get 8 analyses a week.<br/>

### Dynamic Analysis

For dynamic analysis, mutator is using [Valgrind](http://valgrind.org/).<br/>
Currently mutator is using the following checks from Valgrind:<br/>

* Memcheck
* More tests coming soon...

### Smoke Tests

Mutator features more than once build tests everyday. mutator runs over all the TDD tests everytime it runs, to make sure that mutator does not break.<br/>
Also as an extra test, mutator is periodically run over all the C standard library headers to make sure it does not break.<br/>
Needless to say, there are extra measures taken in the source code to make sure that mutator does not crash or hang.<br/>
Smoke tests are integrated into the build process.<br/>

### Code Metrics and Code Coverage

For determining the code coverage, mutator is currently using the facilities provided by Clang though it does not, to the best of my knowledge, provide MC/DC coverage.<br/>
The coverage results will be made public after we start writing the unit tests either through [coveralls](https://coveralls.io) or a similar service.<br/>
Project mutator currently has no means of getting the code metrics. If you have a way, plesee do let me know.<br/>

### Unit Tests

For unit tests, mutator will be using a human oracle for the first time the tests are being conducted. After the results are verified, we'll use the results determined to be true by the human oracle.<br/>
Unit tests are mutator's way of validation.<br/>
mutator will also feature XML and JSON schema files to make sure that the data generated for the reports are in the correct form and by extension find possible faults in the code. The faults detected, needless to say, are not limited to report generation, which is the primary reason of mutator having schemas for the reports it generates.<br/>

##### project mutator is currently looking for unit testers. If you are interested, you can email me at thabogre@gmail.com .<br/>

### Regression Tests

After the test results have been verified by a human oracle for the first time, we'll use the verified results as the oracle. Verifications will be performed by a script.<br/>
Regression tests will be run periodically on the code-base.<br/>

### Code Review

After the implementation is done and we branch a release candidate, code reviews will start.<br/>
Code reviews will be done by someone other than the person who wrote the code(hopefully!) if I manage to find people who are willing to help.<br/>
Due to project mutator's currently limited resources, the initial review plan is to have someone just review code without the presence of the developer and contact him if he/she has any questions.<br/>

#### project mutator is currently looking for code reviewers. Contact me at thabogre@gmail.com if you are interested.<br/>