#include "codexion.h"
// void *ft_test(void *arg)
// {
// 	t_coder *coder = (t_coder *)arg;
// 	int j = 0;
// 	while (j < coder->sim->compiles_required)
// 	{
// 		push_id_to_dongles(coder);
// 		if(coder->id % 2 == 0)
// 		{
// 			request_left_dongle(coder);
// 			request_right_dongle(coder);
// 		}
// 		else
// 		{
// 			request_right_dongle(coder);
// 			request_left_dongle(coder);
// 		}
// 		usleep(200);
// 		if(coder->id % 2 == 0)
// 		{
// 			release_dongle(coder->left);
// 			usleep(2);
// 			release_dongle(coder->right);
// 		}
// 		else
// 		{
// 			release_dongle(coder->right);
// 			usleep(2);
// 			release_dongle(coder->left);
// 		}
// 		j++;
// 	}
// 	return NULL;
// }
void *coder_cycle(void *arg)
{
	t_coder *coder = (t_coder *)arg;
	int j = 0;
	while (j < coder->sim->compiles_required)
	{
		push_id_to_dongles(coder);
		
		request_right_dongle(coder);
		request_left_dongle(coder);

		coder->last_compile_start = fr_get_time_ms();   // for burnout + EDF deadline
		fr_log(coder, "is compiling");
		usleep(coder->sim->time_to_compile * 1000);

		release_dongle(coder->left);
		release_dongle(coder->right);
		coder->compiles_done++;

		fr_log(coder, "is debugging");
		usleep(coder->sim->time_to_debug * 1000);

		fr_log(coder, "is refactoring");
		usleep(coder->sim->time_to_refactor * 1000);
		j++;
	}
	return NULL;
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
	memset(simulation, 0, sizeof(t_sim));
	pthread_mutex_init(&simulation->log_lock, NULL);
	if (pars_args(ac, av, simulation))
    	return (1);
	if(fr_build_dongle(simulation))
	{
		ft_clean_evrithing(simulation);
		return (1);
	}
	if(fr_build_coder(simulation))
	{
		ft_clean_evrithing(simulation);
		return (1);
	}
	simulation->start_time = fr_get_time_ms();
	int i = 0;
	while (i < simulation->n_coders )
	{
		pthread_create(&simulation->coders[i].coder_thread, NULL, coder_cycle, &simulation->coders[i]);
		i++;
	}
	i = 0;
	while (i < simulation->n_coders )
	{
		pthread_join(simulation->coders[i].coder_thread, NULL);
		i++;
	}
	

	ft_clean_evrithing(simulation);
	return (0);
}