#include "threadpool.h"

DWORD WINAPI
ThreadMainFunction(LPVOID Args)
{
	DWORD dwWaitResult;

Wait:	
	dwWaitResult = WaitForSingleObject(ghEvent, INFINITE);

	switch (dwWaitResult)
	{
		case WAIT_OBJECT_0:
		{
			WORK_ITEM* WorkItem = NULL;
			
			EnterCriticalSection(&gCriticalSection);
			BOOL QueueEmpty = QueueIsEmpty(gQueue);
			LeaveCriticalSection(&gCriticalSection);
			if (QueueEmpty == TRUE)
				goto Wait;
			if (ThreadpoolRemoveWorkItem(&WorkItem) == SUCCESS)
			{
				WorkItem->FunctionAddress(WorkItem->FunctionArgument);
				free(WorkItem);
			}
		}

	}
	goto Wait;

	return SUCCESS;
}

int ThreadpoolInit(THREAD_POOL** thPool)
{
	int ReturnCode = SUCCESS;

	if (thPool == NULL)
		return INVALID_PARAMETER;
	if ((*thPool) != NULL)
		return INVALID_PARAMETER;

	(*thPool) = NULL;
	(*thPool) = (THREAD_POOL*)calloc(1,sizeof(THREAD_POOL));
	if ((*thPool) == NULL)
		return MALLOC_FAILURE;

	SYSTEM_INFO SysInfo;
	GetSystemInfo(&SysInfo);

	(*thPool)->hThreads = (HANDLE*)calloc(SysInfo.dwNumberOfProcessors, sizeof(HANDLE));
	if ((*thPool)->hThreads == NULL)
	{
		ReturnCode = MALLOC_FAILURE;
		goto CleanUp;
	}
	(*thPool)->dwThreadSize = SysInfo.dwNumberOfProcessors;

	goto End;
CleanUp:
	if ((*thPool) != NULL)
		free((*thPool));

End:
	return ReturnCode;

}

int ThreadpoolInitEx(THREAD_POOL** thPool, BYTE bMaxNrOfClients)
{
	int ReturnCode = SUCCESS;

	if (thPool == NULL)
		return INVALID_PARAMETER;
	if ((*thPool) != NULL)
		return INVALID_PARAMETER;

	(*thPool) = NULL;
	(*thPool) = (THREAD_POOL*)calloc(1, sizeof(THREAD_POOL));
	if ((*thPool) == NULL)
		return MALLOC_FAILURE;

	(*thPool)->hThreads = (HANDLE*)calloc(bMaxNrOfClients, sizeof(HANDLE));
	if ((*thPool)->hThreads == NULL)
	{
		ReturnCode = MALLOC_FAILURE;
		goto CleanUp;
	}
	(*thPool)->dwThreadSize = (DWORD)bMaxNrOfClients;

	goto End;
CleanUp:
	if ((*thPool) != NULL)
		free((*thPool));

End:
	return ReturnCode;
}

int EventInit()
{
	ghEvent = NULL;
	ghEvent = CreateEvent(
		NULL,
		TRUE,
		FALSE,
		TEXT("MyEvent")
	);

	if (ghEvent == NULL)
		return EVENT_CREATE_FAILED;

	return SUCCESS;
}

int QueueInit()
{
	gQueue = NULL;
	if (QueueCreate(&gQueue) < 0)
		return INNER_FUNC_ERROR;
	return SUCCESS;
}

int ThreadsInit(THREAD_POOL* thPool)
{
	int ReturnCode = SUCCESS;

	if (thPool == NULL)
		return INVALID_PARAMETER;

	DWORD i;
	for (i = 0; i < thPool->dwThreadSize; i++)
	{
		thPool->hThreads[i] = NULL;
		thPool->hThreads[i] = CreateThread(
			NULL,
			0,
			ThreadMainFunction,
			NULL,
			0,
			NULL
		);
		
		if (thPool->hThreads[i] == NULL)
		{
			ReturnCode = THREAD_CREATE_ERROR;
			goto CleanUpThreads;
		}
	}
	goto End;

CleanUpThreads:
	for (i = 0; i < thPool->dwThreadSize; i++)
		if (thPool->hThreads[i] != NULL)
			CloseHandle(thPool->hThreads[i]);

End:
	return ReturnCode;
}

int ThreadpoolCreate(THREAD_POOL** thPool)
{
	(*thPool) = NULL;
	ghEvent = NULL;
	gQueue = NULL;
	int ReturnCode = SUCCESS;
	
	if (ThreadpoolInit(thPool) < 0)
		return INNER_FUNC_ERROR;

	if (EventInit() < 0)
	{
		ReturnCode = INNER_FUNC_ERROR;
		goto CleanUpAll;
	}

	if (QueueInit() < 0)
	{
		ReturnCode = INNER_FUNC_ERROR;
		goto CleanUpAll;
	}

	if (ThreadsInit(*thPool) < 0)
	{
		ReturnCode = INNER_FUNC_ERROR;
		goto CleanUpAll;
	}

	InitializeCriticalSection(&gCriticalSection);


	goto EndFunc;

CleanUpAll:
	if (thPool != NULL)
	{
		free((*thPool)->hThreads);
		free((*thPool));
	}
	
	if (ghEvent != NULL)
		CloseHandle(ghEvent);
	
	if (gQueue != NULL)
		QueueDestroy(&gQueue);
	
EndFunc:
	return ReturnCode;
}

int ThreadpoolCreateEx(THREAD_POOL ** thPool, BYTE bMaxNrOfClients)
{
	(*thPool) = NULL;
	ghEvent = NULL;
	gQueue = NULL;
	int ReturnCode = SUCCESS;

	if (ThreadpoolInitEx(thPool, bMaxNrOfClients) < 0)
		return INNER_FUNC_ERROR;

	if (EventInit() < 0)
	{
		ReturnCode = INNER_FUNC_ERROR;
		goto CleanUpAll;
	}

	if (QueueInit() < 0)
	{
		ReturnCode = INNER_FUNC_ERROR;
		goto CleanUpAll;
	}

	if (ThreadsInit(*thPool) < 0)
	{
		ReturnCode = INNER_FUNC_ERROR;
		goto CleanUpAll;
	}

	InitializeCriticalSection(&gCriticalSection);


	goto EndFunc;

CleanUpAll:
	if (thPool != NULL)
	{
		free((*thPool)->hThreads);
		free((*thPool));
	}

	if (ghEvent != NULL)
		CloseHandle(ghEvent);

	if (gQueue != NULL)
		QueueDestroy(&gQueue);

EndFunc:
	return ReturnCode;
}

int ThreadpoolDestory(THREAD_POOL** thPool)
{
	DWORD i;
	for (i = 0; i < (*thPool)->dwThreadSize; i++)
	{
		//WaitForSingleObject((*thPool)->hThreads[i], INFINITE);
		TerminateThread((*thPool)->hThreads[i], 0);
	}

	for (i = 0; i < (*thPool)->dwThreadSize; i++)
		if ((*thPool)->hThreads[i] != NULL)
			CloseHandle((*thPool)->hThreads[i]);
				

	DeleteCriticalSection(&gCriticalSection);

	if (thPool == NULL)
		return INVALID_PARAMETER;
	if ((*thPool) == NULL)
		return INVALID_PARAMETER;
	CloseHandle(ghEvent);

	if (QueueDestroy(&gQueue) < 0)
		return INNER_FUNC_ERROR;

	
	free((*thPool)->hThreads);
	free((*thPool));

	return SUCCESS;
}

int ThreadpoolAddWorkItem(WORK_ITEM* WorkItem)
{
	int ReturnCode = SUCCESS;

	if (WorkItem == NULL)
		return INVALID_PARAMETER;

	EnterCriticalSection(&gCriticalSection);
	ReturnCode = QueuePush(gQueue, WorkItem);
	SetEvent(ghEvent);
	LeaveCriticalSection(&gCriticalSection);

	return ReturnCode;
}

int ThreadpoolRemoveWorkItem(WORK_ITEM** WorkItem)
{
	int ReturnCode = SUCCESS;

	if (WorkItem == NULL)
		return INVALID_PARAMETER;
	if ((*WorkItem) != NULL)
		return INVALID_PARAMETER;

	EnterCriticalSection(&gCriticalSection);
	ReturnCode = QueuePop(gQueue, WorkItem);
	if (QueueIsEmpty(gQueue) == TRUE)
		ResetEvent(ghEvent);
	LeaveCriticalSection(&gCriticalSection);

	return ReturnCode;
}
