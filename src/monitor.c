/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: badiyaf <badiyaf@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/16 17:41:48 by badiyaf           #+#    #+#             */
/*   Updated: 2026/08/16 17:43:10 by badiyaf          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	fr_check_stop(t_sim *sim)
{
	int	flag;

	pthread_mutex_lock(&sim->stop_lock);
	flag = sim->stop_flag;
	pthread_mutex_unlock(&sim->stop_lock);
	return (flag);
}

int	fr_stoppable_sleep(t_sim *sim, long duration_ms)
{
	long	start_time;
	long	now;

	start_time = fr_get_time_ms();
	while (1)
	{
		if (fr_check_stop(sim))
			return (-1);
		now = fr_get_time_ms();
		if (now >= start_time + duration_ms)
			return (0);
		usleep(2000);
	}
	return (0);
}

static int	check_all_finished(t_sim *sim)
{
	int	finished_c;

	pthread_mutex_lock(&sim->finished_lock);
	finished_c = sim->finished_coders;
	pthread_mutex_unlock(&sim->finished_lock);
	if (finished_c >= sim->n_coders)
	{
		pthread_mutex_lock(&sim->stop_lock);
		sim->stop_flag = 1;
		pthread_mutex_unlock(&sim->stop_lock);
		return (1);
	}
	return (0);
}

static int	check_coder_burnout(t_sim *sim, int i)
{
	int		coder_finished;
	long	last_compile_start;
	long	interval;

	pthread_mutex_lock(&sim->coders[i].meal_lock);
	coder_finished = sim->coders[i].finished;
	last_compile_start = sim->coders[i].last_compile_start;
	pthread_mutex_unlock(&sim->coders[i].meal_lock);
	if (coder_finished == 1)
		return (0);
	interval = fr_get_time_ms() - last_compile_start;
	if (interval > sim->time_to_burnout)
	{
		fr_log(&sim->coders[i], "burned out");
		pthread_mutex_lock(&sim->stop_lock);
		sim->stop_flag = 1;
		pthread_mutex_unlock(&sim->stop_lock);
		return (1);
	}
	return (0);
}

void	*monitor_thread(void *arg)
{
	t_sim	*sim;
	int		i;

	sim = (t_sim *)arg;
	while (1)
	{
		if (check_all_finished(sim))
			return (NULL);
		i = 0;
		while (i < sim->n_coders)
		{
			if (check_coder_burnout(sim, i))
				return (NULL);
			i++;
		}
		usleep(1000);
	}
	return (NULL);
}
