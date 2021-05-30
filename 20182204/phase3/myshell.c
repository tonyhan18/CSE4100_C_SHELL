/////////////////////////////////////
/* develop : han chan hee   */
/* student id : 20182204    */
/* date : 2021-05-29     */
///////////////////////////////////

#include "myshell.h"

char    g_line[1000001];
char    g_homeDir[1001];
char    *g_status[3] = {"Running", "Stopped", "Done"};

int     g_inFlag = 0;
int     g_outFlag = 1;
int     g_JOBCNT = 1;

s_JOBS  g_job[1001];

/********************************/
/* function : parse */
/* purpose : parsing the line by delim */
/* return : void */
/********************************/
void    parse(char *cmd, char **tokens, char *limit)
{
    char *tmp;

    // seperate cmd by limit and save to tokens
    tmp = strtok(cmd, limit);
    int i = 0;
    while (tmp != NULL)
    {
        tokens[i++] = tmp;
        tmp = strtok(NULL, limit);
    }
}

int     pId_to_idx(int pId)
{
    for(int i = 1 ; i< g_JOBCNT; ++i)
    {
        if(g_job[i].pId == pId) return (i);
    }
    return (-1);
}

/********************************/
/* function : trim */
/* purpose : trimming back and front */
/* return : void */
/********************************/
void    trim(char *cmd)
{
    int idx = 0;
    int i = 0;

    // trim front
    while (cmd[idx] == ' ' || cmd[idx] == '\t' || cmd[idx] == '\n')
        idx++;

    while (cmd[i + idx] != '\0')
    {
        cmd[i] = cmd[i + idx];
        i++;
    }
    cmd[i] = '\0';

    // trim back
    i = 0;
    idx = -1;
    while (cmd[i] != '\0')
    {
        if (cmd[i] != ' ' && cmd[i] != '\t' && cmd[i] != '\n')
            idx = i;
        i++;
    }
    cmd[idx + 1] = '\0';
}

/********************************/
/* function : deleteJob */
/* purpose : delete job from g_job list */
/* return : void */
/********************************/
void    delJob(int jobpid)
{
    int i;
    int flag = 0;

    for (i = 1; i < g_JOBCNT; i++)
    {
        //printf("jobpid : %d | %d\n ",jobpid, g_job[i].pId);
        if (g_job[i].pId == jobpid)
            flag = 1;

        if (flag == 1)
            g_job[i] = g_job[i + 1];
    }
    if (g_JOBCNT != 1) g_JOBCNT--;
}

// front Page of Shell
void    frontPage()
{
    printf("CSE4100-SP-P4> ");
}

void    handler(int sig)
{
    union wait state;
    // terminal interrupt
    if (sig == SIGINT)
    {
        printf("SIGINT\n");
        frontPage();
        fflush(stdout);
    }
    // terminal stop
    else if (sig == SIGTSTP)
    {
        //pid_t pid = wait3(&state, WNOHANG, (struct rusage *)NULL);
        int pid = getpid();
        int idx = pId_to_idx(pid);
        g_job[idx].status = 1;
        printf("SIGTSTP\n");
        printf("[%d] %-20s %s\n", idx, g_status[g_job[idx].status], g_job[idx].pName);
        frontPage();
        fflush(stdout);
    }
    // terminal quit
    else if (sig == SIGQUIT)
    {
        printf("SIGQUIT\n");
        frontPage();
        fflush(stdout);
    }
    // child process end
    else if (sig == SIGCHLD)
    {
        pid_t pID;

        printf("SIGCHLD\n");
        while (1)
        {
            pID = wait3(&state, WNOHANG, (struct rusage *)NULL);
            int     idx = pId_to_idx(pID);
            if (pID == 0 || pID == -1 || idx == -1)
                return;
            else
            {
                fprintf(stderr, "\n[%d] %-20s %s\n", idx, g_status[2], g_job[idx].pName);
                delJob(pID);
            }
        }
    }
}

/********************************/
/* function : exctCD */
/* purpose : change dir */
/* return : void */
/********************************/
void    exctCD(char **tokens)
{
    char home_tmp[1000];

    strcpy(home_tmp, g_homeDir);

    int i;
    int len = strlen(g_homeDir);

    //If command is 'cd' , change to ~
    if (tokens[1] == NULL)
        chdir(g_homeDir);
    // if command is 'cd ~/...'
    else if (tokens[1][0] == '~')
    {
        for (i = 1; tokens[1][i] != '\0'; i++)
        {
            home_tmp[i + len - 1] = tokens[1][i];
        }
        home_tmp[i + len - 1] = '\0';
        if (chdir(home_tmp) != 0)
            printf("-bash: cd: %s: No such file or directory", tokens[1]);
            //perror("Error");
    }
    // etc...
    else if (chdir(tokens[1]) != 0)
        printf("-bash: cd: %s: No such file or directory", tokens[1]);
        //perror("Error");
}

/********************************/
/* function : exctPWD */
/* purpose : print cur dir */
/* return : void */
/********************************/
void    exctPWD(void)
{
    char curDir[1000];

    getcwd(curDir, 1000);
    printf("%s\n", curDir);
}

/********************************/
/* function : exctECHO */
/* purpose : print content */
/* return : void */
/********************************/
void    exctECHO(char **tokens)
{
    int i;
    int j;
    char input[1000006];
    int doubleFlag = 0;
    int singleFlag = 0;

    i = 1;
    j = 0;
    //checking for inverted commas
    while (tokens[i] != NULL)
    {
        // read by tokensand chk "" or ''
        for (int k = 0; tokens[i][k] != '\0'; k++)
        {
            // if " is not found
            if (doubleFlag == 1)
            {
                if (tokens[i][k] == '\"')
                    doubleFlag = 0;
                else
                    input[j++] = tokens[i][k];
            }
            // if ' is not found
            else if (singleFlag == 1)
            {
                if (tokens[i][k] == '\'')
                    singleFlag = 0;
                else
                    input[j++] = tokens[i][k];
            }
            else
            {
                if (tokens[i][k] == '\"')
                    doubleFlag = 1;
                else if (tokens[i][k] == '\'')
                    singleFlag = 1;
                else
                    input[j++] = tokens[i][k];
            }
        }
        i++;
    }
    input[j] = '\0';

    // chk if ' or " is not finished
    if (doubleFlag == 0 && singleFlag == 0)
        printf("%s\n", input);
    else
        printf("Error: Wrong Input\n");
}

/********************************/
/* function : exctJOBS */
/* purpose : print all process */
/* return : void */
/********************************/
void    exctJOBS()
{
    int i;

    if (g_JOBCNT == 1)
        return;
    //printf("No background processes running\n");
    for (i = 1; i < g_JOBCNT; i++)
        printf("[%d] %-20s %s\n", i, g_status[g_job[i].status], g_job[i].pName);
}

/********************************/
/* function : exctKILL */
/* purpose : kill process */
/* return : void */
/********************************/
void    exctKILL(char **tokens)
{
    // Error there is no parameter
    if (tokens[1] == NULL)
        printf("kill: usage: kill <jobs>\n");
    else
    {
        kill(g_job[atoi(tokens[1])].pId, SIGINT);
        delJob(g_job[atoi(tokens[1])].pId);
        g_JOBCNT--;
    }
}

void    exctOVERKILL(void)
{
    int i;

    if (g_JOBCNT > 1)
        for (i = g_JOBCNT - 1; i > 0; i--)
        {
            kill(g_job[i].pId, 9);
            signal(SIGCHLD, handler);
        }
    else
        printf("No Background Jobs detected.\n");
    g_JOBCNT = 1;
}

void exctFG(char **tokens)
{
    int flag;
    if (tokens[1] == NULL)
        fprintf(stderr, "Error: invalid format\n");
    else
    {
        int jobno = atoi(tokens[1]);
        if (jobno < g_JOBCNT)
        {
            printf("%s\n",g_job[jobno].pName);
            g_job[jobno].status = 0; 
            kill(g_job[jobno].pId, SIGCONT);
            g_job[jobno].status = 1;
            delJob(g_job[jobno].pId);
            wait(&flag);
        }
        else
            printf("Job number does not exist\n");
    }
}

void    exctBG(char **tokens)
{
    if(tokens[1] == NULL){
            fprintf(stderr, "Error: invalid format\n");
    }
    else{
        int jobno = atoi(tokens[1]);

        if(jobno > g_JOBCNT || g_JOBCNT < 1){
            fprintf(stderr, "Error: invalid job id\n");    
        }
        else{
            g_job[jobno].status = 0;
            kill(g_job[jobno].pId, SIGCONT);            // sigcont to continue process
            g_job[jobno].status = 1;
        }
    }
        
}

/********************************/
/* function : whiteSpace */
/* purpose : check out cmd is exist */
/* return : int */
/********************************/
void leftCrack(char *tokens)
{
    char *tmp[100] = {NULL};

    parse(tokens, tmp, "<");

    if (tmp[1] != NULL)
    {
        char *files[100] = {NULL};
        parse(tmp[1], files, " \t");

        // O_RDONLY : file readonly
        g_inFlag = open(files[0], O_RDONLY);
    }
}

/********************************/
/* function : whiteSpace */
/* purpose : check out cmd is exist */
/* return : int */
/********************************/
void rightCrack(char *tokens)
{
    char *temp[100] = {NULL};

    // parsing by >
    parse(tokens, temp, ">");
    if (temp[1] != NULL)
    {
        char *files[100] = {NULL};
        parse(temp[1], files, " \t");

        // open the file name
        // O_TRUNC : 기존 파일 내용 삭제, O_WRONLY : 쓰기전용, O_CREAT : 해당 파일이 없으면 생성
        // S_IRWXU : 읽기, 쓰기, 실행 권한 부여
        g_outFlag = open(files[0], O_TRUNC | O_WRONLY | O_CREAT, S_IRWXU);
    }
    else
        g_outFlag = 1;
}

/********************************/
/* function : backgroundChk */
/* purpose : checkout if cmd need background */
/* return : int */
/********************************/
int backgroundChk(char **tokens)
{
    int i;

    i = 0;
    while (tokens[i] != NULL)
    {
        if (strcmp(tokens[i], "&") == 0)
        {
            tokens[i] = NULL;
            return (1);
        }
        i++;
    }
    return (0);
}

/********************************/
/* function : whiteSpace */
/* purpose : check out cmd is exist */
/* return : int */
/********************************/
void exctCMD()
{
    char *tempSemi[100] = {NULL};
    char *tempPipe[100] = {NULL};
    int j = 0;

    trim(g_line);
    // parsing if there are multiple commands and save to tempSemi
    parse(g_line, tempSemi, ";");
    // run the cmd
    while (tempSemi[j] != NULL)
    {
        int i = 0;
        int fd[2];
        // parsing if there are pipeline and save to tempPipe
        parse(tempSemi[j], tempPipe, "|");
        while (tempPipe[i] != NULL)
        {
            char    *tokens[100] = {NULL}; // save all the cmd
            char    *temp1[100] = {NULL};
            char    *temp2[100] = {NULL};
            char    cur_cmd[100];
            strcpy(cur_cmd, tempSemi[j]);

            rightCrack(tempPipe[i]);
            leftCrack(tempPipe[i]);

            // Seperating commands and get real command
            parse(tempPipe[i], temp1, "<");
            parse(temp1[0], temp2, ">");
            parse(temp2[0], tokens, " \t");

            if (tokens[0] == NULL)
                return;

            int bgFlag = backgroundChk(tokens); // checkout if there is & mark
            signal(SIGCHLD, handler);           // check out child process is terminated

            // check out if cmd function is built-in
            //strlwr(tokens[0]); // to lower the cmd
            if (strcmp(tokens[0], "exit") == 0)
                _exit(0);
            else if (strcmp(tokens[0], "cd") == 0)
                exctCD(tokens);
            // else if (strcmp(tokens[0], "pwd") == 0)
            //     exctPWD();
            // else if (strcmp(tokens[0], "echo") == 0)
            //     exctECHO(tokens);
            else if (strcmp(tokens[0], "jobs") == 0)
                exctJOBS();
            // else if (strcmp(tokens[0], "kill") == 0)
            //     exctKILL(tokens);
            // // else if (strcmp(tokens[0], "overkill") == 0)
            // //     exctOVERKILL();
            // else if (strcmp(tokens[0], "fg") == 0)
            //     exctFG(tokens);
            // else if (strcmp(tokens[0], "bg") == 0)
            //     exctBG(tokens);
            else
            {
                // communicate to process
                if(pipe(fd) < 0 ){
                    printf("pipe error\n");
                    exit(1);
                }
                if (bgFlag)
                    strcpy(g_job[g_JOBCNT].pName, cur_cmd);

                pid_t pId;
                int flag;

                if((pId = fork()) < 0)
                {
                    printf("fork error\n");
                    exit(1);
                }
                
                // child process
                if (pId == 0)
                {
                    if (g_inFlag != 0)
                    {
                        dup2(g_inFlag, STDIN_FILENO);
                        close(g_inFlag);
                    }
                    if (g_outFlag != 1)
                    {
                        dup2(g_outFlag, STDOUT_FILENO);
                        close(g_outFlag);
                    }
                    // if there is next pipeline
                    if (tempPipe[i + 1] != NULL)
                    {
                        dup2(fd[1], STDOUT_FILENO);
                        close(fd[1]);
                    }
                    // executing command
                    if (execvp(*tokens, tokens) < 0)
                    {
                        perror("Error ");
                        exit(0);
                    }
                }
                // parent
                else
                {
                    // Waiting for child process to end
                    if (!bgFlag)
                        wait(&flag);
                    // if background process is started
                    else
                    {
                        g_job[g_JOBCNT].status = 0;
                        g_job[g_JOBCNT++].pId = pId;
                        printf("[%d] %d\n", g_JOBCNT - 1, g_job[g_JOBCNT - 1].pId);
                    }
                    g_inFlag = fd[0];
                    close(fd[1]);
                }
            }
            i++;
        }
        g_inFlag = 0;
        g_outFlag = 1;
        j++;
    }
}

/********************************/
/* function : whiteSpace */
/* purpose : check out cmd is exist */
/* return : int */
/********************************/
int whiteSpace()
{
    int i = 0;

    while (g_line[i] != '\0')
    {
        if (g_line[i] != ' ' && g_line[i] != '\t')
            return (0);
        i++;
    }
    return (1);
}

int main(int argc, char *argv[])
{
    getcwd(g_homeDir, 1000);
    //Loop to take input repeatedly
    while (1)
    {
        g_line[0] = '\0';

        // signal control
        //signal(SIGINT, SIG_IGN);          // terminal interrupt -> ignore
        //signal(SIGQUIT, SIG_IGN);         // terminal quit -> ignore
        signal(SIGTSTP, SIG_DFL);         // terminal stop -> ignore
        if (signal(SIGINT, handler) == 0) // terminal interrupt -> handler
            continue;
        if (signal(SIGQUIT, handler) == 0) // terminal quit -> handler
            continue;
        // if (signal(SIGTSTP, handler) == 0) // terminal stop -> ignore
        //     continue;

        frontPage(); // show the front page

        if (scanf("%[^\n]s", g_line) != EOF)
        {
            getchar();        // get last word
            if (whiteSpace()) // check out cmd is exist
                continue;
        }
        else
        {
            putchar('\n');
            continue;
        }

        //executing commands
        exctCMD();
    }
    return (0);
}
