.data
    NtApiAddr QWORD 0h

.code

    SetNtApiAddr proc
        mov NtApiAddr, rcx
        ret
    SetNtApiAddr endp

end