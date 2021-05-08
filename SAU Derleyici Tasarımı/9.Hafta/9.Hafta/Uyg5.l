%{
int COMMENT=0;
%}
identifier[a-zA-z][a-zA-Z0-9]*

%%

#.* {printf("\n%s bir kutuphanedir",yytext);}

int | 
float |
char |
double |
while |
for |
struct |
typedef |
do |
if |
break |
continue |
void |
switch |
return |
else |
goto {printf("\n%s bir keyword",yytext);}

"/*" {COMMENT=1;} {printf("\n%s bir yorumdur",yytext);} 

{identifier}\( {if(!COMMENT) printf("\n%s bir fonksiyondur\n",yytext);}

\{ {if(!COMMENT) printf("\nBlok baslangici");}

\} {if(!COMMENT) printf("\nBlok sonu");}

{identifier}(\[[0-9]*\])? {if(!COMMENT) printf("\n%s bir tanımlayici(identifier)",yytext);}

\".*\" {if(!COMMENT) printf("\n%s bir katar",yytext);}

[0-9]+ {if(!COMMENT) printf("\n%s bir rakam",yytext);}

\)(\;)? {if(!COMMENT) printf("\n\t"); ECHO; printf("\n");}

\( ECHO;

= {if(!COMMENT) printf("\n%s bir atama operatoru",yytext);}

\<= |
\>= |
\< |
== |
\> 
= {if(!COMMENT) printf("\n%s bir iliskisel operator",yytext);}

%%

int main(int argc,char **argv)
{
	FILE *file;
	file=fopen("input.c","r");
	if(!file)
	{
		printf("dosya acilamadı");
		exit(0);
	}
	yyin=file;
	yylex();
	printf("\n");
	return(0);
}
int yywrap()
{
	return (1);
}
