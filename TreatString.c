#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int Arguments, char* Argument[])
{
	if (Arguments != 4)
	{
		fprintf(stderr, "Usage:\n");
		fprintf(stderr, "TreatString <StringFile> <FuncPtr> <Operand>\n");
		exit(1);
	}

	FILE *InputFile;
	if (!(InputFile = fopen(Argument[1], "rb")))
	{
		fprintf(stderr, "Could not open %s for binary reading\n", Argument[1]);
		exit(2);
	}

	unsigned short FuncPtr, Operand;
	if (1 != sscanf(Argument[2], "%hx", &FuncPtr))
	{
		fprintf(stderr, "Could not interpret %s as FuncPtr\n", Argument[2]);
		exit(2);
	}
	if (1 != sscanf(Argument[3], "%hx", &Operand))
	{
		fprintf(stderr, "Could not interpret %s as Operand\n", Argument[3]);
		exit(2);
	}

	unsigned short Len, Actual, Pos;
	fread(&Len, sizeof(unsigned short), 1, InputFile);
	unsigned short Short[1000];
	Actual = fread(Short, sizeof(unsigned short), 1000, InputFile);
	fclose(InputFile);

	if (Actual > Len)
		fprintf(stderr, "Len-field %hd but file %s has %hd shorts\n", Len, Argument[1], Actual);
	for (Pos=0; Pos<Actual; Pos++)
		switch (FuncPtr)
		{
			case 0x611:
				putchar(Short[Pos] ^ Operand);
				break;
		}

	printf("%d shorts treated\n", Pos);
	return 0;
}
