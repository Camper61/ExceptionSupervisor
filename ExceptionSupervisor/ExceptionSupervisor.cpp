
/*
*
*	EXCEPTION SUPERVISOR
*	https://github.com/LoukaMB/ExceptionSupervisor
*
*/

#include "ExceptionSupervisor.hpp"
#include <vector>

std::vector<SupervisorExceptionHandler> Supervisors;	/* stores our supervisor exception handlers */
DWORD _ResumeKiUserExceptionDispatcher;	/*	if no supervisor caught the exception (EXCEPTION_CONTINUE_SEARCH), its important 
											to resume to the rest of KiUserExceptionDispatcher to let SEH and VEH occur naturally */
PVOID _ZwContinue;						/*	location of the ZwContinue function, used by KiUserExceptionDispatcher */
DWORD _ResumeAfterZwContinue;			/*	if ZwContinue errors (an exception has occured), we must resume after the
											ZwContinue call in KiUserExceptionDispatcher by jumping to it (this contains the address) */

LONG WINAPI GeneralSupervisorExceptionHandler(PEXCEPTION_RECORD pExceptionRecord, PCONTEXT pContext)
{
	for (SupervisorExceptionHandler Handler : Supervisors)
		if (Handler(pExceptionRecord, pContext) == EXCEPTION_CONTINUE_EXECUTION) return TRUE;
	return FALSE; /* execute normal handlers */
}

/*	KiUserExceptionDispatcher is the function that handles EVERY exception (SEH and VEH) going on in
*	your program. By hooking it, we can essentially catch every exception thrown. The function receives 
*	two arguments, namely an exception record and a CONTEXT struct. we'll simply pass that to our exception handlers */

__declspec(naked) void NTAPI H_KiUserExceptionDispatcher_x86()
{
	static PEXCEPTION_RECORD pExceptionRecord;
	static PCONTEXT pContext;

	__asm
	{	
		mov	ebx, [esp]
		mov	ecx, [esp + 4]
		mov	pExceptionRecord, ebx
		mov	pContext, ecx
	}

	if (GeneralSupervisorExceptionHandler(pExceptionRecord, pContext) == EXCEPTION_CONTINUE_EXECUTION)
	{
		__asm
		{
			mov		ecx, [esp + 4]
			push	0
			push	ecx
			call	_ZwContinue
			jmp		_ResumeAfterZwContinue
		}
	}

	__asm
	{
		cld
		mov	ecx, [esp + 4]
		jmp	_ResumeKiUserExceptionDispatcher
	}
}

inline PVOID ExportLookup(const char* DllName, const char* FnName)
{
	return reinterpret_cast<PVOID>(GetProcAddress(GetModuleHandleA(DllName), FnName));
}

BOOL PlaceHook(DWORD Where, PVOID JumpTo)
{
	DWORD nOldProtect = 0;
	if (!VirtualProtect(reinterpret_cast<LPVOID>(Where), 5, PAGE_EXECUTE_READWRITE, &nOldProtect))
		return FALSE;
	*reinterpret_cast<BYTE*>(Where) = 0xE9;
	*reinterpret_cast<DWORD*>(Where + 1) = reinterpret_cast<DWORD>(JumpTo) - Where - 5;
	if (!VirtualProtect(reinterpret_cast<LPVOID>(Where), 5, nOldProtect, &nOldProtect))
		return FALSE;
	return TRUE;
}

VOID AddSupervisorExceptionHandler(SupervisorExceptionHandler ExceptionHandler)
{
	Supervisors.push_back(ExceptionHandler);
}

BOOL EnableExceptionSupervisor()
{
	const auto KiUserExceptionDispatcher = reinterpret_cast<DWORD>(ExportLookup("ntdll.dll", "KiUserExceptionDispatcher"));
	_ZwContinue = ExportLookup("ntdll.dll", "ZwContinue");
	if (!KiUserExceptionDispatcher || !_ZwContinue)
		return FALSE;
	_ResumeAfterZwContinue = KiUserExceptionDispatcher + 0x41;
	_ResumeKiUserExceptionDispatcher = KiUserExceptionDispatcher + 0x1C;
	return PlaceHook(KiUserExceptionDispatcher + 0x17, H_KiUserExceptionDispatcher_x86);
}
