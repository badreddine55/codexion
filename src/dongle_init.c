/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle_init.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: badiyaf <badiyaf@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/16 17:41:32 by badiyaf           #+#    #+#             */
/*   Updated: 2026/08/18 17:13:11 by badiyaf          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	init_single_dongle(t_sim *simulation, int i, long now)
{
	simulation->dongles[i].owner = NULL;
	simulation->dongles[i].released_at = now - simulation->dongle_cooldown;
	simulation->dongles[i].queue.count = 0;
	simulation->dongles[i].queue.capacity = simulation->n_coders;
	simulation->dongles[i].queue.arr_q = malloc(sizeof(int)
			* simulation->n_coders);
	if (simulation->dongles[i].queue.arr_q == NULL)
	{
		perror("malloc");
		while (--i >= 0)
			free(simulation->dongles[i].queue.arr_q);
		free(simulation->dongles);
		simulation->dongles = NULL;
		return (1);
	}
	pthread_mutex_init(&simulation->dongles[i].lock, NULL);
	return (0);
}

int	fr_build_dongle(t_sim *simulation)
{
	int		i;
	long	now;

	simulation->dongles = malloc(sizeof(t_dongle) * simulation->n_coders);
	if (simulation->dongles == NULL)
	{
		perror("malloc");
		return (1);
	}
	now = fr_get_time_ms();
	i = 0;
	while (i < simulation->n_coders)
	{
		if (init_single_dongle(simulation, i, now))
			return (1);
		i++;
	}
	return (0);
}

int	push_id_to_dongles(t_coder *coder)
{
	int	ret;

	pthread_mutex_lock(&coder->left->lock);
	ret = queue_push(&coder->left->queue, coder->id);
	pthread_mutex_unlock(&coder->left->lock);
	if (ret == -1 || coder->left == coder->right)
		return (ret);
	pthread_mutex_lock(&coder->right->lock);
	ret = queue_push(&coder->right->queue, coder->id);
	pthread_mutex_unlock(&coder->right->lock);
	if (ret == -1)
	{
		pthread_mutex_lock(&coder->left->lock);
		queue_remove_id(&coder->left->queue, coder->id);
		pthread_mutex_unlock(&coder->left->lock);
		return (-1);
	}
	return (0);
}

int	fr_has_priority(t_coder *coder)
{
	int	left_ok;
	int	right_ok;

	pthread_mutex_lock(&coder->left->lock);
	if (strcmp(coder->sim->scheduler, "fifo") == 0)
		left_ok = (queue_front(&coder->left->queue) == coder->id);
	else
		left_ok = fr_earliest_in_queue(coder->sim, &coder->left->queue,
				coder->id);
	pthread_mutex_unlock(&coder->left->lock);
	pthread_mutex_lock(&coder->right->lock);
	if (strcmp(coder->sim->scheduler, "fifo") == 0)
		right_ok = (queue_front(&coder->right->queue) == coder->id);
	else
		right_ok = fr_earliest_in_queue(coder->sim, &coder->right->queue,
				coder->id);
	pthread_mutex_unlock(&coder->right->lock);
	return (left_ok && right_ok);
}

void	fr_release_dongles(t_coder *coder)
{
	long	now;

	now = fr_get_time_ms();
	pthread_mutex_lock(&coder->left->lock);
	coder->left->owner = NULL;
	coder->left->released_at = now;
	pthread_mutex_unlock(&coder->left->lock);
	pthread_mutex_lock(&coder->right->lock);
	coder->right->owner = NULL;
	coder->right->released_at = now;
	pthread_mutex_unlock(&coder->right->lock);
}
