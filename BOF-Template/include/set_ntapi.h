//
// Created by 0xtriboulet on 7/29/2024.
//
#ifndef BOF_TEMPLATE_INCLUDE_SET_NTAPI_H
#define BOF_TEMPLATE_INCLUDE_SET_NTAPI_H

#pragma once
#ifdef __cplusplus
extern "C" {
#endif
    namespace bof::mock::syscall {
        VOID SetNtApiAddr(PVOID ntapi_addr);
    }
#ifdef __cplusplus
}
#endif

#endif //BOF_TEMPLATE_INCLUDE_SET_NTAPI_H
