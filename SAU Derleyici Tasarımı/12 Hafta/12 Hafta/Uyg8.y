%{
	#include <stdio.h>
	int flag=0;
%}
%token NUMBER
%left '+''-'
%left '*''/''%'
%left '('')'

%%

AritmetikIfadeler: E {
	printf("\nSonuc=%d\n",$$);
	return 0;
};
E:	E'+'E {$$=$1+$3;}
	|E'-'E {$$=$1-$3;}
	|E'*'E {$$=$1*$3;}
	|E'/'E {$$=$1/$3;}
	|E'%'E {$$=$1%$3;}
	|'('E')' {$$=$2;}
	|NUMBER {$$=$1;}
;

%%

void main()
{
	printf("\nLutfen hesaplanması için bir ifade giriniz (+,-,/,*,%,() desteklenmektedir!): ");
	yyparse();
	if(flag==0)
		printf("Girilen ifade gecerlidir\n");
}
void yyerror()
{
	printf("\nGirilen ifade gecersizdir\n");
	flag=1;
}
