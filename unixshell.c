#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <sys/types.h>
#include <sys/wait.h>

#include <fcntl.h>

#define MAX_LINE 80 /* The maximum length command */
#define MAX_HISTORY 100
#define READ_END 0
#define WRITE_END 1

int main (void)
{
	/* flag to determine when to exit program */
	int should_run = 1;


	char history[MAX_HISTORY][MAX_LINE+1];
	int history_count = 0;

	while(should_run)
	{
		/* command line arguments */
		char *args[MAX_LINE / 2 + 1] = {NULL};
		int argc = 0;
		/* user input */
		char input[MAX_LINE + 1];


		printf("dawn_shell > ");
		printf("\033[?25h");
		fflush(stdout);

		/* user input  */
		fgets(input, MAX_LINE, stdin);

		input[strcspn(input, "\n")] = '\0';


		pid_t done_pid;
		int status;

		/* collect all done child processes  */
		while ((done_pid = waitpid(-1, &status, WNOHANG)) > 0)
		{
			printf("[%d] Done \n", done_pid);
		}

		if (input[0] == '\0')
		{
			continue;
		}

		/* !! execute the most recentcommand  */
		if (strcmp(input, "!!") == 0)
		{
			if (history_count == 0)
			{
				printf("No commands in history.\n");
				continue;
			}
			strcpy(input, history[history_count - 1]);
			printf("%s\n", input);
		}

		/* record command history  */
		if (history_count < MAX_HISTORY)
		{
			strcpy(history[history_count], input);
			history_count++;
		}

		/* divide input into several tokens by " "(space)  */
		char *token = strtok(input, " ");

		int input_redirect = 0;
		int output_redirect = 0;

		int ispipe = 0;

		char *sub_args[MAX_LINE / 2 - 1] = {NULL};
		int sub_argc = 0;

		char *input_file = NULL;
		char *output_file = NULL;

		while (token != NULL && argc < MAX_LINE / 2)
		{
			if (strcmp(token, ">") == 0)
			{
				output_redirect = 1;
				output_file = strtok(NULL, " ");
				break;
			}

			else if (strcmp(token, "<") == 0)
			{
				input_redirect = 1;
				input_file = strtok(NULL, " ");
				break;
			}

			else if (strcmp(token, "|") == 0)
			{
				ispipe = 1;
				token = strtok(NULL, " ");
				while (token != NULL && sub_argc < MAX_LINE / 2 - 1)
				{
					sub_args[sub_argc] = token;
					sub_argc++;
					token = strtok(NULL, " ");
				}
				sub_args[sub_argc] = NULL;
				break;
			}


			args[argc] = token;
			argc++;
			token = strtok(NULL, " ");
		}
		args[argc] = NULL;

		if (strcmp(args[0], "exit") == 0)
		{
			break;
		}

		if (strcmp(args[0], "history") == 0)
		{
			for (int i = 0; i < history_count; i++)
			{
				printf("%d\t%s\n", i + 1, history[i]);
			}
			continue;
		}


		int isconcurrent = 0;

        if (argc > 0 && strcmp(args[argc - 1], "&") == 0)
        {
			isconcurrent = 1;
            args[argc - 1] = NULL;
            argc--;
        }

		/* create child process */
		pid_t pid;
		pid = fork();

		if (pid < 0) /* error occur */
		{
			fprintf(stderr, "Fork failed");
			return 1;
		}
		else if (pid > 0)  /* parent process */
		{
			if (isconcurrent)
			{
				printf("[%d]\n", pid);
			}
			else
			{
				wait(NULL);
			}
		}
		else /* child process */
		{
			/* output redirect exec */
			if (output_redirect)
			{
				int fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
				if (fd < 0)
				{
					fprintf(stderr, "output redirect occur errors.\n");
					return 1;
				}

				dup2(fd, STDOUT_FILENO);
				close(fd);

				/*  default exec */
				execvp(args[0], args);

				fprintf(stderr, "exec failed\n");
			}


            /* input redirect exec */
            if (input_redirect)
            {
                int fd = open(input_file, O_RDONLY);
                if (fd < 0)
                {
                        fprintf(stderr, "input redirect occur errors.\n");
                        return 1;
                }

                dup2(fd, STDIN_FILENO);
                close(fd);
            	/*  default exec */
            	execvp(args[0], args);

            	fprintf(stderr, "exec failed\n");
            }

			/* pipe exec */
			if (ispipe)
			{
				pid_t sub_pid;
				int fd[2];

				/* create the pipe */
				if (pipe(fd) == -1) {
					fprintf(stderr,"Pipe failed");
					return 1;
				}

				sub_pid = fork();
				if (sub_pid < 0)
				{ /* error occour */
					fprintf(stderr, "Fork failed");
					return 1;
				}
				else if (sub_pid > 0)
				{ /* parent process */

					/* close the unused end of the pipe */
					close(fd[READ_END]);

					/* duplicate the standard output to pipe */
					dup2(fd[WRITE_END], STDOUT_FILENO);

					/* close the write end of the pipe */
					close(fd[WRITE_END]);

					execvp(args[0], args);

					fprintf(stderr, "exec failed\n");
					return 1;

				}
				else
				{ /* child process */
					/* close the unused end of the pipe */
					close(fd[WRITE_END]);

					/* read from the pipe */
					dup2(fd[READ_END], STDIN_FILENO);

					/* close the write end of the pipe */
					close(fd[READ_END]);

					execvp(sub_args[0], sub_args);
					fprintf(stderr, "exec failed\n");
					return 1;
				}
			}
			else
			{
				/*  default exec */
				execvp(args[0], args);

				fprintf(stderr, "exec failed\n");
			}


			return 1;
		}

	}
	return 0;
}
