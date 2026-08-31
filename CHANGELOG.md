# CHANGELOG 3 (ver0.3.0)

## GENERAL
  - Printing floats/doubles via GUIN_printf now leaves no trailing 0s and can show up to 15 decimals
  - Frame stacks and Global variables have been set up
  - Calling a function will first push all its arguments, then the function itself, and then it will execute
  - Keyword `Inf` was added to represent a number64 Infinity
  - User-defined functions are now a thing
  - Dynamic was fully introduced
  - You can first load the VM with a global Bytecode before running
  - Functions are declarable

## API
  - Added loading Bytecode memory and files
  - You can add Global names and write to them