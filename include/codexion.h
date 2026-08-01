#ifndef CODEXION_H
# define CODEXION_H

# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <pthread.h>
# include <unistd.h>
# include <sys/time.h>

# define SIZE 4

/*
** Forward declarations
*/
typedef struct s_coder	t_coder;
typedef struct s_dongle	t_dongle;
typedef struct s_sim	t_sim;

/*
** Queue
*/
typedef struct s_queue
{
	int	arr_q[SIZE];
	int	count;
}	t_queue;

/*
** Dongle
*/
typedef struct s_dongle
{
	int				available;
	long			released_at;
	t_coder			*owner;
	t_queue			queue;
	pthread_mutex_t	lock;
}	t_dongle;

/*
** Coder
*/
typedef struct s_coder
{
	int				id;
	int				compiles_done;
	int				finished;
	long			last_compile_start;
	t_dongle		*left;
	t_dongle		*right;
	pthread_mutex_t	meal_lock;
	pthread_t		coder_thread;
	t_sim			*sim;
}	t_coder;

/*
** Simulation
*/
typedef struct s_sim
{
	int				n_coders;
	long			time_to_burnout;
	long			time_to_compile;
	long			time_to_debug;
	long			time_to_refactor;
	int				compiles_required;
	long			dongle_cooldown;
	char			*scheduler;
	long			start_time;
	int				stopped;
	t_coder			*coders;
	t_dongle		*dongles;
	pthread_t		monitor_thread;
	pthread_mutex_t	scheduler_lock;
	pthread_cond_t	scheduler_cond;
	int				finished_coders;
	pthread_mutex_t	finished_lock;
	pthread_mutex_t	log_lock;
	int				stop_flag;
	pthread_mutex_t	stop_lock;
}	t_sim;

/*
** Parsing
*/
int		pars_args(int ac, char **av, t_sim *simulation);
int		is_valid_number(char *str);

/*
** Time
*/
long	fr_get_time_ms(void);
int		fr_stoppable_sleep(t_sim *sim, long duration_ms);

/*
** Build
*/
int		fr_build_dongle(t_sim *simulation);
int		fr_build_coder(t_sim *simulation);
void	ft_clean_evrithing(t_sim *simulation);

/*
** Logger
*/
void	fr_log(t_coder *coder, char *state);

/*
** Queue
*/
int		queue_push(t_queue *q, int id);
int		queue_pop_front(t_queue *q);
int		queue_front(t_queue *q);
void	push_id_to_dongles(t_coder *coder);

/*
** Scheduler
*/
int		request_dongle(t_coder *coder);
void	fr_release_dongles(t_coder *coder);
int		fr_both_free(t_coder *coder);
int		fr_has_priority(t_coder *coder);
long	fr_deadline(t_sim *sim, int id);
int		fr_earliest_in_queue(t_sim *sim, t_queue *q, int me);

/*
** Stop
*/
int		fr_check_stop(t_sim *sim);

#endif