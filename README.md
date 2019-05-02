# Exception Supervisor
This library hooks the Windows exception handler and redirects thrown exceptions to user-defined functions. It was originally designed to intercept exceptions and modify their data before passing it to the original exception handlers, but it can also be used for debugging or hooking purposes.

# Usage
Call `AddSupervisorExceptionHandler`, passing your exception handler as its sole argument. Your exception handler should function the same way as a normal exception handler, returning status codes such as `EXCEPTION_CONTINUE_EXECUTION` and `EXCEPTION_CONTINUE_SEARCH` (their behaviors rest unmodified). Take note that if the instruction perpetually errors, you will enter an exception loop until you adjust the EIP register to skip over the instruction. Once all handlers have been registered, you can call `EnableExceptionSupervisor`, which will activate exception supervising. I don't personally recommend adding handlers after the supervisor has been enabled.

### Compatibility
This library will only work on **Windows 10** until someone modifies it to work with other Windows versions. Enabling support for Windows 7/Windows Vista should theoretically be easy since it should only be a matter of adjusting offsets. If I have time, I'll look into it.