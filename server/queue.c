#include "queue.h"

int NodeCreate(TElem* Information, NODE** Node)
{
	if ((*Node) != NULL)
		return INVALID_PARAMETER;

	(*Node) = (NODE*)calloc(1, sizeof(NODE));
	(*Node)->Next = NULL;
	(*Node)->Prev = NULL;
	(*Node)->Info = Information;

	return SUCCESS;
}

int NodeDestroy(NODE** Node)
{
	if ((*Node) == NULL)
		return INVALID_PARAMETER;

	free((*Node));
	return SUCCESS;
}

int QueueCreate(QUEUE** Queue)
{
	if (Queue == NULL)
		return INVALID_PARAMETER;
	if ((*Queue) != NULL)
		return INVALID_PARAMETER;

	(*Queue) = NULL;
	(*Queue) = (QUEUE*)calloc(1, sizeof(QUEUE));
	if ((*Queue) == NULL)
		return MALLOC_FAILURE;

	(*Queue)->First = (*Queue)->Last = NULL;
	(*Queue)->Size = 0;

	return SUCCESS;
}

BOOL QueueIsEmpty(QUEUE* Queue)
{
	if (Queue->Size == 0)
		return TRUE;
	return FALSE;
}

BOOL QueueIsFull(QUEUE* Queue)
{
	if (Queue->Size == MAX_ELEMENTS)
		return TRUE;
	return FALSE;
}

int QueuePush(QUEUE* Queue, TElem* Element)
{
	if (Queue == NULL)
		return INVALID_PARAMETER;
	if (Element == NULL)
		return INVALID_PARAMETER;

	if (QueueIsFull(Queue) == TRUE)
		return QUEUE_FULL;

	NODE* Node = NULL;
	if (NodeCreate(Element, &Node) < 0)
		return INNER_FUNC_ERROR;

	if (QueueIsEmpty(Queue) == TRUE)
	{
		// If the node is the first element
		// to be added in the queue
		Queue->First = Node;
		Queue->Last = Node;
		
	}
	else
	{
		// If the queue already has elements
		Queue->Last->Next = Node;
		Node->Prev = Queue->Last;
		Queue->Last = Node;
	}
	Queue->Size++;

	return SUCCESS;
}

int QueuePop(QUEUE* Queue, TElem** Element)
{
	if (Queue == NULL)
		return INVALID_PARAMETER;
	if ((*Element) != NULL)
		return INVALID_PARAMETER;

	if (QueueIsEmpty(Queue) == TRUE)
		return QUEUE_EMPTY;

	NODE* Pop = Queue->First;
	Queue->First = Queue->First->Next;

	if (Queue->First != NULL)
		Queue->First->Prev = NULL;
	(*Element) = Pop->Info;

	if (NodeDestroy(&Pop) < 0)
		return INNER_FUNC_ERROR;
	Queue->Size--;

	return SUCCESS;
}

int QueueDestroy(QUEUE** Queue)
{
	if (Queue == NULL)
		return INVALID_PARAMETER;
	if ((*Queue) == NULL)
		return INVALID_PARAMETER;

	NODE* CurrentNode = (*Queue)->First;
	while (CurrentNode != NULL)
	{
		NODE* ToBeDeleted = CurrentNode;
		CurrentNode = CurrentNode->Next;
		
		free(ToBeDeleted->Info);
		if (NodeDestroy(&ToBeDeleted) < 0)
			return INNER_FUNC_ERROR;
	}
	free((*Queue));

	return SUCCESS;
}

