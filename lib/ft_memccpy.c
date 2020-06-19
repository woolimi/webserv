/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_memccpy.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: wpark <marvin@42.fr>                       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/10/07 16:07:07 by wpark             #+#    #+#             */
/*   Updated: 2019/10/11 10:31:10 by wpark            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

void	*ft_memccpy(void *dst, const void *src, int c, size_t n)
{
	unsigned char	stop_c;
	unsigned int	i;
	char			*str_dst;
	char			*str_src;

	stop_c = (unsigned char)c;
	str_dst = (char*)dst;
	str_src = (char*)src;
	i = 0;
	while (i < n)
	{
		*(str_dst + i) = *(str_src + i);
		if ((unsigned char)*(str_src + i) == stop_c)
			return (dst + i + 1);
		i++;
	}
	return (0);
}
