char* cRed      = "\033[0;31m";
char* cGreen    = "\033[0;32m";
char* cYellow   = "\033[0;33m";
char* cBlue     = "\033[0;34m";
char* cMagenta  = "\033[0;35m";
char* cCyan     = "\033[0;36m";
char* cLightGrey= "\033[0;37m";
char* cNone     = "\033[0m";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int main(int Arguments, char* Argument[])
{
	//////////////////////////////////////////////////////////////////////////////
	// Initialisation

	FILE *InputFile, *OutputFile;
	if (Arguments != 2)
	{
		fprintf(stderr, "Usage:\n");
		fprintf(stderr, "Synacor <binfile>\n");
		exit(1);
	}
	if (!(InputFile = fopen(Argument[1], "rb")))
	{
		fprintf(stderr, "Could not open %s for binary reading.\n", Argument[1]);
		exit(1);
	}

	int InstrPtr=0;
	int StackPtr=0;
	int MaxAddr=0;
	unsigned short *Memory = (unsigned short*)malloc(32768 * sizeof(unsigned short));
	unsigned short Register[8] = {0};
	unsigned short *Stack = (unsigned short*)malloc(32768 * sizeof(unsigned short));
	memset(Stack, 0, 32768 * sizeof(unsigned short));
	unsigned short Short;

	char OutputName[100];
	strcpy(OutputName, Argument[1]);
	int Pos, Len=strlen(OutputName);
	if (!strcmp(OutputName+Len-4, ".bin"))  Len -= 4;
	for (Pos=0; Pos<12; Pos++)
		if ((Len-1-Pos < 0) || (OutputName[Len-1-Pos] < '0') || (OutputName[Len-1-Pos] > '9'))
			break;
	if (Pos >= 12)
	{
		printf(cCyan); printf("Date format found in filename %s\n", OutputName); printf(cNone);
		Len -= 13;
		// Read content of the registers
		fread(Register, sizeof(unsigned short), 8, InputFile);
		// Read InstrPtr and StackPtr
		fread(&Short, sizeof(unsigned short), 1, InputFile);  InstrPtr = (int)Short;
		fread(&Short, sizeof(unsigned short), 1, InputFile);  StackPtr = (int)Short;
		// Read content of the stack
		fread(Stack, sizeof(unsigned short), StackPtr, InputFile);
	}

	MaxAddr = fread(Memory, sizeof(unsigned short), 32768, InputFile);
	fclose(InputFile);
	printf(cCyan); printf("Read %d shorts; first one is %lu\n", MaxAddr, *Memory); printf(cNone);

	//////////////////////////////////////////////////////////////////////////////
	// Helper functions

	unsigned short ReadMem(int Address)
	{
		if (Address >= MaxAddr)
		{
			fprintf(stderr, "%s ReadMem(%d) beyond current MaxAddr=%d %s\n", cRed, Address, MaxAddr, cNone);
			exit(2);
		}
		unsigned short Value = Memory[Address];
		if (Value >= 32768)
		{
			fprintf(stderr, "%s ReadMem(%d) invalid value %hd %s\n", cRed, Address, Value, cNone);
			exit(2);
		}

		return Value;
	} /* unsigned short ReadMem(int) */

	unsigned short GetMem(void)
	{
		if (InstrPtr >= MaxAddr)
		{
			fprintf(stderr, "%s InstrPtr at %d beyond current MaxAddr=%d %s\n", cRed, InstrPtr<<1, MaxAddr, cNone);
			exit(2);
		}
		unsigned short Value = Memory[InstrPtr++];
		if (Value >= 32768+8)
		{
			fprintf(stderr, "%s Read invalid value at %04X: %hd %s\n", cRed, InstrPtr<<1, Value, cNone);
			exit(2);
		}
		if (Value >= 32768)
			Value = Register[Value - 32768];

		return Value;
	} /* unsigned short GetMem() */

	int GetRegAddr(void)
	{
		if (InstrPtr >= MaxAddr)
		{
			fprintf(stderr, "%s InstrPtr at %d beyond current MaxAddr=%d %s\n", cRed, InstrPtr<<1, MaxAddr, cNone);
			exit(2);
		}
		unsigned short Value = Memory[InstrPtr++];
		if ((Value >= 32768+8) || (Value < 32768))
		{
			fprintf(stderr, "%s Read invalid RegAddr at %04X: %hd %s\n", cRed, InstrPtr<<1, Value, cNone);
			exit(2);
		}
		Value -= 32768;
		if (Value >= 8)
		{
			fprintf(stderr, "%s Cannot return RegAddr %h from %04X %s\n", cRed, Value, InstrPtr<<1, cNone);
			exit(2);
		}

		return Value;
	} /* int GetRegAddr() */

	void WriteMem(int Address, unsigned short Value)
	{
		if (Address >= MaxAddr)
			MaxAddr = Address;
		Memory[Address] = Value;
	} /* WriteMem(int, unsigned short) */

	void Push(unsigned short Value)
	{
		if (StackPtr > 32768)
		{
			fprintf(stderr, "%s Attempt to push Value %hd beyond StackPtr %04X %s\n", cRed, Value, StackPtr, cNone);
			exit(2);
		}
		Stack[StackPtr++] = Value;
	} /* Push(unsigned short) */

	unsigned short Pop()
	{
		if (!StackPtr)
		{
			fprintf(stderr, "%s Attempt to pop from StackPtr %04X %s\n", cRed, StackPtr, cNone);
			exit(2);
		}
		return Stack[--StackPtr];
	} /* unsigned short Pop() */



	//////////////////////////////////////////////////////////////////////////////
	// Operation

	for (;;)
	{
		unsigned short Instr = GetMem();
		unsigned short Arg1,Arg2, RetVal;
		int RegAddr, Halted=0;
		switch (Instr)
		{
			case  0: fprintf(stderr, "%s Halt instruction 0 at %04X %s\n", cMagenta, InstrPtr<<1, cNone);
              				Halted = 1;                                                                 break;
			case  1: RegAddr = GetRegAddr();  Register[RegAddr] = GetMem();                     				break;
			case  2: Push(GetMem());                                                            				break;
			case  3: RegAddr = GetRegAddr();  Register[RegAddr] = Pop();                         				break;
			case  4: RegAddr = GetRegAddr();  Arg1 = GetMem();  Arg2 = GetMem();
				         Register[RegAddr] = (Arg1 == Arg2 ? 1 : 0);                    				          break;
			case  5: RegAddr = GetRegAddr();  Arg1 = GetMem();  Arg2 = GetMem();
  				       Register[RegAddr] = (Arg1  > Arg2 ? 1 : 0);                    				          break;
			case  6: Arg1 = GetMem();  InstrPtr = Arg1;                                         				break;
			case  7: Arg1 = GetMem();  Arg2 = GetMem();  if  (Arg1) { InstrPtr = Arg2; }         				break;
			case  8: Arg1 = GetMem();  Arg2 = GetMem();  if (!Arg1) { InstrPtr = Arg2; }         				break;
			case  9: RegAddr = GetRegAddr();  Arg1 = GetMem();  Arg2 = GetMem();
  				       Register[RegAddr] = (Arg1 + Arg2) & 0x7FFF;                    				          break;
			case 10: RegAddr = GetRegAddr();  Arg1 = GetMem();  Arg2 = GetMem();
  				       Register[RegAddr] = (Arg1 * Arg2) & 0x7FFF;                    				          break;
			case 11: RegAddr = GetRegAddr();  Arg1 = GetMem();  Arg2 = GetMem();
  				       Register[RegAddr] = (Arg1 % Arg2) & 0x7FFF;                    				          break;
			case 12: RegAddr = GetRegAddr();  Arg1 = GetMem();  Arg2 = GetMem();
  				       Register[RegAddr] =  Arg1 & Arg2;                               				          break;
			case 13: RegAddr = GetRegAddr();  Arg1 = GetMem();  Arg2 = GetMem();
  				       Register[RegAddr] =  Arg1 | Arg2;                               				          break;
			case 14: RegAddr = GetRegAddr();  Arg1 = GetMem();
				         Register[RegAddr] = (~Arg1) & 0x7FFF;                    				                break;
			case 15: RegAddr = GetRegAddr();  Register[RegAddr] = ReadMem(GetMem());                    break;
			case 16: Arg1 = GetMem();  Arg2 = GetMem(); WriteMem((int)Arg1, Arg2);                      break;
			case 17: Arg1 = GetMem();  Push((unsigned short)InstrPtr);  InstrPtr = Arg1;                break;
			case 18: Arg1 = Pop();  InstrPtr = Arg1;                                                    break;
			case 19: putchar(GetMem() & 0xFF);                                                  				break;
			case 20: RegAddr = GetRegAddr();  printf(cYellow); int Char = getchar();  printf(cNone);
				if (feof(stdin))
				{ 	printf("%s EndOfFile on stdin %s\n", cCyan, cNone);   InstrPtr -= 2;  Halted = 1;  }
				else  Register[RegAddr] = (unsigned short)Char;                                           break;
			case 21:                                                                                    break;
			default:
				fprintf(stderr, "%s Unknown instruction at %04X: %hd %s\n", cRed, InstrPtr<<1, Instr, cNone);
				exit(2);
		} /* switch (Instr) */

#if 0
		// Debugging
		if (Instr != 19)
		{
			printf(cGreen); printf("Instr %hd M[%X]:", Instr, (InstrPtr<<1)&0xFFF0);
			for (int i=0; i<8; i++)  printf(" %hX,", Memory[(InstrPtr&0xFFF8) + i]);
			printf("  Arg1,2: %hX,%hX", Arg1, Arg2);  printf("  Reg[%d]: %hX", RegAddr, Register[RegAddr]);
			printf("  Stack[%X]:", StackPtr);
			for (int i=-2; i<=0; i++)  if (StackPtr+i >= 0)  printf(" %hX,", Stack[StackPtr+i]);
			printf("\n%s", cNone);
		}
#endif

		if (Halted)  break;
	} /* forever */

	printf(cGreen); printf("Registers:"); 	for (int i=0; i<8; i++)  printf(" %hX,", Register[i]);
	printf("\nStack[%d]:", StackPtr);
	if (StackPtr<100)  for (int i=0; i<StackPtr; i++)  printf(" %hX,", Stack[i]);
  putchar('\n');  printf(cNone);

	//////////////////////////////////////////////////////////////////////////////
	// Finalisation

	time_t Time = time(NULL);
	struct tm *LocalTime = localtime(&Time);
	char DateString[100];
	sprintf(DateString, "%04d%02d%02d%02d%02d",
			LocalTime->tm_year+1900, LocalTime->tm_mon+1, LocalTime->tm_mday, LocalTime->tm_hour, LocalTime->tm_min);
	sprintf(OutputName+Len, ".%s.bin", DateString);
	if (!(OutputFile = fopen(OutputName, "wb")))
	{
		fprintf(stderr, "Could not open %s for binary writing.\n", OutputName);
		exit(1);
	}
	fwrite(Register, sizeof(unsigned short), 8, InputFile);
	// Read InstrPtr and StackPtr
	Short = (unsigned short)InstrPtr;  fwrite(&Short, sizeof(unsigned short), 1, InputFile);
	Short = (unsigned short)StackPtr;  fwrite(&Short, sizeof(unsigned short), 1, InputFile);
	// Read content of the stack
	fwrite(Stack, sizeof(unsigned short), StackPtr, InputFile);
	fwrite(Memory, sizeof(unsigned short), MaxAddr, OutputFile);
	printf(cCyan); printf("%d shorts written to %s\n", MaxAddr, OutputName); printf(cNone);
	fclose(OutputFile);
	return 0;
}
