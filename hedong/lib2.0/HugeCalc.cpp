// HugeCalc.cpp: implementation.
//
//////////////////////////////////////////////////////////////////////

#include "HugeCalc.h"

//The check function check if the input number is legal
int  Check(char s[])
{    
	int i;
	if (strlen(s)==0)
		return 0;
	if (strlen(s)>=MAX)
		return 0;
	
	for (i=0; s[i]; i++)
		if (!isdigit(s[i]))
			return 0;
		return 1;
}

//The Iniatilize function ,initialize the result number before calculation
char* Initialize(char s[],int length)
{    
	int i;
	for (i=0; i<length; i++)
		s[i]=MARK;
	return s;
	
}

//The Read function,Read the operands and the operation
int  Read(char a[],char b[],char action[])
{
	printf("************This Program calculats two long integer action!************\n");
	printf("Input long integer A,max length is %d:\n",MAX);
	gets(a);
	while (!Check(a))
	{   
		printf("Input error , Please input a correct value :\n");
		gets(a);
	}
	printf("Iuput the operation over a & b \n");
	printf("addition is '+' ,substraction is '-',multiplication is '*',division is '/'\n");
	printf("                            Warning:\n");
	printf("If you do the division,A must bigger than B ,and B must not equal to zero !\n");
	gets(action);
	while ((Compare(action,"+")!=0) && (Compare(action,"-")!=0) && (Compare(action,"*")!=0) && (Compare(action,"/")!=0))
	{   
		printf("Input error , Please input a correct action :\n");
		gets(action);
	}
	printf("Input long integer b,max length is %d:\n",MAX);
	gets(b);
	while (!Check(b))
	{   printf("Input error , Please input a correct value :\n");
	gets(b);
	}
	return 1;
}

//The Calculate function,calling Addition,Substraction,Multiplication or Division function  in accordance with the action
int  Calculate(char a[],char b[],char c[],char action[])
{
    if (Compare(action,"+")==0)
    {     
		strcpy(c,Addition(a,b));
		return 1;
    }
    if (Compare(action,"-")==0)
    {      
		if  ((Substraction(a,b))!=NULL)
			strcpy(c,Substraction(a,b));
		else
		{      
			strcpy(c,"-");
			strcat(c,Substraction(b,a));
		}
		return 1;
    }
    if (Compare(action,"*")==0)
    {    
		strcpy(c,Multiplication(a,b));
		return 1;
    }
	
    if (Compare(action,"/")==0)
    {     
		if ((Division(a,b))!=NULL)
			strcpy(c,Division(a,b));
		else
		{   
			printf("Press Ctrl-Z to end, Press Enter to recalculate!\n");
			return 0;
		}
		return 1;
	}
	return -1;	
}

//The Print function , print the result
int  Print(char a[],char b[],char c[],char action[])
{    
	printf("The result of \n%s \n%s \n%s \nis :\n",a,action,b);
	puts(c);
	printf("Press Ctrl-Z to end , Press Enter  to recalculate..\n");
	return 1;
}

//The Addition function , add two operands and return the result
char* Addition(char a[],char b[])
{    
	char c[2*MAX],d[2*MAX];
	int i,j,k,a_length,b_length;
	Initialize(c,2*MAX);
	a_length=strlen(a);
	b_length=strlen(b);
	for (i=a_length-1,j=b_length-1,k=2*MAX-1; ( i>=0 || j>=0 ) ; i--,j--,k--)
	{  
		if ( i>=0 && j>=0 )
			c[k]=Value(a[i])+Value(b[j])+'0';
		else
			if (i>=0 && j<0 )
				c[k]=Value(a[i])+'0';
			else
				if ( i<0 && j>=0 )
					c[k]=Value(b[j])+'0';
	}
	Data_Process(c);
	Convert(c,d,2*MAX);
	return d;
}

//The Substraction function , substract one operand from another operand , and return the result
char* Substraction(char a[],char b[])
{    
	char c[2*MAX],d[2*MAX];
	int i,j,k,a_length,b_length,sub_result,symbol,flag[2*MAX]={0};
	Initialize(c,2*MAX);
	a_length=strlen(a);
	b_length=strlen(b);
	if (strcmp(a,b)==0)
		return ("0");
	for (i=a_length-1,j=b_length-1,k=2*MAX-1; ( i>=0 || j>=0 ) ; i--,j--,k--)
	{   
		sub_result=a[i]-b[j];
		symbol=Judge(sub_result);
		if (i>=1 && j>=0)
		{      
			if (flag[k]==0)
			{   if  (a[i]>=b[j])
			c[k]=sub_result+'0';
			else
			{       
				c[k]=sub_result+10+'0';
				flag[k-1]=1;
			}
			}
			else
			{   
				if (a[i]-b[j]>=1)
					c[k]=sub_result-1+'0';
				else
				{     
					c[k]=sub_result+9+'0';
					flag[k-1]=1;
				}
			}
		}
		else
			if  (i==0 && j<0)
			{
				if (flag[k]==0)
					c[k]=a[i];
				else
				{   
					if (a[i]==1)
						;
					else
						c[k]=a[i]-1;
				}
			}
			else
			{  
				if ((i==0) && (j==0))
				{   
					if (flag[k]==0)
					{  
						switch (symbol)
						{  
						case 0:   ;
							break;
						case 1:   c[k]=sub_result+'0';
							break;
						case -1:  return NULL;
							break;
						}
					}
					else
					{   
						switch (Judge(sub_result-1))
						{  
						case 0:   ;
							break;
						case 1:   c[k]=sub_result-1+'0';
							break;
						case -1:  return NULL;
							break;
						}
					}
				}
				else
					if ((i<0) && (j>=0))
						return NULL;
					else
						if ((i>0) && (j<0))
						{  
							if (flag[k]==0)
								c[k]=a[i];
							else
							{   
								if (a[i]>'0')
									c[k]=a[i]-1;
								else
								{  
									c[k]='9';
									flag[k-1]=1;
								}
							}
						}
			}
	}
	Convert(c,d,2*MAX);
	return d;
}
//The Multiplication function,multipy two operands and return the result
char* Multiplication(char a[],char b[])
{    
	char c[2*MAX],d[2*MAX];
	int i,j,k,p,a_length,b_length;
	Initialize(c,2*MAX);
	a_length=strlen(a);
	b_length=strlen(b);
	for (j=b_length-1;  j>=0 ; j--)
	{  
		k=2*MAX-(b_length-j);
		for (i=a_length-1,p=k; i>=0; i--,p--)
			if (c[p]==MARK)
			{    
				c[p]=(Value(a[i]))*(Value(b[j]))%10+'0';
				if (c[p-1]==MARK)
					c[p-1]=(Value(a[i]))*(Value(b[j]))/10+'0';
				else
					c[p-1]+=(Value(a[i]))*(Value(b[j]))/10;
			}
			else
			{    
				c[p]+=(Value(a[i]))*(Value(b[j]))%10;
				if (c[p-1]==MARK)
					c[p-1]=(Value(a[i]))*(Value(b[j]))/10+'0';
				else
					c[p-1]+=(Value(a[i]))*(Value(b[j]))/10;
			}
			Data_Process(c);
	}
	Convert(c,d,2*MAX);
	return d;
	
}
//The Division function,divise one operand from another operand, and return the result
char* Division(char a[],char b[])
{   
	char a1[MAX],c[MAX],d[MAX];
	strcpy(a1,a);
	int i,k,a_length=strlen(a1),b_length=strlen(b);
	for (i=0; b[i]; i++)
		if (b[i]!='0')
			break;
		if (i==strlen(b))
		{    
			printf("Error!\nIf you do division,the dividend must not equal to zero!\n");
			return 0;
		}
		if (Compare(a,b)==-1)
		{    
			printf("Error!\nIf you do division, A must bigger than B!\n");
			return 0;
		}
		for ( i=0,k=0; k<a_length-b_length+1; i++,k++)
			c[k]=Div_per_bit(a1,b_length+i,b);
		c[k]='\0';
		Convert(c,d,MAX);
		return d;
		
}
//The Div_per_bit function , calculate quotient per digit ,and return the result to Division function
char Div_per_bit(char a[],int a_l,char b[])
{   
	int i,j;
	char c[MAX];
	for (i=0,j=0; i<a_l; i++)
		if ( a[i]!=MARK)
		{   
			c[j]=a[i];
			j++;
		}
		c[j]='\0';
		for (i=0; c[i]; i++)
			if (c[i]!='0')
				break;
			if (i==strlen(c))
				return '0';
			if (Compare(c,b)<0)
				return '0';
			if (Compare(c,b)==0)
			{  
				for ( i=0; i<a_l; i++)
					a[i]=MARK;
				return '1';
			}
			i=0;
			while (Compare(c,b)>=0)
				Sub_per_bit(c,b,&i);
			Copy(a,a_l,c);
			return ('0'+i);
			
}
//The Sub_per_bit function, do the division by using substraction time after time
int  Sub_per_bit(char a[],char b[],int *count)
{    
	char c[MAX],d[MAX];
	int i,j,k,a_length,b_length,sub_result,symbol,flag[MAX]={0};
	Initialize(c,MAX);
	a_length=strlen(a);
	b_length=strlen(b);
	if (strcmp(a,b)==0)
		strcpy(c,"0");
	for (i=a_length-1,j=b_length-1,k=MAX-1; ( i>=0 || j>=0 ) ; i--,j--,k--)
	{   
		sub_result=a[i]-b[j];
		symbol=Judge(sub_result);
		if (i>=1 && j>=0)
		{     
			if (flag[k]==0)
			{   
				if  (a[i]>=b[j])
					c[k]=sub_result+'0';
				else
				{       c[k]=sub_result+10+'0';
				flag[k-1]=1;
				}
			}
			else
			{   
				if (a[i]-b[j]>=1)
					c[k]=sub_result-1+'0';
				else
				{     
					c[k]=sub_result+9+'0';
					flag[k-1]=1;
				}
			}
		}
		else
			if  (i==0 && j<0)
			{
				if (flag[k]==0)
					c[k]=a[i];
				else
				{   if (a[i]==1)
				;
				else
					c[k]=a[i]-1;
				}
			}
			else
			{  
				if ((i==0) && (j==0))
				{   
					if (flag[k]==0)
					{  
						switch (symbol)
						{  case 0:   ;
						break;
						case 1:   c[k]=sub_result+'0';
							break;
						case -1:  return 0;
							break;
						}
					}
					else
					{   
						switch (Judge(sub_result-1))
						{  
						case 0:   ;
							break;
						case 1:   c[k]=sub_result-1+'0';
							break;
						case -1:  return 0;
							break;
						}
					}
				}
				else
					if ((i<0) && (j>=0))
					{  
						return 0;
					}
					else
						if ((i>0) && (j<0))
						{  
							if (flag[k]==0)
								c[k]=a[i];
							else
							{   
								if (a[i]>'0')
									c[k]=a[i]-1;
								else
								{  
									c[k]='9';
									flag[k-1]=1;
								}
							}
						}
			}
	}
	Convert(c,d,MAX);
	strcpy(a,d);
	(*count)++;
	return 1;
}

//The Copy function , copy  a number_string  to another , wipe off the inutility symbol
int Copy(char a[],int a_l,char c[])
{   
	int i,j,c_l=strlen(c);
	for (i=0; i<a_l-c_l; i++)
		a[i]=MARK;
	for (i,j=0; j<c_l; i++,j++)
		a[i]=c[j];
	return 1;
}

//The Compare function ,compare two numbers and return the result
int Compare(char a[],char b[])
{   
	char c[MAX];
	if ((strlen(a))>(strlen(b)))
		return 1;
	if ((strlen(a))==(strlen(b)))
		return (strcmp(a,b));
	if ((strlen(a))<(strlen(b)))
		return -1;
}

//The Value function , receiver a digit_char, and return the number
int Value(char c)
{   
	if (isdigit(c))
		return (c-'0');
	else return 0;
}

//The Data_Process function,
int Data_Process(char s[])
{   
	int p,head,tail=2*MAX-1;
	for (head=0; s[head]==MARK ; head++)
		;
	for (p=tail; p>=head  ; p--)
		if (!isdigit(s[p]))
			if (s[p-1]!=MARK)
			{  
				s[p-1]+=(s[p]-'0')/10;
				s[p]=(s[p]-'0')%10+'0';
			}
			else
			{  
				s[p-1]=(s[p]-'0')/10+'0';
				s[p]=(s[p]-'0')%10+'0';
			}
			return 1;
}

//The Judeg function , judge the symbol of the number
int Judge(int i)
{   
	if (i==0)
		return 0;
	else
		if (i>0)
			return 1;
		else
			return  -1;
}

//The Convert function , convert the original result to the final result ,wipe off the inuility symbol and number
int Convert(char c[],char d[],int length)
{   
	int i,j;
	for (i=0; i<length; i++)
		if (c[i]==MARK || c[i]=='0')
			;
		else
			break;
		for (i,j=0; i<length; i++,j++)
			d[j]=c[i];
		d[j]='\0';
		return 1;
}

