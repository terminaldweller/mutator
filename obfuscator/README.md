# obfuscator

obfuscator is a C/C++ source-code obfuscation tool.<br/>

## Status
You can find a list of the implemented features and the ones that will be implemented below.<br/>
If you have suggestions or recommendations for features to add, please make an issue with the `obfuscator` label.<br/>
When an alpha version is ready, `obfuscator` will get its own mirror repo.<br/>

### Implemented Features
* Identifier Obfuscation: Swaps the name of all identifiers with their hash.<br/>
* Comment Deletion: Deletes all comments.<br/>
* SHAKE: you can choose to use SHAKE128 or SHAKE256 as the hashing function.<br/>

### Future Features
* Obfuscation Exclusion List: obfuscator will accept a list of idenftifiers and their namespace and will not obfuscate those. This feature is added so the user can refrain from obfuscating the standard library.<br/>
* Support directory-wide and multiple files as input.<br/>

### Running the Test
running `run.sh` should do the trick. Do note that you need to regenerate the compilation database for the test under the `test` directory to work. You could use `bear`. If you already have `bear`, just run `make clean && bear make`.<br/>

## Notes
* Obfuscator uses the Clang Frontend(CFE) libraries. Most embedded hardware require the use of their custom toolchains and that can result in clang complaining about custom pragmas.<br/>
* If you want the preprocessor conditionals that evaluate to false that pertain to source code inclusion to be removed, use your compiler to get the output of the preprocessor and then pass that to `obfuscator` or just outright don't include them in the source code since `obfuscator` will not look through things that are not included in the source code by the preprocessor.<br/>
* Getting rid of the whitespaces in the source code is a moot point since reverting it is as easy as running something like `clang-format` on it, so the feature is not currently included in obfuscator.<br/>
* At a leter point in time, obfuscator will be moved to a mirror repo of it's own or the mirror might become the main repo.<br/>

## Thanks
obfuscator uses [keccak-tiny](https://github.com/coruus/keccak-tiny).<br/>
