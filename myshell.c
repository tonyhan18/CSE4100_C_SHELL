/////////////////////////////////////
/* develop : han chan hee   */
/* student id : 20182204    */
/* date : 2021-05-29     */
///////////////////////////////////

#include "myshell.h"
#define MAX_LINE 100001

char    g_line[1000001];
char    g_homeDir[1001];
char    *g_status[] = {
    "running",
    "done",
    "suspended",
    "continued",
    "terminated"
};

int     g_inFlag = 0;
int     g_outFlag = 1;
int     g_JOBCNT = 0;

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
        if(g_job[i].pid == pId) return (i);
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

// front Page of Shell
void    frontPage()
{
    printf("CSE4100-SP-P4> ");
}

void    handler(int sig)
{
    // terminal interrupt
    if (sig == SIGINT)
    {
        frontPage();
        fflush(stdout);
    }
    // terminal stop
    else if (sig == SIGTSTP)
    {
        frontPage();
        fflush(stdout);
    }
    // terminal quit
    else if (sig == SIGQUIT)
    {
        frontPage();
        fflush(stdout);
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

////////////// backgroud ///////////////

int     remove_job(int id) {
    if (id > g_JOBCNT && g_JOBCNT > 0) {
        return (-1);
    }

    for(int i = id ; i <= g_JOBCNT && i < MAX_LINE ; ++i)
    {
        g_job[i] = g_job[i+1];
    }
    if (g_JOBCNT > 0) g_JOBCNT --;
    return 0;
}

int     set_job_status(int id, int status) {
    if (id > g_JOBCNT) {
        return (-1);
    }

    g_job[id].status = status;
    return (0);
}

int     print_job_status(int id) {
    if (id > g_JOBCNT) {
        return (-1);
    }

    printf("[%d]", id);

    for (int i = 1 ; i <= g_JOBCNT ; ++i) {
        printf("\t%d %-15s %s\n", g_job[i].pid, g_status[g_job[i].status], g_job[i].pName);
    }
    return (0);
}

int     wait_for_job(int id) {
    if (id > g_JOBCNT) {
        return (-1);
    }

    int wait_pid = -1;
    int status = 0;
    wait_pid = waitpid(g_job[id].pid, &status, WUNTRACED);
    if (WIFEXITED(status)) {
        set_job_status(wait_pid, STATUS_DONE);
    } else if (WIFSIGNALED(status)) {
        set_job_status(wait_pid, STATUS_TERMINATED);
    } else if (WSTOPSIG(status)) {
        status = -1;
        set_job_status(wait_pid, STATUS_SUSPENDED);
    }

    return status;
}

int     exctJobs(void) {
    int i;

    for (i = 1; i <= g_JOBCNT; i++) {
        print_job_status(i);
    }

    return (0);
}

int     exctFg(char **tokens) {
    if (tokens[1] == NULL) {
        printf("usage: bg <jobs>\n");
        return (-1);
    }

    pid_t pid;
    int job_id = -1;

    if (tokens[1][0] == '%') {
        job_id = atoi(tokens[1] + 1);
        pid = g_job[job_id].pid;
        if (pid < 0) {
            printf("fg %s: no such job\n", tokens[1]);
            return (-1);
        }
    } else {
        job_id = atoi(tokens[1]);
        pid = g_job[atoi(tokens[1])].pid;
    }

    if (kill(pid, SIGCONT) < 0) {
        printf("fg %d: job not found\n", pid);
        return (-1);
    }

    if (job_id > 0) {
        set_job_status(job_id, STATUS_CONTINUED);
        print_job_status(job_id);
        if (wait_for_job(job_id) >= 0) {
            remove_job(job_id);
        }
    } else {
        wait_for_job(job_id);
    }

    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTOU, SIG_DFL);

    return 0;
}

void    exctBg(char **tokens)
{
    if (tokens[1] == NULL) {
        printf("usage: bg <jobs>\n");
        return ;
    }

    pid_t pid;
    int job_id = -1;

    if (tokens[1][0] == '%') {
        job_id = atoi(tokens[1] + 1);
        pid = g_job[job_id].pid;
        if (pid < 0) {
            printf("mysh: bg %s: no such job\n", tokens[1]);
            return ;
        }
    } else {
        job_id = atoi(tokens[1]);
        pid = g_job[atoi(tokens[1])].pid;
    }

    if (kill(pid, SIGCONT) < 0) {
        printf("mysh: bg %d: job not found\n", pid);
        return ;
    }

    if (job_id > 0) {
        set_job_status(job_id, STATUS_CONTINUED);
        print_job_status(job_id);
    }

    return ;
}

void    exctKill(char **tokens) {
    if (tokens[1] == NULL) {
        printf("usage: bg <jobs>\n");
        return ;
    }

    pid_t pid;
    int job_id = -1;

    if (tokens[1][0] == '%') {
        job_id = atoi(tokens[1] + 1);
        pid = g_job[job_id].pid;
        //printf("%d %d\n", job_id, pid);
        if (pid < 0) {
            printf("kill %s: no such job\n", tokens[1]);
            return ;
        }
    } else {
        job_id = atoi(tokens[1]);
        pid = g_job[atoi(tokens[1])].pid;
    }

    if (kill(pid, SIGKILL) < 0) {
        printf("kill %d: job not found\n", pid);
        return ;
    }

    if (job_id > 0) {
        set_job_status(job_id, STATUS_TERMINATED);
        print_job_status(job_id);
        if (wait_for_job(job_id) >= 0) {
            remove_job(job_id);
        }
    }

    return ;
}
/********************************/
/* function : leftCrack */
/* purpose : check < and open the file */
/* return : void */
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
/* function : rightCrack */
/* purpose : check > and open the file */
/* return : void */
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
/* function : exctCMD */
/* purpose : excute line */
/* return : void */
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
            else if (strcmp(tokens[0], "jobs") == 0)
                exctJobs();
            else if (strcmp(tokens[0], "fg") == 0)
                exctFg(tokens);
            else if (strcmp(tokens[0], "bg") == 0)
                exctFg(tokens);
            else if(strcmp(tokens[0], "kill") == 0)
                exctKill(tokens);
            else
            {
                // communicate to process
                if(pipe(fd) < 0 ){
                    printf("pipe error\n");
                    exit(1);
                }
                pid_t pid;
                int flag;

                if((pid = fork()) < 0)
                {
                    printf("fork error\n");
                    exit(1);
                }
                signal(SIGINT, SIG_DFL);
                signal(SIGQUIT, SIG_DFL);
                signal(SIGTSTP, SIG_DFL);
                signal(SIGTTIN, SIG_DFL);
                signal(SIGTTOU, SIG_DFL);
                signal(SIGCHLD, SIG_DFL);
                
                // child process
                if (pid == 0)
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
                        g_job[g_JOBCNT++].status = STATUS_RUNNING;
                        g_job[g_JOBCNT].pid = pid;
                        strcpy(g_job[g_JOBCNT].pName, cur_cmd);
                        printf("[%d] %d\n", g_JOBCNT, g_job[g_JOBCNT].pid);
                        //wait_for_job(g_JOBCNT);
                        signal(SIGTTOU, SIG_IGN);
                        signal(SIGTTOU, SIG_DFL);
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
