#include <stdio.h>
#include <stdlib.h>

// The teleporter challenge demands the proper value in r7 such that Ackermann(4, 1) == 6
// with the Ackermann function defined as :
//
// short Ackermann(short r0, short r1)
// {
//   if      (!r0)  return r1+1;
//   else if (!r1)  return Ackermann(r0-1, r7);
//   else           return Ackermann(r0-1, Ackermann(r0, r1-1));
// }
// .. with all arithmetic wrapping into [0..32767]
//
// However due to stack explosions reported online, it's not feasible to use this directly.
// Some kind of caching, or is it called memoization these days, is necessary.
// Note that 32768 * 32768 = 1 Giga of shorts, or 2 GB !..

int main(int Arguments, char* Argument[])
{
	// Cache allocation
	short *Cache[32768];
	for (int i=0; i<32768; i++)
	{
		if (!(Cache[i] = (short*)malloc(sizeof(short) * 32768)))
		{
			fprintf(stderr, "Could not allocate cache[%d] of 32768 shorts\n", i);
			exit(3);
		}
	} /* for (i) */

	short r7, Result;
	int OpenCalls=0;

	short Hackerman(short r0, short r1)
	{
		if (Cache[r0][r1] >= 0)  return Cache[r0][r1];

		// Cache miss ? Need to implement real function
		// But this opens Pandora's box of recursion, keep an eye on it
		if (++OpenCalls >= 3)
		{
			fprintf(stderr, "OpenCalls at %d\n", OpenCalls);
			exit(4);
		}
		//if (!(OpenCalls&0xFFF))  printf("++ OpenCalls at %d\n", OpenCalls);

		short RetVal;
		if (!r0)       RetVal=((r1+1)&0x7FFF);
		else if (!r1)  RetVal=Hackerman((r0-1)&0x7FFF, r7);
		else           RetVal=Hackerman((r0-1)&0x7FFF, Hackerman(r0, (r1-1)&0x7FFF));

		// Now that the return value is known, record it into the cache
		Cache[r0][r1] = RetVal;

		// Upon returning, the call stack decreases by one
		OpenCalls--;
		return RetVal;
	} /* short Hackerman(short, short) */

	// Loop over all possible r7 values
	for (r7=1; r7>0; r7++)
	{
		// Cache clearance
		for (int i=0; i<5; i++)
			for (int j=0; j<32768; j++)
				Cache[i][j] = -1;  // -1 represents cache miss

		// Try clever lookahead filling of cache up to (4,1)
		for (int i=0; i<4; i++)
			for (int j=0; j<32768; j++)
				Hackerman((short)i,(short)j);
		Hackerman(4,0);

		// Demonstrate operation by printing top-left quadrant for r7==3
		if (r7 == 3)
		{
			printf("\nTop-left Results for r7==3 :\n");
			for (int j=0; j<8; j++)
			{
				for (int i=0; i<4; i++)
					printf(" %4x", Cache[i][j]);
				putchar('\n');
			}
			printf("\nBottom-left Results for r7==3 :\n");
			for (int j=32760; j<32768; j++)
			{
				for (int i=0; i<4; i++)
					printf(" %4x", Cache[i][j]);
				putchar('\n');
			}
			getchar();
		}

		Result = Hackerman(4,1);
		printf("r7=%4x yields Ack(4,1)=%4x", r7, Result);
		if (Result == 6)
		{
			printf("  *** FOUND! ***");
			getchar();
		}
		putchar('\n');
	} /* for (r7) */

	return 0;
}
