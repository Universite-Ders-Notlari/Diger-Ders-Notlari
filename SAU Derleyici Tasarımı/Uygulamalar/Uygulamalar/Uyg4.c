//Develop a lexical analyzer to recognize a few patterns in C.

#include<string.h>
#include<ctype.h>
#include<stdio.h>
#include<stdlib.h>

void keyword(char str[10])
{
	if(strcmp("for",str)==0||strcmp("while",str)==0||strcmp("do",str)==0||strcmp("int",str)==0||strcmp("float",str)==0||strcmp("char",str)==0||strcmp("double",str)==0||strcmp("printf",str)==0||strcmp("switch",str)==0||strcmp("case",str)==0)
		printf("\n%s is a keyword",str);
	else
		printf("\n%s is an identifier",str);
}
void main()
{
	FILE *f1,*f2,*f3;
	char c,str[10],st1[10];
	int num[100],lineno=0,tokenvalue=0,i=0,j=0,k=0;
	f1=fopen("input.c","r");
	f2=fopen("identifier","w");
	f3=fopen("specialchar","w");
	
	while((c=getc(f1))!=EOF)
	{
		//if read character is number, keep reading and finally convert it to an integer
		if(isdigit(c))
		{
			tokenvalue=c-'0'; //convert char to number
			c=getc(f1); //read next char
			while(isdigit(c))
			{
				tokenvalue*=10+c-'0'; //convert all digits to integer
				c=getc(f1);
			}
			num[i++]=tokenvalue; //copy to num array
			ungetc(c,f1); //undo the last read char
		}
		else if(isalpha(c))
		{
			putc(c,f2); //write char to f2 (identifier)
			c=getc(f1); //read char from f1
			while(isdigit(c)||isalpha(c)||c=='_'||c=='$') //an identifier can have numbers,letters,$ or _
			{
				putc(c,f2);
				c=getc(f1);
			}
			putc(' ',f2); //append space to the end
			ungetc(c,f1); //undo the last read char
		}
		else if(c==' '||c=='\t')
			printf(" ");
		else if(c=='\n')
			lineno++;
		else //if none of above then its a special character
			putc(c,f3);
	}
	
	//closing the opended files
	fclose(f2);
	fclose(f3);
	fclose(f1);
	
	//printing the numbers in input.c
	printf("\n the no's in the program are:");
	for(j=0;j<i;j++)
		printf("\t%d",num[j]);
	printf("\n");
	
	//open the identifier file and print out keywords and identifiers
	f2=fopen("identifier","r");
	k=0;
	printf("the keywords and identifier are:");
	while((c=getc(f2))!=EOF)
	//read each character, if space is seen call keyword function and check if it is a keyword or not
		if(c!=' ')
			str[k++]=c;
		else
		{
			str[k]='\0';
			keyword(str);
			k=0;
		}
	fclose(f2);
	
	//open specialchar file and print out special characters
	f3=fopen("specialchar","r");
	printf("\n Special Characters are");
	while((c=getc(f3))!=EOF)
		printf("\t%c",c);
	printf("\n");
	fclose(f3);
	
	printf("Total no of lines are:%d",lineno);
}
