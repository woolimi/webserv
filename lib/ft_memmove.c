/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_memmove.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: wpark <marvin@42.fr>                       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/10/07 16:35:10 by wpark             #+#    #+#             */
/*   Updated: 2019/10/07 16:36:01 by wpark            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

void	*ft_memmove(void *dst, const void *src, size_t len)
{
	unsigned char	*pd;
	unsigned char	*ps;
	unsigned int	i;

	pd = (unsigned char*)dst;
	ps = (unsigned char*)src;
	i = 0;
	if (ps < pd)
		while (len-- > 0)
			pd[len] = ps[len];
	else
		while (i < len)
		{
			pd[i] = ps[i];
			i++;
		}
	return (dst);
}
