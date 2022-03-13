#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>

void loop();
void parse(char *input);
int execute(char **args);
int execute_cd(char **args);
void executer(char token[20][100],int ctr);
void handle_pipe(char token[20][100],int ctr);
void execute_pipe(char **args1,char **args2);
void handle_redirect(char token[20][100],int ctr);
void execute_redirect(char **args1,char *args2);

void execute_redirect(char **args1,char *args2)
{
	int child_pid=fork();
	
	if(child_pid<0)
	{
		printf("\n Forking Failed");
	}
	else if(child_pid==0)
	{
	int redirection_target = open(args2,O_WRONLY | O_CREAT, 0777);
	if(redirection_target==-1)
	{
		printf("redirection failed");			
	}
	int target2=dup2(redirection_target,STDOUT_FILENO);
	close(redirection_target);
	
	execvp(args1[0],args1);
	}
	else
		wait(NULL);
	
}

void handle_redirect(char token[20][100],int ctr)
{
	char **args1=malloc(60*sizeof(char *));
	char *args2=malloc(60*sizeof(char ));
	int position;
	for(int i=0;i<ctr;i++)
	{
		if(token[i][0]=='>')
		{
			position=i;
			break;
		}
	}
	int k=0;
	for(int i=0;i<position;i++)
	{
		char *a=strdup(token[i]);
		args1[k++]=a;
	}
	args1[k++]=NULL;
	for(int i=position+1;i<ctr;i++)
	{
		strcat(args2,token[i]);
	}
	execute_redirect(args1,args2);
}

void execute_pipe(char **args1,char **args2)
{
int pipefd[2]; 
    pid_t p1, p2;
  
    if (pipe(pipefd) < 0) {
        printf("\nPipe could not be initialized");
        return;
    }
    p1 = fork();
    if (p1 < 0) {
        printf("\nCould not fork");
        return;
    }
  
    if (p1 == 0) {
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);
  
        if (execvp(args1[0], args1) < 0) {
            printf("\nCould not execute command 1..");
            exit(0);
        }
    } else {
        p2 = fork();
  
        if (p2 < 0) {
            printf("\nCould not fork");
            return;
        }
        if (p2 == 0) {
            close(pipefd[1]);
            dup2(pipefd[0], STDIN_FILENO);
            close(pipefd[0]);
            if (execvp(args2[0],args2) < 0) {
                printf("\nCould not execute command 2..");
                exit(0);
            }
        }
            close(pipefd[0]);
            close(pipefd[1]);
            wait(NULL);
            wait(NULL);
        
    }

}

void handle_pipe(char token[20][100],int ctr)
{
	char **args1=malloc(60*sizeof(char *));
	char **args2=malloc(60*sizeof(char *));
	int position;
	for(int i=0;i<ctr;i++)
	{
		if(token[i][0]=='|')
		{
			position=i;
			break;
		}
	}
	int k=0;
	for(int i=0;i<position;i++)
	{
		char *a=strdup(token[i]);
		args1[k++]=a;
	}
	args1[k++]=NULL;
	k=0;
	for(int i=position+1;i<ctr;i++)
	{
		char *a=strdup(token[i]);
		args2[k++]=a;
	}
	args2[k++]=NULL;
	execute_pipe(args1,args2);
}

void executer(char token[20][100],int ctr)
{
int flag=0;
char **args = malloc(60*sizeof(char *));
int k=0;
	for(int i=0;i<ctr;i++)
	{
		if(token[i][0]=='c' && token[i][1]=='d')
		{
			char *a;
			a=strdup(token[i+1]);
			char *b;
			b=strdup(token[i]);
			args[0]=b;
			args[1]=a;
			args[2]=NULL;
			execute_cd(args);
			free(args);
			i+=2;
			k=0;
			flag=1;
			continue;
		}
		if(token[i][0]=='&' && token[i][1]=='&' && flag==0)
		{
			args[k++]=NULL;
			int status =execute(args);
			if(status!=0)
				loop();
			free(args);
			k=0;
			continue;
		}
		char *a;
		a=strdup(token[i]);	
		args[k++]=a;
		flag=0;	
	}
	args[k++]=NULL;
	int status =execute(args);
}




int execute_cd(char **args)
{
if(args[1]==NULL)
{
printf("%s\n","error:expected argument to cd");
}
chdir(args[1]);
return 0;
}

int execute(char **args)
{
  pid_t pid, wpid;
  int status;

  pid = fork();
  if (pid == 0) 
  {
    if (execvp(args[0], args) == -1) 
    {
      perror("error");
    }
    exit(EXIT_FAILURE);
  } 
  else if (pid < 0) 
  {
    perror("error");
  } else 
  {
    do 
    {
      wpid = waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }

  return status;
}


void parse(char *input)
{
char token[20][100];
int i,j,ctr;
j=0;ctr=0;
	for(i=0;i<=(strlen(input));i++)
	{
		if(input[i]==' '||input[i]=='\n')
		{
			token[ctr][j]='\0';
            		ctr++;  
            		j=0;   
		}
		else
		{
			token[ctr][j]=input[i];
            		j++;
		}
	}
	int pipe_present=0;
	int redirect_present=0;
	for(int i=0;i<ctr;i++)
	{
		if(token[i][0]=='|')
		{
			pipe_present=1;
			break;
		}
		if(token[i][0]=='>')
		{
			redirect_present=1;
			break;
		}
	}
	if(pipe_present==1)
	{
		handle_pipe(token,ctr);
	}
	else if(redirect_present==1)
	{
		handle_redirect(token,ctr);
	}
	else
	{
		executer(token,ctr);
	}
}

void loop()
{
	char input[100];
	do{
	printf("NIKET_12040980>>");
	fgets(input,100,stdin);
	if(input[0]=='e'&&input[1]=='x'&&input[2]=='i'&&input[3]=='t')
	{
		printf("%s\n","goodbye");
		exit(0);
	}
	parse(input);
	}while(5>0);
}


int main(int argc,char *argv[])
{
loop();
}
