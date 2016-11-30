#include<cstdio>
#include<iostream>
#include<stdlib.h>
#include <unistd.h>
#include<fcntl.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<string.h>
#include<signal.h>
#include<string>
FILE *fp,*fd;
char cmdline[2000],readdata[2000];
char command[100][100],curdir[200];
int j,pipepre=0,redpre=0,rec=0;
int env=0,child1=0,child2=0;
int envexp=0,envpwd=0,envvar=0,envhistory=0,bang1=0,bang2=0;
extern char **environ;

void execute_without_piping();
void execute_with_piping();
void bangbang(char *);

void check()
{
int i;
	rec=0;
	if(cmdline[0]=='!'&&cmdline[1]!='\0')
	rec=1;
	for(i=1;i<strlen(cmdline);i++)
	{
		if(cmdline[i-1]==' '&&cmdline[i]=='!')
		rec=1;
	}
	if(rec==0)
	{
	fprintf(fp,"%s",cmdline);
	fflush(fp);
	}
}
void prompt()								//PROMPT FUNCTION
{
if(getcwd(curdir,sizeof(curdir))==NULL)
perror("getcwd");
else
printf("My_Shell:%s$",curdir);
}

void parser()								//STRING PARSER
{
char c;
int i,y,sp,flag,kl;
	pipepre=redpre=0;
	j=y=flag=0;
	sp=1;
	for(i=0;i<100;i++)
	{	for(kl=0;kl<100;kl++)
		command[i][kl]='\0';
	}
	for(i=0;cmdline[i]!='\n';i++)
	{
		c=cmdline[i];
		if(c==' '&&sp==-1&&flag==0)
		{
			sp=1;
			command[j][y]='\0';
			y++;
		}
		else if(c=='"')
		{
			if(flag==0)
			{flag=1;
			}
			else if(flag==1)
			{
			flag=0;
			}		
		}	
		else if (flag==1)
		{
			command[j][y]=c;				
			command[j][y+1]='\0';
			y++;
			sp=-1;
		}
		else if((c!=' '&&c!='|'&&c!='<'&&c!='>'&&flag==0))
		{
			command[j][y]=c;				
			command[j][y+1]='\0';
			y++;
			sp=-1;	
		}
		else if((c=='|'||c=='<'||c=='>')&&flag==0)				
		{
			if(sp==-1)
			{	
				command[j][y]='\0';
			}
			y++;
			command[j][y]='\n';
			j++;
			y=0;
			if(c=='|')
			pipepre=pipepre+1;
			else
			redpre=redpre+1;
		}
	}
			if(sp==-1)
			{	
				command[j][y]='\0';
				y++;			
			}
			command[j][y]='\n';
}

void env_variable(char x[])						//PRINTING ENVIRONMET VARIABLE USING ECHO
{
	char *home=NULL;
	if(getenv(x)!=NULL)
        {
        home = getenv(x);
	printf("%s\n",home);
	}
	else
	printf("\n");
}

void envcd(char x[])							//FUNCTION TO CHANGE DIRECTORY
{
	if(x[0]=='~'||x[0]=='\0')
	{
		char *home;
        	home = getenv("HOME");
       		if(chdir( home )<0)
		perror("chdir");
	}
	else if(x[0]!='\0')
	{
		if(chdir(x)<0)
		perror("chdir");
	}
}

void execute_history(int lineno)				//FUNCTION DEALS WITH HISTORY WITH ARGUMENTS
{
	int totalline=0;
	int rmline=0;
	char c;
	if(fseek(fp,0,SEEK_SET) < 0)
	printf("error1");
	else
	{
		for (c = getc(fp); c != EOF; c = getc(fp))
        	{
        		if (c == '\n') 
				totalline++;
		}
	}
	if(fseek(fp,0,SEEK_SET) < 0)
	printf("error2");
	else
	{
		for (c = getc(fp); c != EOF; c = getc(fp))
		{	
			if (c == '\n') 
			rmline++;
			if(totalline-rmline<=lineno)
			{
				if ((totalline-rmline)!=lineno||((totalline-rmline)==lineno&&c!='\n'))
					printf("%c",c);
			}
		}
	fseek(fp,0,SEEK_CUR) ;
	}
}

void envset( char x[])							//FUNCTION TO SET ENVIRONMENTAL VARIABLE
{
	char var[20],value[100];
	int gh,length;
	for(gh=0;x[gh]!='='&&gh<strlen(x);gh++);
		strncpy(var,&x[0],gh);
	length=strlen(var);
	var[length]='\0';
	if(x[gh]=='=')
		strncpy(value,&x[gh+1],strlen(x)+1);
	else if(x[gh]!='=')
		value[0]='\0';
	setenv(var,value,0);
}

void envexport()							//FUNCTION TO EXECUTE EXPORT COMMAND
{
	int x; 
	char *str = *environ; 
	for (x=1; str; x++) 
	{ 
		printf("declare -x %s\n", str);
		str = *(environ+x); 
	} 
}

void sigintHandler(int sig_num)						//SIGNAL HANDLER FUNCTION 
{
	signal(SIGINT, sigintHandler);
	printf("\n");
	prompt();
	fflush(stdout);
}

void bangbang( char *x)							//FUNCTION TO HANDLE ! OPERATOR
{
int i,kl;
//child1=child2=0;
if(x[0]=='!')
{
	 if(!(fd=popen("tail -1 History.txt | head -1","r")))
	 	printf("error3");    
	 else
	 {    
		memset(cmdline,2000,'\0');	
		if(fgets(cmdline,2000,fd)==NULL)
			printf("error4");  
		else
		{     
        		cmdline[strlen(cmdline)]='\0';
         		printf("%s",cmdline);
         		
         		check();
         		parser();
			//printf("here");
         		if(pipepre==0&&redpre==0)
			{
			//memset(cmdline,2000,'\0');
				child1=1;
				//printf("..%d",child1 );
							
				execute_without_piping();
			}
			else if(pipepre!=0&&redpre==0)
			{
				child2=1;
			//memset(cmdline,2000,'\0');
				
				execute_with_piping();
			
			}
		child1=child2=0;
		}
		pclose(fd);
	}
		
}
else if(x[0]=='-')
{

	int linen=atoi(&(x[1]));
	linen++;
	std::string xyz=std::to_string(linen);
        std::string last=std::string("tail -") + xyz + std::string(" History.txt | head -1");
        const char *read=last.c_str();
        if(!(fd=popen(read,"r")))
        	printf("error");
        else
        {
       		
		memset(cmdline,2000,'\0');	
		if(fgets(cmdline,2000,fd)==NULL)
			printf("error4");  
		else
		{     
        		cmdline[strlen(cmdline)]='\0';
         		printf("%s",cmdline);
         		check();
         		parser();
         		if(pipepre==0&&redpre==0)
			{
				child1=1;
				execute_without_piping();
			}
			else if(pipepre!=0&&redpre==0)
			{
				child2=1;
				//printf("..with");
				execute_with_piping();
			}
		}
		pclose(fd);	
	}
}
else if(x[0]>='1'&&x[0]<='9')
{
	int linen=atoi(x);
	//linen++;
	std::string xyz=std::to_string(linen);
        std::string last=std::string("head -") + xyz +std::string(" History.txt | tail -1");
        const char *read=last.c_str();
        if(!(fd=popen(read,"r")))
        	printf("error");
        else
        {
		memset(cmdline,2000,'\0');	
		if(fgets(cmdline,2000,fd)==NULL)
			printf("error4");  
		else
		{     
        		cmdline[strlen(cmdline)]='\0';
         		printf("%s",cmdline);
         		check();
         		parser();
         		if(pipepre==0&&redpre==0)
			{
				child1=1;
				execute_without_piping();
			}
			else if(pipepre!=0&&redpre==0)
			{
				child2=1;
				//printf("..with");
				execute_with_piping();
			}
		}
	}
}
else
{
        std::string last=std::string("grep ") + x + " History.txt | tail -1 | head -1";
        const char *read=last.c_str();
        if(!(fd=popen(read,"r")))
        	printf("error");
        else
        {
		memset(cmdline,2000,'\0');	
		if(fgets(cmdline,2000,fd)==NULL)
			printf("error4");  
		else
		{     
        		cmdline[strlen(cmdline)]='\0';
         		printf("%s",cmdline);
         		check();
         		parser();
         		if(pipepre==0&&redpre==0)
			{
				child1=1;
				
				execute_without_piping();
			}
			else if(pipepre!=0&&redpre==0)
			{
				child2=1;
				execute_with_piping();
			}
		}
	}
}
}


void execute_without_piping()					  		//FUNCTIONS THAT EXECUTE COMMAND WITHOUT PIPE
{
	char *p[20];
	int count=0,k2,status;
	for(k2=0;k2<20;k2++)
	{						
		p[k2]=NULL;
	}

	p[count]=&(command[0][0]);
	
	for(k2=2;command[0][k2]!='\n';k2++)
	{
		if(command[0][k2-1]=='\0'&&command[0][k2]!='\0'&&command[0][k2]!='\n')
		{
			count++;		
			p[count]=&(command[0][k2]);
		}
	
	}
	env=envexp=envvar=envhistory=0;
	if(child1==0)
	bang1=0;
	char x1[10]="cd";
	if(strcmp(p[0],x1)==0)
	{
	char x[100];
	if(p[1]!=NULL)
	{
	strcpy(x,&(*(p[1])));
	envcd(x);
	env=1;

	}
	else 
	{
		
		strcpy(x,"\0");
		envcd(x);
		env=1;
	}
	}
	char x3[10]="exit";
	if(strcmp(x3,p[0])==0)
	{
	printf("Bye...\n");
	exit(0);
	}
	
	char x2[10]="export";
	if((strcmp(p[0],x2))==0)
	{
		envexp=1;
		if(p[1]==NULL)
		envexport();	
		else
		envset(p[1]);
	}	
	
	char x4[10]="echo";
	if((strcmp(p[0],x4))==0)
	{
		if(*p[1]=='$')
		envvar=1;
		char x[10];
		strncpy(x,&p[1][1],strlen(p[1]));
		if(envvar==1)
		env_variable(x);
	}
	
	char x5[10]="history";
	if((strcmp(p[0],x5))==0)
	{
		envhistory=1;
		if(p[1]==NULL)
		{
		  fseek( fp, 0, SEEK_SET );
			memset(readdata,'\0',2000);
			while(fgets(readdata,2000,fp)!=NULL)
   			{
   				printf("%s",readdata);
    				memset(readdata,'\0',2000);
    				
   			}
   		}
		else
		{
			int lineno=atoi(p[1]);
			execute_history(lineno);
		}	
	}
	if(p[0][0]=='!')
	{
		char *x=&(p[0][1]);
		bang1=1;
		bangbang(x);
	}
		
		
		
		
			
	if(env==0&&envvar==0&&envexp==0&&envhistory==0&&(bang1==0||child1==1))
	{
	pid_t childpid=fork();	
		
		if(child1==1)
		wait(&status);
			
	if(childpid==0)
	{
			
		if(execvp(p[0],p)<0) 
		{
			perror("execvp");
			exit(1);
		}
		exit(0);		
	}
	wait(&status);				
	}
	env=envexp=envvar=envhistory=0;
}
void execute_with_piping()							//FUNCTIONS THAT EXECUTE COMMAND WITH PIPE
{
	char pass[100],*ps;
	memset(pass,100,'\0');
	int stdin_copy=dup(0);
	int stdout_copy=dup(1);
	int l1,status;
	for(l1=0;l1<=j;l1++)
	{
		char *p[20];
		int count=0,k2;
		int pd[2];
	        pipe(pd);
		for(k2=0;k2<20;k2++)
		{						
			p[k2]=NULL;
		}
		p[count]=&(command[l1][0]);
		env=envexp=envvar=envhistory=0;
		if(child2==0)
		bang2=0;

	char x1[10]="cd";
	if(strcmp(p[0],x1)==0)
	{
		if(p[1]!=NULL)
		{
			strcpy(pass,&(*(p[1])));
			env++;
		}
		else 
		{
			strcpy(pass,"\0");
			env++;
		}
	}
	char x2[10]="export";
	if((strcmp(p[0],x2))==0)
	{
		envexp=1;
	}	
	
	char x4[10]="echo";
	if((strcmp(p[0],x4))==0)
	{
		if(*p[1]=='$')
		envvar=1;
		printf("ssadsada");
		//char x[10];
		strncpy(pass,&p[1][1],strlen(p[1]));
		printf("%s",pass);
		//if(envvar==1)
		//env_variable(x);
	}
	
	char x5[10]="history";
	if((strcmp(p[0],x5))==0)
	{
		envhistory=1;
		
	}

	if(p[0][0]=='!')
	{
		ps=&(p[0][1]);
		bang2=1;
		
		bangbang(ps);
	}
			
		for(k2=2;command[l1][k2]!='\n';k2++)
		{
			if(command[l1][k2-1]=='\0'&&command[l1][k2]!='\0'&&command[l1][k2]!='\n')
			{
				count++;		
				p[count]=&(command[l1][k2]);
			}
	
		}
		if(l1<j)
		{
			pid_t childpid=fork();	
			
			if(child2==1)
			wait(&status);	
			
			if(childpid==0)
			{
				dup2(pd[1], 1);
				if(env==1)
				envcd(pass);
				else if(envexp==1)
				{
					//envexport();
					if(p[1]==NULL)
						envexport();	
					else
						envset(p[1]);
				}	
				else if(envvar==1)
				{			
					env_variable(pass);
				}	
				else if(envhistory==1)
				{
					if(p[1]==NULL)
					{
						fseek( fp, 0, SEEK_SET );
						memset(readdata,'\0',2000);
						while(fgets(readdata,2000,fp)!=NULL)
   						{
   							printf("%s",readdata);
    							memset(readdata,'\0',2000);
    				
   						}
  			 		}
				
					else
						execute_history(atoi(p[1]));
				}
				else if(bang2==1&&child2==0)
					bangbang(ps);
				else if(env==0&&envvar==0&&envexp==0&&envhistory==0&&(bang2==0||child2==1))
				{
				
				if(execvp(p[0],p)<0) 
					{
						perror("execvp");
						exit(1);
					}
				}			
				exit(0);		
			}
				wait(&status);			
				dup2(pd[0], 0);
	        		close(pd[1]);
		}
		else
		{
			pid_t childpid=fork();		
			if(childpid==0)
			{
				if(execvp(p[0],p)<0) 
				{
					perror("execvp");
					exit(1);
				}
				abort();
				exit(0);
				
			}
			wait(&status);
		close(pd[0]);
		close(pd[1]);
		dup2(stdin_copy,0);
		dup2(stdout_copy,1);
		}		
	}			
return ;	
}
int main()										//MAIN FUNCTION
{
	signal(SIGINT, sigintHandler);
//int rec=0,i;
prompt();
fp=fopen("History.txt","a+");	
while(1)
{
	//prompt();

	child1=child2=0;
	bang1=bang2=0;
	memset(cmdline,'\0',2000);
	fgets(cmdline,2000,stdin);
	if(cmdline[0]=='\n')
	{
		prompt();
		continue;
	}
	check();
	parser();
	if(pipepre==0&&redpre==0)
	{
	
		execute_without_piping();
	}
	else if(pipepre!=0&&redpre==0)
	{
		execute_with_piping();
	}
	prompt();
}
fclose(fp);
return 0;
}	
