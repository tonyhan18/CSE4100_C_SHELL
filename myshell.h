#ifndef __MYSHELL_H__
#define __MYSHELL_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

typedef struct jobs
{
    char    pName[100];
    int     status;
    pid_t   pId;
}s_JOBS;

#endif