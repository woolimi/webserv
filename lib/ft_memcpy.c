/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_memcpy.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: wpark <marvin@42.fr>                       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/10/07 15:15:47 by wpark             #+#    #+#             */
/*   Updated: 2019/10/11 14:17:15 by wpark            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

void	*ft_memcpy(void *dst, const void *src, size_t n)
{
	unsigned int	i;
	unsigned char	*str_dst;
	unsigned char	*str_src;

	if (dst == 0)
		return (0);
	str_dst = (unsigned char*)dst;
	str_src = (unsigned char*)src;
	i = 0;
	while (i < n)
	{
		*str_dst++ = *str_src++;
		i++;
	}
	return (dst);
}
