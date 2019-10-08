#include "server.h"

void Encrypt(BYTE bKey, BYTE* bString)
{
	WORD wIndex = 0;
	
	while (bString[wIndex] != '\n')
	{
		/// Uncomment the next two lines to allow the 
		/// program to encrypt the passwords before
		/// storing them in the file, or before checking
		/// if a certain password exists;

		//if ((bString[wIndex] >= 'A' && bString[wIndex] <= 'Z') || (bString[wIndex] >= 'a' && bString[wIndex] <= 'z'))
		//	bString[wIndex] = bString[wIndex] + bKey;
		wIndex++;

	}
}

void RegisterSplit(BYTE* bInput, BYTE** bUsername, BYTE** bPassword)
{
	BYTE bIndex = 0;
	BYTE bSize = (BYTE)strlen(bInput);

	while (bIndex < bSize && bInput[bIndex] != ' ')
		bIndex++;

	MyStrCpy((*bUsername), bInput, bIndex);
	(*bUsername)[bIndex] = '\0';
	
	MyStrCpy((*bPassword), bInput + bIndex + 1, bSize - bIndex);
}

void MyStrCpy(BYTE* bDestination, BYTE * bSource, DWORD wCount)
{
	for (DWORD i = 0; i < wCount; i++)
		bDestination[i] = bSource[i];
}

int GetUsernamePassword(BYTE* bArgument, WORD wArgumentSize, BYTE** bUsername, BYTE** bPassword)
{
	WORD wReturnCode = SUCCESS;

	if (bArgument == NULL || (*bUsername) == NULL || (*bPassword) == NULL)
		return INVALID_PARAMETER;

	WORD wIndex = 0;
	while (wIndex < wArgumentSize && bArgument[wIndex] != ' ')
		wIndex++;

	if (wIndex == wArgumentSize)
		return INCORRECT_PARAMETER;

	MyStrCpy((*bUsername), bArgument, wIndex);
	(*bUsername)[wIndex] = '\0';

	MyStrCpy((*bPassword), bArgument + wIndex + 1, wArgumentSize - wIndex);
	(*bPassword)[wArgumentSize - wIndex] = '\0';

	return SUCCESS;
}

WORD RegisterValidation(BYTE* bInput)
{
	BOOL HasCapitalLetter = FALSE;
	BOOL HasNotAlfanumeric = FALSE;
	BOOL HasSpacesOrCommas = FALSE;
	WORD wResult = SUCCESS;
	BYTE bIndex;
	BYTE* bUsername = (BYTE*)calloc(255, sizeof(BYTE));
	BYTE* bPassword = (BYTE*)calloc(255, sizeof(BYTE));
	RegisterSplit(bInput, &bUsername, &bPassword);

	bIndex = 0;
	while (bIndex < (BYTE)strlen(bUsername))
	{
		if (
			(bUsername[bIndex] >= 'a' && bUsername[bIndex] <= 'z') ||
			(bUsername[bIndex] >= 'A' && bUsername[bIndex] <= 'Z') ||
			(bUsername[bIndex] >= '0' && bUsername[bIndex] <= '9')
			)
			bIndex++;
		else
			return INVALID_USERNAME;
	}

	if (strlen(bPassword) < 5)
		return PASSWORD_TOO_WEAK;

	bIndex = 0;
	while (bIndex < (BYTE)strlen(bPassword))
	{
		if (bPassword[bIndex] >= 'A' && bPassword[bIndex] <= 'Z')
			HasCapitalLetter = TRUE;
		if (bPassword[bIndex] == ',' || bPassword[bIndex] == ' ')
			HasSpacesOrCommas = TRUE;
		if ((bPassword[bIndex] > ' ') && (bPassword[bIndex] < '0') && (bPassword[bIndex] != ','))
			HasNotAlfanumeric = TRUE;
		if ((bPassword[bIndex] > '9') && (bPassword[bIndex] < 'A'))
			HasNotAlfanumeric = TRUE;
		if ((bPassword[bIndex] > 'Z') && (bPassword[bIndex] < 'a'))
			HasNotAlfanumeric = TRUE;
		if (bPassword[bIndex] > 'z')
			HasNotAlfanumeric = TRUE;
		bIndex++;
	}

	if (HasSpacesOrCommas == TRUE)
		return INVALID_PASSWORD;
	if (HasCapitalLetter == FALSE)
		return PASSWORD_TOO_WEAK;
	if (HasNotAlfanumeric == FALSE)
		return PASSWORD_TOO_WEAK;


	free(bUsername);
	free(bPassword);

	return SUCCESS;
}

WORD RegisterCheckFile(BYTE* bArgument)
{
	WORD wReturnCode = SUCCESS;
	DWORD dwBytesRead = 1;

	BYTE bBuffer[4096];
	BYTE bUsername[255];
	BYTE bLastEnter, bCurrentEnter;

	memset(bBuffer, 0, 4096 * sizeof(BYTE));
	memset(bUsername, 0, 255 * sizeof(BYTE));
	bLastEnter = bCurrentEnter = 0;
	
	BYTE bIndex = 0;
	
	while (bArgument[bIndex] != ' ')
		bIndex++;
	
	MyStrCpy(bUsername, bArgument, (WORD)bIndex);
	bUsername[bIndex] = '\0';

	SetFilePointer(ghRegistrationFile, 0, 0, FILE_BEGIN);

	DWORD dwCurrent = 0;
	while (dwBytesRead != 0)
	{
		ReadFile(ghRegistrationFile, bBuffer + dwCurrent, 1, &dwBytesRead, NULL);
		if (bBuffer[dwCurrent] == '\n')
		{
			DWORD dwIndex = 0;
			BYTE bTmpUsername[1024];
			while (bBuffer[dwIndex] != ',')
				dwIndex++;
			
			MyStrCpy(bTmpUsername, bBuffer, (WORD)dwIndex);
			bTmpUsername[dwIndex] = '\0';
			
			if (strcmp(bUsername, bTmpUsername) == 0)
			{
				wReturnCode = REGISTER_ALREADY_EXISTS;
				break;
			}
			memset(bBuffer, 0, 4096);
			dwCurrent = -1;
		}
		dwCurrent++;
	}

	return wReturnCode;
}

WORD RegisterStoreFile(BYTE* bInput)
{
	WORD wReturnCode = SUCCESS;
	WORD wIndex = 0;
	DWORD dwBytesWritten = 0;
	while (wIndex < strlen(bInput) || bInput[wIndex] != '\0')
		wIndex++;

	if (bInput[wIndex] == '\0')
		bInput[wIndex] = '\n';

	wIndex = 0;
	while (bInput[wIndex] != ' ')
		wIndex++;
	bInput[wIndex] = ',';
	Encrypt(bInput[0], bInput + wIndex + 1);

	SetFilePointer(ghRegistrationFile, 0, 0, FILE_END);

	if (WriteFile(ghRegistrationFile, bInput, strlen(bInput) * sizeof(BYTE), &dwBytesWritten, NULL) == FALSE)
		wReturnCode = FILE_WRITE_FAILED;

	return wReturnCode;
}

WORD LoginValidation(BYTE* bUsername, BYTE* bPassword)
{
	WORD wReturnCode = INVALID_USERNAME_PASSWORD_COMBINATION;

	DWORD dwBytesRead = 1;

	BYTE bBuffer[4096];
	BYTE bLastEnter, bCurrentEnter;

	memset(bBuffer, 0, 4096 * sizeof(BYTE));
	bLastEnter = bCurrentEnter = 0;

	BYTE bIndex = 0;

	while (bPassword[bIndex] != '\0')
		bIndex++;

	bPassword[bIndex] = '\n';
	bIndex = 0;

	Encrypt(bUsername[0], bPassword);

	SetFilePointer(ghRegistrationFile, 0, 0, FILE_BEGIN);

	DWORD dwCurrent = 0;
	while (dwBytesRead != 0)
	{
		ReadFile(ghRegistrationFile, bBuffer + dwCurrent, 1, &dwBytesRead, NULL);
		if (bBuffer[dwCurrent] == '\n')
		{
			DWORD dwIndex = 0;
			BYTE bTmp[1024];
			memset(bTmp, 0, 1024 * sizeof(BYTE));
			while (bBuffer[dwIndex] != ',')
				dwIndex++;

			MyStrCpy(bTmp, bBuffer, (WORD)dwIndex);
			bTmp[dwIndex] = '\0';

			if (strcmp(bUsername, bTmp) == 0)
			{
				// Username matches
				memset(bTmp, 0, 1024 * sizeof(BYTE));
				MyStrCpy(bTmp, bBuffer + dwIndex + 1, dwCurrent - dwIndex);
				if (strcmp(bPassword, bTmp) == 0)
				{
					// Password matches
					wReturnCode = SUCCESS;
					break;
				}
			}

			// Username doesn't match
			memset(bBuffer, 0, 4096);
			dwCurrent = -1;
		}
		dwCurrent++;
	}

	return wReturnCode;	
}

WORD LoginAdd(BYTE* bUsername, HANDLE hPipe, HANDLE hMessagePipe)
{
	WORD wUsernameSize = (WORD)strlen(bUsername) + 1;
	
	CLIENT Client;
	
	Client.lpsClientName = (BYTE*)calloc(wUsernameSize, sizeof(BYTE));
	if (Client.lpsClientName == NULL) 
		return MALLOC_FAILURE;
	
	Client.hClientPipe = hPipe;
	Client.hMessagesPipe = hMessagePipe;
	MyStrCpy(Client.lpsClientName, bUsername, wUsernameSize);

	EnterCriticalSection(&gCriticalSection);
	gClientVector.Clients[gClientVector.dwSize++] = Client;
	LeaveCriticalSection(&gCriticalSection);

	return SUCCESS;
}

BOOL LoginCheck(BYTE* bUsername)
{
	EnterCriticalSection(&gCriticalSection);
	for (DWORD dwIndex = 0; dwIndex < gClientVector.dwSize; dwIndex++)
		if (strcmp(gClientVector.Clients[dwIndex].lpsClientName, bUsername) == 0)
		{
			LeaveCriticalSection(&gCriticalSection);
			return USERNAME_ALREADY_CONNECTED;
		}
	LeaveCriticalSection(&gCriticalSection);
	return SUCCESS;	
}

WORD LogoutRemoveByUsername(BYTE* bUsername)
{
	DWORD dwIndex = 0;
	EnterCriticalSection(&gCriticalSection);
	while (dwIndex < gClientVector.dwSize)
	{
		if (strcmp(gClientVector.Clients[dwIndex].lpsClientName, bUsername) == 0)
		{
			free(gClientVector.Clients[dwIndex].lpsClientName);
			for (DWORD i = dwIndex; i < gClientVector.dwSize - 1; i++)
			{
				gClientVector.Clients[i].lpsClientName = gClientVector.Clients[i + 1].lpsClientName;
				gClientVector.Clients[i].hClientPipe = gClientVector.Clients[i + 1].hClientPipe;
			}
			gClientVector.dwSize--;
			LeaveCriticalSection(&gCriticalSection);
			return SUCCESS;
		}
		dwIndex++;
	}
	LeaveCriticalSection(&gCriticalSection);
	return SUCCESS;
}

WORD LogoutRemoveByPipeHandle(HANDLE hPipe)
{
	DWORD dwIndex = 0;
	EnterCriticalSection(&gCriticalSection);
	while (dwIndex < gClientVector.dwSize)
	{
		if (gClientVector.Clients[dwIndex].hClientPipe == hPipe)
		{
			free(gClientVector.Clients[dwIndex].lpsClientName);
			for (DWORD i = dwIndex; i < gClientVector.dwSize - 1; i++)
				gClientVector.Clients[i] = gClientVector.Clients[i + 1];
			
			gClientVector.dwSize--;
			LeaveCriticalSection(&gCriticalSection);
			return SUCCESS;
		}
		dwIndex++;
	}
	LeaveCriticalSection(&gCriticalSection);
	return SUCCESS;
}

BOOL ClientLoggedIn(BYTE* bUsername, HANDLE hPipe)
{
	if (bUsername == NULL)
	{
		EnterCriticalSection(&gCriticalSection);
		for (DWORD dwIndex = 0; dwIndex < gClientVector.dwSize; dwIndex++)
			if (gClientVector.Clients[dwIndex].hClientPipe == hPipe)
			{
				LeaveCriticalSection(&gCriticalSection);
				return TRUE;
			}
	}
	else if (hPipe == INVALID_HANDLE_VALUE)
	{
		EnterCriticalSection(&gCriticalSection);
		for (DWORD dwIndex = 0; dwIndex < gClientVector.dwSize; dwIndex++)
			if (strcmp(gClientVector.Clients[dwIndex].lpsClientName, bUsername) == 0)
			{
				LeaveCriticalSection(&gCriticalSection);
				return TRUE;
			}
		
	}
	LeaveCriticalSection(&gCriticalSection);
	return FALSE;
}

BOOL ClientExists(BYTE* bUsername)
{
	DWORD dwBytesRead = 1;

	BYTE bBuffer[4096];
	BYTE bLastEnter, bCurrentEnter;

	memset(bBuffer, 0, 4096 * sizeof(BYTE));
	bLastEnter = bCurrentEnter = 0;

	BYTE bIndex = 0;

	SetFilePointer(ghRegistrationFile, 0, 0, FILE_BEGIN);

	DWORD dwCurrent = 0;
	while (dwBytesRead != 0)
	{
		ReadFile(ghRegistrationFile, bBuffer + dwCurrent, 1, &dwBytesRead, NULL);
		if (bBuffer[dwCurrent] == '\n')
		{
			DWORD dwIndex = 0;
			BYTE bTmpUsername[1024];
			while (bBuffer[dwIndex] != ',')
				dwIndex++;

			MyStrCpy(bTmpUsername, bBuffer, (WORD)dwIndex);
			bTmpUsername[dwIndex] = '\0';

			if (strcmp(bUsername, bTmpUsername) == 0)
				return REGISTER_ALREADY_EXISTS;

			memset(bBuffer, 0, 4096);
			dwCurrent = -1;
		}
		dwCurrent++;
	}

	return INVALID_USERNAME;
}

WORD GetClientByPipe(HANDLE hPipe, BYTE** bUsername)
{
	for (DWORD i = 0; i < gClientVector.dwSize; i++)
	{
		if (gClientVector.Clients[i].hClientPipe == hPipe)
		{
			DWORD dwSize = strlen(gClientVector.Clients[i].lpsClientName) + 1;
			MyStrCpy((*bUsername), gClientVector.Clients[i].lpsClientName, dwSize);
			(*bUsername)[dwSize] = '\0';
			return SUCCESS;
		}
	}
	return INCORRECT_PARAMETER;
}

void WhoAmI(HANDLE hPipe, BYTE** bUsername)
{
	for (DWORD i = 0; i < gClientVector.dwSize; i++)
		if (gClientVector.Clients[i].hClientPipe == hPipe)
		{
			MyStrCpy((*bUsername), gClientVector.Clients[i].lpsClientName, strlen(gClientVector.Clients[i].lpsClientName) + 1);
			return;
		}
}

WORD WriteMessageToHistory(BYTE* bSender, BYTE* bRest)
{
	WORD wReturnCode = SUCCESS;
	DWORD dwBytesWritten, dwSize;
	BYTE bEntry[1024];
	
	memset(bEntry, 0, 1024 * sizeof(BYTE));

	MyStrCpy(bEntry, bSender, strlen(bSender));
	bEntry[strlen(bSender)] = ' ';
	MyStrCpy(bEntry + strlen(bSender) + 1, bRest, strlen(bRest));
	bEntry[strlen(bSender) + strlen(bRest)] = '\n';
	dwSize = (DWORD)(strlen(bSender) + 1 + strlen(bRest) + 1);

	SetFilePointer(ghMessagesHistoryFile, 0, 0, FILE_END);

	if (WriteFile(ghMessagesHistoryFile, bEntry, dwSize * sizeof(BYTE), &dwBytesWritten, NULL) == FALSE)
		wReturnCode = FILE_WRITE_FAILED;

	return wReturnCode;
}

WORD ServerEcho(LPVOID Argument)
{
	HANDLE hPipe = *(HANDLE*)Argument;
	DWORD dwBytesRead, dwBytesWritten;
	WORD wReturnCode = SUCCESS, wArgumentSize, wResponse, wSizeReplyBuffer;
	BYTE bArgument[255], bReplyBuffer[255];

	memset(bArgument, 0, 255 * sizeof(BYTE));
	memset(bReplyBuffer, 0, 255 * sizeof(BYTE));
	memset(&wResponse, 0, sizeof(WORD));

	if (ReadFile(hPipe, &wArgumentSize, sizeof(WORD), &dwBytesRead, NULL) == FALSE)
	{
		wReturnCode = PIPE_READ_FAILED;
		goto CleanUp;
	}
	if (ReadFile(hPipe, bArgument, wArgumentSize * sizeof(BYTE), &dwBytesRead, NULL) == FALSE)
	{
		wReturnCode = PIPE_READ_FAILED;
		goto CleanUp;
	}
	bArgument[wArgumentSize] = '\0';

	wResponse = SUCCESS;
	if (WriteFile(hPipe, &wResponse, sizeof(WORD), &dwBytesWritten, NULL) == FALSE)
	{
		wReturnCode = PIPE_WRITE_FAILED;
		goto CleanUp;
	}
	
	MyStrCpy(bReplyBuffer, bArgument, wArgumentSize);
	wSizeReplyBuffer = wArgumentSize;
	bReplyBuffer[wArgumentSize] = '\0';
	
	printf(__TEXT("%s\n"), bReplyBuffer);

	if (WriteFile(hPipe, &wSizeReplyBuffer, sizeof(WORD), &dwBytesWritten, NULL) == FALSE)
	{
		wReturnCode = PIPE_WRITE_FAILED;
		goto CleanUp;
	}
	if (WriteFile(hPipe, bReplyBuffer, sizeof(BYTE) * wSizeReplyBuffer, &dwBytesWritten, NULL) == FALSE)
	{
		wReturnCode = PIPE_WRITE_FAILED;
		goto CleanUp;
	}

CleanUp:
			
	return wReturnCode;
}

WORD ServerRegister(LPVOID Argument)
{
	BOOL ClientRequiresData;
	HANDLE hPipe;
	BYTE bArgument[255];
	WORD wArgumentSize, wResponse, wReturnCode;
	DWORD dwBytesRead, dwBytesWritten;

	hPipe = *(HANDLE*)Argument;
	wReturnCode = SUCCESS;
	memset(bArgument, 0, 255 * sizeof(BYTE));
	
	ReadFile(hPipe, &ClientRequiresData, sizeof(BOOL), &dwBytesRead, NULL);
	if (!ClientRequiresData)
		return SUCCESS;

	if (ReadFile(hPipe, &wArgumentSize, sizeof(WORD), &dwBytesRead, NULL) == FALSE)
	{
		wReturnCode = PIPE_READ_FAILED;
		goto CleanUp;
	}
	if (ReadFile(hPipe, bArgument, wArgumentSize * sizeof(BYTE), &dwBytesRead, NULL) == FALSE)
	{
		wReturnCode = PIPE_READ_FAILED;
		goto CleanUp;
	}
	bArgument[wArgumentSize] = '\0';

	wResponse = RegisterValidation(bArgument);

	if (wResponse != SUCCESS)
	{
		if (WriteFile(hPipe, &wResponse, sizeof(WORD), &dwBytesWritten, NULL) == FALSE)
		{
			wReturnCode = PIPE_WRITE_FAILED;
			goto CleanUp;
		}
	}
	else
	{
		wResponse = RegisterCheckFile(bArgument);
		if (wResponse != SUCCESS)
		{
			if (WriteFile(hPipe, &wResponse, sizeof(WORD), &dwBytesWritten, NULL) == FALSE)
			{
				wReturnCode = PIPE_WRITE_FAILED;
				goto CleanUp;
			}
		}
		else
		{
			wResponse = RegisterStoreFile(bArgument);
			if (WriteFile(hPipe, &wResponse, sizeof(WORD), &dwBytesWritten, NULL) == FALSE)
			{
				wReturnCode = PIPE_WRITE_FAILED;
				goto CleanUp;
			}
		}
	}

CleanUp:

	return wReturnCode;
}

WORD ServerLogin(LPVOID Argument)
{
	BOOL DataRequested;
	HANDLE hPipe, hMessagePipe;
	BYTE bArgument[255], *bUsername, *bPassword;
	WORD wReturnCode, wArgumentSize, wResponse;
	DWORD dwBytesWritten, dwBytesRead;

	hPipe = *((ARGUMENT*)Argument)->phPipe;
	hMessagePipe = *((ARGUMENT*)Argument)->phMessagePipe;
	wReturnCode = SUCCESS;
	bUsername = NULL;
	bPassword = NULL;
	memset(bArgument, 0, 255 * sizeof(BYTE));
	

	if (ReadFile(hPipe, &DataRequested, sizeof(BOOL), &dwBytesRead, NULL) == FALSE)
	{
		wReturnCode = PIPE_READ_FAILED;
		goto End;
	}
	if (DataRequested == FALSE) 
		return SUCCESS;

	if (ReadFile(hPipe, &wArgumentSize, sizeof(WORD), &dwBytesRead, NULL) == FALSE)
	{
		wReturnCode = PIPE_READ_FAILED;
		goto End;
	}
	if (ReadFile(hPipe, bArgument, wArgumentSize * sizeof(BYTE), &dwBytesRead, NULL) == FALSE)
	{
		wReturnCode = PIPE_READ_FAILED;
		goto End;
	}
	bArgument[wArgumentSize] = '\0';

	bUsername = (BYTE*)calloc(255, sizeof(BYTE));
	bPassword = (BYTE*)calloc(255, sizeof(BYTE));

	if (GetUsernamePassword(bArgument, wArgumentSize, &bUsername, &bPassword) != SUCCESS)
	{
		wReturnCode = INNER_FUNC_ERROR;
		goto End;
	}
	
	wResponse = LoginValidation(bUsername, bPassword);

	if (wResponse != SUCCESS)
	{
		if (WriteFile(hPipe, &wResponse, sizeof(WORD), &dwBytesWritten, NULL) == FALSE)
		{
			wReturnCode = PIPE_WRITE_FAILED;
			goto End;
		}
	}
	else
	{
		wResponse = LoginCheck(bUsername);
		if (wResponse == USERNAME_ALREADY_CONNECTED)
		{
			if (WriteFile(hPipe, &wResponse, sizeof(WORD), &dwBytesWritten, NULL) == FALSE)
			{
				wReturnCode = PIPE_WRITE_FAILED;
				goto End;
			}
			goto End;
		}

		wResponse = LoginAdd(bUsername, hPipe, hMessagePipe);
		if (WriteFile(hPipe, &wResponse, sizeof(WORD), &dwBytesWritten, NULL) == FALSE)
		{
			wReturnCode = PIPE_WRITE_FAILED;
			goto End;
		}
		if (wResponse != SUCCESS)
			LogoutRemoveByUsername(bUsername);
	}

End:
	if (bUsername != NULL)
		free(bUsername);
	if (bPassword != NULL)
		free(bPassword);

	return wReturnCode;
}

WORD ServerLogout(LPVOID Argument)
{
	HANDLE hPipe;
	WORD wResponse, wReturnCode;
	DWORD dwBytesWritten;

	hPipe = *(HANDLE*)Argument;
	wReturnCode = SUCCESS;

	if (ClientLoggedIn(NULL, hPipe) == FALSE)
		return SUCCESS;

	wResponse = LogoutRemoveByPipeHandle(hPipe);
	if (WriteFile(hPipe, &wResponse, sizeof(WORD), &dwBytesWritten, NULL) == FALSE)
		wReturnCode = PIPE_WRITE_FAILED;

	return wReturnCode;
}

WORD ServerMsg(LPVOID Argument)
{
	BOOL ClientRequiresData, UserOnline;
	BYTE bArgument[255], *bUsername, *bMessage, *bSenderUsername;
	HANDLE hPipe;
	WORD wReturnCode, wArgumentSize, wResponse;
	DWORD dwBytesRead, dwBytesWritten;

	hPipe = *(HANDLE*)Argument;
	UserOnline = FALSE;
	wReturnCode = SUCCESS;
	memset(bArgument, 0, 255 * sizeof(BYTE));
	bUsername = NULL;
	bMessage = NULL;
	bSenderUsername = NULL;

	bUsername = (BYTE*)calloc(255, sizeof(BYTE));
	bMessage = (BYTE*)calloc(255, sizeof(BYTE));
	bSenderUsername = (BYTE*)calloc(255, sizeof(BYTE));

	if (bUsername == NULL || bMessage == NULL || bSenderUsername == NULL)
	{
		wReturnCode = MALLOC_FAILURE;
		goto CleanUp;
	}

	if (ReadFile(hPipe, &ClientRequiresData, sizeof(BOOL), &dwBytesRead, NULL) == FALSE)
	{
		wReturnCode = PIPE_READ_FAILED;
		goto CleanUp;
	}
	if (ClientRequiresData == FALSE)
		goto CleanUp;

	if (ReadFile(hPipe, &wArgumentSize, sizeof(WORD), &dwBytesRead, NULL) == FALSE)
	{
		wReturnCode = PIPE_READ_FAILED;
		goto CleanUp;
	}
	if (ReadFile(hPipe, bArgument, wArgumentSize * sizeof(BYTE), &dwBytesRead, NULL) == FALSE)
	{
		wReturnCode = PIPE_READ_FAILED;
		goto CleanUp;
	}
	bArgument[wArgumentSize] = '\0';

	RegisterSplit(bArgument, &bUsername, &bMessage);

	wResponse = ClientExists(bUsername);

	if (wResponse == INVALID_USERNAME)
	{
		if (WriteFile(hPipe, &wResponse, sizeof(WORD), &dwBytesWritten, NULL) == FALSE)
			wReturnCode = PIPE_WRITE_FAILED;
		goto CleanUp;
	}

	WhoAmI(hPipe, &bSenderUsername);

	for (DWORD i = 0; i < gClientVector.dwSize; i++)
		if (strcmp(bUsername, gClientVector.Clients[i].lpsClientName) == 0)
		{
			DWORD dwBytesWritten;
			WORD wSize = (WORD)strlen(bSenderUsername) + 1;
			BYTE bTypeOfSend = MSG;

			WriteFile(gClientVector.Clients[i].hMessagesPipe, &bTypeOfSend, sizeof(BYTE), &dwBytesWritten, NULL);
			
			WriteFile(gClientVector.Clients[i].hMessagesPipe, &wSize, sizeof(WORD), &dwBytesWritten, NULL);
			WriteFile(gClientVector.Clients[i].hMessagesPipe, bSenderUsername, wSize * sizeof(BYTE), &dwBytesWritten, NULL);
			
			wSize = (WORD)strlen(bMessage) + 1;
			WriteFile(gClientVector.Clients[i].hMessagesPipe, &wSize, sizeof(WORD), &dwBytesWritten, NULL);
			WriteFile(gClientVector.Clients[i].hMessagesPipe, bMessage, wSize * sizeof(BYTE), &dwBytesWritten, NULL);

			wResponse = SUCCESS;
			UserOnline = TRUE;
			break;
		}
	if (!UserOnline)
		wResponse = USER_NOT_CONNECTED;

	if (WriteFile(hPipe, &wResponse, sizeof(WORD), &dwBytesWritten, NULL) == FALSE)
		wReturnCode = PIPE_WRITE_FAILED;

	//WriteMessageToHistory(bSenderUsername, bArgument);

CleanUp:
	if (bUsername != NULL)
		free(bUsername);
	if (bMessage != NULL)
		free(bMessage);
	if (bSenderUsername != NULL)
		free(bSenderUsername);

	return wReturnCode;
}

WORD ServerBroadcast(LPVOID Argument)
{
	HANDLE hPipe;
	BOOL ClientRequiresData;
	BYTE bArgument[255], *bUsername, *bMessage;
	WORD wResponse, wReturnCode, wArgumentSize;
	DWORD dwBytesRead, dwBytesWritten;

	hPipe = *(HANDLE*)Argument;
	wReturnCode = SUCCESS;
	bUsername = bMessage = NULL;

	memset(bArgument, 0, 255 * sizeof(BYTE));
	bUsername = (BYTE*)calloc(255, sizeof(BYTE));
	bMessage = (BYTE*)calloc(255, sizeof(BYTE));

	if (bUsername == NULL || bMessage == NULL)
		return MALLOC_FAILURE;
	
	if (ReadFile(hPipe, &ClientRequiresData, sizeof(BOOL), &dwBytesRead, NULL) == FALSE)
	{
		wReturnCode = PIPE_READ_FAILED;
		goto End;
	}
	if (ClientRequiresData == FALSE)
	{
		wReturnCode = SUCCESS;
		goto End;
	}

	if (ReadFile(hPipe, &wArgumentSize, sizeof(WORD), &dwBytesRead, NULL) == FALSE)
	{
		wReturnCode = PIPE_READ_FAILED;
		goto End;
	}
	if (ReadFile(hPipe, bArgument, wArgumentSize * sizeof(BYTE), &dwBytesRead, NULL) == FALSE)
	{
		wReturnCode = PIPE_READ_FAILED;
		goto End;
	}
	bArgument[wArgumentSize] = '\0';

	MyStrCpy(bMessage, bArgument, wArgumentSize);
	bMessage[wArgumentSize] = '\0';

	wResponse = GetClientByPipe(hPipe, &bUsername);
	if (wResponse != SUCCESS)
	{
		if (WriteFile(hPipe, &wResponse, sizeof(WORD), &dwBytesWritten, NULL) == FALSE)
			wReturnCode = PIPE_WRITE_FAILED;
		goto End;
	}

	for (DWORD i = 0; i < gClientVector.dwSize; i++)
	{
		if (strcmp(bUsername, gClientVector.Clients[i].lpsClientName) != 0)
		{
			DWORD dwBytesWritten;
			WORD wSize = (WORD)strlen(bMessage) + 1;
			WORD wSizeUsername = (WORD)strlen(bUsername) + 1;
			BYTE bTypeOfSend = BROADCAST;

			WriteFile(gClientVector.Clients[i].hMessagesPipe, &bTypeOfSend, sizeof(BYTE), &dwBytesWritten, NULL);

			WriteFile(gClientVector.Clients[i].hMessagesPipe, &wSizeUsername, sizeof(WORD), &dwBytesWritten, NULL);
			WriteFile(gClientVector.Clients[i].hMessagesPipe, bUsername, wSizeUsername * sizeof(BYTE), &dwBytesWritten, NULL);

			WriteFile(gClientVector.Clients[i].hMessagesPipe, &wSize, sizeof(WORD), &dwBytesWritten, NULL);
			WriteFile(gClientVector.Clients[i].hMessagesPipe, bMessage, wSize * sizeof(BYTE), &dwBytesWritten, NULL);
		}
	}	
	
	if (WriteFile(hPipe, &wResponse, sizeof(WORD), &dwBytesWritten, NULL) == FALSE)
		wReturnCode = PIPE_WRITE_FAILED;

End:

	if (bUsername != NULL)
		free(bUsername);
	if (bMessage != NULL)
		free(bMessage);

	return wReturnCode;
}

WORD ServerSendfile(LPVOID Argument)
{
	HANDLE hPipe, hFileSenderThread;
	BOOL ClientRequiresData;
	BYTE bReceiver[255], bFileName[255], *bSender;
	BYTE *pbSenderName, *pbReceiverName, *pbFileName;
	BYTE** bSenderNameReceiverNameFileName;
	WORD wResponse, wSenderNameSize, wReceiverNameSize, wReturnCode, wFileNameSize;
	DWORD dwBytesRead, dwBytesWritten;

	wReturnCode = SUCCESS;
	bSender = NULL;
	pbSenderName = pbReceiverName = pbFileName = NULL;
	bSenderNameReceiverNameFileName = NULL;

	hPipe = *(HANDLE*)Argument;

	memset(bReceiver, 0, 255 * sizeof(BYTE));
	memset(bFileName, 0, 255 * sizeof(BYTE));

	bSender = (BYTE*)calloc(255, sizeof(BYTE));

	if (ReadFile(hPipe, &ClientRequiresData, sizeof(BOOL), &dwBytesRead, NULL) == FALSE)
	{
		wReturnCode = PIPE_READ_FAILED;
		goto End;
	}
	if (!ClientRequiresData)
	{
		wReturnCode = SUCCESS;
		goto End;
	}

	if (ReadFile(hPipe, &wReceiverNameSize, sizeof(WORD), &dwBytesRead, NULL) == FALSE)
	{
		wReturnCode = PIPE_READ_FAILED;
		goto End;
	}
	if (ReadFile(hPipe, bReceiver, wReceiverNameSize * sizeof(BYTE), &dwBytesRead, NULL) == FALSE)
	{
		wReturnCode = PIPE_READ_FAILED;
		goto End;
	}
	bReceiver[wReceiverNameSize] = '\0';

	wResponse = ClientExists(bReceiver);
	if (wResponse == INVALID_USERNAME)
	{
		if (WriteFile(hPipe, &wResponse, sizeof(WORD), &dwBytesWritten, NULL) == FALSE)
			wReturnCode = PIPE_WRITE_FAILED;
		goto End;
		
	}

	wResponse = GetClientByPipe(hPipe, &bSender);
	if (wResponse != SUCCESS)
	{
		if (WriteFile(hPipe, &wResponse, sizeof(WORD), &dwBytesWritten, NULL) == FALSE)
			wReturnCode = PIPE_WRITE_FAILED;
		goto End;
	}
	
	
	wResponse = ClientLoggedIn(bReceiver, INVALID_HANDLE_VALUE);
	if (WriteFile(hPipe, &wResponse, sizeof(WORD), &dwBytesWritten, NULL) == FALSE)
	{
		wReturnCode = PIPE_WRITE_FAILED;
		goto End;
	}

	if (wResponse == FALSE)
	{
		wReturnCode = SUCCESS;
		goto End;
	}

	if (ReadFile(hPipe, &ClientRequiresData, sizeof(BOOL), &dwBytesRead, NULL) == FALSE)
	{
		wReturnCode = PIPE_READ_FAILED;
		goto End;
	}

	if (!ClientRequiresData)
	{
		wReturnCode = SUCCESS;
		goto End;
	}

	if (ReadFile(hPipe, &wFileNameSize, sizeof(WORD), &dwBytesRead, NULL) == FALSE)
	{
		wReturnCode = PIPE_READ_FAILED;
		goto End;
	}
	if (ReadFile(hPipe, bFileName, wFileNameSize * sizeof(BYTE), &dwBytesRead, NULL) == FALSE)
	{
		wReturnCode = PIPE_READ_FAILED;
		goto End;
	}

	bSenderNameReceiverNameFileName = (BYTE**)calloc(3, sizeof(BYTE*));
	pbSenderName = (BYTE*)calloc(255, sizeof(BYTE));
	pbReceiverName = (BYTE*)calloc(255, sizeof(BYTE));
	pbFileName = (BYTE*)calloc(255, sizeof(BYTE));

	wSenderNameSize = (WORD)strlen(bSender) + 1;
	wReceiverNameSize = (WORD)strlen(bReceiver) + 1;
	wFileNameSize = (WORD)strlen(bFileName) + 1;

	MyStrCpy(pbSenderName, bSender, wSenderNameSize);
	MyStrCpy(pbReceiverName, bReceiver, wReceiverNameSize);
	MyStrCpy(pbFileName, bFileName, wFileNameSize);
	pbSenderName[wSenderNameSize] = '\0';
	pbReceiverName[wReceiverNameSize] = '\0';
	pbFileName[wFileNameSize] = '\0';	

	bSenderNameReceiverNameFileName[0] = pbSenderName;
	bSenderNameReceiverNameFileName[1] = pbReceiverName;
	bSenderNameReceiverNameFileName[2] = pbFileName;

	hFileSenderThread = CreateThread(NULL, 0, &ThreadSendFile, bSenderNameReceiverNameFileName, 0, NULL);

End:
	if (bSender != NULL)
		free(bSender);

	return wReturnCode;
}

WORD ServerList(LPVOID Argument)
{
	HANDLE hPipe;
	DWORD dwBytesWritten;

	hPipe = *(HANDLE*)Argument;
	
	EnterCriticalSection(&gCriticalSection);
	if (WriteFile(hPipe, &(gClientVector.dwSize), sizeof(DWORD), &dwBytesWritten, NULL) == FALSE)
	{
		printf(__TEXT("Unexpected error: communication with the client failed (error code: %d)\n"), GetLastError());
		LeaveCriticalSection(&gCriticalSection);
		return PIPE_WRITE_FAILED;
	}
	if (gClientVector.dwSize != 0)
	{
		for (DWORD i = 0; i < gClientVector.dwSize; i++)
		{
			DWORD dwSize = strlen(gClientVector.Clients[i].lpsClientName) + 1;
			if (WriteFile(hPipe, &dwSize, sizeof(DWORD), &dwBytesWritten, NULL) == FALSE)
			{
				printf(__TEXT("Unexpected error: communication with the client failed (error code: %d)\n"), GetLastError());
				LeaveCriticalSection(&gCriticalSection);
				return PIPE_WRITE_FAILED;
			}
			if (WriteFile(hPipe, gClientVector.Clients[i].lpsClientName, dwSize * sizeof(BYTE), &dwBytesWritten, NULL) == FALSE)
			{
				printf(__TEXT("Unexpected error: communication with the client failed (error code: %d)\n"), GetLastError());
				LeaveCriticalSection(&gCriticalSection);
				return PIPE_WRITE_FAILED;
			}
		}
	}
	LeaveCriticalSection(&gCriticalSection);

	return SUCCESS;
}

DWORD WINAPI ThreadSendFile(LPVOID Argument)
{
	HANDLE hPipe_fromSender;
	BYTE bSenderName[255], bReceiverName[255], bFileName[255];
	WORD wSenderNameSize, wReceiverNameSize, wFileNameSize;
	DWORD dwBytesRead, dwBytesWritten;

	BYTE** Names = (BYTE**)Argument;
	wSenderNameSize = (WORD)strlen(Names[0]) + 1;
	wReceiverNameSize = (WORD)strlen(Names[1]) + 1;
	wFileNameSize = (WORD)strlen(Names[2]) + 1;

	memset(bSenderName, 0, 255 * sizeof(BYTE));
	memset(bReceiverName, 0, 255 * sizeof(BYTE));
	memset(bFileName, 0, 255 * sizeof(BYTE));

	MyStrCpy(bSenderName, Names[0], wSenderNameSize);
	MyStrCpy(bReceiverName, Names[1], wReceiverNameSize);
	MyStrCpy(bFileName, Names[2], wFileNameSize);
	bSenderName[wSenderNameSize] = '\0';
	bReceiverName[wReceiverNameSize] = '\0';
	bFileName[wFileNameSize] = '\0';

	free(Names[2]);
	free(Names[1]);
	free(Names[0]);
	free(Names);

	hPipe_fromSender = CreateNamedPipeA(PIPE_FILE_NAME, PIPE_ACCESS_DUPLEX, PIPE_TYPE_MESSAGE, PIPE_UNLIMITED_INSTANCES, 4096, 4096, 0, NULL);
	if (ConnectNamedPipe(hPipe_fromSender, NULL) == FALSE)
		return -1;

	BYTE bTypeOfSend = SENDFILE;
	for (DWORD i = 0; i < gClientVector.dwSize; i++)
	{
		if (strcmp(bReceiverName, gClientVector.Clients[i].lpsClientName) == 0)
		{
			BYTE bBuffer[4096];
			memset(bBuffer, 0, 4096 * sizeof(BYTE));

			WriteFile(gClientVector.Clients[i].hMessagesPipe, &bTypeOfSend, sizeof(BYTE), &dwBytesWritten, NULL);

			WriteFile(gClientVector.Clients[i].hMessagesPipe, &wSenderNameSize, sizeof(WORD), &dwBytesRead, NULL);
			WriteFile(gClientVector.Clients[i].hMessagesPipe, bSenderName, wSenderNameSize * sizeof(BYTE), &dwBytesRead, NULL);

			WriteFile(gClientVector.Clients[i].hMessagesPipe, &wFileNameSize, sizeof(WORD), &dwBytesRead, NULL);
			WriteFile(gClientVector.Clients[i].hMessagesPipe, bFileName, wFileNameSize * sizeof(BYTE), &dwBytesRead, NULL);

			ReadFile(hPipe_fromSender, bBuffer, 4096, &dwBytesRead, NULL);
			while (dwBytesRead != 0 || dwBytesWritten != 0)
			{
				WriteFile(gClientVector.Clients[i].hMessagesPipe, bBuffer, dwBytesRead, &dwBytesWritten, NULL);
				if (dwBytesRead < 4096)
					break;
				memset(bBuffer, 0, 4096 * sizeof(BYTE));
				ReadFile(hPipe_fromSender, bBuffer, 4096, &dwBytesRead, NULL);
			}
		}
	}
	return SUCCESS;
}

BYTE ClientVectorInitialize(void)
{
	gClientVector.dwSize = 0;
	gClientVector.dwCapacity = gdwMaxClients + 1;
	gClientVector.Clients = (CLIENT*)calloc(gClientVector.dwCapacity, sizeof(CLIENT));
	
	if (gClientVector.Clients == NULL)
		return MALLOC_FAILURE;
	return SUCCESS;
}

BYTE ClientVectorDestroy(void)
{
	for (DWORD i = 0; i < gClientVector.dwSize; i++)
		free(gClientVector.Clients[i].lpsClientName);
	free(gClientVector.Clients);
	return SUCCESS;
}

WORD RegistrationFileInitialize(void)
{
	WORD wReturnCode = SUCCESS;

	// Tries to open an existing file
	ghRegistrationFile = CreateFileA(
		FILE_PATH,
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (ghRegistrationFile == INVALID_HANDLE_VALUE)
	{
		if (GetLastError() == 2)
		{
			// If the file doesn't exists, it
			// will be created and opened

			ghRegistrationFile = CreateFileA(
				FILE_PATH,
				GENERIC_READ | GENERIC_WRITE,
				0,
				NULL,
				OPEN_ALWAYS,
				FILE_ATTRIBUTE_NORMAL,
				NULL);

			if (ghRegistrationFile == INVALID_HANDLE_VALUE)
				wReturnCode = FILE_CREATE_FAILED;
		}
		else
			wReturnCode = FILE_CREATE_FAILED;
	}

	return wReturnCode;
}

WORD MessageHistoryFileInitialize(void)
{
	WORD wReturnCode;
	wReturnCode = SUCCESS;

	ghMessagesHistoryFile = CreateFileA(MESSAGE_HISTORY_FILE_PATH, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (ghMessagesHistoryFile == INVALID_HANDLE_VALUE)
		wReturnCode = FILE_CREATE_FAILED;

	return wReturnCode;
}
