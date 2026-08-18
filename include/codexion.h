/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: badiyaf <badiyaf@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/16 18:49:51 by badiyaf           #+#    #+#             */
/*   Updated: 2026/08/16 18:49:53 by badiyaf          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CODEXION_H
# define CODEXION_H

# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <pthread.h>
# include <unistd.h>
# include <limits.h>
# include <sys/time.h>
# include <time.h>

typedef struct s_coder	t_coder;
typedef struct s_dongle	t_dongle;
typedef struct s_sim	t_sim;

typedef struct s_queue
{
	int	*arr_q;
	int	count;
	int	capacity;
}	t_queue;

typedef struct s_dongle
{
	long			released_at;
	t_coder			*owner;
	t_queue			queue;
	pthread_mutex_t	lock;
}	t_dongle;

typedef struct s_coder
{
	int				id;
	int				compiles_done;
	int				finished;
	long			last_compile_start;
	int				has_compiled;
	t_dongle		*left;
	t_dongle		*right;
	pthread_mutex_t	meal_lock;
	pthread_t		coder_thread;
	t_sim			*sim;
}	t_coder;

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
	int				finished_coders;
	pthread_mutex_t	finished_lock;
	pthread_mutex_t	log_lock;
	int				stop_flag;
	pthread_mutex_t	stop_lock;
}	t_sim;

int		pars_args(int ac, char **av, t_sim *simulation);
int		is_valid_number(char *str);
int		is_valid_chars(const char *str);

long	fr_get_time_ms(void);
int		fr_stoppable_sleep(t_sim *sim, long duration_ms);

int		fr_build_dongle(t_sim *simulation);
int		fr_build_coder(t_sim *simulation);
void	ft_clean_evrithing(t_sim *simulation);
void	*coder_cycle(void *arg);
void	*monitor_thread(void *arg);

void	fr_log(t_coder *coder, char *state);

int		queue_push(t_queue *q, int id);
int		queue_pop_front(t_queue *q);
int		queue_front(t_queue *q);
int		queue_remove_id(t_queue *q, int id);
int		push_id_to_dongles(t_coder *coder);

int		request_dongle(t_coder *coder);
void	fr_release_dongles(t_coder *coder);
int		fr_try_acquire(t_coder *coder);
int		fr_has_priority(t_coder *coder);
long	fr_deadline(t_sim *sim, int id);
int		fr_earliest_in_queue(t_sim *sim, t_queue *q, int me);

int		fr_check_stop(t_sim *sim);

#endif