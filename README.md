Honestly, I wouldn't attempt to run this personally. 

I wrote this before I knew how to write a proper makefile, so it's a little messy. The things I know are worth changing: 

1) the makefile could be so much better. Make should build the executables and make clean should remove them
2) is there a way to manage dependencies in C++?
3) the command line arg interpreter is... I mean there are tools to do that for me.
There are definitely a lot of little things.

The reason for all the different executables is because, at some point I had them all running in the same executable in sequence. This was causing issues that I suspect came from the OS. It seemed like things weren't being properly cleaned up between runs, and so there would be partially destructed threads that were created by one library that another was trying to claim (or something like that).
