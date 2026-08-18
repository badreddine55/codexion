/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: badiyaf <badiyaf@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/16 17:41:45 by badiyaf           #+#    #+#             */
/*   Updated: 2026/08/18 16:57:59 by badiyaf          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	join_coders(t_sim *sim, int count)
{
	int	i;

	i = 0;
	while (i < count)
	{
		pthread_join(sim->coders[i].coder_thread, NULL);
		i++;
	}
}

static int	create_coder_batch(t_sim *sim, int parity, int *created)
{
	int	i;
	int	ret;

	i = 0;
	while (i < sim->n_coders)
	{
		if ((i % 2) != parity)
		{
			i++;
			continue ;
		}
		ret = pthread_create(&sim->coders[i].coder_thread,
				NULL, coder_cycle, &sim->coders[i]);
		if (ret != 0)
		{
			pthread_mutex_lock(&sim->stop_lock);
			sim->stop_flag = 1;
			pthread_mutex_unlock(&sim->stop_lock);
			return (1);
		}
		(*created)++;
		i++;
	}
	return (0);
}

static int	setup_simulation(t_sim *sim, int ac, char **av)
{
	memset(sim, 0, sizeof(t_sim));
	pthread_mutex_init(&sim->log_lock, NULL);
	pthread_mutex_init(&sim->stop_lock, NULL);
	pthread_mutex_init(&sim->finished_lock, NULL);
	if (pars_args(ac, av, sim))
		return (1);
	if (fr_build_dongle(sim))
		return (1);
	if (fr_build_coder(sim))
		return (1);
	return (0);
}

static int	launch_simulation(t_sim *sim)
{
	int	created;
	int	ret;

	created = 0;
	sim->start_time = fr_get_time_ms();
	create_coder_batch(sim, 1, &created);
	usleep(2000);
	create_coder_batch(sim, 0, &created);
	if (created != sim->n_coders)
	{
		join_coders(sim, created);
		return (1);
	}
	ret = pthread_create(&sim->monitor_thread, NULL, monitor_thread, sim);
	if (ret != 0)
	{
		pthread_mutex_lock(&sim->stop_lock);
		sim->stop_flag = 1;
		pthread_mutex_unlock(&sim->stop_lock);
		join_coders(sim, sim->n_coders);
		return (1);
	}
	join_coders(sim, sim->n_coders);
	pthread_join(sim->monitor_thread, NULL);
	return (0);
}

int	main(int ac, char **av)
{
	t_sim	*simulation;

	simulation = malloc(sizeof(t_sim));
	if (simulation == NULL)
	{
		perror("malloc");
		return (1);
	}
	if (setup_simulation(simulation, ac, av))
	{
		ft_clean_evrithing(simulation);
		return (1);
	}
	if (launch_simulation(simulation))
	{
		ft_clean_evrithing(simulation);
		return (1);
	}
	ft_clean_evrithing(simulation);
	return (0);
}
