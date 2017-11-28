# obfuscator

obfuscator is a C/C++ source-code obfuscation tool.<br/>

## Status
obfuscator is not feature-complete yet. Below you can find a list of the implemented features and the ones that will be implemented.<br/>
If you have suggestions or recommendations for features to add please make an issue with the `obfuscator` label.<br/>

### Implemented Features
* Identifier Obfuscation: Swaps the name of all identifiers with their hash. <br/>

### Future Features
* Obfuscation Exclusion List: obfuscator will accept a list of idenftifiers and their namespace and will not obfuscate those. This feature is added so the user can refrain from obfuscating the standard library.<br/>
* Whitespace Deletion: Pretty much kills all whitespace where it doesn't change the syntax.<br/>
* Comment Deletion: Deletes all comments.<br/>

## Notes
* Currently the hash function that is being used is `std::hash<>`. The GCC implementation will be probably the default option since the digest is shorter than 32 characters long. The decision was made since quite a few embedded C/C++ compilers can't correctly handle identifiers longer than 32 characters(implementation limitations).<br/>
