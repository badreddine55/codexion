#ifndef CODEXION_H
#define CODEXION_H


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>

#define SIZE 4

typedef struct s_queue
{
    int arr_q[SIZE];
    int count;
} t_queue;

typedef struct s_dongle
{
    int             available;
    long            released_at;
    t_queue         queue;
    pthread_mutex_t lock;
    pthread_cond_t  cond;
} t_dongle;

typedef struct s_coder
{
    int             id;                // 1..N
    int             compiles_done;
    int             finished;
    long  last_compile_start;
    t_dongle        *left;
    t_dongle        *right;
    pthread_mutex_t meal_lock; // this is for the last compile start time, to avoid race conditions
    pthread_t       coder_thread;
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
    int              stopped;
    t_coder         *coders;
    t_dongle        *dongles;
    pthread_t       monitor_thread;

    int finished_coders;
    pthread_mutex_t finished_lock;
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
int queue_pop_front(t_queue *q);
int queue_push(t_queue *q, int id);
int queue_front(t_queue *q);
void push_id_to_dongles(t_coder *coder);
int release_dongle(t_dongle *dongle);
int request_left_dongle(t_coder *coder);
int request_right_dongle(t_coder *coder);
int fr_stoppable_sleep(t_sim *sim, long duration_ms);
int fr_check_stop(t_sim *sim);
#endif
