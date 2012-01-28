Here's how the automated test suite for the debugger engine works:

1) The autotest.so plug-in is built.

2) The test shell scripts writes out some C++ code,
   and an autotest script. The C++ compiler is invoked.

3) The language for the autotest script has an ad-hoc grammar that I
   made up. Basically, it allows one to call debugger commands, and
   compare the output with an expected result. See the example file.

4) The test shell scripts invoke the debugger on C++ compiled programs;
   the autotest plugin is loaded, and it responds to the first on_event
   notification, indicating to the debugger that it is going to take
   over the user-interaction. 

   From this point on, instead of interacting with the user via the 
   command line prompt, the debugger executes the commands given to it
   by the autotest plugin. The autotest plugin reads the commands from
   the autotest script, that was generated at step 2).

5) Debugger outputs that don't match the expected output (as specified by
   the script) are recorded as test failures.


To run a test individually, invoke the shell script with the --run argument.

