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
	if (Arguments != 3)
	{
		fprintf(stderr, "Usage:\n");
		fprintf(stderr, "Disasm <binfile> <hex fileoffset\n");
		exit(1);
	}
	if (!(InputFile = fopen(Argument[1], "rb")))
	{
		fprintf(stderr, "Could not open %s for binary reading.\n", Argument[1]);
		exit(1);
	}

	int InstrPtr=0;
	int MaxAddr=0;
	unsigned short *Memory = (unsigned short*)malloc(32768 * sizeof(unsigned short));
	unsigned short Short;

	char OutputName[100];
	strcpy(OutputName, Argument[1]);
	int Pos, Len=strlen(OutputName);
	if (!strcmp(OutputName+Len-4, ".bin"))  Len -= 4;

	MaxAddr = fread(Memory, sizeof(unsigned short), 32768, InputFile);
	fclose(InputFile);
	printf(cCyan); printf("Read %d shorts; first one is %lu\n", MaxAddr, *Memory); printf(cNone);

	if (1 != sscanf(Argument[2], "%x", &InstrPtr))
	{
		fprintf(stderr, "Could not interpret %s as hex fileoffset.\n", Argument[2]);
		exit(1);
	}
	sprintf(OutputName+Len, ".%x.asm", InstrPtr);
	if (!(OutputFile = fopen(OutputName, "w")))
	{
		fprintf(stderr, "Could not open %s for writing.\n", OutputName);
		exit(1);
	}
	InstrPtr >>= 1;

	//////////////////////////////////////////////////////////////////////////////
	// Helper functions
	int NrOfShortsPrinted;
	char Bytes[50];

	void GetMem(char* Text)
	{
		sprintf(Bytes + NrOfShortsPrinted*6, " %02X %02X", (unsigned char)(Memory[InstrPtr] & 0xFF),
				(unsigned char)(Memory[InstrPtr] >> 8));
		NrOfShortsPrinted++;
		unsigned short Value = Memory[InstrPtr++];
		if (Value >= 32768+8)
		{
			fprintf(stderr, "%s Read invalid value at %04X: %hd %s\n", cRed, InstrPtr<<1, Value, cNone);
			exit(2);
		}
		if (Value >= 32768)
			sprintf(Text, "r%hd", Value - 32768);
		else
			sprintf(Text, "%hx", Value);
	} /* void GetMem(char*) */

	void GetRegAddr(char* Text)
	{
		sprintf(Bytes + NrOfShortsPrinted*6, " %02X %02X", (unsigned char)(Memory[InstrPtr] & 0xFF),
				(unsigned char)(Memory[InstrPtr] >> 8));
		NrOfShortsPrinted++;
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
		sprintf(Text, "r%hd", Value);
	} /* void GetRegAddr(char*) */


	//////////////////////////////////////////////////////////////////////////////
	// Operation

	char OutputLine[100];
	char ParseLine[100];
	int  NrOfLines=0;
	char Arg1[10], Arg2[10], RegAddr[10], Dummy[10];

	for (;;)
	{
		sprintf(OutputLine, "%5x:", InstrPtr);
		unsigned short Instr = Memory[InstrPtr];
		NrOfShortsPrinted = 0;
		unsigned short Short;
		char Char;
		int UnknownInstr=0;

		GetMem(Dummy);
		switch (Instr)
		{
			case  0: sprintf(ParseLine, "Halt");                                                        break;
			case  1: GetRegAddr(RegAddr); GetMem(Arg1);
				sprintf(ParseLine, "Set %s <- %s", RegAddr, Arg1);                                				break;
			case  2: GetMem(Arg1);  sprintf(ParseLine, "Push %s", Arg1);                         				break;
			case  3: GetRegAddr(RegAddr);  sprintf(ParseLine, "Pop -> %s", RegAddr);             				break;
			case  4: GetRegAddr(RegAddr);  GetMem(Arg1);  GetMem(Arg2);
				sprintf(ParseLine, "Eq? if (%s == %s) %s <- 1 else %s <- 0",
						Arg1, Arg2, RegAddr, RegAddr);                    				                            break;
			case  5: GetRegAddr(RegAddr);  GetMem(Arg1);  GetMem(Arg2);
				sprintf(ParseLine, "Gt? if (%s > %s) %s <- 1 else %s <- 0",
						Arg1, Arg2, RegAddr, RegAddr);                    				                            break;
			case  6: GetMem(Arg1);  sprintf(ParseLine, "Jump %s", Arg1);                         				break;
			case  7: GetMem(Arg1);  GetMem(Arg2);
				sprintf(ParseLine, "JumpTrue %s if %s != 0", Arg2, Arg1);                         				break;
			case  8: GetMem(Arg1);  GetMem(Arg2);
				sprintf(ParseLine, "JumpFalse %s if %s == 0", Arg2, Arg1);                         				break;
			case  9: GetRegAddr(RegAddr);  GetMem(Arg1);  GetMem(Arg2);
				sprintf(ParseLine, "Add %s <- %s + %s", RegAddr, Arg1, Arg2);                             break;
			case 10: GetRegAddr(RegAddr);  GetMem(Arg1);  GetMem(Arg2);
				sprintf(ParseLine, "Mult %s <- %s * %s", RegAddr, Arg1, Arg2);                            break;
			case 11: GetRegAddr(RegAddr);  GetMem(Arg1);  GetMem(Arg2);
				sprintf(ParseLine, "Modulo %s <- %s %% %s", RegAddr, Arg1, Arg2);                         break;
			case 12: GetRegAddr(RegAddr);  GetMem(Arg1);  GetMem(Arg2);
				sprintf(ParseLine, "And %s <- %s & %s", RegAddr, Arg1, Arg2);                             break;
			case 13: GetRegAddr(RegAddr);  GetMem(Arg1);  GetMem(Arg2);
				sprintf(ParseLine, "Or %s <- %s | %s", RegAddr,	Arg1, Arg2);                              break;
			case 14: GetRegAddr(RegAddr);  GetMem(Arg1);
				sprintf(ParseLine, "Not %s <- ~%s", RegAddr, Arg1);                                       break;
			case 15: GetRegAddr(RegAddr);  GetMem(Arg1);
				sprintf(ParseLine, "ReadMem %s <- [%s]", RegAddr, Arg1);                           				break;
			case 16: GetMem(Arg1);  GetMem(Arg2);
				sprintf(ParseLine, "WriteMem [%s] <- %s", Arg1, Arg2);                             				break;
			case 17: GetMem(Arg1);  sprintf(ParseLine, "Call %s", Arg1);                         				break;
			case 18: sprintf(ParseLine, "Return");                                                      break;
			case 19: Short = Memory[InstrPtr];  GetMem(Dummy);  Char = (char)(Short & 0xFF);
				if (Char == '\n')  sprintf(ParseLine, "Out \\n");
				else if (*Dummy == 'r')  sprintf(ParseLine, "Out %s", Dummy);
				else	             sprintf(ParseLine, "Out \'%c\'", Char);                         				break;
			case 20: GetRegAddr(RegAddr);  sprintf(ParseLine, "In -> %s", RegAddr);             				break;
			case 21: sprintf(ParseLine, "Nop");                                                         break;
			default:
				fprintf(stderr, "%s Unknown instruction at %04X: %hd %s\n", cRed, InstrPtr<<1, Instr, cNone);
				UnknownInstr = 1;
		} /* switch (Instr) */

		if (UnknownInstr)        break;
		if (++NrOfLines >= 256)  break;

		strcat(OutputLine, Bytes);
		for (int i=0; i<6*(4-NrOfShortsPrinted); i++)  strcat(OutputLine, " ");
		fprintf(OutputFile, "%s : %s\n", OutputLine, ParseLine);
	} /* forever */

	//////////////////////////////////////////////////////////////////////////////
	// Finalisation

	printf(cCyan); printf("%d lines written to %s\n", NrOfLines, OutputName); printf(cNone);
	fclose(OutputFile);
	return 0;
}
