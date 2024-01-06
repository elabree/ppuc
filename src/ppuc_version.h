#pragma once

#define PPUC_EXECUTABLE_VERSION_MAJOR 0 // X Digits
#define PPUC_EXECUTABLE_VERSION_MINOR 2 // Max 2 Digits
#define PPUC_EXECUTABLE_VERSION_PATCH 0 // Max 2 Digits

#define _PPUC_EXECUTABLE_STR(x)       #x
#define PPUC_EXECUTABLE_STR(x)        _PPUC_EXECUTABLE_STR(x)

#define PPUC_EXECUTABLE_VERSION                                                                                        \
    PPUC_STR(PPUC_EXECUTABLE_VERSION_MAJOR)                                                                            \
    "." PPUC_STR(PPUC_EXECUTABLE_VERSION_MINOR) "." PPUC_STR(PPUC_EXECUTABLE_VERSION_PATCH)
#define PPUC_EXECUTABLE_MINOR_VERSION                                                                                  \
    PPUC_STR(PPUC_EXECUTABLE_VERSION_MAJOR) "." PPUC_STR(PPUC_EXECUTABLE_VERSION_MINOR)
