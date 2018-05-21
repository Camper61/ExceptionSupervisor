# ExceptionSupervisor
This library implements an additional chain that receives every thrown exception before the SEH or VEH chain does. It can be used to prevent an exception from being caught by normal exception handlers entirely; it can also be used to hide or modify exception data before proceeding to the other exception handlers set by your or another program.

# Usage
Simply call `AddSupervisorExceptionHandler`, passing your exception handler (accepting a `PEXCEPTION_RECORD` and `PCONTEXT` struct as arguments) as its argument (you can add an infinite amount of exceptiono handlers, similar to VEH.) Your exception handler should function the same way as a normal exception handler, returning proper values such as `EXCEPTION_CONTINUE_EXECUTION` and `EXCEPTION_CONTINUE_SEARCH`, which will perform their expected behavior. Take note that if the instruction perpetually errors, you will enter an exception loop until you adjust the EIP register to skip over the instruction. Once all handlers have been registered, you can call `EnableExceptionSupervisor`, which will activate exception supervising.

### Compatibility
To use this library with different Windows versions, you'll have to disassemble ntdll.dll, take a look at KiUserExceptionHandler and adjust offsets in the main .cpp file. Currently, this version was built for **Windows 10**, and it is unsure it will work on precedent or newer Windows versions.
