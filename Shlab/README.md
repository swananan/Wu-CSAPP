
###具体支持的功能
>  内置命令 

 *  quit 退出 shell
* jobs 列出所有的后台任务
* bg job 给后台 job 发送 SIGCONT 信号来继续执行该任务，具体的 job 数值可以是 PID 或 JID
* fg job 给前台 job 发送 SIGCONT 信号来继续执行该任务，具体的 job 数值可以是 PID 或 JID

> 外部命令，即输入程序路径与参数，进行执行

> 键盘输入 ctrl-c 或 ctrl-z 会令当前的前台进程（包括其子进程）终止或挂起
> 
> 重定向 (待做)
> 
> 管道(待做)