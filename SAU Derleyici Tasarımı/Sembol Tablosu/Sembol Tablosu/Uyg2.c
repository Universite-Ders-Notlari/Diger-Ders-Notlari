//Linux sistemlerde çaıştırmak için conio yerine curses header dosyası gerektiğinden; build komutlarına "-lcurses" eklenmelidir

//kütüphaneler
#include <stdio.h>
#include <curses.h> //windows-> # include <conio.h>
#include <memory.h> //windows-> # include <alloc.h>
#include <string.h>

//define ifadeleri
# define null 0

//global değişkenler ve fonk tanımları
int size=0;
void insert();
void del();
int search(char lab[]);
void modify();
void display();

//sembol tablosu yapısının tanımı
struct symbtab
{
          char label[10];
          int addr;
          struct symtab *next; //tablodaki bir sonraki girdiyi de tutalım
};
struct symbtab *first,*last;  //sembol tablosunu işaret eden 2 pointer tanımladık


void main()
{
          int op;
          int y;
          char la[10];
          system("clear"); // windows -> clrscr();
          do
          {
                   printf("\nSYMBOL TABLE IMPLEMENTATION\n");
                   printf("1. INSERT\n");
                   printf("2. DISPLAY\n");
                   printf("3. DELETE\n");
                   printf("4. SEARCH\n");
                   printf("5. MODIFY\n");
                   printf("6. END\n");
                   printf("Enter your option : ");
                   scanf("%d",&op);
                   switch(op)
                   {
                             case 1:
                             insert();
                             display();
                             break;
                             case 2:
                             display();
                             break;
                             case 3:
                             del();
                             display();
                             break;
                             case 4:
                             printf("Enter the label to be searched : ");
                             scanf("%s",la);
                             y=search(la);
                             if(y==1) { printf("The label is already in the symbol Table"); }
                             else { printf("The label is not found in the symbol table"); }
                             break;
                             case 5:
                             modify();
                             display();
                             break;
                             case 6:
                             break;
                   }
          }
          while(op<6);
          getch();
}
void insert()
{
          int n;
          char l[10];
          printf("Enter the label : ");
          scanf("%s",l);
          n=search(l);  //tabloda bu etiketten daha önce tanımlanmış mı kontrol edilir
          if(n==1) { printf("The label already exists. Duplicate cant be inserted\n"); }
          else
          {
                   struct symbtab *p;
                   p=malloc(sizeof(struct symbtab));
                   strcpy(p->label,l);
                   printf("Enter the address : ");
                   scanf("%d",&p->addr);
                   p->next=null;
                   if(size==0)
                   {
                             first=p;
                             last=p;
                   }
                   else
                   {
                             last->next=p;
                             last=p;
                   }
                   size++;
          }
}
void display()
{
          int i;
          struct symbtab *p;
          p=first;
          printf("LABEL\tADDRESS\n");
          for(i=0;i<size;i++)
          {
                   printf("%s\t%d\n",p->label,p->addr);
                   p=p->next;
          }
}
int search(char lab[])
{
          int i,flag=0;
          struct symbtab *p;
          p=first;
          for(i=0;i<size;i++)  //ilk durumda size=0 olduğundan 1 kez girer
          {
                   if(strcmp(p->label,lab)==0)
                   {
                             flag=1;
                   }
                   p=p->next;
          }
          return flag;
}
void modify()
{
          char l[10],nl[10];
          int add, choice, i, s;
          struct symbtab *p;
          p=first;
          printf("What do you want to modify?\n");
          printf("1. Only the label\n");
          printf("2. Only the address of a particular label\n");
          printf("3. Both the label and address\n");
          printf("Enter your choice : ");
          scanf("%d",&choice);
          switch(choice)
          {
                   case 1:
                   printf("Enter the old label\n");
                   scanf("%s",l);
                   printf("Enter the new label\n");
                   scanf("%s",nl);
                   s=search(l);
                   if(s==0)
                   {
                             printf("NO such label");
                   }
                   else //etiket bulunduysa tüm kayıtları gez, eski etiketi bulduğunda güncel olanla değiştir
                   {
                             for(i=0;i<size;i++)
                             {
                                      if(strcmp(p->label,l)==0)
                                      {
                                                strcpy(p->label,nl);
                                      }
                                      p=p->next;
                             }
                   }
                   break;
                   case 2:
                   printf("Enter the label whose address is to modified\n");
                   scanf("%s",l);
                   printf("Enter the new address\n");
                    scanf("%d",&add);
                   s=search(l);
                   if(s==0)
                   {
                             printf("NO such label");
                   }
                   else //etiket bulunduysa tüm kayıtları gez, eski etiketi bulduğunda güncel adresle değiştir
                   {
                             for(i=0;i<size;i++)
                             {
                                      if(strcmp(p->label,l)==0)
                                      {
                                                p->addr=add;
                                      }
                                      p=p->next;
                             }
                   }
                   break;
                   case 3:
                   printf("Enter the old label : ");
                   scanf("%s",l);
                   printf("Enter the new label : ");
                   scanf("%s",nl);
                   printf("Enter the new address : ");
                   scanf("%d",&add);
                   s=search(l);
                   if(s==0)
                   {
                             printf("NO such label");
                   }
                   else
                   {
                             for(i=0;i<size;i++)
                             {
                                      if(strcmp(p->label,l)==0)
                                      {
                                                strcpy(p->label,nl);
                                                p->addr=add;
                                      }
                                      p=p->next;
                             }
                   }
                   break;
          }
}
void del()
{
          int a;
          char l[10];
          struct symbtab *p,*q;
          p=first;
          printf("Enter the label to be deleted\n");
          scanf("%s",l);
          a=search(l);
          if(a==0)
          {
                   printf("Label not found\n");
          }
          else
          {
                   if(strcmp(first->label,l)==0)  //silinecek etiket ilkse, ikinci girdiyi first olarak ata.
                   {
                             first=first->next;
                   }
                   else if(strcmp(last->label,l)==0) //silinecek etiket sondaysa, girdiler aranan etikete eşit olmadığı sürece bir sonrakinin adresini güncel etiket (p) göstersin, sonra gelene kadar bu şekilde gidiyoruz. Sona gelince p sonu gösterecek ve "next" olarak null gösterecek
                   {
                             q=p->next;
                             while(strcmp(q->label,l)!=0)
                             {
                                      p=p->next;
                                      q=q->next;
                             }
                             p->next=null;
                             last=p;
                   }
                   else //silinecek etiket aralardaysa, girdiler aranan etikete eşit olmadığı sürece bir sonrakinin adresini güncel etiket (p) göstersin, eşitliği gördüğü anda-> silinecek olandan önceki etiket, silinecek olanın sonrakini next olarak işaret etsin
                   {
                             q=p->next; //q her zaman bir sonraki girdiyi (satırı) tutar
                             while(strcmp(q->label,l)!=0) 
                             {
                                      p=p->next;
                                      q=q->next; 
                             }
                             p->next=q->next; //silinen girdi q'dur. q'dan önceki eleman next olarak artık q'yu değil, q'dan sonraki elemanı gösterecek
                   }
                   size--;
          }
}
