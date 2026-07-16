#include "codexion.h"

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
	int j = 0;
	while (j < simulation->n_coders)
	{
		int left_idx = simulation->coders[j].left - simulation->dongles;
		int right_idx = simulation->coders[j].right - simulation->dongles;
		printf("coder %d -> left = dongle[%d], right = dongle[%d]\n",
			simulation->coders[j].id, left_idx, right_idx);
		j++;
	}
	printf("######################-->> next step sumlation <<--###################");
	// the insitail statue of the beging of the sumulation befor any tread created 
	simulation->start_time = fr_get_time_ms();
	ft_clean_evrithing(simulation);
	return (0);
}