#ifndef _DOMAIN_H_
#define _DOMAIN_H_

#include <Windows.h>

#include "return.h"

#define PIPE_NAME "\\\\.\\pipe\\cc_server_client_pipe"
#define PIPE_MESSAGE_NAME "\\\\.\\pipe\\cc_server_client_msg_pipe"
#define PIPE_FILE_NAME "\\\\.\\pipe\\cc_server_client_file_pipe"

/*
 * The avaliable commands supported 
 * by the client's application, and
 * the commands' number
 */
static LPCSTR gpszCommands[] = { "echo", "register", "login", "logout", "msg", "broadcast", "sendfile", "list", "exit", "history" };
static BYTE gbNumberOfCommands = 10;

/*
 * The request's structure, that 
 * has a byte which represents 
 * the type of command (eg. echo,
 * exit), the size of the argument
 * and the argument itself
 */
typedef struct _REQUEST
{
	BYTE bType;
	WORD wMessageSize;
	BYTE* bMessage;
} REQUEST;

/*
 * The pipes' handles;
 * One pipe for regular communication
 * and the other one for receiving 
 * messages from other users only
 */
HANDLE ghPipe;
HANDLE ghMessagePipe;

/*
 * A boolean variable which tells if
 * the user's curren session is logged
 * in at the server
 */
BOOL gUserLoggedIn;

/*
 * The lock used for printing,
 * so user inputs and outputs
 * won't merge unethically with
 * any messages received
 */
CRITICAL_SECTION gCriticalSection;

#endif _DOMAIN_H_