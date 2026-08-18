/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   logger.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: badiyaf <badiyaf@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/16 17:41:41 by badiyaf           #+#    #+#             */
/*   Updated: 2026/08/16 17:41:42 by badiyaf          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	fr_log(t_coder *coder, char *state)
{
	long	timestamp_in_ms;

	pthread_mutex_lock(&coder->sim->log_lock);
	if (fr_check_stop(coder->sim) && strcmp(state, "burned out") != 0)
	{
		pthread_mutex_unlock(&coder->sim->log_lock);
		return ;
	}
	timestamp_in_ms = (fr_get_time_ms() - coder->sim->start_time);
	printf("%ld %d %s\n", timestamp_in_ms, coder->id, state);
	if (strcmp(state, "burned out") == 0)
	{
		pthread_mutex_lock(&coder->sim->stop_lock);
		coder->sim->stop_flag = 1;
		pthread_mutex_unlock(&coder->sim->stop_lock);
	}
	pthread_mutex_unlock(&coder->sim->log_lock);
}
