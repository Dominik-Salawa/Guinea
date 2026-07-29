# CHANGELOG 2 (ver0.3.0)

## GENERAL
  - Printing floats/doubles via GUIN_printf now leaves no trailing 0s and can show up to 15 decimals
  - VM can now interpret (some) bytecode
  - Frame stacks and Global variables have been set up
  - Calling a function will first push all its arguments,
  then the function itself, and then it will execute

## API
  - Added loading Bytecode memory and files
  - You can add Global names and write to them