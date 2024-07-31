#pragma once
#ifdef __cplusplus
extern "C" {
#endif
    namespace bof::mock::syscall {
        NTSTATUS ExecNtApi(...);
    }
#ifdef __cplusplus
}
#endif