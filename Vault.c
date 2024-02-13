#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int Arguments, char* Argument[])
{
	// Grid of numbers and operators
	// Represent operators as numbers : + 1     - 2     * 3
	int Grid[4][4] = {{ 3, 8, 2, 1},
										{ 4, 3,11, 3},
										{ 1, 4, 2,18},
										{22, 2, 9, 3}};

	// Breadth-first search : keep exploring all options at current NrOfSteps
	typedef struct {
		int Value;
		int History[20];   // Represent directions N,E,S,W as 1,2,3,4
		int YPos, XPos;
	} tPath;
	tPath *Path[2]={NULL,NULL};
	int NrOfPaths[2]={0};
	for (int i=0; i<2; i++)  Path[i]=(tPath*)malloc(sizeof(tPath)*1e7);

	// Populate one array with one element: the starting situation
	Path[0][0].Value = 22;
	Path[0][0].YPos = 3;
	Path[0][0].XPos = 0;
	NrOfPaths[0] = 1;

	// Expand all Paths out of one array into the other
	// Always move 2 steps at a time: select Operator and Operand
	for (int Step=0; Step<20; Step+=2)
	{
		int Old,New;
		int OldPathNr, NewPathNr;
		int FirstDir, SecondDir;
		int Operator, Operand;
		int HalfYPos, HalfXPos, NewYPos, NewXPos;
		int OldValue, NewValue;

		if ((Step>>1)&1) { Old=1; New=0; } else { Old=0; New=1; }
		printf("After %2d Steps : %6d paths\n", Step, NrOfPaths[Old]);
		NewPathNr=0;

		for  (OldPathNr=0; OldPathNr<NrOfPaths[Old]; OldPathNr++)
		{
			OldValue = Path[Old][OldPathNr].Value;

			// Pick first step, to obtain an Operator
			for (FirstDir=1; FirstDir<=4; FirstDir++)
			{
				HalfYPos = Path[Old][OldPathNr].YPos;
				HalfXPos = Path[Old][OldPathNr].XPos;
				switch (FirstDir)
				{
					case 1: HalfYPos--;  break;
					case 2: HalfXPos++;  break;
					case 3: HalfYPos++;  break;
					case 4: HalfXPos--;  break;
				}
				if ((HalfYPos < 0) || (HalfYPos > 3))  continue;
				if ((HalfXPos < 0) || (HalfXPos > 3))  continue;
				Operator = Grid[HalfYPos][HalfXPos];

			// Pick second step, to obtain an Operand
			for (SecondDir=1; SecondDir<=4; SecondDir++)
			{
				NewYPos = HalfYPos;
				NewXPos = HalfXPos;
				switch (SecondDir)
				{
					case 1: NewYPos--;  break;
					case 2: NewXPos++;  break;
					case 3: NewYPos++;  break;
					case 4: NewXPos--;  break;
				}
				if ((NewYPos < 0) || (NewYPos > 3))  continue;
				if ((NewXPos < 0) || (NewXPos > 3))  continue;
				Operand = Grid[NewYPos][NewXPos];

				// Now perform the operation
				switch (Operator)
				{
					case 1: NewValue = OldValue + Operand;  break;
					case 2: NewValue = OldValue - Operand;  break;
					case 3: NewValue = OldValue * Operand;  break;
				}
				// Out of bounds ?
				if ((NewValue < 0) || (NewValue > 200))  continue;
				// Returned to Antechamber ?
				if ((NewYPos==3) && (NewXPos==0))  continue;
				// Arriving at Vault Door ?
				if ((NewYPos==0) && (NewXPos==3))
				{
					if (NewValue != 30)  continue;
					// Print out this solution
					for (int StepNr=0; StepNr<Step; StepNr++)
						switch (Path[Old][OldPathNr].History[StepNr])
						{
							case 1: printf("N,");  break;
							case 2: printf("E,");  break;
							case 3: printf("S,");  break;
							case 4: printf("W,");  break;
						}
					switch (FirstDir)
						{
							case 1: printf("N,");  break;
							case 2: printf("E,");  break;
							case 3: printf("S,");  break;
							case 4: printf("W,");  break;
						}
					switch(SecondDir)
						{
							case 1: printf("N\n");  break;
							case 2: printf("E\n");  break;
							case 3: printf("S\n");  break;
							case 4: printf("W\n");  break;
						}
				}
				else /* not arrived at Vault Door */
				{
					// Register this new entry in the table
					Path[New][NewPathNr].YPos = NewYPos;
					Path[New][NewPathNr].XPos = NewXPos;
					Path[New][NewPathNr].Value = NewValue;
					memcpy(Path[New][NewPathNr].History, Path[Old][OldPathNr].History, sizeof(int)*Step);
					Path[New][NewPathNr].History[Step] = FirstDir;
					Path[New][NewPathNr].History[Step+1] = SecondDir;
					if (++NewPathNr >= 1e7)
					{
						fprintf(stderr, "NewPathNr %d too large\n", NewPathNr);
						exit(3);
					}
				}
			} /* for (SecondDir) */
			} /* for (FirstDir) */
		} /* for (OldPathNr) */

		NrOfPaths[New] = NewPathNr;
	} /* for (Step) */

	return 0;
}
