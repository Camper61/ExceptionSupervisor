
#include <Windows.h>
#include <iostream>
#include "../ExceptionSupervisor/ExceptionSupervisor.hpp"
#pragma comment(lib, "../Release/ExceptionSupervisor.lib")

LONG WINAPI ExampleSupervisor(PEXCEPTION_RECORD pExceptionRecord, PCONTEXT pContext)
{
	std::cout << "[#] Caught exception (" << std::hex << pExceptionRecord->ExceptionCode << ") at EIP=0x" << std::hex << pContext->Eip << std::endl;
	pContext->Eip += 10; /* skip over instruction to prevent error rethrow */
	return EXCEPTION_CONTINUE_EXECUTION;
}

int main()
{
	std::cout << "[#] Applying ExceptionSupervisor... ";
	AddSupervisorExceptionHandler(ExampleSupervisor);
	if (!EnableExceptionSupervisor())
	{
		std::cout << "FAILURE";
		return -1;
	}
	std::cout << "SUCCESS\n";
	__try
	{
		volatile int* px = nullptr;
		*px = 0;
	}
	__except (GetExceptionInformation(), GetExceptionCode())
	{
		/* This should never get executed, as our supervisor returns EXCEPTION_CONTINUE_EXECUTION */
		std::cout << "Our exception was caught, this should not happen!\n";
		return -1;
	}
    return 0;
}

