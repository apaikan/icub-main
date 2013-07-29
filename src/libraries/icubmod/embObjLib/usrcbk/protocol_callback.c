/*
 * protocol_callback.c
 *
 *  Created on: Jun 6, 2013
 *  Author: Cardellino Alberto
 */

#include <stdint.h>
#include <string.h>

//#if defined(OVERRIDE_eo_receiver_callback_incaseoferror_in_sequencenumberReceived)
extern void eo_receiver_callback_incaseoferror_in_sequencenumberReceived(uint32_t remipv4addr, uint64_t rec_seqnum, uint64_t expected_seqnum)
{
#warning "Check of sequence number active"
   printf("Error in sequence number from 0x%x!!!! \t Expected %llu, received %llu\n", remipv4addr, expected_seqnum, rec_seqnum);
};
//#endif


