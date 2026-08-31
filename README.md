## Task

1. Creating the child process and executing the command in the child 
2. Providing a history feature 
3. Adding support of input and output redirection 
4. Allowing the parent and child processes to communicate via a pipe

## Executing Command in a Child Process

```shell
args[0] = "ps" 

args[1] = "-ael" 

args[2] = NULL
```



```shell
execvp(char *command, char *params[])
```



## Creating a History Feature

provides a history feature 

execute the most recent command by entering !!



## Redirecting Input and Output

```shell
ls > out.txt
sort < in.txt
```



```shell
dup2(fd, STDOUT FILENO)
```



## Communication via a Pipe

```shell
ls -l | less
```

Notice: since this project doesn't consider collect child process, better use below command to conduct this experiment.

```shell
apt list | grep openjdk

ps | grep your-shell-name

ls -l | grep txt
```

