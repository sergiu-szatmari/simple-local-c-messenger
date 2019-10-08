#ifndef _THREAD_POOL_H_
#define _THREAD_POOL_H_

#include <Windows.h>
#include <stdlib.h>
#include <stdio.h>

#include "domain_server.h"
#include "queue.h"

int ThreadpoolCreate(_Inout_ THREAD_POOL** thPool);
int ThreadpoolCreateEx(_Inout_ THREAD_POOL** thPool, _In_ BYTE bMaxNrOfClients);
int ThreadpoolDestory(_In_ THREAD_POOL** thPool);
int ThreadpoolAddWorkItem(_In_ WORK_ITEM* WorkItem);
int ThreadpoolRemoveWorkItem(_Out_ WORK_ITEM** WorkItem);

#endif // _THREAD_POOL_H_
