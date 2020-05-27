/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_calloc.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: wpark <marvin@42.fr>                       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/10/11 11:13:45 by wpark             #+#    #+#             */
/*   Updated: 2019/10/11 11:23:30 by wpark            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

void	*ft_calloc(size_t count, size_t size)
{
	unsigned char	*ptr;

	if (!(ptr = (unsigned char*)malloc(size * count)))
		return (0);
	ft_memset(ptr, 0, size * count);
	return ((void*)ptr);
}
