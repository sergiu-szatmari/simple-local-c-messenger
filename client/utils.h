#ifndef _UTILS_H_
#define _UTILS_H_

#include <Windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "domain_client.h"
#include "return.h"
#include "commands.h"

DWORD InputValidation(_In_ BYTE** string);
BOOL CommandExists(_In_ BYTE* Command);
void GetCommand(_In_ BYTE** Input, _Out_ BYTE** Command, _Out_ BYTE** Rest);
BYTE GetCommandType(_In_ BYTE* Input);
void ValidateCommand(_Inout_ BYTE** bCommand);
BOOL SingleWordCommand(_In_ BYTE* bCommand);

#endif _UTILS_H_