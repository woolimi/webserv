/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strrchr.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: wpark <marvin@42.fr>                       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/10/08 12:36:15 by wpark             #+#    #+#             */
/*   Updated: 2019/10/08 13:19:37 by wpark            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

char	*ft_strrchr(const char *s, int c)
{
	char	*start;
	char	*cur;

	start = (char*)s;
	cur = start;
	while (*cur != '\0')
		cur++;
	while (cur >= start)
	{
		if (*cur == (char)c)
			return (cur);
		cur--;
	}
	return (0);
}
