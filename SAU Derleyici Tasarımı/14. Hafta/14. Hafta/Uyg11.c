//To implement type checking
#include<stdio.h>
#include<stdlib.h>
int main()
{
	int n,i,k,flag=0;
	char vari[15],typ[15],b[15],c;
	printf("Enter the number of variables:");
	scanf(" %d",&n);
	for(i=0;i<n;i++)
	{
		//storing variables and their types into 2 different arrays
		printf("Enter the variable[%d]:",i);
		scanf(" %c",&vari[i]);
		printf("Enter the variable-type[%d](float-f,int-i):",i);
		scanf(" %c",&typ[i]);
		
		if(typ[i]=='f') //if there is a float type then flag is set.
			flag=1;
	}
	printf("Enter the Expression(end with $):");
	i=0;
	getchar();
	while((c=getchar())!='$')
	{
		b[i]=c; //assigning the expression into char array
		i++;
	}
	k=i;
	for(i=0;i<k;i++)
	{
		if(b[i]=='/')
		{
			//if there is no float type but division is found set a flag, because it will change the type of result
			flag=1;
			break;
		}
	}
	for(i=0;i<n;i++)
	{
		if(b[0]==vari[i]) //if the first symbol of the expression is a variable then its a valid expression, so continue.
		{
			if(flag==1) //either division if found or a float variable exists
			{
				if(typ[i]=='f') //if result type is float, no problem
				{
					printf("\nthe datatype is correctly defined..!\n");
					break;
				}
				else
				{
					printf("Identifier %c must be a float type..!\n",vari[i]);
					break;
				}
			}
			else //if no division is found, no problem
			{
				printf("\nthe datatype is correctly defined..!\n");
				break;
			}
		}
	}
	return 0;
}
