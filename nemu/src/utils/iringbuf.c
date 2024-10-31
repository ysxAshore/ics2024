#include <common.h>
#define MAX_INST_TO_PRINT 10
typedef struct IringNode
{
    bool state;
    char p[128];
} INode;
static INode nodes[MAX_INST_TO_PRINT];
static int cur = 0, last = 0;
void initNodes()
{
    for (int i = 0; i < MAX_INST_TO_PRINT; i++)
    {
        nodes[i].state = true;
        strcpy(nodes[i].p, "\0");
    }
}
void insertNode(bool state, char *p)
{
    INode temp;
    temp.state = state;
    strcpy(temp.p, p);
    nodes[cur] = temp;
    last = cur;
    cur = (cur + 1) % MAX_INST_TO_PRINT;
}
void modifyNodeState(bool state)
{
    nodes[last].state = state;
}
void printNodes()
{
    for (int i = 0; i < MAX_INST_TO_PRINT; i++)
    {
        if (nodes[i].p[0] == '\0')
            continue;
        else
        {
            if (nodes[i].state)
                printf("    ");
            else
                printf("--> ");
            printf("%s \n", nodes[i].p);
        }
    }
}