#include <common.h>
#define MAX_INST_TO_PRINT 10
typedef struct IringNode
{
    bool state;
    char p[128];
    struct IringNode *next;
} INode;
typedef struct IringList
{
    int len;
    INode *head;
    INode *tail;
} IList;
static IList *list;

void initIList() // initial list
{
    list = (IList *)malloc(sizeof(IList));
    list->len = 0;
    list->head = NULL;
    list->tail = NULL;
}

// 创建新节点
INode *createNode(bool state, char *p)
{
    INode *newNode = (INode *)malloc(sizeof(INode));
    newNode->state = state;
    strcpy(newNode->p, p);
    newNode->next = NULL;
    return newNode;
}

// 尾部插入
void insertAtTail(bool state, char *p)
{
    INode *newNode = createNode(state, p);
    // 如果链表为空，将新节点设置为头和尾
    if (list->head == NULL)
    {
        list->head = newNode;
        list->tail = newNode;
        list->len++;
    }
    else
    {
        list->tail->next = newNode;
        list->tail = newNode;
        list->len++;
        if (list->len > MAX_INST_TO_PRINT)
        {
            INode *tmp = list->head;
            list->head = list->head->next;
            free(tmp);
        }
    }
}
void modifyTheTail(bool state)
{
    list->tail->state = state;
}
void printList()
{
    INode *temp = list->head;
    while (temp != NULL)
    {
        if (temp->state)
            printf("    ");
        else
            printf("--> ");
        printf("%s\n", temp->p);
        temp = temp->next;
    }
    printf("NULL\n");
}