#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <fixedptc.h>
#include <NDL.h>

int main()
{
    fixedpt A = fixedpt_rconst(-1.5);
    fixedpt B = fixedpt_fromint(1.2);
    void *pA = malloc(sizeof(uint32_t));
    void *pB = malloc(sizeof(uint32_t));
    *((uint32_t *)pA) = fixedpt2float(A);
    *((uint32_t *)pB) = fixedpt2float(B);
    printf("%d\n", fixedpt_add(A, B));
    printf("%d\n", fixedpt_mul(A, B));
    printf("%d\n", fixedpt_div(B, A));
    printf("%d\n", fixedpt_fromfloat(pA));
    printf("%d\n", fixedpt_fromfloat(pB));
    // but when pA and pB both assigned,will encouter a float ins
}
