#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <common.h>
#include <elf.h>
typedef struct ftraceNode
{
    int type; // 0 is call,1 is return
    vaddr_t pc;
    char originName[64];
    char p[128];
    struct ftraceNode *next;
} FNode;
typedef struct symFuncNode
{
    char name[64];
    Elf64_Addr addr;
    uint64_t size;
    struct symFuncNode *next;
} SFNode;

static FNode *f_list;  // ftrace list with head node
static SFNode *s_list; // symFunctab with head node
static FNode *tail;

/*
initial ftrace list and read symbol table and string table
*/
void init_ftrace(char *elf_file)
{
    char *strtab = NULL;
    // initialize ftrace and symFunc list head node
    f_list = malloc(sizeof(FNode));
    f_list->next = NULL;
    tail = f_list;
    s_list = malloc(sizeof(SFNode));
    s_list->next = NULL;

    // make sure that elf_file is not null
    if (elf_file == NULL)
        assert("Please given the path of elf file\n");

    // if not,initialize file pointer
    FILE *fp = fp = fopen(elf_file, "rb");
    if (fp == NULL)
        assert("Can not open the given path elf_file ");

    // read ELF header
    Elf64_Ehdr edhr;
    if (fread(&edhr, sizeof(Elf64_Ehdr), 1, fp) < 1)
        assert("Reading the ELF header failed\n");

    // make sure the ELF header magic number
    if (edhr.e_ident[0] != 0x7F ||
        edhr.e_ident[1] != 'E' ||
        edhr.e_ident[2] != 'L' ||
        edhr.e_ident[3] != 'F')
        assert("The given path is not a elf file");

    // read section table and judge the section is or not is strtab
    // if is read strtab
    fseek(fp, edhr.e_shoff, SEEK_SET);
    Elf64_Shdr shdr;
    for (int i = 0; i < edhr.e_shnum; ++i)
    {
        if (fread(&shdr, sizeof(Elf64_Shdr), 1, fp) < 1)
            assert("Reading the ELF Section header failed\n");
        if (shdr.sh_type == SHT_STRTAB)
        {
            fseek(fp, shdr.sh_offset, SEEK_SET);
            strtab = malloc(shdr.sh_size);
            if (fread(strtab, shdr.sh_size, 1, fp) < 1)
                assert("Reading the string tab section failed\n");
            break;
        }
    }

    // read section table and judge the section is or not is symtab
    // if is,read symtab and judge the line is or not is func symbol
    // if is,read the attribute to the list node and add to the list
    fseek(fp, edhr.e_shoff, SEEK_SET);
    for (int i = 0; i < edhr.e_shnum; ++i)
    {
        if (fread(&shdr, sizeof(Elf64_Shdr), 1, fp) < 1)
            assert("Reading the ELF Section header failed\n");
        if (shdr.sh_type == SHT_SYMTAB)
        {
            fseek(fp, shdr.sh_offset, SEEK_SET);
            Elf64_Sym sym;
            int num = shdr.sh_size / shdr.sh_entsize;

            for (int j = 0; j < num; ++j)
            {
                if (fread(&sym, sizeof(Elf64_Sym), 1, fp) < 1)
                    assert("Reading the Symbol line failed\n");
                if (ELF64_ST_TYPE(sym.st_info) == STT_FUNC)
                {
                    SFNode *temp = malloc(sizeof(SFNode));
                    strcpy(temp->name, (char *)(strtab + sym.st_name));
                    temp->addr = sym.st_value;
                    temp->size = sym.st_size;
                    temp->next = s_list->next; // with head node insert
                    s_list->next = temp;
                }
            }
        }
    }
    fclose(fp);
    free(strtab);
}

/*
call this function in J or Ret insturction,insert this trace to ftrace
*/
void insert_ftrace(int type, vaddr_t insAddr, vaddr_t target)
{
    SFNode *p = s_list->next;
    FNode *temp = malloc(sizeof(FNode));
    temp->pc = insAddr;
    temp->type = type;
    while (p)
    {
        if (insAddr >= p->addr && insAddr < p->addr + p->size)
            strcpy(temp->originName, p->name);
        p = p->next;
    }
    p = s_list->next;
    while (p)
    {
        if (target >= p->addr && target < p->addr + p->size)
        {
            sprintf(temp->p, temp->type ? "ret [%s to %s]" : "call [%s to %s@%lx]", temp->originName, p->name, p->addr);
            // in order to output easily,use tail insert
            temp->next = NULL;
            tail->next = temp;
            tail = temp;
            break;
        }
        p = p->next;
    }
}

/*
output the ftrace,call this function at the program ends
 */
void print_ftrace()
{
    FNode *p = f_list->next;
    int k = 0; // control space num
    while (p)
    {
        if (p->type)
            --k;
        printf("0x%lx: ", p->pc);
        for (int i = 0; i < k; i++)
            printf(" ");
        printf("%s\n", p->p);
        if (!p->type)
            ++k;
        p = p->next;
    }
}