This is the code for the builtin expression interpreter, so
that C-like expressions can be evaluated from within the debugger.

I have started with a yacc ANSI-C grammar specification and stripped off
the declarations and (for now) the statements. The expressions should use
variables and functions already declared in the program that is debugged;
I don't think there's really a need for supporting declarations of temp
variables, nor if/else/switch/for statements and so forth.

The valuable and interesting parts (in my opinion) are: pointer arithmetics,
calling functions inside of the debugee, and casting.

For calling functions in the debugged program, I plan to do something along this
line: save the user area of the debugged program (registers, etc). Setup a new
stack (how do I do this is not clear at this point); verify the parameter types
and push them on this new stack, and finally force the instruction pointer to 
calling the function. There should be some sort of breakpoint on this stack so
that when the function returns, the breakpoint is hit and it thus tells the
debugger to restore the user area, and all the things it has altered in the
program.

Now, an interesting aspect is when we force the calling of a function func()
that has a breakpoint somewhere in the middle. At this point, the user may want
to evaluate another expression (that may contain function calls) -- so I should
maintain some sort of stack of expressions pending evaluation...

NOTE: most of the mumbo-jumbo about calling functions should be hidden behind
and implementation of the Context interface. At this point, the Context mainly
provides a way for the intepreter to look up symbols by name, but it will be
extended to handle more stuff.
