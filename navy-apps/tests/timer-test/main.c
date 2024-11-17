#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <NDL.h>

int main()
{

	uint32_t old = 0, new = 0, flags = 0;
	NDL_Init(flags);
	old = NDL_GetTicks();
	while (1)
	{
		new = NDL_GetTicks();
		if (new - old > 500)
		{
			printf("Gettimeofday Success!\n");
			old = new;
		}
	}
	NDL_Quit();
	return 0;
}