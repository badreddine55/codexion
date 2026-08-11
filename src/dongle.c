#include "codexion.h"

static int	wait_scheduler_event(t_sim *sim)
{
	struct timespec	ts;

	if (clock_gettime(CLOCK_REALTIME, &ts) != 0)
		return (-1);
	ts.tv_nsec += 2000000;
	if (ts.tv_nsec >= 1000000000)
	{
		ts.tv_sec += 1;
		ts.tv_nsec -= 1000000000;
	}
	pthread_cond_timedwait(&sim->scheduler_cond, &sim->scheduler_lock, &ts);
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
	if (queue_push(&coder->left->queue, coder->id) == -1)
		return (-1);
	if (queue_push(&coder->right->queue, coder->id) == -1)
	{
		queue_remove_id(&coder->left->queue, coder->id);
		return (-1);
	}
	return (0);
}

int	fr_both_free(t_coder *coder)
{
	long	now;

	now = fr_get_time_ms();

	if (coder->left == coder->right)
		return (0);
	if (coder->left->owner != NULL || coder->right->owner != NULL)
		return (0);
	if (now < coder->left->released_at + coder->sim->dongle_cooldown)
		return (0);
	if (now < coder->right->released_at + coder->sim->dongle_cooldown)
		return (0);
	return (1);
}

int	fr_has_priority(t_coder *coder)
{
	if (strcmp(coder->sim->scheduler, "fifo") == 0)
	{
		if (queue_front(&coder->left->queue) != coder->id)
			return (0);
		if (queue_front(&coder->right->queue) != coder->id)
			return (0);
		return (1);
	}
	if (!fr_earliest_in_queue(coder->sim, &coder->left->queue, coder->id))
		return (0);
	if (!fr_earliest_in_queue(coder->sim, &coder->right->queue, coder->id))
		return (0);
	return (1);
}

int	request_dongle(t_coder *coder)
{
	pthread_mutex_lock(&coder->sim->scheduler_lock);
	if (push_id_to_dongles(coder) == -1)
	{
		pthread_mutex_unlock(&coder->sim->scheduler_lock);
		return (-1);
	}
	while (1)
	{
		if (fr_check_stop(coder->sim))
		{
			queue_remove_id(&coder->left->queue, coder->id);
			queue_remove_id(&coder->right->queue, coder->id);
			pthread_mutex_unlock(&coder->sim->scheduler_lock);
			return (-1);
		}
		if (fr_both_free(coder) && fr_has_priority(coder))
		{
			coder->left->owner = coder;
			coder->right->owner = coder;
			queue_remove_id(&coder->left->queue, coder->id);
			queue_remove_id(&coder->right->queue, coder->id);
			fr_log(coder, "has taken a dongle");
			fr_log(coder, "has taken a dongle");
			pthread_mutex_unlock(&coder->sim->scheduler_lock);
			return (0);
		}
		wait_scheduler_event(coder->sim);
	}
}

void	fr_release_dongles(t_coder *coder)
{
	long	now;

	pthread_mutex_lock(&coder->sim->scheduler_lock);
	now = fr_get_time_ms();
	coder->left->owner = NULL;
	coder->left->released_at = now;
	coder->right->owner = NULL;
	coder->right->released_at = now;
	pthread_cond_broadcast(&coder->sim->scheduler_cond);
	pthread_mutex_unlock(&coder->sim->scheduler_lock);
}
