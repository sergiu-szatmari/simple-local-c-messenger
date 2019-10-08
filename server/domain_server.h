#ifndef _DOMAIN_H_
#define _DOMAIN_H_

#include <Windows.h>

#include "commands.h"
#include "return.h"

#define PIPE_NAME "\\\\.\\pipe\\cc_server_client_pipe"
#define PIPE_MESSAGE_NAME "\\\\.\\pipe\\cc_server_client_msg_pipe"
#define PIPE_FILE_NAME "\\\\.\\pipe\\cc_server_client_file_pipe"

/**
 * The handle to the pipe that 
 * enables the server to 
 * comunicate with the clients;
 * The handles to the pipe which
 * the clients receive messages through
 */
HANDLE ghPipe;
HANDLE ghMessagePipe;

/**
 * The request that gathers
 * the information received
 * from the client through
 * the pipe
 */
typedef struct _REQUEST
{
	HANDLE hPipeToReply;
	BYTE bTypeOfRequest;
	WORD wArgumentSize;
	BYTE* bArgument;
} REQUEST;

/**
 * Threadpool's threads' work item structure,
 * which has a function that has to be executed
 * along with a pointer to its argument
 */
typedef struct _WORK_ITEM
{
	void(*FunctionAddress)(LPVOID);
	LPVOID FunctionArgument;
} WORK_ITEM;

/*
 * The argument of a work item, which
 * has two pipes, one for regular 
 * communication between the client and
 * the server, and the other one is mainly
 * used for sending messages (the client only
 * reads).
 */
typedef struct _ARGUMENT
{
	HANDLE* phPipe;
	HANDLE* phMessagePipe;
} ARGUMENT;

/**
 * Items required for the proper 
 * functioning of the queue. The 
 * queue is implemented using a 
 * double linked list.
 */
#define TElem WORK_ITEM
#define MAX_ELEMENTS 1024

/**
 * The node structure
 */
typedef struct _NODE
{
	struct _NODE* Prev;
	struct _NODE* Next;
	TElem* Info;
} NODE;

/**
 * The Queue structure
 */
typedef struct _QUEUE
{
	struct _NODE* First;
	struct _NODE* Last;
	int Size;
} QUEUE;


/**
 * The event that is being waited
 * for by the threads when a new
 * work item has been added to the
 * queue.
 */
HANDLE ghEvent;

/**
 * The queue which will store
 * the work items that will be
 * executed by the threads
 */
QUEUE* gQueue;

/**
 * The tool for the proper syncronization
 * of the threads and for a good working
 * parrallelism, especially when multiple
 * threads work with the items of the queue
 */
CRITICAL_SECTION gCriticalSection;

/**
 * The threadpool structure, which
 * has an array of handles for the 
 * threads created when the server 
 * process begins, and also their
 * number.
 */
typedef struct _THREAD_POOL
{
	HANDLE* hThreads;
	DWORD dwThreadSize;
} THREAD_POOL;

THREAD_POOL* gpThreadPool;

DWORD gdwClientsConnected;
DWORD gdwMaxClients;

/*
 * A handle to the registration 
 * file, where all the usernames 
 * and passwords are stored.
 */
#define FILE_PATH "C:\\registration.txt"
HANDLE ghRegistrationFile;

/*
 * A handle to the message history
 * file, along with the path to it
 */
#define MESSAGE_HISTORY_FILE_PATH "C:\\messagesHistory.txt"
HANDLE ghMessagesHistoryFile;

/*
 * A client's structure, which contains
 * a string with the client's name, and
 * his pipe's handle, so he can be reached
 */
typedef struct _CLIENT
{
	LPSTR lpsClientName;
	HANDLE hClientPipe;
	HANDLE hMessagesPipe;
} CLIENT;

/*
 * An array of CLIENT structures, so the
 * server can manage the messages between
 * the clients
 */
typedef struct _CLIENT_VECTOR
{
	CLIENT* Clients;
	DWORD dwSize;
	DWORD dwCapacity;
} CLIENT_VECTOR;
CLIENT_VECTOR gClientVector;

#endif _DOMAIN_H_