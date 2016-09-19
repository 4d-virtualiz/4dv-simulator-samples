#ifndef PACKET_H
#define PACKET_H

#include <stdlib.h>


/**
 * //////////////////////////////
 * //////////////////////////////
 */
typedef struct vz_packet
{
    char   buffer[0x1000];
    size_t packetLength;
    int    poolIndex;

    struct vz_packet* vz_next;
    struct vz_packet* vz_previous;

} vz_packet_t;

/**
 * //////////////////////////////
 * //////////////////////////////
 */
typedef struct vz_packet_list
{
    struct vz_packet* vz_begin;  /* first element of list */
    struct vz_packet* vz_end;    /* last element of list */
} vz_packet_list_t;

/**
 * //////////////////////////////
 * //////////////////////////////
 */
#define VZ_PACKET_POOL_SIZE (unsigned)0x100

typedef struct vz_packet_pool
{
    vz_packet_t   packets[VZ_PACKET_POOL_SIZE];   /* packets static list */
    int           used[VZ_PACKET_POOL_SIZE];      /* 'used' flags */

} vz_packet_pool_t;


typedef struct vz_packet_entry {

    char      name[0x100];    /* name of entry */
    unsigned  length;         /* number of elements (values) */
    char      type[0x100];    /* type of entry */
    char      values[0x40][0x100];    /* values */

} vz_packet_entry_t;


typedef struct vz_packet_raw {

  vz_packet_entry_t entries[0x20];   /* entries list */
  int               entryCount;       /* entries count */

} vz_packet_raw_t;



/**
 * //////////////////////////////
 * //////////////////////////////
 */
void vz_packet_destroy(vz_packet_t* inPacket);
int  vz_packet_pool_take_id(vz_packet_pool_t* inPacketPool);
void vz_packet_pool_release_id(vz_packet_pool_t* inPacketPool,
                               int releaseId);
vz_packet_list_t* vz_packet_list_create();
void vz_packet_list_destroy(vz_packet_list_t* inPacketList);
void vz_packet_list_init(vz_packet_list_t* inPacketList);
void vz_packet_destroy(vz_packet_t* inPacket);
void vz_packet_init(vz_packet_t* inPacket);
vz_packet_t* vz_packet_create();
void  vz_packet_pool_initialize(vz_packet_pool_t* inPacketPool);
void vz_packet_list_push_back(vz_packet_list_t* inPacketList,
                              vz_packet_t* inPacket);
vz_packet_t* vz_packet_list_pop_front(vz_packet_list_t* inPacketList);
void vz_packet_list_clear(vz_packet_list_t* inPacketList);
void vz_packet_entry_parse(char* inBuffer,
                           vz_packet_entry_t* rfPacketEntry);
void vz_packet_decode(vz_packet_t* inPacket,
                      vz_packet_raw_t* rfRawInfo);



#endif
