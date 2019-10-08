#include <Windows.h>
#include <crtdbg.h>
#include <stdlib.h>

#include "server.h"

#define _CRTDBG_MAP_ALLOC

void ClientConnected(LPVOID Argument)
{
	REQUEST* Request = NULL;
	
	ARGUMENT* Arg = (ARGUMENT*)Argument;
	HANDLE hPipe = *(Arg->phPipe);
	HANDLE hMessagePipe = *(Arg->phMessagePipe);
	HANDLE hFileSenderThread = NULL;
	
	DWORD dwBytesRead;
	WORD wResponse;
	UNREFERENCED_PARAMETER(wResponse);

	while (TRUE)
	{
		Request = (REQUEST*)calloc(1, sizeof(REQUEST));
		Request->bArgument = (BYTE*)calloc(255, sizeof(BYTE));
		Request->bTypeOfRequest = -1;
		Request->wArgumentSize = -1;
		Request->hPipeToReply = hPipe;


		if (ReadFile(hPipe, &(Request->bTypeOfRequest), sizeof(BYTE), &dwBytesRead, NULL) == FALSE)
		{
			printf("Unexpected error: A problem occured while communicating with a client (error code: %d)\n", GetLastError());
			goto CleanUp;
		}
		
		switch (Request->bTypeOfRequest)
		{
			case ECHO:
			{
				if (ServerEcho((LPVOID)&hPipe) != SUCCESS)
					printf(__TEXT("Unexpected error: echo failed\n"));
				break;
			}

			case REGISTER:
			{
				if (ServerRegister((LPVOID)&hPipe) != SUCCESS)
					printf(__TEXT("Unexpected error: register failed\n"));
				break;
			}

			case LOGIN:
			{
				ARGUMENT* arg = (ARGUMENT*)calloc(1, sizeof(ARGUMENT));
				arg->phPipe = &hPipe;
				arg->phMessagePipe = &hMessagePipe;

				if (ServerLogin((LPVOID)arg) != SUCCESS)
					printf(__TEXT("Unexpected error: login failed\n"));

				free(arg);
				break;
			}

			case LOGOUT:
			{
				if (ServerLogout((LPVOID)&hPipe) != SUCCESS)
					printf(__TEXT("Unexpected error: logout failed\n"));
				break;
			}

			case MSG:
			{
				if (ServerMsg((LPVOID)&hPipe) != SUCCESS)
					printf(__TEXT("Unexpected error: messaging failed\n"));
				break;
			}

			case BROADCAST:
			{
				if (ServerBroadcast((LPVOID)&hPipe) != SUCCESS)
					printf(__TEXT("Unexpected error: broadcasting a message failed\n"));				
				break;
			}

			case SENDFILE:
			{
				if (ServerSendfile((LPVOID)&hPipe) != SUCCESS)
					printf(__TEXT("Unexpected error: sending the file failed\n"));
				break;
			}

			case LIST:
			{
				if (ServerList((LPVOID)&hPipe) != SUCCESS)
					printf(__TEXT("Unexpected error: list failed\n"));
				break;
			}

			case EXIT:
			{
				if (ClientLoggedIn(NULL, hPipe) == TRUE)
					LogoutRemoveByPipeHandle(hPipe);
				EnterCriticalSection(&gCriticalSection);
				gdwClientsConnected--;
				LeaveCriticalSection(&gCriticalSection);

				if (Request->bArgument != NULL)
					free(Request->bArgument);
				if (Request != NULL)
					free(Request);
				goto CleanUp;
			}

			case HISTORY:
				break;

		}

		if (Request->bArgument != NULL)
			free(Request->bArgument);
		if (Request != NULL)
			free(Request);
	}

CleanUp:
	
	CloseHandle(hPipe);
	CloseHandle(hMessagePipe);
	
	free(Arg);

	TerminateThread(GetCurrentThread(), 0);
}

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		printf(__TEXT("Unexpected error: Invalid number of arguments\n"));
		return INVALID_PARAMETER;
	}

	gdwMaxClients = atoi(argv[1]);
	if (gdwMaxClients == 0)
	{
		printf(__TEXT("Error: invalid maximum number of connections\n"));
		return INCORRECT_PARAMETER;
	}

	gpThreadPool = NULL;
	DWORD dwBytesRead = 0, dwBytesWritten = 0;
	BOOL bResult;
	HANDLE* phDupPipe = NULL;
	HANDLE* phDupMessagePipe = NULL;
	WORD wResponse = TRUE;
	ghRegistrationFile = INVALID_HANDLE_VALUE;

	if (RegistrationFileInitialize() == FILE_CREATE_FAILED)
	{
		printf("Unexpected error: creating the registration file failed\n");
		goto End;
	}

	//if (MessageHistoryFileInitialize() == FILE_CREATE_FAILED)
	//{
	//	printf("Unexpected error: creating the registration file failed\n");
	//	goto End;
	//}

	if (ClientVectorInitialize() < 0)
	{
		printf(__TEXT("Unexpected error: creating the client vector failed\n"));
		goto End;
	}

	if (ThreadpoolCreateEx(&gpThreadPool, (BYTE)gdwMaxClients) < 0)
	{
		printf("Unexpected error: Threadpool creation failed\n");
		return INNER_FUNC_ERROR;
	}

	printf("Success\n");

	while (TRUE)
	{
		ghPipe = CreateNamedPipeA(PIPE_NAME, PIPE_ACCESS_DUPLEX, PIPE_TYPE_MESSAGE, PIPE_UNLIMITED_INSTANCES, 4096, 4096, 0, NULL);
		if (ghPipe == INVALID_HANDLE_VALUE)
		{
			printf("Unexpected error: creating an instance of the pipe failed (error code: %d)\n", GetLastError());
			return INNER_FUNC_ERROR;
		}
		
		ghMessagePipe = CreateNamedPipeA(PIPE_MESSAGE_NAME, PIPE_ACCESS_DUPLEX, PIPE_TYPE_MESSAGE | PIPE_WAIT, PIPE_UNLIMITED_INSTANCES, 4096, 4096, 0, NULL);
		if (ghMessagePipe == INVALID_HANDLE_VALUE)
		{
			printf("Unexpected error: creating an instance of the pipe failed (error code: %d)\n", GetLastError());
			return INNER_FUNC_ERROR;
		}
		
		bResult = ConnectNamedPipe(ghPipe, NULL);
		if (bResult == FALSE)
		{
			printf("Unexpected error: failed to connect with a client (error code: %d)\n", GetLastError());
			return INNER_FUNC_ERROR;
		}
		
		phDupPipe = (HANDLE*)calloc(1, sizeof(HANDLE));
		phDupMessagePipe = (HANDLE*)calloc(1, sizeof(HANDLE));

		EnterCriticalSection(&gCriticalSection);
		if (gdwClientsConnected == gdwMaxClients) 
		{
			// Maximum number of clients 
			// connected already reached
			LeaveCriticalSection(&gCriticalSection);
			wResponse = MAX_CONNECTION_COUNT_REACHED;
			if (WriteFile(ghPipe, &wResponse, sizeof(WORD), &dwBytesWritten, NULL) == FALSE)
				printf("Unexpected error: declining the client's connection failed (max number of connection reached; Error code: %d)\n", GetLastError());
			continue;
		}
		else
		{
			gdwClientsConnected++;
			LeaveCriticalSection(&gCriticalSection);
			wResponse = SUCCESSFUL_CONNECTION;
			if (WriteFile(ghPipe, &wResponse, sizeof(WORD), &dwBytesWritten, NULL) == FALSE)
			{
				printf("Unexpected error: accepting the client's connection failed (error code: %d)\n", GetLastError());
				return -1;
			}
		}

		bResult = ConnectNamedPipe(ghMessagePipe, NULL);
		if (bResult == FALSE)
		{
			printf("Unexpected error: failed to connect with a client (error code: %d)\n", GetLastError());
			return INNER_FUNC_ERROR;
		}
		
		if (DuplicateHandle(GetCurrentProcess(), ghPipe, GetCurrentProcess(), phDupPipe, 0, TRUE, DUPLICATE_SAME_ACCESS) == FALSE)
		{
			printf("Unexpected error: duplicating handle failed (error code: %d)\n", GetLastError());
			return INNER_FUNC_ERROR;
		}
		if (DuplicateHandle(GetCurrentProcess(), ghMessagePipe, GetCurrentProcess(), phDupMessagePipe, 0, TRUE, DUPLICATE_SAME_ACCESS) == FALSE)
		{
			printf("Unexpected error: duplicating handle failed (error code: %d)\n", GetLastError());
			return INNER_FUNC_ERROR;
		}

		ARGUMENT* Arg = (ARGUMENT*)calloc(1, sizeof(ARGUMENT));
		Arg->phPipe = phDupPipe;
		Arg->phMessagePipe = phDupMessagePipe;		

		WORK_ITEM* WorkItem = (WORK_ITEM*)calloc(1, sizeof(WORK_ITEM));
		WorkItem->FunctionAddress = &ClientConnected;
		WorkItem->FunctionArgument = (LPVOID)Arg;
		
		ThreadpoolAddWorkItem(WorkItem);

		Sleep(20);
		CloseHandle(ghPipe);
		CloseHandle(ghMessagePipe);
	}

End:

	if (ghRegistrationFile != INVALID_HANDLE_VALUE)
		CloseHandle(ghRegistrationFile);

	//if (ghMessagesHistoryFile != INVALID_HANDLE_VALUE)
	//	CloseHandle(ghMessagesHistoryFile);

	if (ghPipe != INVALID_HANDLE_VALUE)
		CloseHandle(ghPipe);

	if (ghMessagePipe != INVALID_HANDLE_VALUE)
		CloseHandle(ghMessagePipe);
	
	ClientVectorDestroy();

	ThreadpoolDestory(&gpThreadPool);

	return SUCCESS;
}