#include <Windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <crtdbg.h>

#include "domain_client.h"
#include "return.h"
#include "utils.h"
#include "client.h"

#define _CRTDBG_MAP_ALLOC

int main(void)
{
	DWORD dwBytesWritten = 0, dwBytesRead = 0, dwReturnCode = SUCCESS;
	WORD wResponse, wArgumentSize, wReplySize;
	BYTE* bInput = NULL;
	BYTE* bCommand = NULL;
	BYTE* bArgument = NULL;
	BYTE* bReply = NULL;
	BYTE bTypeOfRequest;	
	HANDLE hMessageThread = NULL;
	HANDLE hFileSenderThread = NULL;

	InitializeCriticalSection(&gCriticalSection);

	ghPipe = CreateFileA(PIPE_NAME, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (ghPipe == INVALID_HANDLE_VALUE)
	{
		printf(__TEXT("Error: no running server found\n"));
		return PIPE_CREATE_FAILED;
	}
	
	if (ReadFile(ghPipe, &wResponse, sizeof(WORD), &dwBytesRead, NULL) == FALSE)
	{
		printf(__TEXT("Unexpected error: communication with the server failed (error code: %d)\n"), GetLastError());
		return PIPE_READ_FAILED;
	}

	if (wResponse == MAX_CONNECTION_COUNT_REACHED)
	{
		printf(__TEXT("Error: maximum concurrent connection count reached\n"));
		return -1;
	}
	else if (wResponse == SUCCESSFUL_CONNECTION)
	{
		printf(__TEXT("Successful connection\n"));
		gUserLoggedIn = FALSE;
		bInput = (BYTE*)calloc(255, sizeof(BYTE));
		bCommand = (BYTE*)calloc(255, sizeof(BYTE));
		bArgument = (BYTE*)calloc(255, sizeof(BYTE));

		if (bInput == NULL || bCommand == NULL || bArgument == NULL)
		{
			printf(__TEXT("Unexpected error: memory allocation failed (error code: %d)\n"), GetLastError());
			dwReturnCode = MALLOC_FAILURE;
			goto CleanUp;
		}
		
		ghMessagePipe = CreateFileA(PIPE_MESSAGE_NAME, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
		if (ghMessagePipe == INVALID_HANDLE_VALUE)
		{
			printf(__TEXT("Unexpected error: connection of the message pipe with the server failed (error code: %d)\n"), GetLastError());
			return PIPE_CREATE_FAILED;
		}
		
		hMessageThread = CreateThread(NULL, 0, &ClientWaitForMessages, NULL, 0, NULL);
		if (hMessageThread == NULL)
		{
			printf(__TEXT("Unexpected error: thread creating failed (error code: %d)\n"), GetLastError());
			return THREAD_CREATE_ERROR;
		}
		Sleep(20);

		while (TRUE)
		{
			memset(bInput, 0, 255);
			fgets(bInput, 254, stdin);
			InputValidation(&bInput);
			if (strlen(bInput) == 0)
				continue;
			GetCommand(&bInput, &bCommand, &bArgument);
			
			if (CommandExists(bCommand) == FALSE)
			{
				printf(__TEXT("Unexpected error: command does not exit\n"));
				continue;
			}
			ValidateCommand(&bCommand);
			bTypeOfRequest = GetCommandType(bCommand);

			
			if (WriteFile(ghPipe, &bTypeOfRequest, sizeof(BYTE), &dwBytesWritten, NULL) == FALSE)
			{
				printf(__TEXT("Unexpected error: a problem occured while communicating with the server (error code: %d)\n"), GetLastError());
				dwReturnCode = PIPE_WRITE_FAILED;
				goto CleanUp;
			}
			
			switch (bTypeOfRequest)
			{
				case ECHO:
				{
					wArgumentSize = (WORD)strlen(bArgument) + 1;
					if (WriteFile(ghPipe, &wArgumentSize, sizeof(WORD), &dwBytesWritten, NULL) == FALSE)
					{
						printf(__TEXT("Unexpected error: a problem occured while communicating with the server (error code: %d)\n"), GetLastError());
						dwReturnCode = PIPE_WRITE_FAILED;
						goto CleanUp;
					}
					if (WriteFile(ghPipe, bArgument, wArgumentSize * sizeof(BYTE), &dwBytesWritten, NULL) == FALSE)
					{
						printf(__TEXT("Unexpected error: a problem occured while communicating with the server (error code: %d)\n"), GetLastError());
						dwReturnCode = PIPE_WRITE_FAILED;
						goto CleanUp;
					}

					memset(&wResponse, 0, sizeof(WORD));
					if (ReadFile(ghPipe, &wResponse, sizeof(WORD), &dwBytesRead, NULL) == FALSE)
					{
						printf(__TEXT("Unexpected error: a problem occured while communicating with the server (error code: %d)\n"), GetLastError());
						dwReturnCode = PIPE_READ_FAILED;
						goto CleanUp;
					}
					if (wResponse != SUCCESS)
					{
						printf(__TEXT("Unexpected error: something has not been sent right (response code: %d)\n"), wResponse);
						goto CleanUp;
					}

					if (ReadFile(ghPipe, &wReplySize, sizeof(WORD), &dwBytesRead, NULL) == FALSE)
					{
						printf(__TEXT("Unexpected error: a problem occured while communicating with the server (error code: %d)\n"), GetLastError());
						dwReturnCode = PIPE_READ_FAILED;
						goto CleanUp;
					}
					bReply = (BYTE*)calloc(wReplySize, sizeof(BYTE));
					if (bReply == NULL)
					{
						printf(__TEXT("Unexpected error: memory allocation failed (error code: %d)\n"), GetLastError());
						dwReturnCode = MALLOC_FAILURE;
						goto CleanUp;
					}
					if (ReadFile(ghPipe, bReply, sizeof(BYTE) * wReplySize, &dwBytesRead, NULL) == FALSE)
					{
						printf(__TEXT("Unexpected error: memory allocation failed (error code: %d)\n"), GetLastError());
						dwReturnCode = MALLOC_FAILURE;
						goto CleanUp;
					}

					printf(__TEXT("%s\n"), bReply);
					free(bReply);
					break;
				}

				case REGISTER:
				{
					BOOL NeedData = TRUE;
					if (gUserLoggedIn == TRUE)
					{
						printf(__TEXT("Error: User already logged in\n"));
						NeedData = FALSE;
					}

					WriteFile(ghPipe, &NeedData, sizeof(BOOL), &dwBytesWritten, NULL);
					if (!NeedData)
						break;
					
					wArgumentSize = (WORD)strlen(bArgument) + 1;
					if (WriteFile(ghPipe, &wArgumentSize, sizeof(WORD), &dwBytesWritten, NULL) == FALSE)
					{
						printf(__TEXT("Unexpected error: a problem occured while communicating with the server (error code: %d)\n"), GetLastError());
						dwReturnCode = PIPE_WRITE_FAILED;
						goto CleanUp;
					}
					if (WriteFile(ghPipe, bArgument, wArgumentSize * sizeof(BYTE), &dwBytesWritten, NULL) == FALSE)
					{
						printf(__TEXT("Unexpected error: a problem occured while communicating with the server (error code: %d)\n"), GetLastError());
						dwReturnCode = PIPE_WRITE_FAILED;
						goto CleanUp;
					}

					if (ReadFile(ghPipe, &wResponse, sizeof(WORD), &dwBytesRead, NULL) == FALSE)
					{
						printf(__TEXT("Unexpected error: a problem occured while communicating with the server (error code: %d)\n"), GetLastError());
						dwReturnCode = PIPE_READ_FAILED;
						goto CleanUp;
					}
					if (wResponse != SUCCESS)
					{
						switch (wResponse)
						{
							case INVALID_USERNAME:
							{
								printf(__TEXT("Error: Invalid username\n"));
								break;
							}
							case INVALID_PASSWORD:
							{
								printf(__TEXT("Error: Invalid password\n"));
								break;
							}
							case PASSWORD_TOO_WEAK:
							{
								printf(__TEXT("Error: Password too weak\n"));
								break;
							}
							case REGISTER_ALREADY_EXISTS:
							{
								printf(__TEXT("Error: Username already registered\n"));
								break;
							}
							case FILE_WRITE_FAILED:
							{
								printf(__TEXT("Unexpected error: failed to register\n"));
								break;
							}
						}
					}
					else
						printf(__TEXT("Success\n"));

					break;
				}

				case LOGIN:
				{
					if (gUserLoggedIn == TRUE)
					{
						printf(__TEXT("Error: Another user already logged in\n"));
						BOOL RequiresResponse = FALSE;
						WriteFile(ghPipe, &RequiresResponse, sizeof(BOOL), &dwBytesWritten, NULL);
						break;
					}
					else
					{
						BOOL RequiresResponse = TRUE;
						WriteFile(ghPipe, &RequiresResponse, sizeof(BOOL), &dwBytesWritten, NULL);

						wArgumentSize = (WORD)strlen(bArgument) + 1;
						if (WriteFile(ghPipe, &wArgumentSize, sizeof(WORD), &dwBytesWritten, NULL) == FALSE)
						{
							printf(__TEXT("Unexpected error: a problem occured while communicating with the server (error code: %d)\n"), GetLastError());
							dwReturnCode = PIPE_WRITE_FAILED;
							goto CleanUp;
						}
						if (WriteFile(ghPipe, bArgument, wArgumentSize * sizeof(BYTE), &dwBytesWritten, NULL) == FALSE)
						{
							printf(__TEXT("Unexpected error: a problem occured while communicating with the server (error code: %d)\n"), GetLastError());
							dwReturnCode = PIPE_WRITE_FAILED;
							goto CleanUp;
						}

						WORD wReply;
						if (ReadFile(ghPipe, &wReply, sizeof(WORD), &dwBytesRead, NULL) == FALSE)
						{
							printf(__TEXT("Unexpected error: a problem occured while communicating with the server (error code: %d)\n"), GetLastError());
							goto CleanUp;
						}

						if (wReply == INVALID_USERNAME_PASSWORD_COMBINATION)
							printf(__TEXT("Error: Invalid username / password combination\n"));
						else if (wReply == USERNAME_ALREADY_CONNECTED)
							printf(__TEXT("Error: User already logged in\n"));
						else if (wReply != SUCCESS)
							printf(__TEXT("Unexpected error: communication with the server failed (error code: %d)\n"), GetLastError());
						else if (wReply == SUCCESS)
						{
							gUserLoggedIn = TRUE;
							printf(__TEXT("Success\n"));
						}
					}
					break;
				}

				case LOGOUT:
				{
					if (gUserLoggedIn == FALSE)
					{
						printf(__TEXT("Error: No user currently logged in\n"));
						break;
					}
					WORD wResponse;
					if (ReadFile(ghPipe, &wResponse, sizeof(WORD), &dwBytesRead, NULL) == FALSE)
					{
						printf(__TEXT("Unexpected error: communication with the server failed (error code: %d)\n"), GetLastError());
						break;
					}
					if (wResponse == SUCCESS)
					{
						gUserLoggedIn = FALSE;
						printf(__TEXT("Success\n"));
					}
					else
						printf(__TEXT("Unexpected error: an error occured (error code: %d)\n"), GetLastError());
					break;
				}

				case MSG:
				{
					BOOL NeedData = TRUE;
					if (gUserLoggedIn == FALSE)
					{
						printf(__TEXT("Error: No user currently logged in\n"));
						NeedData = FALSE;
					}

					if (WriteFile(ghPipe, &NeedData, sizeof(BOOL), &dwBytesWritten, NULL) == FALSE)
					{
						printf(__TEXT("Unexpected error: a problem occured while communicating with the server (error code: %d)\n"), GetLastError());
						dwReturnCode = PIPE_WRITE_FAILED;
						goto CleanUp;
					}
					if (NeedData == FALSE)
						break;

					WORD wResponse;
					wArgumentSize = (WORD)strlen(bArgument) + 1;
					if (WriteFile(ghPipe, &wArgumentSize, sizeof(WORD), &dwBytesWritten, NULL) == FALSE)
					{
						printf(__TEXT("Unexpected error: a problem occured while communicating with the server (error code: %d)\n"), GetLastError());
						dwReturnCode = PIPE_WRITE_FAILED;
						goto CleanUp;
					}
					if (WriteFile(ghPipe, bArgument, wArgumentSize * sizeof(BYTE), &dwBytesWritten, NULL) == FALSE)
					{
						printf(__TEXT("Unexpected error: a problem occured while communicating with the server (error code: %d)\n"), GetLastError());
						dwReturnCode = PIPE_WRITE_FAILED;
						goto CleanUp;
					}

					if (ReadFile(ghPipe, &wResponse, sizeof(WORD), &dwBytesRead, NULL) == FALSE)
					{
						printf(__TEXT("Unexpected error: a problem occured while communicating with the server (error code: %d)\n"), GetLastError());
						dwReturnCode = PIPE_WRITE_FAILED;
						goto CleanUp;
					}

					if (wResponse == INVALID_USERNAME)
					{
						printf(__TEXT("Error: No such user\n"));
						break;
					}
					else if (wResponse == SUCCESS)
					{
						printf(__TEXT("Success\n"));
						break;
					}
					else if (wResponse == USER_NOT_CONNECTED)
					{
						printf(__TEXT("Unexpected error: user not online\n"));
						break;
					}
					else
					{
						printf(__TEXT("Unexpected error: failed to send the message (error code: %d)\n"), GetLastError());
						break;
					}
				}

				case BROADCAST:
				{
					BOOL NeedData = TRUE;
					if (gUserLoggedIn == FALSE)
					{
						printf(__TEXT("Error: No user currently logged in\n"));
						NeedData = FALSE;
					}

					if (WriteFile(ghPipe, &NeedData, sizeof(BOOL), &dwBytesWritten, NULL) == FALSE)
					{
						printf(__TEXT("Unexpected error: a problem occured while communicating with the server (error code: %d)\n"), GetLastError());
						dwReturnCode = PIPE_WRITE_FAILED;
						goto CleanUp;
					}
					if (NeedData == FALSE)
						break;

					WORD wResponse;
					wArgumentSize = (WORD)strlen(bArgument) + 1;
					if (WriteFile(ghPipe, &wArgumentSize, sizeof(WORD), &dwBytesWritten, NULL) == FALSE)
					{
						printf(__TEXT("Unexpected error: a problem occured while communicating with the server (error code: %d)\n"), GetLastError());
						dwReturnCode = PIPE_WRITE_FAILED;
						goto CleanUp;
					}
					if (WriteFile(ghPipe, bArgument, wArgumentSize * sizeof(BYTE), &dwBytesWritten, NULL) == FALSE)
					{
						printf(__TEXT("Unexpected error: a problem occured while communicating with the server (error code: %d)\n"), GetLastError());
						dwReturnCode = PIPE_WRITE_FAILED;
						goto CleanUp;
					}

					if (ReadFile(ghPipe, &wResponse, sizeof(WORD), &dwBytesRead, NULL) == FALSE)
					{
						printf(__TEXT("Unexpected error: a problem occured while communicating with the server (error code: %d)\n"), GetLastError());
						dwReturnCode = PIPE_WRITE_FAILED;
						goto CleanUp;
					}

					if (wResponse == SUCCESS)
						printf(__TEXT("Success\n"));
					else
						printf(__TEXT("Unexpected error: something went wrong when broadcasting the message\n"));
						
					break;
				}

				case SENDFILE:
				{
					BOOL NeedData = TRUE;
					WORD wResponse, wReceiverNameSize, wSizeOfFilename;
					HANDLE hFileExists;
					BYTE* bReceiver = (BYTE*)calloc(255, sizeof(BYTE));
					BYTE* bPath = (BYTE*)calloc(255, sizeof(BYTE));
					BYTE* bFilename = (BYTE*)calloc(255, sizeof(BYTE));
					GetCommand(&bArgument, &bReceiver, &bPath);

					
					if (gUserLoggedIn == FALSE)
					{
						printf(__TEXT("Error: No user currently logged in\n"));
						NeedData = FALSE;
					}

					if (WriteFile(ghPipe, &NeedData, sizeof(BOOL), &dwBytesWritten, NULL) == FALSE)
					{
						printf(__TEXT("Unexpected error: a problem occured while communicating with the server (error code: %d)\n"), GetLastError());
						dwReturnCode = PIPE_WRITE_FAILED;
						goto CleanUp;
					}

					if (!NeedData)
						break;

					wReceiverNameSize = (WORD)strlen(bReceiver) + 1;
					WriteFile(ghPipe, &wReceiverNameSize, sizeof(WORD), &dwBytesWritten, NULL);
					WriteFile(ghPipe, bReceiver, wReceiverNameSize * sizeof(BYTE), &dwBytesWritten, NULL);

					ReadFile(ghPipe, &wResponse, sizeof(WORD), &dwBytesRead, NULL);
					
					if (wResponse == INVALID_USERNAME)
					{
						printf(__TEXT("Error: No such user\n"));
						break;
					}
					else if (wResponse == FALSE)
					{
						printf(__TEXT("Error: User not active\n"));
						break;
					}
					
					GetFileName(bPath, &bFilename);
					wSizeOfFilename = (WORD)strlen(bFilename) + 1;

					hFileExists = CreateFileA(bPath, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
					NeedData = TRUE;

					if (hFileExists == INVALID_HANDLE_VALUE && GetLastError() == 2)
					{
						printf(__TEXT("Error: File not found\n"));
						NeedData = FALSE;
					}
					CloseHandle(hFileExists);
					WriteFile(ghPipe, &NeedData, sizeof(BOOL), &dwBytesWritten, NULL);
					if (!NeedData)
						break;

					WriteFile(ghPipe, &wSizeOfFilename, sizeof(WORD), &dwBytesWritten, NULL);
					WriteFile(ghPipe, bFilename, wSizeOfFilename * sizeof(BYTE), &dwBytesRead, NULL);
					
					if (hFileSenderThread != NULL)
					{
						WaitForSingleObject(hFileSenderThread, INFINITE);
						CloseHandle(hFileSenderThread);
					}

					Sleep(100);
					
					free(bReceiver);
					free(bFilename);

					hFileSenderThread = CreateThread(NULL, 0, &ClientSendFile, (LPVOID)bPath, 0, NULL);

					if (hFileSenderThread == NULL)
						printf(__TEXT("Unexpected error: error sending the file (error code: %d)\n"), GetLastError());
					
					printf(__TEXT("Success\n"));

					break;
				}

				case LIST:
				{
					DWORD dwNumberOfClientsConnected;
					if (ReadFile(ghPipe, &dwNumberOfClientsConnected, sizeof(DWORD), &dwBytesRead, NULL) == FALSE)
					{
						printf(__TEXT("Unexpected error: communication with the client failed (error code: %d)\n"), GetLastError());
						break;
					}
					if (dwNumberOfClientsConnected == 0)
						printf(__TEXT("\n"));
					else
					{
						for (DWORD i = 0; i < dwNumberOfClientsConnected; i++)
						{
							DWORD dwSize;
							BYTE bUsername[255];
							memset(bUsername, 0, 255 * sizeof(BYTE));
							ReadFile(ghPipe, &dwSize, sizeof(DWORD), &dwBytesRead, NULL);
							ReadFile(ghPipe, bUsername, dwSize * sizeof(BYTE), &dwBytesRead, NULL);
							bUsername[dwSize] = '\0';
							printf(__TEXT("%s\n"), bUsername);
						}
					}

					break;
				}

				case EXIT:
				{
					gUserLoggedIn = FALSE;
					goto CleanUp;
				}

				case HISTORY:
				{
					if (gUserLoggedIn == FALSE)
						printf(__TEXT("Error: No user currently logged in\n"));
					else
						printf(__TEXT("Unexpected error: this command has not been implemented yet\n"));
					break;
				}
			}
		}
	}
	
CleanUp:
	if (bArgument != NULL)
		free(bArgument);
	if (bCommand != NULL)
		free(bCommand);
	if (bInput != NULL)
		free(bInput);

	if (hMessageThread != NULL)
	{
		TerminateThread(hMessageThread, 0);
		WaitForSingleObject(hMessageThread, INFINITE);
		CloseHandle(hMessageThread);
	}

	if (hFileSenderThread != NULL)
	{
		TerminateThread(hFileSenderThread, 0);
		WaitForSingleObject(hFileSenderThread, INFINITE);
		CloseHandle(hFileSenderThread);
	}

	DisconnectNamedPipe(ghPipe);
	DisconnectNamedPipe(ghMessagePipe);
	CloseHandle(ghPipe);
	CloseHandle(ghMessagePipe);

	DeleteCriticalSection(&gCriticalSection);

	return 0;
}