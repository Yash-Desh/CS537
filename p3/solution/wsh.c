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

// #################################  Free Heap Memory  ######################################
void free_memory()
{
    // free history linked list
    struct HNode *hprev = NULL;
    while (Hfirst != NULL)
    {
        hprev = Hfirst;
        Hfirst = Hfirst->next;
        free(hprev->command);
        free(hprev);
    }

    // free shell variables linked list
    struct SNode *sprev = NULL;
    while (Sfirst != NULL)
    {
        sprev = Sfirst;
        Sfirst = Sfirst->next;
        free(sprev->key);
        free(sprev->value);
        free(sprev);
    }
    // printf("freed memory\n");
}

void free_input_tokens(char **arg_arr, int arg_cnt)
{
    // free input tokens
    for (int i = 0; i < arg_cnt; i++)
    {
        free(arg_arr[i]);
        arg_arr[i] = NULL;
    }
}

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
    // compare if 1st term = "history"
    // checking for format "history [n]"
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
            //free(str2);
            //printf("history replace\n");
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

// ################################## alphasort ####################################

int mysort(const struct dirent **a, const struct dirent **b)
{
    return strcmp((*a)->d_name, (*b)->d_name);
}

// ######################################## Parsers ##############################################

int cnt_tokens(char *str)
{
    int cnt = -1;
    if (str[0] == ' ')
    {
        cnt = 0;
    }
    else
    {
        cnt = 1;
    }
    int character_flag = 0;
    for (int i = 0; str[i] != '\0'; i++)
    {
        if (str[i] != ' ' && str[i] != '\0' && str[i] != '\n')
        {
            character_flag = 1;
        }

        if (str[i] == ' ' && str[i + 1] != ' ' && str[i + 1] != '\0' && str[i + 1] != '\n')
        {
            cnt++;
        }
    }
    if (character_flag)
    {
        return cnt;
    }
    else
    {
        return 0;
    }
}

int arg_parse(char *str, char **arg_arr, char *delims)
{
    // using strtok()
    // Returns pointer to first token
    char *token = strtok(str, delims);

    // count the number of arguments
    int arg_cnt = 0;

    // Keep counting tokens while one of the
    // delimiters present in str[].
    while (token != NULL)
    {
        // printf(" % s\n", token);
        arg_cnt++;
        arg_arr[arg_cnt - 1] = token;
        token = strtok(NULL, delims);
    }

    // comment out
    str = NULL;
    // debug statement
    // printf("tokenization done %d\n", arg_cnt);
    return arg_cnt;
}

// ################################### Variable Substitution ##################################
char *variable_sub(int pos, char **arg_arr, int arg_cnt, char *str)
{
    // lose the dollar sign
    arg_arr[pos]++;
    // printf("%s\n", arg_arr[pos]);
    struct SNode *ptr = Sfirst;

    // search in environment variables
    char *env_var = NULL;
    env_var = getenv(arg_arr[pos]);
    if (env_var != NULL)
    {
        // free(arg_arr[pos]);
        arg_arr[pos] = strdup(env_var);
    }

    // search in shell variables
    else
    {
        while (ptr != NULL)
        {
            if (strcmp(ptr->key, arg_arr[pos]) == 0)
            {
                // free(arg_arr[pos]);
                arg_arr[pos] = strdup(ptr->value);
                break;
            }
            ptr = ptr->next;
        }
    }
    // if word not found then ptr would reach the end of shell variables linked list
    if (ptr != NULL || env_var != NULL)
    {
        char *modified = (char *)malloc(MAXLINE * sizeof(char));
        strcpy(modified, arg_arr[0]);
        for (int i = 1; i < arg_cnt; i++)
        {
            strcat(modified, " ");
            strcat(modified, arg_arr[i]);
        }
        // release the strdup of the found variable value
        free(arg_arr[pos]);
        return modified;
    }
    else
    {
        // $word - word not found in both shell & env variables list
        return str;
    }
}

// ################### handle redirection ######################
int redirection(char **arg_arr, int arg_cnt)
{
    // parameters : space seperated array of tokens, number of tokens
    // return : 1=redirection successful
    //          0=redirection unsuccessful, no redirection symbol
    //          2=redirection unsuccessful, [n] is not a number

    // case : redirection token is always the last one on the command line
    char *temp = strdup(arg_arr[arg_cnt - 1]);

    // position of the 1st redirection symbol
    int pos = -1;
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
    // check if [n] is an an integer number
    // copy the number into num_strg
    // get its integer format
    if (pos > 0)
    {
        for (int i = 0; i < pos; i++)
        {
            if (!(temp[i] >= '0' && temp[i] < '9'))
            {
                return_code = -1;
                return 2;
            }
            num_strg[i] = temp[i];
        }
        num_strg[pos] = '\0';
        n = atoi(num_strg);
    }

    // file descriptor of new file
    int file_desc = -1;
    // file descriptor of old file
    int fd = -1;

    // file name pointer
    char *temp2 = NULL;

    // case : redirect stdout
    if (pos != -1)
    {
        // redirection symbol present
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
        }
        // free the copy of last token
        free(temp);
        temp = NULL;
        // set the file name to NULL
        free(arg_arr[arg_cnt - 1]);
        arg_arr[arg_cnt - 1] = NULL;
        close(file_desc);
        return 1;
    }
    else
    {
        // no redirection symbol present
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
    no_of_files = scandir(path, &fileListTemp, NULL, mysort);
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

void builtin_cd(char *arg_arr)
{
    // printf("Entered Builtin_cd\n");
    char *token_cd = strdup(arg_arr);
    int rc = chdir(token_cd);
    // printf("chdir complete\n");
    free(token_cd);
    token_cd = NULL;

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
    char *token = strdup(arg);
    int cnt = arg_parse(token, temp, "=");
    // token = NULL;

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
            // release the previous value
            free(ptr->value);
            ptr->value = NULL;

            // assign new value
            ptr->value = strdup(temp[1]);

            // release token
            free(token);
            token = NULL;

            // return code=0 for successfull operation
            return_code = 0;
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
    free(token);
    token = NULL;
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
    // create copy of 'a=b' for usage within function
    char *env_var[2] = {NULL};
    char *token = strdup(arg_arr[1]);
    int cnt = arg_parse(token, env_var, "=");

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

    // release token created for usage within this function
    free(token);
    token = NULL;
    return_code = 0;
}

// ################################### main solver function #######################################

void solve(char **arg_arr, int arg_cnt)
{
    // for builtins & fork-execv
    // exit built-in command
    if ((strcmp(arg_arr[0], "exit") == 0))
    {
        if ((arg_cnt == 1))
        {
            // It is an error to pass any arguments to exit

            // printf("Called exit with code %d\n", return_code);
            // free_input_tokens(arg_arr, arg_cnt);
            free_memory();
            free(arg_arr[0]);
            exit(return_code);
        }

        // It is an error to pass any arguments to exit
        else
        {
            return_code = -1;
        }
    }

    // cd built-in
    else if (strcmp(arg_arr[0], "cd") == 0)
    {
        // check if it takes only one argument
        // printf("entered cd condition\n");
        if (arg_cnt == 2)
        {
            // printf("entered if cnt=2 condition\n");
            char *token = strdup(arg_arr[1]);
            builtin_cd(token);
            // printf("built-in cd call complete\n");
            free(token);
            token = NULL;
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
        if (arg_cnt == 2)
        {
            // Having a dollar sign on the left side is an error, e.g. local $a=b
            if (arg_arr[1][0] == '$')
            {
                return_code = -1;
                return;
            }

            // search for an "=" sign
            int i;
            for (i = 0; arg_arr[1][i] != '\0'; i++)
            {
                if (arg_arr[1][i] == '=')
                    break;
            }
            if (arg_arr[1][i] == '\0')
            {
                // No = sign encountered
                return_code = -1;
            }
            else
            {
                builtin_export(arg_arr);
            }
        }
        else
        {
            return_code = -1;
        }
    }

    // local built-in
    else if (strcmp(arg_arr[0], "local") == 0)
    {
        if (arg_cnt == 2)
        {
            // Having a dollar sign on the left side is an error, e.g. local $a=b
            if (arg_arr[1][0] == '$')
            {
                return_code = -1;
                return;
            }

            // search for an "=" sign
            int i;
            for (i = 0; arg_arr[1][i] != '\0'; i++)
            {
                if (arg_arr[1][i] == '=')
                    break;
            }
            if (arg_arr[1][i] == '\0')
            {
                // No = sign encountered
                return_code = -1;
            }
            else
            {
                // compare that the variable is not already present
                // if yes : update variable
                // if no : create new variable & store new value
                builtin_local(arg_arr[1], &shellvars_len);
            }
        }
        else
        {
            // for anthing other than "local a=b" format command
            return_code = -1;
        }

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
            if (redirection(arg_arr, arg_cnt) == 1)
            {
                // successful redirection
                builtin_vars();
                return_code = 0;
                dup2(saved_stdin, 0);
                dup2(saved_stdout, 1);
                dup2(saved_stderr, 2);
            }
            else
            {
                // redirection unsuccessful
                return_code = -1;
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

        else
        {
            return_code = -1;
        }
    }

    // history built-n
    else if (strcmp(arg_arr[0], "history") == 0)
    {
        // printf("Entered history condition\n");
        if (arg_cnt == 3 && !strcmp(arg_arr[1], "set"))
        {
            // works only when "history set [n]"

            // check whether n is numerical value
            for (int i = 0; arg_arr[2][i] != '\0'; i++)
            {
                if (!(arg_arr[2][i] >= '0' && arg_arr[2][i] < '9'))
                {
                    return_code = -1;
                    return;
                }
            }

            change_history_size(arg_arr[2]);
        }
        else if (arg_cnt == 2)
        {
            int saved_stdin = dup(0);
            int saved_stdout = dup(1);
            int saved_stderr = dup(2);
            if (redirection(arg_arr, arg_cnt) == 1)
            {
                // redirection successfull
                builtin_history();
                return_code = 0;
                dup2(saved_stdin, 0);
                dup2(saved_stdout, 1);
                dup2(saved_stderr, 2);
            }
            else
            {
                // redirection unsuccessfull
                return_code = -1;
            }
            close(saved_stderr);
            close(saved_stdout);
            close(saved_stderr);
        }
        else if (arg_cnt == 1)
        {
            builtin_history();
            return_code = 0;
        }
        else
        {
            return_code = -1;
        }
    }

    // ls built-in command
    // else if ((strcmp(arg_arr[0], "ls") == 0) && (arg_cnt <= 2))
    // check if 2nd argument has access(X_OK)
    else if ((strcmp(arg_arr[0], "ls") == 0))
    {
        if (arg_cnt == 2)
        {
            int saved_stdin = dup(0);
            int saved_stdout = dup(1);
            int saved_stderr = dup(2);
            if (redirection(arg_arr, arg_cnt) == 1)
            {
                // redirection successful
                builtin_ls();
                return_code = 0;
                dup2(saved_stdin, 0);
                dup2(saved_stdout, 1);
                dup2(saved_stderr, 2);
            }
            else
            {
                // redirection unsuccessful
                return_code = -1;
            }
            close(saved_stderr);
            close(saved_stdout);
            close(saved_stderr);
        }

        else if (arg_cnt == 1)
        {
            builtin_ls();
            return_code = 0;
        }
        else
        {
            return_code = -1;
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
                myargs[i] = strdup(arg_arr[i]);
            }
            myargs[arg_cnt] = NULL; // marks end of array
            if (redirection(myargs, arg_cnt) == 2)
            {
                return_code = -1;
            }
            else
            {
                execv(myargs[0], myargs); // runs word count
            }

            fprintf(stderr, "this shouldn't print out\n");
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

        // container to store all possible PATHs
        char *path_arg[100];
        int path_cnt = 0;
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
                myargs[i] = strdup(arg_arr[i]);
            }
            myargs[arg_cnt] = NULL; // marks end of array

            if (redirection(myargs, arg_cnt) == 2)
            {
                // don't exec if redirection has been called erroneously
                return_code = -1;
            }
            else
            {
                // pointer to path string
                char *path;
                // path variable has the entire path string now.
                path = getenv("PATH");
                // printf("%s\n", path);
                // char *path_arg[100];

                // contains the number of added paths
                path_cnt = arg_parse(path, path_arg, ":");
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
        // free_input_tokens(path_arg, path_cnt);
    }

    // Crux lies here
    // free_input_tokens(arg_arr, arg_cnt);
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

    // Your initial shell PATH environment variable should contain one directory: /bin
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
        if (file_ptr == NULL)
        {
            return -1;
        }
        // printf("Entered Batch mode\n");
        // while ((len = getline(&input, &size, file_ptr)) != -1)
        while (1)
        {

            // ######################## Take user input ###########################
            // printf("wsh> ");
            // fflush(stdout);
            if ((len = getline(&input, &size, file_ptr)) == -1)
            {
                free(input);
                input = NULL;
                free_memory();
                break;
            }
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
            // printf("No of tokens = %d\n", cnt_tokens(str1));
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

            // ################### handle history ###################

            // history_replace() sets used_history=1 if command of format "history [n]"
            int used_history = 0;
            char *actual_input = history_replace(input, arg_arr1, arg_cnt, &used_history);

            // record history if command is NOT of format "history [n]"
            if (used_history == 0)
            {
                record_history(actual_input, arg_arr1[0]);
            }

            // ################### handle variable substitution ######################
            // char *str5 = strdup(input);
            char *sub_input = NULL;

            // loop to find a '$' symbol
            for (int i = 0; i < arg_cnt; i++)
            {
                // substitute variable for every '$' found
                if (arg_arr1[i][0] == '$')
                {
                    // in case of multiple '$', release previously allocated memory
                    if (sub_input != NULL)
                    {
                        free(sub_input);
                        sub_input = NULL;
                    }
                    // input untouched in this function
                    sub_input = variable_sub(i, arg_arr1, arg_cnt, actual_input);
                }
            }
            if (sub_input == NULL)
            {
                sub_input = actual_input;
            }

            // I think post this point I no longer use str1
            free(str1);

            // Tokenise the history-replaced variable-substituted input
            char *arg_arr2[MAXARGS] = {NULL};
            arg_cnt = arg_parse(sub_input, arg_arr2, " ");
            solve(arg_arr2, arg_cnt);

            // release input, actual_input, sub_input
            if (input != NULL && (strcmp(input, sub_input) != 0) && (strcmp(input, actual_input) != 0))
            {
                free(input);
            }

            if (actual_input != NULL && strcmp(actual_input, sub_input) != 0)
            {
                free(actual_input);
            }

            free(sub_input);
            sub_input = NULL;
            input = NULL;
            actual_input = NULL;
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
            {
                free(input);
                input = NULL;
                free_memory();
                break;
            }

            // remove \n character that getline() reads by default
            if (input[strlen(input) - 1] == '\n')
            {
                input[strlen(input) - 1] = '\0';
            }

            // ################## parse the input for spaces #######################
            // make a deep copy of the user input
            char *str1 = strdup(input);
            // printf("No of tokens = %d\n", cnt_tokens(str1));
            char *arg_arr1[MAXARGS] = {NULL};
            int arg_cnt = arg_parse(str1, arg_arr1, " ");

            // free str1 somewhere

            // ######################## Handle spaces/newlines in files ######################
            if (arg_cnt == 0)
                continue;

            // ####################### Handle comments ###############################
            // Ignore all lines starting with #
            if (arg_arr1[0][0] == '#')
            {
                // printf("Treated as interactive comment\n");
                continue;
            }

            // ################### handle history ###################

            int used_history = 0;
            char *actual_input = history_replace(input, arg_arr1, arg_cnt, &used_history);

            // record history if command is NOT of format "history [n]"
            if (used_history == 0)
            {
                // if command != history [n], only then record history
                record_history(actual_input, arg_arr1[0]);
            }

            // ################### handle variable substitution ######################
            
            // char *str2 = strdup(actual_input);
            // char *arg_arr2[MAXARGS] = {NULL};;
            // if(used_history == 1)
            // {
            //      str2 = strdup(actual_input);
            //      arg_cnt = arg_parse(str2, arg_arr2, " ");
            //      printf("used history & new command tokenized\n");
            // }
            
            
            char *sub_input = NULL;

            // loop to find a '$' symbol
            for (int i = 0; i < arg_cnt; i++)
            {
                // substitute variable for every '$' found
                if (arg_arr1[i][0] == '$')
                {
                    // in case of multiple '$', release previously allocated memory
                    if (sub_input != NULL)
                    {
                        free(sub_input);
                        sub_input = NULL;
                    }
                    // input untouched in this function
                    sub_input = variable_sub(i, arg_arr1, arg_cnt, actual_input);
                }
            }
            // No '$' symbol found
            if (sub_input == NULL)
            {
                sub_input = actual_input;
            }

            // I think post this point I no longer use str1
            free(str1);

            // Tokenise the history-replaced variable-substituted input
            char *arg_arr3[MAXARGS] = {NULL};
            arg_cnt = arg_parse(sub_input, arg_arr3, " ");

            solve(arg_arr3, arg_cnt);

            // release input, actual_input, sub_input
            //printf("Nothing freed yet\n");
            if (input != NULL && (strcmp(input, sub_input) != 0) && (strcmp(input, actual_input) != 0))
            {
                free(input);
            }
            //printf("Input freed\n");

            if (actual_input != NULL && strcmp(actual_input, sub_input) != 0)
            {
                free(actual_input);
                
            }
            //printf("Actual Input freed\n");
            free(sub_input);
            sub_input = NULL;
            input = NULL;
            actual_input = NULL;
        }
    }

    else
    {
        // The shell can be invoked with either no arguments or a
        // single argument; anything else is an error.
        return -1;
    }

    // free(input);
    input = NULL;
    free_memory();
    // printf("Before returning main %d\n", return_code);
    return return_code;
}