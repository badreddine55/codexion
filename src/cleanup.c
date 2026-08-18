/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cleanup.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: badiyaf <badiyaf@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/16 17:41:10 by badiyaf           #+#    #+#             */
/*   Updated: 2026/08/16 17:42:49 by badiyaf          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	clean_coders(t_sim *simulation)
{
	int	i;

	if (!simulation->coders)
		return ;
	i = 0;
	while (i < simulation->n_coders)
	{
		pthread_mutex_destroy(&simulation->coders[i].meal_lock);
		i++;
	}
	free(simulation->coders);
	simulation->coders = NULL;
}

static void	clean_dongles(t_sim *simulation)
{
	int	i;

	if (!simulation->dongles)
		return ;
	i = 0;
	while (i < simulation->n_coders)
	{
		free(simulation->dongles[i].queue.arr_q);
		simulation->dongles[i].queue.arr_q = NULL;
		pthread_mutex_destroy(&simulation->dongles[i].lock);
		i++;
	}
	free(simulation->dongles);
	simulation->dongles = NULL;
}

void	ft_clean_evrithing(t_sim *simulation)
{
	if (simulation == NULL)
		return ;
	clean_coders(simulation);
	clean_dongles(simulation);
	pthread_mutex_destroy(&simulation->finished_lock);
	pthread_mutex_destroy(&simulation->log_lock);
	pthread_mutex_destroy(&simulation->stop_lock);
	free(simulation);
}
