D			[0-9]
L			[a-zA-Z_]
H			[a-fA-F0-9]
E			([Ee][+-]?{D}+)
P			([Pp][+-]?{D}+)
FS			(f|F|l|L)
IS			((u|U)|(u|U)?(l|L|ll|LL)|(l|L|ll|LL)(u|U))

%{
#include <stdio.h>
#include <stdlib.h>
#include "y.tab.h"

void count(void);
void comment(void);
void report_error(void);

char *p;
int column = 0;
int line = 1;

%}

%%
"/*"			{ comment(); }
"//"[^\n]*		{ /* consume //-comment */ }

"else"			{ count(); return(ELSE); }
"float"			{ count(); return(FLOAT); }
"if"			{ count(); return(IF); }
"int"			{ count(); return(INT); }
"return"		{ count(); return(RETURN); }
"struct"		{ count(); return(STRUCT); }

{L}({L}|{D})*				{ count(); return(NAME); }

0[xX]{H}+{IS}?				{ count(); return(NUMBER); }
0[0-7]*{IS}?				{ count(); return(NUMBER); }
[1-9]{D}*{IS}?				{ count(); return(NUMBER); }

{D}+{E}{FS}?				{ count(); return(NUMBER); }
{D}*"."{D}+{E}?{FS}?		{ count(); return(NUMBER); }
{D}+"."{D}*{E}?{FS}?		{ count(); return(NUMBER); }
0[xX]{H}+{P}{FS}?			{ count(); return(NUMBER); }
0[xX]{H}*"."{H}+{P}?{FS}?	{ count(); return(NUMBER); }
0[xX]{H}+"."{H}*{P}?{FS}?	{ count(); return(NUMBER); }

"=="			{ count(); return(EQUAL); }
";"				{ count(); return(SEMICOLON); }
("{"|"<%")		{ count(); return(LBRACE); }
("}"|"%>")		{ count(); return(RBRACE); }
","				{ count(); return(COMMA); }
"="				{ count(); return(ASSIGN); }
"("				{ count(); return(LPAR); }
")"				{ count(); return(RPAR); }
("["|"<:")		{ count(); return(LBRACK); }
("]"|":>")		{ count(); return(RBRACK); }
"."				{ count(); return(DOT); }
"-"				{ count(); return(MINUS); }
"+"				{ count(); return(PLUS); }
"*"				{ count(); return(TIMES); }
"/"				{ count(); return(DIVIDE); }

[ \t\v\n\f]		{ count(); }
.				{ report_error(); }

%%

char input() {
    return *p++;
}

void comment(void) {
	char c, prev = 0;
	while ((c = input()) != 0)      /* (EOF maps to 0) */
	{
		if (c == '/' && prev == '*')
			return;
		prev = c;
	}
	printf("unterminated comment");
}

void count(void) {
	int i;
	for (i = 0; yytext[i] != '\0'; i++)
		if (yytext[i] == '\n')
			column = 0;
		else if (yytext[i] == '\t')
			column += 8 - (column % 8);
		else
			column++;
}

void report_error(void) {
    printf("undefined token at %d, %d", line, column);
}

int main(int argc, char* argv[]) {
    if (argc!=2) {
	    printf("Usage: %s c_file_name",argv[0]);
	    exit(0);
	}
    FILE *rp = fopen(argv[1], "r");
    FILE *wp = fopen("token.txt", "w");
    if (rp == NULL) {
        printf("Reading Failure.");
        exit(0);
    }
    if (wp == NULL) {
        printf("Writing Failure.");
        exit(0);
    }
    char ch;
	int fileLen = 0;
	while (ch = fgetc(rp) != EOF) {
		++fileLen;
	}
	p = (char*)malloc(fileLen * sizeof(char) + 1);
	rewind(rp);
	for (int i = 0; i < fileLen; ++i) {
		p[i] = fgetc(rp);
	}
	p[fileLen] = '\0';
    while (*p) {
        fprintf(wp, "%d,%s\n", yylex(), yytext);
    }
    fclose(rp);
    fclose(wp);
    return 0;
}