/* 
 * tsh - A tiny shell program with job control
 * 
 * <Put your name and login ID here>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

/* Misc manifest constants */
#define MAXLINE    1024   /* max line size */
#define MAXARGS     128   /* max args on a command line */
#define MAXJOBS      16   /* max jobs at any point in time */
#define MAXJID    1<<16   /* max job ID */

/* Job states */
#define UNDEF 0 /* undefined */
#define FG 1    /* running in foreground */
#define BG 2    /* running in background */
#define ST 3    /* stopped */

/* 
 * Jobs states: FG (foreground), BG (background), ST (stopped)
 * Job state transitions and enabling actions:
 *     FG -> ST  : ctrl-z
 *     ST -> FG  : fg command
 *     ST -> BG  : bg command
 *     BG -> FG  : fg command
 * At most 1 job can be in the FG state.
 */

/* Global variables */
extern char **environ;      /* defined in libc */
char prompt[] = "tsh> ";    /* command line prompt (DO NOT CHANGE) */
int verbose = 0;            /* if true, print additional output */
int nextjid = 1;            /* next job ID to allocate */
char sbuf[MAXLINE];         /* for composing sprintf messages */

struct job_t {              /* The job struct */
    pid_t pid;              /* job PID */
    int jid;                /* job ID [1, 2, ...] */
    int state;              /* UNDEF, BG, FG, or ST */
    char cmdline[MAXLINE];  /* command line */
};
struct job_t jobs[MAXJOBS]; /* The job list */
/* End global variables */


/* Function prototypes */

/* Here are the functions that you will implement */
void eval(char *cmdline);
int builtin_cmd(char **argv);
void do_bgfg(char **argv);
void waitfg(pid_t pid);

void sigchld_handler(int sig);
void sigtstp_handler(int sig);
void sigint_handler(int sig);

int send_signal(pid_t pid,int sig);
sigset_t mask_signal(int how,int sig);
/* Here are helper routines that we've provided for you */
void do_echo(char **argv);

int parseline(const char *cmdline, char **argv); 
void sigquit_handler(int sig);

void clearjob(struct job_t *job);
void initjobs(struct job_t *jobs);
int maxjid(struct job_t *jobs); 
int addjob(struct job_t *jobs, pid_t pid, int state, char *cmdline);
int deletejob(struct job_t *jobs, pid_t pid); 
pid_t fgpid(struct job_t *jobs);
struct job_t *getjobpid(struct job_t *jobs, pid_t pid);
struct job_t *getjobjid(struct job_t *jobs, int jid); 
int pid2jid(pid_t pid); 
void listjobs(struct job_t *jobs);

void usage(void);
void unix_error(char *msg);
void app_error(char *msg);
typedef void handler_t(int);
handler_t *Signal(int signum, handler_t *handler);

/*
 * main - The shell's main routine 
 */
int main(int argc, char **argv) 
{
    char c;
    char cmdline[MAXLINE];
    int emit_prompt = 1; /* emit prompt (default) */

    /* Redirect stderr to stdout (so that driver will get all output
     * on the pipe connected to stdout) */
    //将stderr重定向指向stdout，这样所有输出都指向了stdout
    dup2(1, 2);

    /* Parse the command line */
    //解析命令行，一般命令行中有命令名称、可选字符以及命令行参数三部分。
    //linux提供函数getopt，来解析命令行参数和可选字符
    while ((c = getopt(argc, argv, "hvp")) != EOF) {
        switch (c) {
        case 'h':             /* print help message */
            usage();
	    break;
        case 'v':             /* emit additional diagnostic info */
            verbose = 1;
	    break;
        case 'p':             /* don't print a prompt */
            emit_prompt = 0;  /* handy for automatic testing */
	    break;
	default:
            usage();
	}
    }

    /* Install the signal handlers */

    /* These are the ones you will need to implement */
    //signal函数，只要有信号发出，执行信号处理函数，与main函数并发进行
    Signal(SIGINT,  sigint_handler);   /* ctrl-c */
    Signal(SIGTSTP, sigtstp_handler);  /* ctrl-z */
    Signal(SIGCHLD, sigchld_handler);  /* Terminated or stopped child */

    /* This one provides a clean way to kill the shell */
    Signal(SIGQUIT, sigquit_handler); 

    /* Initialize the job list */
    initjobs(jobs);

    /* Execute the shell's read/eval loop */
    while (1) {

	/* Read command line */
	if (emit_prompt) {
	    printf("%s", prompt);
	    fflush(stdout);
	}
	if ((fgets(cmdline, MAXLINE, stdin) == NULL) && ferror(stdin))
	    app_error("fgets error");
	if (feof(stdin)) { /* End of file (ctrl-d) */
	    fflush(stdout);
	    exit(0);
	}

	/* Evaluate the command line */
	eval(cmdline);
	fflush(stdout);
	fflush(stdout);
    } 

    exit(0); /* control never reaches here */
}
  
/* 
 * eval - Evaluate the command line that the user has just typed in
 * 
 * If the user has requested a built-in command (quit, jobs, bg or fg)
 * then execute it immediately. Otherwise, fork a child process and
 * run the job in the context of the child. If the job is running in
 * the foreground, wait for it to terminate and then return.  Note:
 * each child process must have a unique process group ID so that our
 * background children don't receive SIGINT (SIGTSTP) from the kernel
 * when we type ctrl-c (ctrl-z) at the keyboard.  
*/
void eval(char *cmdline) 
{
    char *argv[MAXARGS];
    char buf[MAXLINE];
    int bg;//判断前后台
    pid_t pid;

    strcpy(buf,cmdline);
    bg=parseline(buf,argv);
    if(argv[0]==NULL){
        return;
    }
    //builtin_cmd 函数是用来检查，命令行操作是不是内置的！！！
    if(bg!=-1&&!builtin_cmd(argv)){
        //先阻塞SIGCHLD信号，直到子进程的job被添加到job集合里面
        mask_signal(SIG_BLOCK,SIGCHLD);
        if((pid=fork())==0){
            mask_signal(SIG_UNBLOCK,SIGCHLD);
            //setpgid(0,0)的意思是把当前调用此函数的进程号作为进程组号，并将该进程加入该新进程组
            if(setpgid(0,0)<0) unix_error("eval:setpgid failed");
            if(execve(argv[0],argv,environ)<0){
                printf("%s:Commond not found.\n",argv[0]);
                exit(0);}
            }
        else{
            if(!bg){
                addjob(jobs,pid,FG,cmdline);
            }
            else{
                addjob(jobs,pid,BG,cmdline);
            }
            //解除阻塞信号SIGCHLD，因为已经将job添加，不会影响信号处理程序的job删除操作
            mask_signal(SIG_UNBLOCK,SIGCHLD);
            //阻塞到pid进程的作业状态state不是FG
            if(!bg){
                //一直sleep该进程，如果没被设为当前进程。
                waitfg(pid);
            }
            else{
                printf("[%d] (%d) %s",pid2jid(pid),pid,cmdline);
            }
        }
    }
    return;
}

/*建立命令行参数 
 * parseline - Parse the command line and build the argv array.
 * 
 * Characters enclosed in single quotes are treated as a single
 * argument.  Return true if the user has requested a BG job, false if
 * the user has requested a FG job.  
 */
int parseline(const char *cmdline, char **argv) 
{
    static char array[MAXLINE]; /* holds local copy of command line */
    char *buf = array;          /* ptr that traverses command line */
    char *delim;                /* points to first space delimiter */
    int argc;                   /* number of args */
    int bg;                     /* background job? */

    strcpy(buf, cmdline);
    buf[strlen(buf)-1] = ' ';  /* replace trailing '\n' with space */
    //空格ascii码中对应32，也是有数值的。只有'/0'对应NULL为代表0
    while (*buf && (*buf == ' ')) /* ignore leading spaces */
	buf++;

    /* Build the argv list */
    argc = 0;
    if (*buf == '\'') {
	buf++;
    //strchr，找到第二个参数在第一个参数中的位置，并且返回一个指针指向该位置
	delim = strchr(buf, '\'');
    }
    else {
	delim = strchr(buf, ' ');
    }

    while (delim) {
	argv[argc++] = buf;
	*delim = '\0';//这样可以隔断argv指针数组里面的元素，因为c风格字符串结尾就是'\0'
	buf = delim + 1;
	while (*buf && (*buf == ' ')) /* ignore spaces */
	       buf++;

	if (*buf == '\'') {
	    buf++;
	    delim = strchr(buf, '\'');
	}
	else {
	    delim = strchr(buf, ' ');
	}
    }
    argv[argc] = NULL;
    
    if (argc == 0)  /* ignore blank line */
	return 1;
    //命令行的末尾如果有&，则默认是后台执行
    /* should the job run in the background? */
    if ((bg = (*argv[argc-1] == '&')) != 0) {
	argv[--argc] = NULL;
    }
    return bg;
}

/* 
 * builtin_cmd - If the user has typed a built-in command then execute
 *    it immediately.  
 */
int builtin_cmd(char **argv) 
{
    //strcmp,若是相等则返回0
    if(!strcmp(argv[0],"quit"))
        exit(0);
    if(!strcmp(argv[0],"&"))
        return 1;
    if(!strcmp(argv[0],"echo"))
        do_echo(argv);
        return 1;
    if(!strcmp(argv[0],"bg")||!strcmp(argv[0],"fg")){
        printf("bg/fg \n");
        do_bgfg(argv);
        return 1;
    }
    if(!strcmp(argv[0],"jobs")){
        listjobs(jobs);
        return 1;
    }
    return 0;     /* not a builtin command */
}

/* 
 * do_bgfg - Execute the builtin bg and fg commands
 */
void do_bgfg(char **argv) 
{
    char *id=argv[1];
    struct job_t *job;
    int jobid;
    //防止 bg %21331 这种格式的
    if(id[0]=='%')
        jobid=atoi(id+=sizeof(char));
    if((job=getjobjid(jobs,jobid))==NULL){
        printf("no current job");
        return;
    }
    if(!strcmp(argv[0],"bg")){
        job->state=BG;
        //给前台进程组发信号，让它们继续执行任务
        send_signal(-1*job->pid,SIGCONT);
    }
    else if(!strcmp(argv[0],"fg")){
        job->state=FG;
        send_signal(-1*job->pid,SIGCONT);
        waitfg(job->pid);
    }
    return;
}
void do_echo(char**argv){
    printf("%s",argv[1]);
}

/* 
 * waitfg - Block until process pid is no longer the foreground process
 */
void waitfg(pid_t pid)
{
    while(pid==fgpid(jobs)){
        sleep(0);
    }
    return;
}

/*****************
 * Signal handlers
 *****************/

/* 
 * sigchld_handler - The kernel sends a SIGCHLD to the shell whenever
 *     a child job terminates (becomes a zombie), or stops because it
 *     received a SIGSTOP or SIGTSTP signal. The handler reaps all
 *     available zombie children, but doesn't wait for any other
 *     currently running children to terminate.  
 */
void sigchld_handler(int sig) 
{
    pid_t pid;
    int status,child_sig;
    while((pid=waitpid(-1,&status,WUNTRACED|WNOHANG))>0){
        printf("Handle child %d \n",(int)pid);
        sigint_handler(sig);//书中P496
        if(WIFSTOPPED(status)){
            sigtstp_handler(WSTOPSIG(status));
        }
        else if(WIFSIGNALED(status)){
            child_sig=WTERMSIG(status);
            if(child_sig==SIGINT)
                sigint_handler(child_sig);
            else
                unix_error("uncaught signal \n");
        }
        else{
            deletejob(jobs,pid);
        }
    }
    return;
}

/* 
 * sigint_handler - The kernel sends a SIGINT to the shell whenver the
 *    user types ctrl-c at the keyboard.  Catch it and send it along
 *    to the foreground job.  
 */
 //来自键盘的中断,给前台进程组发送SIGINT信号
void sigint_handler(int sig) 
{
    pid_t pid= fgpid(jobs);
    int jid=pid2jid(pid);
    if(pid!=0){
        printf("Job %d terminated by signal: Interrupt",jid);
        deletejob(jobs,pid);
        send_signal(-pid,sig);//把信号发给进程组，防止有漏网之鱼
    }
    return;
}

/*
 * sigtstp_handler - The kernel sends a SIGTSTP to the shell whenever
 *     the user types ctrl-z at the keyboard. Catch it and suspend the
 *     foreground job by sending it a SIGTSTP.  
 */
 //暂停前台进程组，即挂起
void sigtstp_handler(int sig) 
{
    //printf("stop sig \n");
    //获取后台进程的pid
    pid_t pid=fgpid(jobs);
    int jid=pid2jid(pid);
    if(pid!=0){
        printf("Job [%d] (%d) stopped by signal %d \n",jid,pid,sig);
        (*(getjobpid(jobs,pid))).state=ST;//job 状态设为暂停
        send_signal(-pid,sig);
    }
    return;
}

/*********************
 * End signal handlers
 *********************/

/***********************************************
 * Helper routines that manipulate the job list
 **********************************************/

/* clearjob - Clear the entries in a job struct */
//注意job_t的结构
void clearjob(struct job_t *job) {
    job->pid = 0;
    job->jid = 0;
    job->state = UNDEF;
    job->cmdline[0] = '\0';
}

/* initjobs - Initialize the job list */
void initjobs(struct job_t *jobs) {
    int i;
    for (i = 0; i < MAXJOBS; i++)
	clearjob(&jobs[i]);
}

/* maxjid - Returns largest allocated job ID */
int maxjid(struct job_t *jobs) 
{
    int i, max=0;

    for (i = 0; i < MAXJOBS; i++)
	if (jobs[i].jid > max)
	    max = jobs[i].jid;
    return max;
}

/* addjob - Add a job to the job list */
int addjob(struct job_t *jobs, pid_t pid, int state, char *cmdline) 
{
    int i;
    
    if (pid < 1)
	return 0;

    for (i = 0; i < MAXJOBS; i++) {
	if (jobs[i].pid == 0) {
	    jobs[i].pid = pid;
	    jobs[i].state = state;
	    jobs[i].jid = nextjid++;
	    if (nextjid > MAXJOBS)
		nextjid = 1;
	    strcpy(jobs[i].cmdline, cmdline);
        //verbose 若命令行可选参数中有-v，则打出加入job的信息，没有就不打。
  	    if(verbose){
	        printf("Added job [%d] %d %s\n", jobs[i].jid, jobs[i].pid, jobs[i].cmdline);
            }
            return 1;
	}
    }
    printf("Tried to create too many jobs\n");
    return 0;
}

/* deletejob - Delete a job whose PID=pid from the job list */
int deletejob(struct job_t *jobs, pid_t pid) 
{
    int i;

    if (pid < 1)
	return 0;

    for (i = 0; i < MAXJOBS; i++) {
	if (jobs[i].pid == pid) {
	    clearjob(&jobs[i]);
	    nextjid = maxjid(jobs)+1;
	    return 1;
	}
    }
    return 0;
}

/* fgpid - Return PID of current foreground job, 0 if no such job */
pid_t fgpid(struct job_t *jobs) {
    int i;

    for (i = 0; i < MAXJOBS; i++)
	if (jobs[i].state == FG)
	    return jobs[i].pid;
    return 0;
}

/* getjobpid  - Find a job (by PID) on the job list */
struct job_t *getjobpid(struct job_t *jobs, pid_t pid) {
    int i;

    if (pid < 1)
	return NULL;
    for (i = 0; i < MAXJOBS; i++)
	if (jobs[i].pid == pid)
	    return &jobs[i];
    return NULL;
}

/* getjobjid  - Find a job (by JID) on the job list */
struct job_t *getjobjid(struct job_t *jobs, int jid) 
{
    int i;

    if (jid < 1)
	return NULL;
    for (i = 0; i < MAXJOBS; i++)
	if (jobs[i].jid == jid)
	    return &jobs[i];
    return NULL;
}

/* pid2jid - Map process ID to job ID */
int pid2jid(pid_t pid) 
{
    int i;

    if (pid < 1)
	return 0;
    for (i = 0; i < MAXJOBS; i++)
	if (jobs[i].pid == pid) {
            return jobs[i].jid;
        }
    return 0;
}

/* listjobs - Print the job list */
void listjobs(struct job_t *jobs) 
{
    int i;
    
    for (i = 0; i < MAXJOBS; i++) {
	if (jobs[i].pid != 0) {
	    printf("[%d] (%d) ", jobs[i].jid, jobs[i].pid);
	    switch (jobs[i].state) {
		case BG: 
		    printf("Running ");
		    break;
		case FG: 
		    printf("Foreground ");
		    break;
		case ST: 
		    printf("Stopped ");
		    break;
	    default:
		    printf("listjobs: Internal error: job[%d].state=%d ", 
			   i, jobs[i].state);
	    }
	    printf("%s", jobs[i].cmdline);
	}
    }
}
/******************************
 * end job list helper routines
 ******************************/


/***********************
 * Other helper routines
 ***********************/

/*
 * usage - print a help message
 */
void usage(void) 
{
    printf("Usage: shell [-hvp]\n");
    printf("   -h   print this message\n");
    printf("   -v   print additional diagnostic information\n");
    printf("   -p   do not emit a command prompt\n");
    exit(1);
}

/*sigint_handler
 * unix_error - unix-style error routine
 */
void unix_error(char *msg)
{
    fprintf(stdout, "%s: %s\n", msg, strerror(errno));
    exit(1);
}

/*
 * app_error - application-style error routine
 */
void app_error(char *msg)
{
    fprintf(stdout, "%s\n", msg);
    exit(1);
}

/*
 * Signal - wrapper for the sigaction function
 */
//CSAPP书中P516页提到的一个对sigaction函数的包装函数
//除了给信号定义相应的信号处理函数，还可以恢复被中断的慢速系统调用如read等
handler_t *Signal(int signum, handler_t *handler) 
{
    struct sigaction action, old_action;

    action.sa_handler = handler;  
    sigemptyset(&action.sa_mask); /* block sigs of type being handled */
    action.sa_flags = SA_RESTART; /* restart syscalls if possible */

    if (sigaction(signum, &action, &old_action) < 0)
	unix_error("Signal error");
    return (old_action.sa_handler);
}

/*
 * sigquit_handler - The driver program can gracefully terminate the
 *    child shell by sending it a SIGQUIT signal.
 */
void sigquit_handler(int sig) 
{
    printf("Terminating after receipt of SIGQUIT signal\n");
    exit(1);
}


sigset_t mask_signal (int how,int sig){
    sigset_t signals,result;//定义信号集合
    if(sigemptyset(&signals)<0)
        unix_error("初始化信号集合为空失败");
    if(sigaddset(&signals,sig)<0)
        unix_error("添加相应的信号失败");
    //how可以是SIG_BLOCK和SIG_UNBLOCK
    //分别让sigprocmask函数阻塞和解除阻塞相应信号sig
    if(sigprocmask(how,&signals,&result)<0)
        unix_error("改变当前阻塞信号集合失败");
    return result;
}
//kill 函数的pid，若是小于0，则发给abs(pid)所在进程组的每个进程
int send_signal(pid_t pid, int sig){
    if(kill(pid,sig)<0){
        if(errno!=ESRCH)
            unix_error("杀死信号失败");
    }
    return 0;
}
