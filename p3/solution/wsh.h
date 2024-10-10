#ifndef WSH_H
#define WSH_H

// not used
void record_input(char *dest, char *src);

void prune_history(int history_size, int history_cnt, int func_flag);
void change_history_size(char *n);
// void record_history(char *arg, char *firstarg);
char *history_replace(char *str2, char **input_tokens, int arg_cnt, int *flag);

int mysort(const struct dirent **a, const struct dirent **b);

int arg_parse(char *str, char **arg_arr, char *delims);
char *variable_sub(char **arg_arr, int arg_cnt, char *str);
int redirection(int caller, char **arg_arr, int arg_cnt);

void builtin_ls();
void builtin_cd(char *arg_arr);
void builtin_local(char *arg, int *shellvars_len);
void builtin_vars();
void builtin_history();
void builtin_export(char **arg_arr);

void solve(char **arg_arr, int arg_cnt);

void free_memory();

#endif

