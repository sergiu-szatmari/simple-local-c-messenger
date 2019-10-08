#include "utils.h"

DWORD InputValidation(BYTE** string)
{
	size_t wSize = -1;
	wSize = strlen((*string));
	for (size_t i = 0; i < wSize; i++)
		if ((*string)[i] == '\n')
			(*string)[i] = '\0';

	return SUCCESS;
}

BOOL CommandExists(BYTE* Command)
{
	for (BYTE i = 0; i < gbNumberOfCommands; i++)
		if (strcmp(Command, gpszCommands[i]) == 0)
			return TRUE;
	return FALSE;
}

void GetCommand(BYTE** Input, BYTE** Command, BYTE** Rest)
{
	BOOL bSingleWordInput;
	WORD SizeOfString = (WORD)strlen((*Input));
	WORD i = 0;
	while ((*Input)[i] != ' ' && (*Input)[i] != '\0')
		i++;

	bSingleWordInput = (*Input)[i] == '\0';

	strncpy((*Command), (*Input), i);
	(*Command)[i] = '\0';

	if (bSingleWordInput == FALSE)
		strncpy((*Rest), (*Input) + i + 1, SizeOfString - i);
}

BYTE GetCommandType(BYTE* Input)
{
	if (strcmp(Input, "echo") == 0)
		return ECHO;
	else if (strcmp(Input, "register") == 0)
		return REGISTER;
	else if (strcmp(Input, "login") == 0)
		return LOGIN;
	else if (strcmp(Input, "logout") == 0)
		return LOGOUT;
	else if (strcmp(Input, "msg") == 0)
		return MSG;
	else if (strcmp(Input, "broadcast") == 0)
		return BROADCAST;
	else if (strcmp(Input, "sendfile") == 0)
		return SENDFILE;
	else if (strcmp(Input, "list") == 0)
		return LIST;
	else if (strcmp(Input, "exit") == 0)
		return EXIT;
	else if (strcmp(Input, "history") == 0)
		return HISTORY;
	return INVALID_PARAMETER;
}

void ValidateCommand(BYTE** bCommand)
{
	int size = strlen((*bCommand));
	for (int i = 0; i < size; i++)
		if ((*bCommand)[i] > 'A' && (*bCommand)[i] < 'Z')
			(*bCommand)[i] += 32;
}

BOOL SingleWordCommand(BYTE* bCommand)
{
	int i = 0;
	while (bCommand[i] != ' ' && bCommand[i] != '\0')
		i++;

	if (bCommand[i] == '\0')
		return TRUE;
	return FALSE;
}
