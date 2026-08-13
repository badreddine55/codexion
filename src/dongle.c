#include "codexion.h"

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
		i++;
	}
	return (0);
}

int	push_id_to_dongles(t_coder *coder)
{
	int	ret;

	if (coder->left == coder->right)
	{
		pthread_mutex_lock(&coder->left->lock);
		ret = queue_push(&coder->left->queue, coder->id);
		pthread_mutex_unlock(&coder->left->lock);
		return (ret);
	}
	pthread_mutex_lock(&coder->left->lock);
	ret = queue_push(&coder->left->queue, coder->id);
	pthread_mutex_unlock(&coder->left->lock);
	if (ret == -1)
		return (-1);
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

static int	fr_try_acquire_pair(t_coder *coder)
{
	long		now;
	int			acquired;
	t_dongle	*first;
	t_dongle	*second;

	first = (coder->left < coder->right) ? coder->left : coder->right;
	second = (coder->left < coder->right) ? coder->right : coder->left;
	pthread_mutex_lock(&first->lock);
	pthread_mutex_lock(&second->lock);
	acquired = 0;
	now = fr_get_time_ms();
	if (coder->left->owner == NULL && coder->right->owner == NULL
		&& now >= coder->left->released_at + coder->sim->dongle_cooldown
		&& now >= coder->right->released_at + coder->sim->dongle_cooldown)
	{
		coder->left->owner = coder;
		coder->right->owner = coder;
		queue_remove_id(&coder->left->queue, coder->id);
		queue_remove_id(&coder->right->queue, coder->id);
		acquired = 1;
	}
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
	if (push_id_to_dongles(coder) == -1)
		return (-1);
	while (1)
	{
		if (fr_check_stop(coder->sim))
			return (-1);
		if (fr_has_priority(coder))
		{
			if(fr_try_acquire(coder) == -1)
			{
				return -1;
			}
			return (0);
		}
		usleep(1000);
	}
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