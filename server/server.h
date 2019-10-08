#ifndef _SERVER_H_
#define _SERVER_H_

#include <Windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "domain_server.h"
#include "threadpool.h"

void MyStrCpy(_Inout_ BYTE* bDestination, _In_ BYTE* bSource, _In_ DWORD wCount);
int GetUsernamePassword(_In_ BYTE* bArgument, _In_ WORD wArgumentSize, _Out_ BYTE** bUsername, _Out_ BYTE** bPassword);
void WhoAmI(_In_ HANDLE hPipe, _Out_ BYTE** bUsername);
WORD WriteMessageToHistory(_In_ BYTE* bSender, _In_ BYTE* bRest);

BYTE ClientVectorInitialize(void);
BYTE ClientVectorDestroy(void);

WORD RegistrationFileInitialize(void);
WORD MessageHistoryFileInitialize(void);
void RegisterSplit(_In_ BYTE* bInput, _Out_ BYTE** bUsername, _Out_ BYTE** bPassword);
WORD RegisterValidation(_In_ BYTE* bInput);
WORD RegisterCheckFile(_In_ BYTE* bArgument);
WORD RegisterStoreFile(_In_ BYTE* bInput);

void ShowLoggedInUsers(void);

WORD LoginValidation(_In_ BYTE* bUsername, _In_ BYTE* bPassword);
WORD LoginAdd(_In_ BYTE* bUsername, _In_ HANDLE hPipe, _In_ HANDLE hMessagePipe);
BOOL LoginCheck(_In_ BYTE* bUsername);

WORD LogoutRemoveByUsername(_In_ BYTE* bUsername);
WORD LogoutRemoveByPipeHandle(_In_ HANDLE hPipe);

BOOL ClientLoggedIn(_In_opt_ BYTE* bUsername, _In_opt_ HANDLE hPipe);
BOOL ClientExists(_In_ BYTE* bUsername);
WORD GetClientByPipe(_In_ HANDLE hPipe, _Out_ BYTE** bUsername);

WORD ServerEcho(_In_ LPVOID Argument);
WORD ServerRegister(_In_ LPVOID Argument);
WORD ServerLogin(_In_ LPVOID Argument);
WORD ServerLogout(_In_ LPVOID Argument);
WORD ServerMsg(_In_ LPVOID Argument);
WORD ServerBroadcast(_In_ LPVOID Argument);
WORD ServerSendfile(_In_ LPVOID Argument);
WORD ServerList(_In_ LPVOID Argument);

DWORD WINAPI ThreadSendFile(_In_ LPVOID Argument);

#endif _SERVER_H_
