/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   scheduler.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: badiyaf <badiyaf@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/16 17:41:56 by badiyaf           #+#    #+#             */
/*   Updated: 2026/08/16 17:43:19 by badiyaf          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

long	fr_deadline(t_sim *sim, int id)
{
	long	start;

	pthread_mutex_lock(&sim->coders[id - 1].meal_lock);
	start = sim->coders[id - 1].last_compile_start;
	pthread_mutex_unlock(&sim->coders[id - 1].meal_lock);
	return (start + sim->time_to_burnout);
}

int	fr_earliest_in_queue(t_sim *sim, t_queue *q, int me)
{
	int		i;
	int		best_index;
	long	best_deadline;
	long	current;

	if (q == NULL || q->count == 0)
		return (0);
	best_index = 0;
	best_deadline = fr_deadline(sim, q->arr_q[0]);
	i = 1;
	while (i < q->count)
	{
		current = fr_deadline(sim, q->arr_q[i]);
		if (current < best_deadline
			|| (current == best_deadline && q->arr_q[i] < q->arr_q[best_index]))
		{
			best_deadline = current;
			best_index = i;
		}
		i++;
	}
	return (q->arr_q[best_index] == me);
}
