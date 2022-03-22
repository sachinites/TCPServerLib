/*
 * =====================================================================================
 *
 *       Filename:  network_utils.h
 *
 *    Description: This file is an interface for Network Utils 
 *
 *        Version:  1.0
 *        Created:  10/06/2020 04:16:17 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  ABHISHEK SAGAR (), sachinites@gmail.com
 *   Organization:  Juniper Networks
 *
 * =====================================================================================
 */

#ifndef __NETWORK_UTILS__
#define __NETWORK_UTILS__

/* General Nw utilities */
char *
network_convert_ip_n_to_p(uint32_t ip_addr,
                        char *output_buffer);

uint32_t
network_covert_ip_p_to_n(const char *ip_addr);

#endif /* __NETWORK_UTILS__  */
