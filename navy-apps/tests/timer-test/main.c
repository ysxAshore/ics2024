#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <sys/time.h>

int main()
{

	unsigned long old = 0;
	struct timeval tv;

	if (gettimeofday(&tv, NULL) == 0)
	{
		old = tv.tv_usec;
		while (1)
		{
			if (gettimeofday(&tv, NULL) == 0)
			{
				if (tv.tv_usec - old > 500000)
				{
					printf("Gettimeofday Success!\n");
					old = tv.tv_usec;
				}
			}
			else
				printf("Gettimeofday Error!\n");
		}
	}
	else
		printf("Gettimeofday Error!\n");

	return 0;
}