#include <stdio.h>
#include <curses.h> //conio.h win
#include <ctype.h>
#include <string.h>

void main()
{
		FILE *fp;
		int i,j;
		char arr[100],k;
		char kw[10][10]={"int","float","double","end","main","void","include","printf","scanf"};
		char hf[2][10]={"<stdio.h>","<curses.h>"};
		char op[5]={'+','-','*','=','%'};
		char punc[6]={'{','}','(',')',','};
		
		clear(); //clrsrc(); win
		fp=fopen("input.c","r");
		printf("\nOkunan Program\n");
		while(!feof(fp))
		{
				arr[0]=fgetc(fp);
				printf("%c",arr[0]);
		}
		fclose(fp);
		printf("\n\n*Sembol Tablosu*\n\n");
		fp=fopen("input.c","r");
		printf("\nKeywords >> ");
		while(!feof(fp))
		{
				fscanf(fp,"%s",arr);
				for(i=0;i<10;i++)
				{
						if(strcmp(arr,kw[i])==0)
						{
								printf("\t%s",arr);
						}
				}
		}
		fclose(fp);
		
		fp=fopen("input.c","r");
		printf("\nHeader Files >> ");
		while(!feof(fp))
		{
				fscanf(fp,"%s",arr);
				for(i=0;i<2;i++)
				{
						if(strcmp(arr,hf[i])==0)
						{
								printf("\t%s",arr);
						}
				}
		}
		fclose(fp);
		
		fp=fopen("input.c","r");
		printf("\nOperators >> ");
		while(!feof(fp))
		{
				arr[0]=fgetc(fp);
				for(i=0;i<5;i++)
				{
						if(arr[0]==op[i])
						{
							printf("\t%c",arr[0]);
						}
				}
		}
		fclose(fp);
		
		fp=fopen("input.c","r");
		printf("\nPunctuation >> ");
		while(!feof(fp))
		{
				arr[0]=fgetc(fp);
				for(i=0;i<6;i++)
				{
					if(arr[0]==punc[i])
					{
						printf("\t%c",arr[0]);
					}
				}
		}		
		fclose(fp);
		
		fp=fopen("input.c","r");
		printf("\nConstants >> ");
		while(!feof(fp))
		{
				arr[0]=fgetc(fp);
				if(isdigit(arr[0]))
				{
					printf(" %c",arr[0]);
				}
		}
		fclose(fp);
		
		fp=fopen("input.c","r");
		printf("\nIdentifier >> ");
		while(!feof(fp))
		{
				fscanf(fp,"%s",arr);
				for(i=0;i<3;i++)
				{
						if(strcmp(arr,kw[i])==0)
						{
								fscanf(fp,"%s",arr);
								j=0;
								while(j<strlen(arr)&&arr[j]!=';')
								{
										printf("%c",arr[j]);
										j++;
								}
						}
				}
		}
		fclose(fp);
		getch();
		
}
