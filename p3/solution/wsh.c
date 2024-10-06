#define _GNU_SOURCE // for getline()
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>   // for readdir()
#include <unistd.h>   // for system calls
#include <sys/wait.h> // for wait()
#include <fcntl.h>    // for redirection
#include "wsh.h"

// Piazza @226
// constraints on the input like command length and number of arguments
#define MAXLINE 1024
#define MAXARGS 128

int return_code = 0;

// Linked List for shell variables
struct SNode
{
    char *key;
    char *value;
    struct SNode *next;
};
struct SNode *Sfirst = NULL;
int shellvars_len = 0;

// Linked List for history
struct HNode
{
    char *command;
    struct HNode *next;
};
struct HNode *Hfirst = NULL;

// ##################################### History ##############################################

void record_input(char *dest, char *src)
{
    int i = 0;
    for (; src[i] != '\0'; i++)
    {
        dest[i] = src[i];
    }
    dest[i] = '\0';
}

int Hcnt = 0; // variable for history length
int Hsize = 5;

// prune history is called twice
// flag=0 => from function change_history_size()
// flag=1 => from function record_history()

void prune_history(int history_size, int history_cnt, int func_flag)
{
    // Corner case : history size = 1, history count = 1, prune_history() called from record_history()
    if (history_size == 1 && history_cnt == 1 && func_flag == 1)
    {
        free(Hfirst);
        Hfirst = NULL;
        Hcnt = 0;
        return;
    }

    // if history has 5 elements, make 3 jumps to reach 4th element
    int jumps = 0;

    // prune called from "history set <n>" command
    if (func_flag == 0)
    {
        jumps = history_size - 1;
    }
    // history size full
    else
    {
        jumps = history_size - 2;
    }

    // remove the extra elements
    if (history_cnt >= history_size)
    {
        // mechanism to jump nodes
        struct HNode *ptr = Hfirst;
        while (Hfirst != NULL && jumps > 0)
        {
            ptr = ptr->next;
            jumps--;
        }
        // temp node to point on the last node
        struct HNode *temp = ptr->next;
        struct HNode *prev = NULL;
        ptr->next = NULL;
        // space created to add new node
        while (temp != NULL)
        {
            prev = temp;
            temp = temp->next;
            free(prev);
        }
        // modifiy Hcnt
        if (func_flag == 0)
        {
            Hcnt = history_size;
            // printf("history cnt is now %d\n", Hcnt);
        }
        else
        {
            Hcnt = history_size - 1;
        }
    }
}

void change_history_size(char *n)
{
    int newsize = atoi(n);
    Hsize = newsize;
    prune_history(newsize, Hcnt, 0);
    // printf("History size changed to %d\n", Hsize);
    return_code = 0;
}

void record_history(char *arg, char *firstarg)
{
    // debug
    // printf("In record_history str is: %s\n", arg);
    if (!strcmp(firstarg, "history") ||
        !strcmp(firstarg, "exit") ||
        !strcmp(firstarg, "cd") ||
        !strcmp(firstarg, "export") ||
        !strcmp(firstarg, "local") ||
        !strcmp(firstarg, "vars") ||
        !strcmp(firstarg, "ls") ||
        ((Hfirst != NULL) && !strcmp(Hfirst->command, arg)))
    {
        return;
    }

    // remove the extra elements

    // printf("before prune history\n");
    prune_history(Hsize, Hcnt, 1);
    // printf("after prune history\n");

    // create a new node
    struct HNode *temp = (struct HNode *)malloc(sizeof(struct HNode));
    temp->command = strdup(arg);
    // temp->command = arg;
    temp->next = NULL;

    // case : History Empty
    if (Hfirst == NULL)
    {
        Hfirst = temp;
    }
    else // case History Not Empty
    {
        temp->next = Hfirst;
        Hfirst = temp;
    }
    Hcnt++;
    // printf("History has %d commands\n", Hcnt);
}

char *history_replace(char *str2, char **input_tokens, int arg_cnt, int *flag)
{
    // step-1 parse the input_ptr
    // char *input_tokens[MAXARGS] = {NULL};
    // int arg_cnt = arg_parse(input_copy, input_tokens, " ");

    // compare if 1st term = "history"
    if (arg_cnt > 1 && strcmp(input_tokens[0], "history") == 0)
    {
        // check if 2nd token is a valid number command_no
        int command_no = atoi(input_tokens[1]);
        if (command_no > 0 && command_no <= Hcnt)
        {
            // jump to the command_noth instruction
            int jumps = command_no - 1;
            struct HNode *ptr = Hfirst;
            while (Hfirst != NULL && jumps > 0 && ptr != NULL)
            {
                ptr = ptr->next;
                jumps--;
            }
            char *str = strdup(ptr->command);
            *flag = 1;
            return str;
        }
        else
        {
            // if command_no is not valid
            return str2;
        }
    }

    else
    {
        // if command doesn't start with "history"
        return str2;
    }
}

// ######################################## Parsers ##############################################

int arg_parse(char *str, char **arg_arr, char *delims)
{
    // using strtok()
    // Returns first token
    char *token = strtok(str, delims);

    // count the number of arguments
    int arg_cnt = 0;

    // Keep printing tokens while one of the
    // delimiters present in str[].
    while (token != NULL)
    {
        // printf(" % s\n", token);
        arg_cnt++;
        arg_arr[arg_cnt - 1] = token;
        token = strtok(NULL, delims);
    }

    // debug statement
    // printf("tokenization done %d\n", arg_cnt);
    return arg_cnt;
}

// ################################### Variable Substitution ##################################
char *variable_sub(int pos, char **arg_arr, int arg_cnt, char *str)
{
    arg_arr[pos]++;
    // printf("%s\n", arg_arr[pos]);
    struct SNode *ptr = Sfirst;

    // search in environment variables
    char *env_var = NULL;
    env_var = getenv(arg_arr[pos]);
    if (env_var != NULL)
    {
        arg_arr[pos] = strdup(env_var);
    }

    // search in shell variables
    else
    {
        while (ptr != NULL)
        {
            if (strcmp(ptr->key, arg_arr[pos]) == 0)
            {
                arg_arr[pos] = strdup(ptr->value);
                break;
            }
            ptr = ptr->next;
        }
    }
    // if word not found then ptr would reach the end of shell variables linked list
    if (ptr != NULL || env_var != NULL)
    {
        char *modified = (char *)malloc(MAXLINE * sizeof(char *));
        strcpy(modified, arg_arr[0]);
        for (int i = 1; i < arg_cnt; i++)
        {
            strcat(modified, " ");
            strcat(modified, arg_arr[i]);
        }
        // printf("New command becomes %s\n", modified);
        return modified;
    }
    else
    {
        return str;
    }
}

// ################### handle redirection ######################
int redirection(char **arg_arr, int arg_cnt)
{
    // parameters : space seperated array of tokens, number of tokens
    // return : 1=redirection successful
    //          0=redirection unsuccessful

    // case : redirection token is always the last one on the command line
    char *temp = strdup(arg_arr[arg_cnt - 1]);

    // position of the 1st redirection symbol
    int pos = 0;
    for (int i = 0; temp[i] != '\0'; i++)
    {
        if (temp[i] == '>' || temp[i] == '<' || temp[i] == '&')
        {
            pos = i;
            break;
        }
    }

    int n = -1;
    char num_strg[1024];
    // copy the number into num_strg
    // get its integer format
    if (pos != 0)
    {
        for (int i = 0; i < pos; i++)
        {
            num_strg[i] = temp[i];
        }
        num_strg[pos] = '\0';
        n = atoi(num_strg);
        
    }

    // file descriptor of new file
    int file_desc = -1;
    // file descriptor of old file
    int fd = -1;

    char *temp2 = NULL;

    // case : redirect stdout
    if (temp[pos] == '<')
    {
        temp2 = temp + pos + 1;
        file_desc = open(temp2, O_CREAT | O_RDONLY, 0777);
        if (pos == 0)
        {
            fd = STDIN_FILENO;
        }
        else if (pos != 0)
        {
            fd = n;
        }
        dup2(file_desc, fd);

        // set the file name to NULL
        arg_arr[arg_cnt - 1] = NULL;
        close(file_desc);
        return 1;
    }

    else if (temp[pos] == '&')
    {
        if (temp[pos + 1] == '>' && temp[pos + 2] == '>')
        {
            temp2 = temp + 3;
            file_desc = open(temp2, O_CREAT | O_APPEND | O_WRONLY, 0777);
        }
        else if (temp[pos + 1] == '>')
        {
            temp2 = temp + 2;
            file_desc = open(temp2, O_CREAT | O_TRUNC | O_WRONLY, 0777);
        }
        dup2(file_desc, STDOUT_FILENO);
        dup2(file_desc, STDERR_FILENO);

        // set the file name to NULL
        arg_arr[arg_cnt - 1] = NULL;
        close(file_desc);
        return 1;
    }

    else if (temp[pos] == '>')
    {
        if (pos == 0)
        {
            if (temp[pos] == '>' && temp[pos + 1] == '>')
            {
                temp2 = temp + 2;
                file_desc = open(temp2, O_CREAT | O_APPEND | O_WRONLY, 0777);
            }
            else if (temp[pos] == '>')
            {
                temp2 = temp + 1;
                file_desc = open(temp2, O_CREAT | O_WRONLY | O_TRUNC, 0777);
            }
            fd = STDOUT_FILENO;
        }
        else if (pos != 0)
        {
            if (temp[pos] == '>' && temp[pos + 1] == '>')
            {
                temp2 = temp + pos + 2;
                file_desc = open(temp2, O_CREAT | O_APPEND | O_WRONLY, 0777);
            }
            else if (temp[pos] == '>')
            {
                temp2 = temp + pos + 1;
                file_desc = open(temp2, O_CREAT | O_WRONLY | O_TRUNC, 0777);
            }
            fd = n;
        }
        dup2(file_desc, fd);

        // set the file name to NULL
        arg_arr[arg_cnt - 1] = NULL;
        close(file_desc);
        return 1;
    }

    // else if (temp[1] == '<')
    // {
    //     char *temp2 = temp + 2;
    //     int fd = temp[0] - '0';
    //     file_desc = open(temp2, O_CREAT | O_RDONLY, 0777);
    //     dup2(file_desc, fd);

    //     // set the file name to NULL
    //     arg_arr[arg_cnt - 1] = NULL;
    //     close(file_desc);
    //     return 1;
    // }

    // else if (temp[0] == '>' && temp[1] == '>')
    // {
    //     // lose the >> operator
    //     char *temp2 = temp + 2;

    //     file_desc = open(temp2, O_CREAT | O_APPEND | O_WRONLY, 0777);
    //     dup2(file_desc, STDOUT_FILENO);

    //     // set the file name to NULL
    //     arg_arr[arg_cnt - 1] = NULL;
    //     close(file_desc);
    //     return 1;
    // }

    // else if (temp[0] == '>')
    // {
    //     // lose the > operator
    //     char *temp2 = temp + 1;
    //     // 0777 grants read, write, and execute permissions to everyone
    //     file_desc = open(temp2, O_CREAT | O_WRONLY | O_TRUNC, 0777);
    //     dup2(file_desc, STDOUT_FILENO);

    //     // set the file name to NULL
    //     arg_arr[arg_cnt - 1] = NULL;
    //     close(file_desc);
    //     return 1;
    // }

    // else if (temp[0] == '&' && temp[1] == '>' && temp[2] == '>')
    // {
    //     // lose the &> operator
    //     char *temp2 = temp + 3;
    //     file_desc = open(temp2, O_CREAT | O_APPEND | O_WRONLY, 0777);
    //     dup2(file_desc, STDOUT_FILENO);
    //     dup2(file_desc, STDERR_FILENO);

    //     // set the file name to NULL
    //     arg_arr[arg_cnt - 1] = NULL;
    //     close(file_desc);
    //     return 1;
    // }

    // else if (temp[1] == '>' && temp[2] == '>')
    // {
    //     // lose the > operator
    //     char *temp2 = temp + 3;

    //     // subtracted the ASCII value of 0
    //     int fd = temp[0] - '0';
    //     // 0777 grants read, write, and execute permissions to everyone
    //     file_desc = open(temp2, O_CREAT | O_WRONLY | O_APPEND, 0777);
    //     dup2(file_desc, fd);

    //     // set the file name to NULL
    //     arg_arr[arg_cnt - 1] = NULL;
    //     close(file_desc);
    //     return 1;
    // }

    // // Redirecting Standard Output and Standard Error at once
    // else if (temp[0] == '&' && temp[1] == '>')
    // {
    //     // lose the &> operator
    //     char *temp2 = temp + 2;
    //     file_desc = open(temp2, O_CREAT | O_TRUNC | O_WRONLY, 0777);
    //     dup2(file_desc, STDOUT_FILENO);
    //     dup2(file_desc, STDERR_FILENO);

    //     // set the file name to NULL
    //     arg_arr[arg_cnt - 1] = NULL;
    //     close(file_desc);
    //     return 1;
    // }

    // else if (temp[1] == '>')
    // {
    //     // lose the > operator
    //     char *temp2 = temp + 2;

    //     // subtracted the ASCII value of 0
    //     int fd = temp[0] - '0';
    //     // 0777 grants read, write, and execute permissions to everyone
    //     file_desc = open(temp2, O_CREAT | O_WRONLY | O_TRUNC, 0777);
    //     dup2(file_desc, fd);

    //     // set the file name to NULL
    //     arg_arr[arg_cnt - 1] = NULL;
    //     close(file_desc);
    //     return 1;
    // }
    // // arg_arr[arg_cnt-1] = NULL;
    // // close(file_desc);
    else
    {
        return 0;
    }
}

// ######################################### Built-in #############################################

void builtin_ls()
{
    int no_of_files;
    // char** fileList;
    struct dirent **fileListTemp;
    char *path = ".";
    no_of_files = scandir(path, &fileListTemp, NULL, alphasort);
    // printf("no of files : %d\n", no_of_files);
    for (int i = 0; i < no_of_files; i++)
    {
        if (fileListTemp[i]->d_name[0] == '.')
            continue;
        printf("%s\n", fileListTemp[i]->d_name);
    }

    // free the memory
    for (int i = 0; i < no_of_files; i++)
    {
        free(fileListTemp[i]);
    }

    free(fileListTemp);
    return_code = 0;
}

void builtin_cd(char **arg_arr)
{
    int rc = chdir(arg_arr[1]);

    if (rc == 0)
    {
        // chdir successfull
        return_code = 0;
    }
    else
    {
        // chdir unsuccessful
        fprintf(stderr, "chdir error\n");
        return_code = -1;
    }
}

void builtin_local(char *arg, int *shellvars_len)
{
    // check if variable is already present in list
    struct SNode *ptr = Sfirst;

    // parse the argument to local
    char *temp[2];
    int cnt = arg_parse(arg, temp, "=");

    // handle empty shell variable assignments
    if (cnt != 2)
    {
        temp[1] = " ";
    }

    // check for variable substitution
    if (temp[1][0] == '$')
    {
        // tokenize for :
        // printf("$ encountered in local command\n");
        variable_sub(1, temp, cnt, temp[0]);
    }

    // if shell variable is already present
    while (ptr != NULL)
    {

        if (strcmp(ptr->key, temp[0]) == 0)
        {
            ptr->value = strdup(temp[1]);
            return;
        }
        ptr = ptr->next;
    }

    // return the iterator to head pointer
    ptr = Sfirst;
    struct SNode *t;
    t = (struct SNode *)malloc(sizeof(struct SNode));
    t->key = strdup(temp[0]);
    t->value = strdup(temp[1]);
    t->next = NULL;
    // printf("Node created\n");

    if (Sfirst == NULL)
    {
        // printf("inside if condition\n");
        Sfirst = t;
        ptr = t;
    }
    else
    {
        //     // struct shell_var *ptr = head;
        // printf("Inside else condition\n");
        while (ptr->next != NULL)
        {
            // printf("inside while loop\n");
            ptr = ptr->next;
        }
        ptr->next = t;
    }
    // debug statements : to be removed
    (*shellvars_len)++;
    // printf("Number of shell variables = %d\n", *shellvars_len);
    return_code = 0;
}

void builtin_vars()
{
    // printf("entered builtin_vars function\n");
    if (Sfirst == NULL)
    {
        printf("No shell variable\n");
    }
    struct SNode *ptr = Sfirst;
    while (ptr != NULL)
    {
        printf("%s", ptr->key);
        printf("=");
        printf("%s\n", ptr->value);
        ptr = ptr->next;
    }
    // printf("exiting builtin_vars function\n");
    return_code = 0;
}

void builtin_history()
{
    // prints the nodes in the LL & also increments counter
    struct HNode *hptr = Hfirst;
    int i = 1;
    while (hptr != NULL)
    {
        printf("%d) %s\n", i, hptr->command);
        i++;
        hptr = hptr->next;
    }
    return_code = 0;
}

void builtin_export(char **arg_arr)
{
    char *env_var[2];
    int cnt = arg_parse(arg_arr[1], env_var, "=");
    // handle empty shell variable assignments
    // if((cnt != 2) && (env_var[0][strlen(env_var[0])-1] != '='))
    // {
    //     // Doing just export VAR without definition is not allowed and should produce error.
    //     return_code = -1;
    //     return;
    // }
    // if ((cnt != 2) && (arg_arr[1][sizeof(env_var[1])-1] == '='))
    if (cnt != 2)
    {
        env_var[1] = " ";
    }
    // check for variable substitution
    if (env_var[1][0] == '$')
    {
        // tokenize for :
        // printf("$ encountered in local command\n");
        variable_sub(1, env_var, cnt, env_var[0]);
    }
    setenv(env_var[0], env_var[1], 1);
    return_code = 0;
}

// ################################### main solver function #######################################

void solve(char **arg_arr, int arg_cnt)
{

    // exit built-in command
    if ((strcmp(arg_arr[0], "exit") == 0) && (arg_cnt == 1))
    {
        // fflush(stdout);
        // printf("Called exit with code %d\n", return_code);
        exit(return_code);
    }

    // cd built-in
    else if (strcmp(arg_arr[0], "cd") == 0)
    {
        // check if it takes only one argument
        if (arg_cnt == 2)
        {
            builtin_cd(arg_arr);
        }
        // more than 1 arguments to ls
        else
        {
            return_code = -1;
        }
    }

    // export built-in command
    else if (strcmp(arg_arr[0], "export") == 0)
    {
        // Having a dollar sign on the left side is an error, e.g. local $a=b
        if (arg_arr[1][0] == '$')
        {
            return_code = -1;
            return;
        }
        builtin_export(arg_arr);
    }

    // local built-in
    else if (strcmp(arg_arr[0], "local") == 0)
    {
        // Having a dollar sign on the left side is an error, e.g. local $a=b
        if (arg_arr[1][0] == '$')
        {
            return_code = -1;
            return;
        }

        // compare that the variable is not already present
        // if yes : update variable
        // if no : create new variable & store new value
        builtin_local(arg_arr[1], &shellvars_len);

        // NOTE: run this command "local varname"
    }

    // vars built-n
    else if (strcmp(arg_arr[0], "vars") == 0)
    {
        // printf("Entered vars condition\n");
        if (arg_cnt == 2)
        {
            int saved_stdin = dup(0);
            int saved_stdout = dup(1);
            int saved_stderr = dup(2);
            if (redirection(arg_arr, arg_cnt))
            {
                builtin_vars();
                dup2(saved_stdin, 0);
                dup2(saved_stdout, 1);
                dup2(saved_stderr, 2);
            }
            close(saved_stderr);
            close(saved_stdout);
            close(saved_stderr);
        }
        else if (arg_cnt == 1)
        {
            builtin_vars();
            return_code = 0;
        }
    }

    // history built-n
    else if (strcmp(arg_arr[0], "history") == 0)
    {
        // printf("Entered history condition\n");
        if (arg_cnt > 2 && !strcmp(arg_arr[1], "set"))
        {
            change_history_size(arg_arr[2]);
        }
        else if (arg_cnt == 2)
        {
            int saved_stdin = dup(0);
            int saved_stdout = dup(1);
            int saved_stderr = dup(2);
            if (redirection(arg_arr, arg_cnt))
            {
                builtin_history();
                dup2(saved_stdin, 0);
                dup2(saved_stdout, 1);
                dup2(saved_stderr, 2);
            }
            close(saved_stderr);
            close(saved_stdout);
            close(saved_stderr);
        }
        else if (arg_cnt == 1)
        {
            builtin_history();
        }
    }

    // ls built-in command
    else if ((strcmp(arg_arr[0], "ls") == 0) && (arg_cnt <= 2))
    {
        if (arg_cnt == 2)
        {
            int saved_stdin = dup(0);
            int saved_stdout = dup(1);
            int saved_stderr = dup(2);
            if (redirection(arg_arr, arg_cnt))
            {
                builtin_ls();
                dup2(saved_stdin, 0);
                dup2(saved_stdout, 1);
                dup2(saved_stderr, 2);
            }
            close(saved_stderr);
            close(saved_stdout);
            close(saved_stderr);
        }

        else if (arg_cnt == 1)
        {
            builtin_ls();
        }
    }

    // ################################### Path based #################################

    // absolute + relative path
    else if (access(arg_arr[0], X_OK) != -1)
    {
        // printf("Found relative/absolute path\n");
        int rc = fork();
        if (rc < 0)
        {
            // fork failed; exit
            fprintf(stderr, "fork failed\n");
            exit(1);
        }
        else if (rc == 0)
        {
            // child (new process)
            // printf("hello, I am child (pid:%d)\n", (int)getpid());
            char *myargs[arg_cnt + 1];

            for (int i = 0; i < arg_cnt; i++)
            {
                myargs[i] = arg_arr[i];
            }
            myargs[arg_cnt] = NULL; // marks end of array
            redirection(myargs, arg_cnt);
            execv(myargs[0], myargs); // runs word count
            printf("this shouldn't print out\n");
            // kill the child if the execv failed
            exit(-1);
        }
        else
        {
            // parent goes down this path (original process)
            // int wc =
            wait(&return_code);
            if (return_code > 0)
                return_code = -1;
            // printf("hello, I am parent of %d (wc:%d) (pid:%d)\n", rc, wc, (int)getpid());
        }
    }

    // $PATH
    else // if(strcmp(arg_arr[0], "ps") == 0)
    {
        int rc = fork();
        if (rc < 0)
        {
            // fork failed; exit
            fprintf(stderr, "fork failed\n");
            exit(1);
        }
        else if (rc == 0)
        {
            // child (new process)
            // printf("hello, I am child (pid:%d)\n", (int)getpid());
            char *myargs[arg_cnt + 1];
            for (int i = 0; i < arg_cnt; i++)
            {
                myargs[i] = arg_arr[i];
            }
            myargs[arg_cnt] = NULL; // marks end of array

            redirection(myargs, arg_cnt);
            // pointer to path string
            char *path;
            // path variable has the entire path string now.
            path = getenv("PATH");
            // printf("%s\n", path);
            char *path_arg[100];

            // contains the number of added paths
            int path_cnt = arg_parse(path, path_arg, ":");
            // printf("%d\n", path_cnt);
            // loop over each added path
            for (int i = 0; i < path_cnt; i++)
            {
                char temp[1000];
                strcpy(temp, path_arg[i]);
                // printf("%s\n", temp);
                strcat(temp, "/");
                // printf("%s\n", temp);
                strcat(temp, myargs[0]);
                // printf("%s\n", temp);
                if (access(temp, X_OK) != -1)
                {
                    int ret = execv(temp, myargs); // runs word count
                    printf("%d\n", ret);
                }
            }
            // strcat(path, myargs[0]);
            // printf("%s\n", path);

            // check errno & perror
            // printf("%s: command not found\n", myargs[0]);
            // in case failed exec, the child needs to be killed
            exit(-1);
        }
        else
        {
            // fflush : glibc buffer
            // parent goes down this path (original process)
            // int wc = wait(NULL);
            // printf("before wait writes to return_code %d\n", return_code);
            wait(&return_code);
            // printf("After wait writes, the return_code is %d\n", return_code);
            //  printf("hello, I am parent of %d (wc:%d) (pid:%d)\n", rc, wc, (int)getpid());
            if (return_code > 0)
                return_code = -1;
        }
    }
}

void free_memory()
{
    // free history linked list
    struct HNode *hprev = NULL;
    while (Hfirst != NULL)
    {
        hprev = Hfirst;
        Hfirst = Hfirst->next;
        free(hprev);
    }

    // free shell variables linked list
    struct SNode *sprev = NULL;
    while (Sfirst != NULL)
    {
        sprev = Sfirst;
        Sfirst = Sfirst->next;
        free(sprev);
    }
    // printf("freed memory\n");
}

// // Piazza @226
// // constraints on the input like command length and number of arguments
// #define MAXLINE 1024
// #define MAXARGS 128

// #####################################################################################
// #####################################################################################
// #####################################################################################
// #####################################################################################

int main(int argc, char *argv[])
{
    // entered process wsh
    // setting environment variable of process-wsh to
    // PATH=/bin as mentioned in write-up
    setenv("PATH", "/bin", 1);

    // declarations for the getline() function
    char *input = NULL;
    size_t size = 0;
    ssize_t len = 0;

    // select between batch mode & interactive mode
    if (argc == 2)
    {
        // open argv[1] as a file
        FILE *file_ptr = NULL;
        file_ptr = fopen(argv[1], "r");
        // printf("Entered Batch mode\n");
        // while ((len = getline(&input, &size, file_ptr)) != -1)
        while (1)
        {

            // ######################## Take user input ###########################
            // printf("wsh> ");
            // fflush(stdout);
            if ((len = getline(&input, &size, file_ptr)) == -1)
                break;
            // printf("\n");
            // remove new-line character that getline() reads by default
            if (input[strlen(input) - 1] == '\n')
            {
                input[strlen(input) - 1] = '\0';
            }
            // printf("line read :%s\n", input);

            // ################## parse the input for spaces #######################
            // make a deep copy of the user input
            char *str1 = strdup(input);
            char *arg_arr1[MAXARGS] = {NULL};
            int arg_cnt = arg_parse(str1, arg_arr1, " ");

            // ######################## Handle spaces/newlines in files ######################
            if (arg_cnt == 0)
                continue;

            // ####################### Handle comments ###############################
            // char *comment_token = arg_arr1[0];
            if (arg_arr1[0][0] == '#')
            {
                // printf("Treated as batch comment\n");
                continue;
            }

            // ################### handle variable substitution ######################
            char *str5 = strdup(input);
            char *sub_input = NULL;
            for (int i = 0; i < arg_cnt; i++)
            {
                char *temp_token = arg_arr1[i];
                if (temp_token[0] == '$')
                {
                    // printf("$ encountered\n");
                    sub_input = variable_sub(i, arg_arr1, arg_cnt, str5);
                }
            }
            if (sub_input == NULL)
            {
                sub_input = input;
            }

            // ################### handle history ###################
            // create a deep copy of input
            char *str2 = strdup(sub_input);
            int used_history = 0;
            char *actual_input = history_replace(str2, arg_arr1, arg_cnt, &used_history);

            // only if needed record history
            char *str3 = strdup(sub_input);
            char input_copy[MAXLINE];
            record_input(input_copy, str3);
            if (used_history == 0)
            {
                record_history(input_copy, arg_arr1[0]);
            }

            // execute the needed command
            // create a deep copy of input
            char *str4 = strdup(actual_input);
            char *arg_arr2[MAXARGS] = {NULL};
            arg_cnt = arg_parse(str4, arg_arr2, " ");
            solve(arg_arr2, arg_cnt);
        }
        fclose(file_ptr);
    }
    else if (argc == 1)
    {
        // printf("Interactive Mode On\n");
        while (1)
        {
            // ######################## Take user input ###########################
            printf("wsh> ");
            fflush(stdout);
            if ((len = getline(&input, &size, stdin)) == -1)
                break;
            // printf("\n");
            // remove new-line character that getline() reads by default
            if (input[strlen(input) - 1] == '\n')
            {
                input[strlen(input) - 1] = '\0';
            }

            // ################## parse the input for spaces #######################
            // make a deep copy of the user input
            char *str1 = strdup(input);
            char *arg_arr1[MAXARGS] = {NULL};
            int arg_cnt = arg_parse(str1, arg_arr1, " ");

            // ######################## Handle spaces/newlines in files ######################
            if (arg_cnt == 0)
                continue;

            // ####################### Handle comments ###############################
            // char *comment_token = arg_arr1[0];
            if (arg_arr1[0][0] == '#')
            {
                // printf("Treated as interactive comment\n");
                continue;
            }

            // ################### handle variable substitution ######################
            char *str5 = strdup(input);
            char *sub_input = NULL;
            for (int i = 0; i < arg_cnt; i++)
            {
                char *temp_token = arg_arr1[i];
                if (temp_token[0] == '$')
                {
                    // printf("$ encountered\n");
                    sub_input = variable_sub(i, arg_arr1, arg_cnt, str5);
                }
            }
            // if no $ encountered
            if (sub_input == NULL)
            {
                sub_input = input;
            }

            // ################### handle history ###################
            // create a deep copy of input
            char *str2 = strdup(sub_input);
            int used_history = 0;
            char *actual_input = history_replace(str2, arg_arr1, arg_cnt, &used_history);

            // only if needed record history
            char *str3 = strdup(sub_input);
            char input_copy[MAXLINE];
            record_input(input_copy, str3);
            if (used_history == 0)
            {
                record_history(input_copy, arg_arr1[0]);
            }

            // execute the needed command
            // create a deep copy of input
            char *str4 = strdup(actual_input);
            char *arg_arr2[MAXARGS] = {NULL};
            arg_cnt = arg_parse(str4, arg_arr2, " ");
            solve(arg_arr2, arg_cnt);

            free(str1);
            str1 = NULL;
            free(str2);
            str2 = NULL;
            free(str3);
            str3 = NULL;
            free(str4);
            str4 = NULL;
            free(str5);
            str5 = NULL;

            // for (int i = 0; arg_arr2[i] != NULL; i++)
            // {
            //     free(arg_arr2[i]);
            // }

            /// free(arg_arr2);

        } // while(1); //((len != -1));
    }

    else
    {
        // The shell can be invoked with either no arguments or a
        // single argument; anything else is an error.
        return -1;
    }

    free(input);
    input = NULL;
    free_memory();
    // printf("Before returning main %d\n", return_code);
    return return_code;
}