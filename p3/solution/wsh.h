#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h> // for readdir()
#include <unistd.h> // for system calls

// Linked List for shell variables
struct SNode
{
    char *key;
    char *value;
    struct SNode* next;
};
struct SNode *Sfirst = NULL;

// Linked List for history
struct HNode
{
    char *command;
    struct HNode* next;
} ;
struct HNode *Hfirst=NULL;

// ##################################### History ##############################################

int Hcnt=0;     // variable for history length
void record_history(char *arg)
{
    // debug
    // printf("In record_history str is: %s\n", arg);

    // create a new node
    struct HNode *temp = (struct HNode*)malloc(sizeof(struct HNode));
    temp->command = strdup(arg);
    // temp->command = arg;
    temp->next = NULL;

    if(Hfirst == NULL)
    {
        Hfirst = temp;
    }
    else{
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
    struct SNode* ptr = Sfirst;
    
    // parse the argument to local
    char* temp[2];
    arg_parse(arg, temp, "=");
    
    while (ptr != NULL)
    {
        if(strcmp(ptr->key, temp[0])==0)
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
        while(ptr->next != NULL)
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
    if(Sfirst == NULL)
    {
        printf("No shell variable\n");
    }
    struct SNode* ptr = Sfirst;
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
    while(hptr != NULL)
    {
        printf("%s\n", hptr->command);
        hptr = hptr->next;
    }
}
