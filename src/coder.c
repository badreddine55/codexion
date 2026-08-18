/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: badiyaf <badiyaf@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/16 17:41:16 by badiyaf           #+#    #+#             */
/*   Updated: 2026/08/16 17:42:53 by badiyaf          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	init_single_coder(t_sim *simulation, int i)
{
	pthread_mutex_init(&simulation->coders[i].meal_lock, NULL);
	simulation->coders[i].id = i + 1;
	simulation->coders[i].compiles_done = 0;
	simulation->coders[i].finished = 0;
	simulation->coders[i].last_compile_start = fr_get_time_ms();
	simulation->coders[i].right = &simulation->dongles[i];
	simulation->coders[i].left
		= &simulation->dongles[(i + 1) % simulation->n_coders];
	simulation->coders[i].sim = simulation;
}

int	fr_build_coder(t_sim *simulation)
{
	int	i;

	simulation->coders = malloc(sizeof(t_coder) * simulation->n_coders);
	if (simulation->coders == NULL)
	{
		perror("malloc");
		return (1);
	}
	i = 0;
	while (i < simulation->n_coders)
	{
		init_single_coder(simulation, i);
		i++;
	}
	return (0);
}

static int	coder_routine_step(t_coder *coder)
{
	int	interrupted;

	if (fr_check_stop(coder->sim) || request_dongle(coder) == -1)
		return (1);
	pthread_mutex_lock(&coder->meal_lock);
	coder->last_compile_start = fr_get_time_ms();
	pthread_mutex_unlock(&coder->meal_lock);
	fr_log(coder, "is compiling");
	interrupted = (fr_stoppable_sleep(coder->sim,
				coder->sim->time_to_compile) == -1);
	fr_release_dongles(coder);
	if (interrupted)
		return (1);
	pthread_mutex_lock(&coder->meal_lock);
	coder->compiles_done++;
	pthread_mutex_unlock(&coder->meal_lock);
	fr_log(coder, "is debugging");
	if (fr_stoppable_sleep(coder->sim, coder->sim->time_to_debug) == -1)
		return (1);
	fr_log(coder, "is refactoring");
	if (fr_stoppable_sleep(coder->sim, coder->sim->time_to_refactor) == -1)
		return (1);
	return (0);
}

static void	handle_coder_finish(t_coder *coder, int j)
{
	int	is_finished;

	pthread_mutex_lock(&coder->meal_lock);
	if (j >= coder->sim->compiles_required)
		coder->finished = 1;
	is_finished = coder->finished;
	pthread_mutex_unlock(&coder->meal_lock);
	if (is_finished)
	{
		pthread_mutex_lock(&coder->sim->finished_lock);
		coder->sim->finished_coders += 1;
		pthread_mutex_unlock(&coder->sim->finished_lock);
	}
}

void	*coder_cycle(void *arg)
{
	t_coder	*coder;
	int		j;

	coder = (t_coder *)arg;
	j = 0;
	while (j < coder->sim->compiles_required)
	{
		if (coder_routine_step(coder))
			break ;
		j++;
	}
	handle_coder_finish(coder, j);
	return (NULL);
}
