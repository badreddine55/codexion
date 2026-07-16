#ifndef CODEXION_H
#define CODEXION_H


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>
typedef struct s_dongle
{
    int             available;
    long  released_at;      // for cooldown tracking
    pthread_mutex_t lock;
    pthread_cond_t  cond;
}   t_dongle;

typedef struct s_coder
{
    int             id;                // 1..N
    int             compiles_done;
    long  last_compile_start;
    t_dongle        *left;
    t_dongle        *right;
    struct s_sim    *sim;              // back-pointer to shared config/state
}   t_coder;

typedef struct s_sim
{
    int             n_coders;
    long            time_to_burnout;
    long            time_to_compile;
    long            time_to_debug;
    long            time_to_refactor;
    int             compiles_required;
    long            dongle_cooldown;
    char            *scheduler;      // "fifo" or "edf"
    long            start_time;
    t_coder         *coders;
    t_dongle        *dongles;
    pthread_t       *threads;
    pthread_t       monitor_thread;

    pthread_mutex_t log_lock;
    int             stop_flag;         // set on burnout or completion
    pthread_mutex_t stop_lock;
}   t_sim;

int pars_args(int ac, char **av, t_sim	*simulation);
int is_valid_number(char *str);
long fr_get_time_ms(void);
int fr_build_dongle(t_sim *simulation);
int fr_build_coder(t_sim *simulation);
void ft_clean_evrithing(t_sim *simulation);
void fr_log(t_coder *coder, char *state);
#endif
