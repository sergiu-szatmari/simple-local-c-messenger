#include "client.h"

void MyStrCpy(BYTE* bDestination, BYTE * bSource, DWORD wCount)
{
	for (DWORD i = 0; i < wCount; i++)
		bDestination[i] = bSource[i];
}

void GetFileName(BYTE* bPath, BYTE** bFilename)
{
	WORD wIndexOfLastSlash = 0, wSizeOfPath = (WORD)strlen(bPath) + 1, wIndex = 0;

	while (wIndex < wSizeOfPath)
	{
		if (bPath[wIndex] == '\\')
			wIndexOfLastSlash = wIndex;
		wIndex++;
	}

	MyStrCpy((*bFilename), bPath + wIndexOfLastSlash + 1, wSizeOfPath - wIndexOfLastSlash);
	(*bFilename)[wSizeOfPath - wIndexOfLastSlash] = '\0';
}

void GetCurrentFolder(BYTE* bPath, BYTE** bPathToCurrentFolder)
{
	WORD wIndex = 0;
	WORD wIndexLastSlash = -1;
	DWORD dwPathSize = (DWORD)strlen(bPath) + 1;

	while (wIndex < dwPathSize)
	{
		if (bPath[wIndex] == '\\')
			wIndexLastSlash = wIndex;
		wIndex++;
	}

	MyStrCpy((*bPathToCurrentFolder), bPath, wIndexLastSlash + 1);
	(*bPathToCurrentFolder)[wIndexLastSlash + 1] = '\0';
}

DWORD WINAPI ClientWaitForMessages(LPVOID Argument)
{
	while (TRUE)
	{
		DWORD dwBytesRead = 0;
		WORD wSize = 0;
		BYTE bSenderUsername[255];
		BYTE bMessage[255];
		BYTE bTypeOfSend;

		memset(bSenderUsername, 0, 255 * sizeof(BYTE));
		memset(bMessage, 0, 255 * sizeof(BYTE));

		ReadFile(ghMessagePipe, &bTypeOfSend, sizeof(BYTE), &dwBytesRead, NULL);

		switch (bTypeOfSend)
		{
			case MSG:
			{
				ReadFile(ghMessagePipe, &wSize, sizeof(WORD), &dwBytesRead, NULL);
				ReadFile(ghMessagePipe, bSenderUsername, wSize * sizeof(BYTE), &dwBytesRead, NULL);
				bSenderUsername[wSize] = '\0';

				ReadFile(ghMessagePipe, &wSize, sizeof(WORD), &dwBytesRead, NULL);
				ReadFile(ghMessagePipe, bMessage, wSize * sizeof(BYTE), &dwBytesRead, NULL);
				bMessage[wSize] = '\0';

				if (gUserLoggedIn == TRUE)
					printf(__TEXT("Message from %s: %s\n"), bSenderUsername, bMessage);
				break;
			}

			case BROADCAST:
			{
				ReadFile(ghMessagePipe, &wSize, sizeof(WORD), &dwBytesRead, NULL);
				ReadFile(ghMessagePipe, bSenderUsername, wSize * sizeof(BYTE), &dwBytesRead, NULL);
				bSenderUsername[wSize] = '\0';

				ReadFile(ghMessagePipe, &wSize, sizeof(WORD), &dwBytesRead, NULL);
				ReadFile(ghMessagePipe, bMessage, wSize * sizeof(BYTE), &dwBytesRead, NULL);
				bMessage[wSize] = '\0';

				if (gUserLoggedIn == TRUE)
					printf(__TEXT("Broadcast from %s: %s\n"), bSenderUsername, bMessage);
				break;
			}

			case SENDFILE:
			{
				BYTE bCurrentPath[1024];
				BYTE* bPath;
				BYTE bSender[255];
				BYTE bFilename[1024];
				BYTE bBuffer[4096];
				WORD wFilenameSize, wSenderNameSize;
				DWORD dwBytesWritten = 1;

				memset(bCurrentPath, 0, 1024 * sizeof(BYTE));
				memset(bFilename, 0, 1024 * sizeof(BYTE));
				memset(bSender, 0, 255 * sizeof(BYTE));
				memset(bBuffer, 0, 4096 * sizeof(BYTE));

				GetModuleFileNameA(NULL, bCurrentPath, 1024 * sizeof(BYTE));
				bPath = (BYTE*)calloc(255, sizeof(BYTE));
				GetCurrentFolder(bCurrentPath, &bPath);

				ReadFile(ghMessagePipe, &wSenderNameSize, sizeof(WORD), &dwBytesRead, NULL);
				ReadFile(ghMessagePipe, bSender, wSenderNameSize * sizeof(BYTE), &dwBytesRead, NULL);

				ReadFile(ghMessagePipe, &wFilenameSize, sizeof(WORD), &dwBytesRead, NULL);
				ReadFile(ghMessagePipe, bFilename, wFilenameSize * sizeof(BYTE), &dwBytesRead, NULL);

				bPath = strcat(bPath, bFilename);

				HANDLE hFileReceived = CreateFileA(bPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

				ReadFile(ghMessagePipe, bBuffer, 4096, &dwBytesRead, NULL);
				while (dwBytesRead != 0 || dwBytesWritten != 0)
				{
					WriteFile(hFileReceived, bBuffer, dwBytesRead, &dwBytesWritten, NULL);
					if (dwBytesRead < 4096)
						break;
					memset(bBuffer, 0, 4096 * sizeof(BYTE));
					ReadFile(ghMessagePipe, bBuffer, 4096, &dwBytesRead, NULL);
				}

				CloseHandle(hFileReceived);
				printf(__TEXT("Received file: %s from %s\n"), bFilename, bSender);
				break;
			}
		}
	}
}

DWORD WINAPI
ClientSendFile(LPVOID Argument)
{
	DWORD dwBytesRead, dwBytesWritten;

	BYTE bPath[255];
	BYTE bName[255];
	memset(bPath, 0, 255 * sizeof(BYTE));
	memset(bName, 0, 255 * sizeof(BYTE));

	BYTE* bArgument = (BYTE*)Argument;
	DWORD dwSize = strlen(bArgument) + 1;

	MyStrCpy(bPath, bArgument, dwSize);
	bPath[dwSize] = '\0';

	DWORD dwIndex = 0, dwLastSlash = -1;
	while (dwIndex < dwSize)
	{
		if (bPath[dwIndex] == '\\')
			dwLastSlash = dwIndex;
		dwIndex++;
	}

	MyStrCpy(bName, bPath + dwLastSlash + 1, dwSize - dwLastSlash);
	bName[dwSize - dwLastSlash] = '\0';

	free(bArgument);

	HANDLE hFileSendPipe = CreateFileA(
		PIPE_FILE_NAME,
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);

	if (hFileSendPipe == INVALID_HANDLE_VALUE)
	{
		printf(__TEXT("Unexpected error: sending the file failed\n"));
		return -1;
	}

	HANDLE hFileToSend = CreateFileA(
		bPath,
		GENERIC_READ,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);

	if (hFileToSend == INVALID_HANDLE_VALUE && GetLastError() == 2)
	{
		printf("Error: File not found\n");
		return -1;
	}

	BYTE bBuffer[4096];
	memset(bBuffer, 0, 4096 * sizeof(BYTE));

	ReadFile(hFileToSend, bBuffer, 4096, &dwBytesRead, NULL);
	while (dwBytesRead != 0)
	{
		WriteFile(hFileSendPipe, bBuffer, dwBytesRead, &dwBytesWritten, NULL);
		if (dwBytesRead < 4096)
			break;
		memset(bBuffer, 0, 4096 * sizeof(BYTE));
		ReadFile(hFileToSend, bBuffer, 4096, &dwBytesRead, NULL);
	}

	printf(__TEXT("Succesfully sent file: %s\n"), bPath);
	CloseHandle(hFileToSend);
	CloseHandle(hFileSendPipe);

	return 0;
}
