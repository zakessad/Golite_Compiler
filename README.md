This repository holds the GoLite project for the COMP520 course at McGill University. 

The project is divided as follows:
* `programs` and `internal-test`: Test programs are organized by compilation phase and by expected result. Valid programs should output `OK` and status code `0`, while invalid programs should output `Error: <description>` and status code `1`.
  * `Scan+parse`: Runs both the scanner and parser phases
  * `Typecheck`: Runs until the end of the typechecker phase
  * `Codegen`: Runs until the compiler outputs the target code
* `src`: Source code for the project
* `build.sh`: Builds the compiler using `Make` or similar. You should replace the commands here if necessary to build your compiler
* `run.sh`: Runs your compiler using two arguments (mode - $1 and input file - $2). You should replace the commands here if necessary to invoke your compiler
* `test.sh`: Automatically runs the compiler against test programs in the programs directory and checks the output. The script takes the directory name as input.
* `clean.sh`: Deletes the generated python files.

The compiler has the folloqing modes:
* `tokens`: Prints the programs tokes.
* `scan`: Runs the scanner. Outputs `OK` if successful.
* `parse`: Runs the parser. Outputs `OK` if successful.
* `pretty`: Runs the pretty printer. Outputs the pretty format of the code.
* `symbol`: Prints the program's symbol.
* `typecheck`: Runs the typechecker. Outputs `OK` if successful.
* `codegen`: Generates Python code corresponding to the program.
