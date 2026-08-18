/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   queue.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: badiyaf <badiyaf@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/16 17:41:53 by badiyaf           #+#    #+#             */
/*   Updated: 2026/08/18 17:13:34 by badiyaf          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	queue_push(t_queue *q, int id)
{
	if (q == NULL || q->arr_q == NULL)
		return (-1);
	if (q->count < q->capacity)
	{
		q->arr_q[q->count] = id;
		q->count += 1;
		return (0);
	}
	return (-1);
}

int	queue_remove_id(t_queue *q, int id)
{
	int	i;
	int	j;

	if (q == NULL || q->arr_q == NULL)
		return (-1);
	i = 0;
	while (i < q->count)
	{
		if (q->arr_q[i] == id)
		{
			j = i;
			while (j < q->count - 1)
			{
				q->arr_q[j] = q->arr_q[j + 1];
				j++;
			}
			q->count -= 1;
			return (0);
		}
		i++;
	}
	return (-1);
}

int	queue_pop_front(t_queue *q)
{
	int	i;
	int	id;

	if (q == NULL || q->arr_q == NULL || q->count == 0)
		return (-1);
	id = q->arr_q[0];
	i = 0;
	while (i < q->count - 1)
	{
		q->arr_q[i] = q->arr_q[i + 1];
		i++;
	}
	q->count -= 1;
	return (id);
}

int	queue_front(t_queue *q)
{
	if (q == NULL || q->arr_q == NULL || q->count == 0)
		return (-1);
	return (q->arr_q[0]);
}
