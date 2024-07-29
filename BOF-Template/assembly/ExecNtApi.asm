.data 
    NtApiAddr QWORD 0h

.code

    ExecNtApi proc ; Takes in SYSCALL_API_ENTRY
        jmp qword ptr [NtApiAddr]
        ret

    ExecNtApi endp
end