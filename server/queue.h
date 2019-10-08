#ifndef _QUEUE_H_
#define _QUEUE_H_

#include <stdlib.h>
#include <Windows.h>
#include <stdio.h>

#include "domain_server.h"

int QueueCreate(_Inout_ QUEUE** Queue);
BOOL QueueIsEmpty(_In_ QUEUE* Queue);
BOOL QueueIsFull(_In_ QUEUE* Queue);
int QueuePush(_In_ QUEUE* Queue, _In_ TElem* Element);
int QueuePop(_In_ QUEUE* Queue, _Out_ TElem** Element);
int QueueDestroy(_In_ QUEUE** Queue);

#endif _QUEUE_H_