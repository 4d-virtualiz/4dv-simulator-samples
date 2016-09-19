#ifndef GLOBAL_H
#define GLOBAL_H

#include "packet.h"


/**
   Decode raw packet callback type
 */
typedef void (*packet_decode_callback_t)(vz_packet_raw_t*);

/**
 * /////////////////////////////////////
 * /////////////////////////////////////
 */
struct global_info
{
  /* socket file descriptor of TCP connection */
  int socket_fd;

  char ipAddrStrRemote[0x1000];
  char portStrRemote[0x1000];
  struct sockaddr_in* remoteSockAddr;
  struct addrinfo* remoteLookupRes;

  /* addr used for TCP connection*/
  struct addrinfo* remoteAddrInfo;

  /* current parsed packet */
  vz_packet_t*        unfinishedPacket;

  /* previously parsed packet to decode */
  vz_packet_list_t*   packetList;

  /* callback to execute when 'protocol raw' packet is decoded */
  packet_decode_callback_t packetDecodeCallback;

  /* packet pool */
  vz_packet_pool_t    packetPool;
};


extern struct global_info  ginfo;

#endif
