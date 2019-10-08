#ifndef _CLIENT_H_
#define _CLIENT_H_

#include <Windows.h>
#include <stdlib.h>
#include <stdio.h>

#include "domain_client.h"
#include "commands.h"

void MyStrCpy(_Inout_ BYTE* bDestination, _In_ BYTE * bSource, _In_ DWORD wCount);
void GetFileName(_In_ BYTE* bPath, _Out_ BYTE** bFilename);
void GetCurrentFolder(_In_ BYTE* bPath, _Out_ BYTE** bPathToCurrentFolder);

DWORD WINAPI ClientWaitForMessages(_In_ LPVOID Argument);
DWORD WINAPI ClientSendFile(_In_ LPVOID Argument);

#endif _CLIENT_H_