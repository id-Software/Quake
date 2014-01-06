/*
Copyright (C) 1996-1997 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
//
// gas to MASM source code converter
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_TOKENS			100
#define MAX_TOKEN_LENGTH	1024
#define LF					0x0A

typedef enum {NOT_WHITESPACE, WHITESPACE, TOKEN_AVAILABLE, LINE_DONE, FILE_DONE, PARSED_OKAY} tokenstat;
typedef enum {NOSEG, DATASEG, TEXTSEG} segtype;

int		tokennum;
int		inline, outline;

char	*token;
char	tokens[MAX_TOKENS][MAX_TOKEN_LENGTH+1];

segtype	currentseg = NOSEG;

typedef struct {
	char	*text;
	char	*emit;
	int		numtokens;
	void	(*parsefunc) (void);
} parsefield;


void errorexit (void);


//==============================================

typedef struct {
	char	*text;
	char	*emit;
	int		len;
} regdesc;

regdesc	reglist[] = {
	{"%eax", "eax", 4},
	{"%ebx", "ebx", 4},
	{"%ecx", "ecx", 4},
	{"%edx", "edx", 4},
	{"%esi", "esi", 4},
	{"%edi", "edi", 4},
	{"%ebp", "ebp", 4},
	{"%esp", "esp", 4},
	{"%ax", "ax", 3},
	{"%bx", "bx", 3},
	{"%cx", "cx", 3},
	{"%dx", "dx", 3},
	{"%si", "si", 3},
	{"%di", "di", 3},
	{"%bp", "bp", 3},
	{"%sp", "sp", 3},
	{"%al", "al", 3},
	{"%bl", "bl", 3},
	{"%cl", "cl", 3},
	{"%dl", "dl", 3},
	{"%ah", "ah", 3},
	{"%bh", "bh", 3},
	{"%ch", "ch", 3},
	{"%dh", "dh", 3},
	{"%st(0)", "st(0)", 6},
	{"%st(1)", "st(1)", 6},
	{"%st(2)", "st(2)", 6},
	{"%st(3)", "st(3)", 6},
	{"%st(4)", "st(4)", 6},
	{"%st(5)", "st(5)", 6},
	{"%st(6)", "st(6)", 6},
	{"%st(7)", "st(7)", 6},
};

int	numregs = sizeof (reglist) / sizeof (reglist[0]);

//==============================================


void emitanoperand (int tnum, char *type, int notdata)
{
	int		i, index, something_outside_parens, regfound;
	int		parencount;
	char	*pt;
	char	temp[MAX_TOKEN_LENGTH+1];

	pt = tokens[tnum];

	if (pt[0] == '%')
	{
	// register
		for (i=0 ; i<numregs ; i++)
		{
			if (!strcmpi (pt, reglist[i].text))
			{
				printf ("%s", reglist[i].emit);
				return;
			}
		}

		fprintf (stderr, "Error: bad register %s\n", pt);
		errorexit ();
	}
	else if (pt[0] == '$')
	{
	// constant
		if (pt[1] == '(')
		{
			if ((pt[2] > '9') || (pt[2] < '0'))
			{
				i = 2;
				printf ("offset ");

				parencount = 1;

				while ((pt[i] != ')') || (parencount > 1))
				{
					if (!pt[i])
					{
						fprintf (stderr, "mismatched parens");
						errorexit ();
					}

					if (pt[i] == ')')
						parencount--;
					else if (pt[i] == '(')
						parencount++;

					printf ("%c", pt[i]);
					i++;
				}
			}
			else
			{
				pt++;

				parencount = 1;

				for (i=1 ; (pt[i] != ')') || (parencount > 1) ; i++)
				{
					if (!pt[i])
					{
						fprintf (stderr, "mismatched parens");
						errorexit ();
					}

					if (pt[i] == ')')
						parencount--;
					else if (pt[i] == '(')
						parencount++;
				}

				pt[i] = 0;

				if ((pt[1] == '0') && ((pt[2] == 'x') || (pt[2] == 'X')))
				{
					printf ("0%sh", &pt[3]);
				}
				else
				{
					printf ("%s", &pt[1]);
				}
 			}
		}
		else if ((pt[1] == '0') && ((pt[2] == 'x') || (pt[2] == 'X')))
		{
			printf ("0%sh", &pt[3]);
		}
		else if ((pt[1] >= '0') && (pt[1] <= '9'))
		{
			printf ("%s", &pt[1]);
		}
		else
		{
			printf ("offset %s", &pt[1]);
		}
	}
	else if (!notdata && ((pt[0] >= '0') && (pt[0] <= '9')))
	{
		pt--;

		if ((pt[1] == '0') && ((pt[2] == 'x') || (pt[2] == 'X')))
		{
			printf ("0%sh", &pt[3]);
		}
		else
		{
			printf ("%s", &pt[1]);
		}
	}
	else
	{
	// must be a memory location
		strcpy (temp, type);
		index = strlen (temp);

		if (notdata)
			temp[index++] = '[';

		something_outside_parens = 0;

		while (*pt)
		{
			if (index > (MAX_TOKEN_LENGTH - 10))
			{
				fprintf (stderr, "Error: operand too long %s\n",
						 tokens[tnum]);
				errorexit ();
			}

			if (*pt != ')')
			{
				if (*pt == '(')
				{
					if (something_outside_parens)
						temp[index++] = '+';
				}
				else if (*pt == '%')
				{
					regfound = 0;

					for (i=0 ; i<numregs ; i++)
					{
						if (!strnicmp (pt, reglist[i].text,
							reglist[i].len))
						{
							strcpy (&temp[index], reglist[i].emit);
							index += strlen (reglist[i].emit);
							pt += strlen (reglist[i].text) - 1;
							regfound = 1;
							break;
						}
					}

					if (!regfound)
					{
						fprintf (stderr, "Error: bad register %s\n", pt);
						errorexit ();
					}
				}
				else if (*pt == ',')
				{
					pt++;

					if ((*pt >= '1') && (*pt <= '8'))
					{
						temp[index++] = '*';
						temp[index++] = *pt;
					}
					else if (*pt != ')')
					{
						if (temp[index-1] != '+')
							temp[index++] = '+';
					}
				}
				else
				{
					something_outside_parens = 1;

					// handle hexadecimal constants in addresses
					if ((*pt == '0') &&
						((*(pt+1) == 'x') || (*(pt+1) == 'X')))
					{
						pt += 2;

						do
						{
							temp[index++] = *pt++;
						} while (((*pt >= '0') && (*pt <= '9'))     ||
								 ((*pt >= 'a') && (*pt <= 'f')) ||
								 ((*pt >= 'A') && (*pt <= 'F')));

						pt--;
						temp[index++] = 'h';
					}
					else
					{
						temp[index++] = *pt;
					}
				}
			}

			pt++;
		}

		if (notdata)
			temp[index++] = ']';

		temp[index] = 0;
		printf ("%s", temp);
	}
}


void datasegstart (void)
{
	if (currentseg == DATASEG)
		return;

	if (currentseg == TEXTSEG)
		printf ("_TEXT ENDS\n");

	printf ("_DATA SEGMENT");

	currentseg = DATASEG;
}


void textsegstart (void)
{
	if (currentseg == TEXTSEG)
		return;

	if (currentseg == DATASEG)
		printf ("_DATA ENDS\n");

	printf ("_TEXT SEGMENT");

	currentseg = TEXTSEG;
}


void emitdata (void)
{
	int		i;

	for (i=1 ; i<(tokennum-1) ; i++)
		printf (" %s,", tokens[i]);

	printf (" %s", tokens[tokennum-1]);
}


void emitonedata (void)
{

	printf (" %s", tokens[1]);
}


void emitonecalldata (void)
{
	int	i, isaddr, len;

	if (tokens[1][0] == '*')
	{
		printf (" dword ptr[%s]", &tokens[1][1]);
	}
	else
	{
		isaddr = 0;
		len = strlen(tokens[1]);

		for (i=0 ; i<len ; i++)
		{
			if (tokens[1][i] == '(')
			{
				isaddr = 1;
				break;
			}
		}

		if (!isaddr)
		{
			printf (" near ptr %s", tokens[1]);
		}
		else
		{
			emitanoperand (1, " dword ptr", 1);
		}
	}
}


void emitonejumpdata (void)
{
	int	i, isaddr, len;

	if (tokens[1][0] == '*')
	{
		printf (" dword ptr[%s]", &tokens[1][1]);
	}
	else
	{
		isaddr = 0;
		len = strlen(tokens[1]);

		for (i=0 ; i<len ; i++)
		{
			if (tokens[1][i] == '(')
			{
				isaddr = 1;
				break;
			}
		}

		if (!isaddr)
		{
			printf (" %s", tokens[1]);
		}
		else
		{
			emitanoperand (1, " dword ptr", 1);
		}
	}
}


void emitexterndef (void)
{

	printf (" %s:dword", tokens[1]);
}


void nooperands (void)
{

}


void emitoneoperandl (void)
{

	printf (" ");
	emitanoperand (1, "ds:dword ptr", 1);
}


void emitoneoperandb (void)
{

	printf (" ");
	emitanoperand (1, "ds:byte ptr", 1);
}


void emitoneoperandw (void)
{

	printf (" ");
	emitanoperand (1, "ds:word ptr", 1);
}


void emittwooperandsl (void)
{

	printf (" ");
	emitanoperand (2, "ds:dword ptr", 1);
	printf (",");
	emitanoperand (1, "ds:dword ptr", 1);
}


void emittwooperandsb (void)
{

	printf (" ");
	emitanoperand (2, "ds:byte ptr", 1);
	printf (",");
	emitanoperand (1, "ds:byte ptr", 1);
}


void emittwooperandsw (void)
{

	printf (" ");
	emitanoperand (2, "ds:word ptr", 1);
	printf (",");
	emitanoperand (1, "ds:word ptr", 1);
}


void emit_0_or_1_operandsl (void)
{

	if (tokennum == 2)
	{
		printf (" ");
		emitanoperand (1, "ds:dword ptr", 1);
	}
}


void emit_1_or_2_operandsl (void)
{
	int		j;

	if (tokennum == 2)
	{
		printf (" ");
		emitanoperand (1, "ds:dword ptr", 1);
	}
	else if (tokennum == 3)
	{
		printf (" ");
		emitanoperand (2, "ds:dword ptr", 1);
		printf (",");
		emitanoperand (1, "ds:dword ptr", 1);
	}
	else
	{

		fprintf (stderr, "Error: too many operands\n");

		for (j=0 ; j<tokennum ; j++)
			fprintf (stderr, "%s\n", tokens[j]);

		fprintf (stderr, "\n");
		errorexit ();
	}
}


void emit_1_or_2_operandsl_vartext (char *str0, char *str1)
{
	int		j;

	if (tokennum == 2)
	{
		printf (" %s ", str0);
		emitanoperand (1, "ds:dword ptr", 1);
	}
	else if (tokennum == 3)
	{
		if (!strcmpi (tokens[2], "%st(0)"))
			printf (" %s ", str0);
		else
			printf (" %s ", str1);

		emitanoperand (2, "ds:dword ptr", 1);
		printf (",");
		emitanoperand (1, "ds:dword ptr", 1);
	}
	else
	{

		fprintf (stderr, "Error: too many operands\n");

		for (j=0 ; j<tokennum ; j++)
			fprintf (stderr, "%s\n", tokens[j]);

		fprintf (stderr, "\n");
		errorexit ();
	}
}


void special_fdivl (void)
{

	emit_1_or_2_operandsl_vartext ("fdiv", "fdivr");
}


void special_fdivpl (void)
{

	emit_1_or_2_operandsl_vartext ("fdivp", "fdivrp");
}


void special_fdivrl (void)
{

	emit_1_or_2_operandsl_vartext ("fdivr", "fdiv");
}


void special_fdivrpl (void)
{

	emit_1_or_2_operandsl_vartext ("fdivrp", "fdivp");
}


void special_fsubl (void)
{

	emit_1_or_2_operandsl_vartext ("fsub", "fsubr");
}


void special_fsubpl (void)
{

	emit_1_or_2_operandsl_vartext ("fsubp", "fsubrp");
}


void special_fsubrl (void)
{

	emit_1_or_2_operandsl_vartext ("fsubr", "fsub");
}


void special_fsubrpl (void)
{

	emit_1_or_2_operandsl_vartext ("fsubrp", "fsubp");
}


void emit_multiple_data (void)
{
	int		i;

	printf (" ");

	for (i=1 ; i<(tokennum-1) ; i++)
	{
		emitanoperand (i, "", 0);
		printf (", ");
	}

	emitanoperand (i, "", 0);
}


//==============================================

parsefield	parsedata[] = {
	{".align", " align", 2, emitonedata},
	{".byte",  " db", -2, emit_multiple_data},
	{".data",  "", 1, datasegstart},
	{".extern"," externdef", 2, emitexterndef},
	{".globl", " public", -2, emit_multiple_data},
	{".long",  " dd", -2, emit_multiple_data},
	{".single"," dd", -2, emit_multiple_data},
	{".text",  "", 1, textsegstart},
	{"adcl",   " adc", 3, emittwooperandsl},
	{"addb",   " add", 3, emittwooperandsb},
	{"addl",   " add", 3, emittwooperandsl},
	{"andb",   " and", 3, emittwooperandsb},
	{"andl",   " and", 3, emittwooperandsl},
	{"call",   " call", 2, emitonecalldata},
	{"cmpb",   " cmp", 3, emittwooperandsb},
	{"cmpl",   " cmp", 3, emittwooperandsl},
	{"cmpw",   " cmp", 3, emittwooperandsw},
	{"decl",   " dec", 2, emitoneoperandl},
	{"decw",   " dec", 2, emitoneoperandw},
	{"divl",   " div", 2, emitoneoperandl},
	{"fadd",   " fadd", -2, emit_1_or_2_operandsl},
	{"faddp",  " faddp", -2, emit_1_or_2_operandsl},
	{"faddps", " faddp", -2, emit_1_or_2_operandsl},
	{"fadds",  " fadd", -2, emit_1_or_2_operandsl},
	{"fcom",   " fcom", 2, emitoneoperandl},
	{"fcoms",  " fcom", 2, emitoneoperandl},
	{"fcomp",  " fcomp", 2, emitoneoperandl},
	{"fcomps", " fcomp", 2, emitoneoperandl},
	{"fdiv",   "", -2, special_fdivl}, 
	{"fdivp",  "", -2, special_fdivpl}, 
	{"fdivr",  "", -2, special_fdivrl},
	{"fdivrp", "", -2, special_fdivrpl},
	{"fdivrs", "", -2, special_fdivrl},
	{"fildl",  " fild", 2, emitoneoperandl},
	{"fistl",  " fist", 2, emitoneoperandl},
	{"fistpl", " fistp", 2, emitoneoperandl},
	{"fld",    " fld", 2, emitoneoperandl},
	{"fldcw",  " fldcw", 2, emitoneoperandw},
	{"fldenv", " fldenv", 2, emitoneoperandl},
	{"flds",   " fld", 2, emitoneoperandl},
	{"fmul",   " fmul", -2, emit_1_or_2_operandsl},
	{"fmulp",  " fmulp", -2, emit_1_or_2_operandsl},
	{"fmulps", " fmulp", -2, emit_1_or_2_operandsl},
	{"fmuls",  " fmul", -2, emit_1_or_2_operandsl},
	{"fnstcw", " fnstcw", 2, emitoneoperandw},
	{"fnstenv"," fnstenv", 2, emitoneoperandl},
	{"fnstsw", " fnstsw", 2, emitoneoperandw},
	{"fstp",   " fstp", 2, emitoneoperandl},
	{"fstps",  " fstp", 2, emitoneoperandl},
	{"fsts",   " fst", 2, emitoneoperandl},
	{"fsubr",  "", -2, special_fsubrl},
	{"fsubrp", "", -2, special_fsubrpl},
	{"fsubrs", "", -2, special_fsubrl},
	{"fsub",   "", -2, special_fsubl},
	{"fsubp",  "", -2, special_fsubpl},
	{"fsubps", "", -2, special_fsubpl},
	{"fsubs",  "", -2, special_fsubl},
	{"fxch",   " fxch", 2, emitoneoperandl},
	{"imull",  " imul", -2, emit_1_or_2_operandsl},
	{"incl",   " inc", 2, emitoneoperandl},
	{"ja",     " ja", 2, emitonedata},
	{"jae",    " jae", 2, emitonedata},
	{"jb",     " jb", 2, emitonedata},
	{"jbe",    " jbe", 2, emitonedata},
	{"jc",     " jc", 2, emitonedata},
	{"je",     " je", 2, emitonedata},
	{"jg",     " jg", 2, emitonedata},
	{"jge",    " jge", 2, emitonedata},
	{"jl",     " jl", 2, emitonedata},
	{"jle",    " jle", 2, emitonedata},
	{"jmp",    " jmp", 2, emitonejumpdata},
	{"jna",    " jna", 2, emitonedata},
	{"jnae",   " jnae", 2, emitonedata},
	{"jnb",    " jnb", 2, emitonedata},
	{"jnbe",   " jnbe", 2, emitonedata},
	{"jnc",    " jnc", 2, emitonedata},
	{"jne",    " jne", 2, emitonedata},
	{"jng",    " jng", 2, emitonedata},
	{"jnge",   " jnge", 2, emitonedata},
	{"jnl",    " jnl", 2, emitonedata},
	{"jnle",   " jnle", 2, emitonedata},
	{"jns",    " jns", 2, emitonedata},
	{"jnz",    " jnz", 2, emitonedata},
	{"js",     " js", 2, emitonedata},
	{"jz",     " jz", 2, emitonedata},
	{"leal",   " lea", 3, emittwooperandsl},
	{"movb",   " mov", 3, emittwooperandsb},
	{"movl",   " mov", 3, emittwooperandsl},
	{"movw",   " mov", 3, emittwooperandsw},
	{"negl",   " neg", 2, emitoneoperandl},
	{"orb",    " or", 3, emittwooperandsb},
	{"orl",    " or", 3, emittwooperandsl},
	{"popl",   " pop", 2, emitoneoperandl},
	{"pushl",  " push", 2, emitoneoperandl},
	{"ret",    " ret", -1, emit_0_or_1_operandsl},
	{"rorl",   " ror", 3, emittwooperandsl},
	{"sarl",   " sar", 3, emittwooperandsl},
	{"sbbl",   " sbb", 3, emittwooperandsl},
	{"shll",   " shl", 3, emittwooperandsl},
	{"shrl",   " shr", 3, emittwooperandsl},	
	{"subl",   " sub", 3, emittwooperandsl},
	{"testb",  " test", 3, emittwooperandsb},
	{"testl",  " test", 3, emittwooperandsl},
	{"xorb",   " xor", 3, emittwooperandsb},
	{"xorl",   " xor", 3, emittwooperandsl},
};

int	numparse = sizeof (parsedata) / sizeof (parsedata[0]);

//==============================================

void errorexit (void)
{
	fprintf (stderr, "In line: %d, out line: %d\n", inline, outline);
	exit (1);
}


tokenstat whitespace (char c)
{
	if (c == '\n')
		return LINE_DONE;

	if ((c <= ' ') ||
		(c > 127) ||
		(c == ','))
	{
		return WHITESPACE;
	}

	return NOT_WHITESPACE;
}


int gettoken (void)
{
	char		c;
	int			count, parencount;
	tokenstat	stat;

	do
	{
		if ((c = getchar ()) == EOF)
			return FILE_DONE;

		if ((stat = whitespace (c)) == LINE_DONE)
			return LINE_DONE;
	} while (stat == WHITESPACE);

	token[0] = c;
	count = 1;

	if (c == '~')
	{
		count--;
		token[count++] = 'n';
		token[count++] = 'o';
		token[count++] = 't';
		token[count++] = ' ';
	}

	if (c == '(')
	{
		do
		{
			if ((c = getchar ()) == EOF)
			{
				fprintf (stderr, "EOF in middle of parentheses\n");
				errorexit ();
			}

			token[count++] = c;

		} while (c != ')');
	}
	
	for ( ;; )
	{
		if ((c = getchar ()) == EOF)
		{
			token[count] = 0;
			return TOKEN_AVAILABLE;
		}

		if (whitespace (c) == LINE_DONE)
		{
			if (ungetc (c, stdin) == EOF)
			{
				fprintf (stderr, "Couldn't unget character\n");
				errorexit ();
			}

			token[count] = 0;
			return TOKEN_AVAILABLE;
		}

		if (whitespace (c) == WHITESPACE)
		{
			token[count] = 0;
			return TOKEN_AVAILABLE;
		}

		if (count >= MAX_TOKEN_LENGTH)
		{
			fprintf (stderr, "Error: token too long\n");
			errorexit ();
		}

		token[count++] = c;

		if (c == '~')
		{
			count--;
			token[count++] = 'n';
			token[count++] = 'o';
			token[count++] = 't';
			token[count++] = ' ';
		}
		else if (c == '(')
		{
			parencount = 1;

			do
			{
				if ((c = getchar ()) == EOF)
				{
					fprintf (stderr, "EOF in middle of parentheses\n");
					errorexit ();
				}

				if (c == '(')
					parencount++;
				else if (c == ')')
					parencount--;

				if (c == '~')
				{
					token[count++] = 'n';
					token[count++] = 'o';
					token[count++] = 't';
					token[count++] = ' ';
				}
				else
				{
					token[count++] = c;
				}

			} while ((c != ')') || (parencount > 0));
		}
	}
}


tokenstat parseline (void)
{
	tokenstat	stat;
	int			i, j, firsttoken, labelfound;
	int			mnemfound;

	firsttoken = 1;
	tokennum = 0;
	labelfound = 0;

	for ( ;; )
	{
		token = tokens[tokennum];
		stat = gettoken ();

		switch (stat)
		{
		case FILE_DONE:
			return FILE_DONE;

		case LINE_DONE:
			if (!firsttoken && tokennum)
			{
				mnemfound = 0;

				for (i=0 ; i<numparse; i++)
				{
					if (!strcmpi (tokens[0], parsedata[i].text))
					{
						if (((parsedata[i].numtokens > 0) &&
							 (parsedata[i].numtokens != tokennum)) ||
							((parsedata[i].numtokens < 0) &&
							 (tokennum < -parsedata[i].numtokens)))
						{
							fprintf (stderr, "mismatched number of tokens\n");

							for (j=0 ; j<tokennum ; j++)
								fprintf (stderr, "%s\n", tokens[j]);

							fprintf (stderr, "\n");
							errorexit ();
						}

						printf ("%s", parsedata[i].emit);
						(*parsedata[i].parsefunc) ();

						mnemfound = 1;
						break;
					}
				}

				if (!mnemfound)
				{
					fprintf (stderr, "Error: unknown mnemonic\n");

					for (j=0 ; j<tokennum ; j++)
						fprintf (stderr, "%s\n", tokens[j]);

					fprintf (stderr, "\n");
					errorexit ();
				}
			}

			if (!firsttoken)
			{
				if ((currentseg == DATASEG) && labelfound && !tokennum)
					printf (":\n");
				else
					printf ("\n");

				outline++;
			}
			return PARSED_OKAY;

		case TOKEN_AVAILABLE:
			if (firsttoken)
			{
				if (token[strlen(token) - 1] == ':')
				{
					labelfound = 1;

					if (currentseg == DATASEG)
					{
						token[strlen(token) - 1] = 0;
						printf ("%s", token);
					}
					else if (currentseg == TEXTSEG)
					{
						printf ("%s", token);
					}
					else
					{
						fprintf (stderr, "Error: not in segment block\n");
						errorexit ();
					}

					firsttoken = 0;
					break;
				}
			}

			firsttoken = 0;

			if (tokennum >= MAX_TOKENS)
			{
				fprintf (stderr, "Error: too many tokens\n");
				exit (0);
			}

			tokennum++;

			break;

		default:
			fprintf (stderr, "Error: unknown tokenstat %d\n", stat);
			exit (0);
		}
	}
}


void main (int argc, char **argv)
{
	tokenstat	stat;

	printf (" .386P\n"
            " .model FLAT\n");
	inline = 1;
	outline = 3;

	for ( ;; )
	{
		stat = parseline ();
		inline++;

		switch (stat)
		{
		case FILE_DONE:
			if (currentseg == TEXTSEG)
				printf ("_TEXT ENDS\n");
			else if (currentseg == DATASEG)
				printf ("_DATA ENDS\n");

			printf (" END\n");
			exit (0);
		
		case PARSED_OKAY:
			break;

		default:
			fprintf (stderr, "Error: unknown tokenstat %d\n", stat);
			exit (0);
		}
	}
}

