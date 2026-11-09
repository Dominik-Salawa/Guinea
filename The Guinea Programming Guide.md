<h1 style="justify-content:center">The Guinea Programming Language</h1>

Hello, welcome to *The Guinea Programming Language Guide* by Dominik Salawa which covers all of Guineas quirks and behaviors. Guinea is designed to be a High-Level programming language,which ships the simplicity, transparency, and readability of C, but, keeps the safety of Python and the embeddability/portability of Lua.

Guinea is designed to be unique and separate from it's High-Level relatives, it has built-in fat pointer system which allows you to reference/copy memory across the program and you can be 100% certain it's a copy, or a reference. It is interpreted via Bytecode, and relies on the Guinea Virtual Machine in order to run.

<h1 id="#main()-how-the-program starts">main(): how the program starts</h1>
Before we get into Guinea, this is how you print "Hello, World!":

```Guinea
include <stdlib/io>

func main(void): int
{
	Io::printf("Hello, World!\n")
	return 0
}
```

Guinea has a hard separation between the Global, and Local scopes. So, the program starts at a variable, in the general Guinea CLI, this is the `main` variable, it has to be a function returning an integer or returns nothing also known as a void function (in other words: `func int` or `func void`) variable. Only declaration, file includes, and namespacing are allowed in the Global scope, meanwhile the Local scope allows all statements but file including. For more info on scoping laws, check out *<a href="#global-local-scopes-and-macros">Global/Local Scopes, and macros</a>*.

All programs run by the general guinea CLI *must* have a `main` starting point. When the program starts via the guinea CLI, it is guaranteed to have a (in order):
 - string array to the arguments: `{string}`
 - string array to the env `{string}`

The Guinea CLI *will* reject running the program if the main function does not return an integer or is not void. Returning an integer is the code on which the program will exit; if it's void, the CLI will exit with code 0 by default if there were no runtime errors, else, will exit with code 1.

If the first 2 parameters do not follow (in order) of the datatypes of the provided parameters, then it will throw an error; functions, in general, do not need to handle all possible arguments in a function call, which is why just doing main(const argv: {string}) or main(void) are entirely legal parameters. More info on function behaviors with arguments are shown in *<a href="#functions-behaviors-and-declarations">Function behaviors and declarations</a>*.


The full version of main is:

```Guinea
func main(const argv: {string}, const env: {string}): int
{
	...
}

or 

func main(const argv: {string}, const env: {string}): void
{
	...
}
```
**NOTE: none of the parameters have to be const, they can be mutable if you want to but I'd frown upon doing so.*




<h1 id="control-flow">Control flow</h1>

Control flows are the way to compute special behaviors in Guinea. A `true expression` must result in a *non-nil* value, *and* is *not a false:boolean* value. *Empty arrays*, *empty strings*, and *0:int* **do *not*** result in a false expression, as they are *not* nil nor false.<br>

**Please note:** { ... } is called a block, which is defined as a statement. `if (true) x = 5` is an entirely OK statement.

Guinea currently has 3 ways to handle control flows:

<h3>If statements</h3>

```Guinea
if (expression)
	statement
elif (expression)
	statement

... // else if chain

else 
	statement
```

If the expression is true, then it will execute the statement, else, it will hop onto the next `else` or `elif` until it hits a true branch, or the `else` branch. You cannot declare an else, and then another else if, it is illegal to do so.

---

<h3>While statements</h3>

```Guinea
while (expression)
	statement
```

If the expression results in a true `statement`. It will execute the `statement`, it will then re-evaluate the `expression`, and repeat until it's `expression` is false. There are *no* else branches.

It allows both `continue` and `break`. Where break completely breaks the loop and continues executing the program, while continue goes back to the evaluation for whether it should execute the `statement`.

---

<h3>For statements</h3>

```Guinea
for (statement_1; expression; statement_2)
	statement_3
```

It will run the first statement, and run

It will keep running both statement 2,3 until the expression is false. For allows both `break` and `continue` keywords. `break` will skip `statement_2` and continue running the program. `continue` will run `statement_2` before returning to the evaluation stage and deciding if it has to execute `statement_3` again.

Guinea does not care what `statement_1` or `statement_2` is. It does not have to be a declaration, nor does it have to be an expression. Even though Guinea does not need semi-colons[;], in this case, it is required for the separations of `statement_1`, `expression`, and `statement_2`, and for readability. You do want a project you can actively read and maintain, don't you?




<h1 id="expressions">Expressions</h1>

Expressions are the way we can evaluate and assign variables. Unlike Python or other languages, an expression as a statement, or is a part of a statement, follows the same set of rules, but can differ slightly in terms of optimisation, use and assignment.

Precedence is the order of execution, it follows the standard **PEMDAS** order **(with the sole exception of power and negation, please use parenthesis when dealing with expressions like `-5^2` for no unexpected behaviors)**, in expressions, here is the order from first to last:

```
(...)                      <- parenthesis
f(x) a[i] a.i              <- function calls,
                              array indexing,
							  field fetching
! - & @                    <- not, negation,
                              addressing,
                              dereferencing,
^                          <- pow
* / %                      <- mul, div, mod
+ -  					   <- add, sub
== != > < >= <= <>         <- equ, not equ, gt, 
                              lt, gt equ, lt equ, 
                              lt or gt
&& ||                      <- and, or
=                          <- assignment
```

An example would be:
``` Guinea
-a[i]^2 == 15 && f(x).i == true
```
**In english, whether element i of a, negated, at the power of 2, equals to 15, and, f of x at the field i is true.*

<h3 id="expression-statements">Expression Statements</h3>

Expression Statements are everywhere, they range from `add(4,6)` all the way to `x = 5`. Statements like `16`—yes *just* `16`—are entirely legal as a statement, but will be optimised away and never actually executed. For standalone function calls, when you return a value or an object, but it isn't being assigned. It will be swept away as well. Void functions return nothing, so they cannot be used mathematically.

```Guinea
x = 5 + 6 * 7 /* valid */
15 + 5        /* valid, but optimised away */
16            /* valid, but optimised away */
-(x = f(x)^2) /* valid, if f is not a void function, 
                 it will not negate x's value */
5 + f(x)	  /* valid, unless f is a void function, 
                 5 is optimised away so it's a 
			     standalone f(x) call */
f(x)          /* valid, even if f(x) is void, no 
                 worry of a memory leak */
f(x) == f(47) /* valid if f(x) is not void, and will 
                 be optimised to just 2 standalone 
				 function calls */
```
**NOTE: No matter if it's an assignment expression, or an expression doomed to being optimised away, the Guinea Compiler will still nevertheless reject your code if the function is void and is in an operation such as +, -, /, \*, etc...*

<h3 id="expressions-in-a-statement">Expressions in a Statement</h3>

<h3 id="if-statements-in-expressions">If statements in Expressions</h3>


<hr>

<h1>The Guinea Programming Language</h1>

<h1>Intro to Beginner Guinea</h1>
<h1>main(): how the program starts</h1>
<h1>Control flow</h1>
	<h2>If statements</h2>
	<h2>While statements</h2>
	<h2>For statements</h2>
<h1>Expression</h1>
	<h2>Expressions in a Statement</h2>
	<h2>Expression Statements</h2>
	<h2>If statements in Expressions</h2>
<h1>Datatypes, Variables and Arrays</h1>
	<h2>Core datatypes in Guinea</h2>
	<h2>Variable declaration</h2>
	<h2>Arrays</h2>
<h1>Function behaviors and declarations</h1>
	<h2>Function behaviors</h2>
	<h2>Declaring Functions</h2>
<h1>Pointers, Allocation and Garbage Collection</h1>
	<h2>Pointers and behaviors</h2>
	<h2>Allocating memory</h2>
	<h2>Garbage Collection</h2>
<h1>Classes and Object Oriented Programming</h1>
	<h2>Declaration and Access Specifiers</h2>
	<h2>Constructers and Deconstructors</h2>
	<h2>Special Methods</h2>
<h1>The standard library (stdlib)</h1>

<h1>Intro to Advanced Guinea</h1>
<h1>Macros, and the Global/Local scope</h1>
	<h2>Macros</h2>
		<h3>include: stdlib and libraries</h3>
		<h3>define</h3>
		<h3>ifdef</h3>
		<h3>ifndef</h3>
		<h3>Examples of Macros</h3>
	<h2>Global Scope</h2>
	<h2>Local Scope</h2>
<h1>Foreign Function Interface in Guinea: Making Libraries</h1>
	<h2>Main Entry Point: Declaring what you're importing</h2>
		<h3>Defining Variables</h3>
		<h3>Defining C and C++ Function objects in Guinea</h3>
		<h3>Defining Class Objects</h3>
	<h2>GC and GC Objects</h2>
		<h3>Constructors and Deconstructors</h3>
		<h3>Responsibilities</h3>
		<h3>Adding GC Objects to a GC</h3>
	<h2>Making types</h2>
		<h3>Constructors and Deconstructors</h3>
		<h3>Special Methods</h3>
		<h3>Using GC Objects</h3>
	<h2>Raising Exceptions</h2>
	<h2>Examples: How string datatype actually looks in Guinea</h2>
<h1>Foreign Function Interface in Guinea: Embedding in C and C++</h1>
	<h2>Embedding</h2>
	<h2>Initialising, running and destroying VM instances</h2>
		<h3>Init and Destroy</h3>
		<h3>Adding Variables and Checking Variable information</h3>
		<h3>Calling Functions (running from a starting point)</h3>
			<h4>Getting return type</h4>
			<h4>Running a VM instance</h4>
			<h4>Raising Exceptions</h4>
	<h2>Manipulating the VM stack and GC behavior</h2>
	<h2>Special APIs</h2>