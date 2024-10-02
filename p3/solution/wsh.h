#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>   // for readdir()
#include <unistd.h>   // for system calls
#include <sys/wait.h> // for wait()

// Piazza @226
// constraints on the input like command length and number of arguments
#define MAXLINE 1024
#define MAXARGS 128

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
    // if history has 5 elements, make 3 jumps to reach 4th element
    int jumps = 0;
    if (func_flag == 0)
    {
        jumps = history_size - 1;
    }
    else
    {
        jumps = history_size - 2;
    }

    // remove the extra elements
    if (history_cnt >= history_size)
    {
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
    printf("History size changed to %d\n", Hsize);
}

void record_history(char *arg, char *firstarg)
{
    // debug
    printf("In record_history str is: %s\n", arg);
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
    // if (Hcnt >= Hsize)
    // {
    //     struct HNode *ptr = Hfirst;
    //     while (Hfirst != NULL && ptr->next->next != NULL)
    //     {
    //         ptr = ptr->next;
    //     }
    //     // temp node to point on the last node
    //     struct HNode *temp = ptr->next;
    //     // space created to add new node
    //     ptr->next = NULL;
    //     // free last node memory
    //     free(temp);

    // }
    printf("before prune history\n");
    prune_history(Hsize, Hcnt, 1);
    printf("after prune history\n");

    // create a new node
    struct HNode *temp = (struct HNode *)malloc(sizeof(struct HNode));
    temp->command = strdup(arg);
    // temp->command = arg;
    temp->next = NULL;

    if (Hfirst == NULL)
    {
        Hfirst = temp;
    }
    else
    {
        temp->next = Hfirst;
        Hfirst = temp;
        // struct HNode *hptr = Hfirst;
        // while(hptr->next != NULL)
        // {
        //     hptr = hptr->next;
        // }
        // hptr->next = temp;
    }
    Hcnt++;
    printf("History has %d commands\n", Hcnt);
}


char * history_replace(char *str2, char ** input_tokens, int arg_cnt, int *flag)
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
            char* str = strdup(ptr->command);
            *flag = 1;
            return str;
        }
        else
        {
            // if command_no is not valid
            return str2;
        }
    }

    else {
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
    printf("tokenization done %d\n", arg_cnt);
    return arg_cnt;
}

// ######################################### Built-in #############################################

void builtin_ls()
{
    DIR *d;
    struct dirent *dir;
    d = opendir(".");
    if (d)
    {
        while ((dir = readdir(d)) != NULL)
        {
            // Prevent "." & ".." from getting printed
            if (dir->d_name[0] == '.')
                continue;
            printf("%s\n", dir->d_name);
        }
        closedir(d);
    }
}

void builtin_cd(char **arg_arr)
{
    char s[100];

    // using the command
    int rc = chdir(arg_arr[1]);

    if (rc == 0)
    {
        // printing current working directory
        printf("%s\n", getcwd(s, 100));
    }
    else
    {
        printf("chdir error\n");
    }
}

void builtin_export(char *env_var, char **environ, int *environ_len)
{
    environ[*environ_len] = env_var;
    (*environ_len)++;
    printf("Number of environment variables = %d\n", *environ_len);
}

void builtin_local(char *arg, int *shellvars_len)
{
    // check if variable is already present in list
    struct SNode *ptr = Sfirst;

    // parse the argument to local
    char *temp[2];
    arg_parse(arg, temp, "=");

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
            printf("inside while loop\n");
            ptr = ptr->next;
        }
        ptr->next = t;
    }
    // debug statements : to be removed
    (*shellvars_len)++;
    printf("Number of shell variables = %d\n", *shellvars_len);
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
}

void builtin_history()
{
    struct HNode *hptr = Hfirst;
    while (hptr != NULL)
    {
        printf("%s\n", hptr->command);
        hptr = hptr->next;
    }
}



void solve(char ** arg_arr, int arg_cnt)
{
    // ######################### old stuff commented ######################
    // flag to denote if history was used
    // int used_history = 0;
    // debug
    // printf("before record_history() command is: %s\n", str);
    // record history before strtok as it modifies input string
    // char input_copy[MAXLINE];
    // record_input(input_copy, str);
    // char *input_ptr = strdup(str);
    // int used_history =  history_replace(str, input_ptr);
    // printf("before record_history() command is: %s\n", input_copy);
    // record_history(input_copy, arg_arr[0]);
    // printf("after arg_parse() command is: %s\n", input_copy);

    // ###################### Built-in Commands #######################

    // exit built-in command
    if ((strcmp(arg_arr[0], "exit") == 0) && (arg_cnt == 1))
    {
        exit(0);
    }

    // cd built-in
    else if (strcmp(arg_arr[0], "cd") == 0)
    {
        // check if it takes only one argument
        if (arg_cnt == 2)
        {
            builtin_cd(arg_arr);
        }
    }

    // export built-in command
    else if (strcmp(arg_arr[0], "export") == 0)
    {
        char *env_var[2];
        arg_parse(arg_arr[1], env_var, "=");
        setenv(env_var[0], env_var[1], 1);
    }

    // local built-in
    else if (strcmp(arg_arr[0], "local") == 0)
    {
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
        builtin_vars();
    }

    // history built-n
    else if (strcmp(arg_arr[0], "history") == 0)
    {
        // printf("Entered history condition\n");
        if (arg_cnt > 2 && !strcmp(arg_arr[1], "set"))
        {
            change_history_size(arg_arr[2]);
        }
        else if(arg_cnt == 1)
        {
            builtin_history();
        }
    }

    // ls built-in command
    else if ((strcmp(arg_arr[0], "ls") == 0) && (arg_cnt == 1))
    {
        builtin_ls();
    }

    // ################################### Path based #################################

    // absolute + relative path
    else if (access(arg_arr[0], X_OK) != -1)
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
            myargs[arg_cnt] = NULL;   // marks end of array
            execv(myargs[0], myargs); // runs word count
            printf("this shouldn't print out\n");
            // kill the child if the execv failed
            exit(0);
        }
        else
        {
            // parent goes down this path (original process)
            int wc = wait(NULL);
            printf("hello, I am parent of %d (wc:%d) (pid:%d)\n", rc, wc, (int)getpid());
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
            printf("hello, I am child (pid:%d)\n", (int)getpid());
            char *myargs[arg_cnt + 1];
            for (int i = 0; i < arg_cnt; i++)
            {
                myargs[i] = arg_arr[i];
            }
            myargs[arg_cnt] = NULL; // marks end of array

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
                char temp[100];
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
            printf("%s: command not found\n", myargs[0]);
            // in case failed exec, the child needs to be killed
            exit(1);
        }
        else
        {
            // fflush : glibc buffer
            // parent goes down this path (original process)
            int wc = wait(NULL);
            printf("hello, I am parent of %d (wc:%d) (pid:%d)\n", rc, wc, (int)getpid());
        }
    }
}
