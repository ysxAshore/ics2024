#include <common.h>
#define MAX_INST_TO_PRINT 10
typedef struct IringNode
{
    bool state;
    vaddr_t pc;
    char p[128];
    uint8_t *inst;
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
INode *createNode(bool state, vaddr_t pc, char *p, uint32_t inst)
{
    INode *newNode = (INode *)malloc(sizeof(INode));
    newNode->state = state;
    newNode->pc = pc;
    strcpy(newNode->p, p);
    newNode->inst = (uint8_t *)&inst;
    newNode->next = NULL;
    return newNode;
}

// 尾部插入
void insertAtTail(bool state, vaddr_t pc, char *p, uint32_t inst)
{
    INode *newNode = createNode(state, pc, p, inst);
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
        if (list->len > 10)
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
        printf("%lx ", temp->pc);
        printf("%s ", temp->p);
        printf("%02x %02x %02x %02x\n", *temp->inst, *(temp->inst + 1), *(temp->inst + 2), *(temp->inst + 3));
        temp = temp->next;
    }
    printf("NULL\n");
}