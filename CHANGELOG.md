# CHANGELOG 1 (ver0.2.0)

## GENERAL
  - Moved from v0.1.0 -> v0.2.0
  - Added include/ folder with the guinea.h and moved declarations.h there;
  - Added a .gitignore;
  - Added and made a working VM with its stack but currently a non operating 
  frame stack;
  - Changed all global names to end with GUIN_ in order to be embeddable 
  compliant and not cause name clashes during linking with other projects;
  - You can use CMake to compile Guinea now, these are the commands to build
  Guinea:
  ```
      cmake -B build
      cmake --build build
  ```
  And guinea will be located in build/bin/;
  - Function calls are now allowed in expressions such as if add(2,3) == 5;
  - Booleans and chars can now be treated like int values when doing arithmetic
  with true = 1, false = 0, and characters being their ASCII values
      i.e: true - 4 (equ -3), 'a' + 102 (equ 199);
  - Fixed \t causing the lexer to glitch and the compiler claiming theres an error

## API
  - Added working API calls for VMs which works on C and C++;
  - Added GUIN_VALUE(...) to quickly exchange C values into their Guinea 
  variants for both C and C++;
  - Added function calls and all the current arithmetic and logic available 
  in Guinea as API (+, -, *, /, %, ^), (==, !=, >, <, >=, <=, !, &&, ||)

## LICENSE
  - Migrated from MIT to Apache v2.0 License for Guinea