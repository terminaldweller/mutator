# obfuscator

obfuscator is a C/C++ source-code obfuscation tool.<br/>

## Status
You can find a list of the implemented features and the ones that will be implemented below.<br/>
If you have suggestions or recommendations for features to add, please make an issue with the `obfuscator` label.<br/>
When an alpha version is ready, `obfuscator` will get its own mirror repo.<br/>

## CLI options
In addition to the clang options,obfuscator defines some custom CLI options:<br/>
* --shake: the accepted values are 128 and 256 for SHAKE128 and SHAKE256 respectively.<br/>
* --shake_len: the length of the hash value in bytes. see NOTES for some explanation.<br/>

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
Just some points to keep in mind while using obfuscator:<br/>
* If you choose to use SHAKE as the hashing algorithm, you have the flexibility of getting a variable-length digest. Just bear in mind that the value provided by `-shake_len` is the number of bytes in the hash itself. The stringified digest will be twice that length plus, file names have `FILE` attached to their begenning and any other identifier in the source code will have `ID` prepended to the digest value.If your compiler has limitations regarding the length of identifiers(some embedded C/C++ compilers do), you should keep that in mind.
* The hashed file names can end up being much longer than the original file name. Some tools have limitations regarding the length of arguments or file paths.<br/>
* Obfuscator uses the Clang Frontend(CFE) libraries. Most embedded hardware require the use of their custom toolchains and that can result in clang complaining about custom pragmas.<br/>
* If you want the preprocessor conditionals that evaluate to false that pertain to source code inclusion to be removed, use your compiler to get the output of the preprocessor and then pass that to `obfuscator` or just outright don't include them in the source code since `obfuscator` will not look through things that are not included in the source code by the preprocessor.<br/>
* Getting rid of the whitespaces in the source code is a moot point since reverting it is as easy as running something like `clang-format` on it, so the feature is not currently included in obfuscator.<br/>
* At a leter point in time, obfuscator will be moved to a mirror repo of it's own or the mirror might become the main repo.<br/>

## Thanks
obfuscator uses [keccak-tiny](https://github.com/coruus/keccak-tiny).<br/>
