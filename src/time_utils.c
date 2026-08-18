/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   time_utils.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: badiyaf <badiyaf@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/16 17:41:59 by badiyaf           #+#    #+#             */
/*   Updated: 2026/08/16 17:43:51 by badiyaf          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

long	fr_get_time_ms(void)
{
	struct timeval	now;
	long			milliseconds;

	gettimeofday(&now, NULL);
	milliseconds = (now.tv_sec * 1000) + (now.tv_usec / 1000);
	return (milliseconds);
}
