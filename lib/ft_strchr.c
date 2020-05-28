/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strchr.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: wpark <marvin@42.fr>                       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/10/08 12:34:43 by wpark             #+#    #+#             */
/*   Updated: 2019/10/11 12:30:39 by wpark            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

/*
** Return
** not found c  : NULL
** if c is '\0' : '\0'
*/

char	*ft_strchr(const char *s, int c)
{
	char	*ret;

	ret = (char*)s;
	while (*ret != c)
	{
		if (*ret == '\0')
			return (0);
		ret++;
	}
	return (ret);
}
