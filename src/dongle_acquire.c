/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle_acquire.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: badiyaf <badiyaf@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/16 17:41:25 by badiyaf           #+#    #+#             */
/*   Updated: 2026/08/16 17:42:56 by badiyaf          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	fr_try_acquire_single(t_coder *coder)
{
	long	now;
	int		acquired;

	acquired = 0;
	pthread_mutex_lock(&coder->left->lock);
	now = fr_get_time_ms();
	if (coder->left->owner == NULL
		&& now >= coder->left->released_at + coder->sim->dongle_cooldown)
	{
		coder->left->owner = coder;
		queue_remove_id(&coder->left->queue, coder->id);
		acquired = -1;
	}
	pthread_mutex_unlock(&coder->left->lock);
	if (acquired)
		fr_log(coder, "has taken a dongle");
	return (acquired);
}

static int	check_and_take_pair(t_coder *coder, long now)
{
	if (coder->left->owner == NULL && coder->right->owner == NULL
		&& now >= coder->left->released_at + coder->sim->dongle_cooldown
		&& now >= coder->right->released_at + coder->sim->dongle_cooldown)
	{
		coder->left->owner = coder;
		coder->right->owner = coder;
		queue_remove_id(&coder->left->queue, coder->id);
		queue_remove_id(&coder->right->queue, coder->id);
		return (1);
	}
	return (0);
}

static int	fr_try_acquire_pair(t_coder *coder)
{
	int			acquired;
	t_dongle	*first;
	t_dongle	*second;

	if (coder->left < coder->right)
	{
		first = coder->left;
		second = coder->right;
	}
	else
	{
		first = coder->right;
		second = coder->left;
	}
	pthread_mutex_lock(&first->lock);
	pthread_mutex_lock(&second->lock);
	acquired = check_and_take_pair(coder, fr_get_time_ms());
	pthread_mutex_unlock(&second->lock);
	pthread_mutex_unlock(&first->lock);
	if (acquired)
	{
		fr_log(coder, "has taken a dongle");
		fr_log(coder, "has taken a dongle");
	}
	return (acquired);
}

int	fr_try_acquire(t_coder *coder)
{
	if (coder->left == coder->right)
		return (fr_try_acquire_single(coder));
	return (fr_try_acquire_pair(coder));
}

int	request_dongle(t_coder *coder)
{
	int	res;

	if (push_id_to_dongles(coder) == -1)
		return (-1);
	while (1)
	{
		if (fr_check_stop(coder->sim))
			return (-1);
		if (fr_has_priority(coder))
		{
			res = fr_try_acquire(coder);
			if (res == 1)
				return (0);
			if (res == -1)
				return (-1);
		}
		usleep(500);
	}
}
