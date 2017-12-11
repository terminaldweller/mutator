# obfuscator

obfuscator is a C/C++ source-code obfuscation tool.<br/>

## Status
You can find a list of the implemented features and the ones that will be implemented below.<br/>
If you have suggestions or recommendations for features to add, please make an issue with the `obfuscator` label.<br/>

### Implemented Features
* Identifier Obfuscation: Swaps the name of all identifiers with their hash. <br/>
* Comment Deletion: Deletes all comments.<br/>

### Future Features
* Obfuscation Exclusion List: obfuscator will accept a list of idenftifiers and their namespace and will not obfuscate those. This feature is added so the user can refrain from obfuscating the standard library.<br/>
* Support directory-wide and multiple files as input.<br/>
* Provide an option to choose which hashing function to use.<br/>

## Notes
* Currently the hash function that is being used is `std::hash<>`. The GCC implementation will be probably the default option since the digest is shorter than 32 characters long. The decision was made since quite a few embedded C/C++ compilers can't correctly handle identifiers longer than 32 characters.<br/>
* If you want the preprocessor conditionals that evaluate to false that pertain to source code inclusion to be removed, use your compiler to get the output of the preprocessor and then pass that to `obfuscator` or just outright don't include them in the source code since `obfuscator` will not look through things that are not included in the source code by the preprocessor.<br/>
* Getting rid of the whitespaces in the source code is a moot point since reverting it is as easy as running something like `clang-format` on it, so the feature is not currently included in obfuscator.<br/>
