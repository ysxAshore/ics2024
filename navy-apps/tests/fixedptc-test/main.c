#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <fixedptc.h>
#include <NDL.h>

int main()
{
    fixedpt A = fixedpt_rconst(-3.75);
    fixedpt B = fixedpt_fromint(1);
    void *pA = malloc(sizeof(float));
    void *pB = malloc(sizeof(float));
    *((float *)pA) = fixedpt_tofloat(A);
    printf("%d\n", fixedpt_add(A, B));
    printf("%d\n", fixedpt_mul(A, B));
    printf("%d\n", fixedpt_div(B, A));
    printf("%d\n", fixedpt_fromfloat(pA));
    // but when pA and pB both assigned,will encouter a float ins
}
