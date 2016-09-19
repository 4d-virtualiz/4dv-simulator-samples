#include "packet.h"

#include "global.h"
#include "utils.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>


/**
 * //////////////////////////////
 * //////////////////////////////
    Takes a packet from the pool.
 */
int vz_packet_pool_take_id(vz_packet_pool_t* inPacketPool)
{
  int ret = -1;
  int i = 0;

  assert(inPacketPool != NULL);

  for (i = 0 ; i < 0x100 ; i++)
  {
    if (!inPacketPool->used[i])
    {
      ret = i;
      break;
    }
  }

  if (ret != -1)
  {
    inPacketPool->used[i] = 1;
  }

  return ret;
}


/**
 * //////////////////////////////
 * //////////////////////////////
   Releases packet with id given
   to the pool.
 */
void vz_packet_pool_release_id(vz_packet_pool_t* inPacketPool,
                               int releaseId)
{
  assert(inPacketPool != NULL);
  inPacketPool->used[releaseId] = 0;
}

/**
 * /////////////////////////////////////
 * /////////////////////////////////////
 * Creates packet list on heap.
 */
vz_packet_list_t* vz_packet_list_create()
{
  vz_packet_list_t* packetList = NULL;

  if ((packetList = malloc(sizeof(vz_packet_list_t))) == NULL)
  {
    perror("malloc");
    ERR_EXIT("malloc error");
  }

  return packetList;
}

/**
 * /////////////////////////////////////
 * /////////////////////////////////////
 * Destroys packet list content, clear
 * list, and destroy packet list.
 */
void vz_packet_list_destroy(vz_packet_list_t* inPacketList)
{
  vz_packet_t* it = NULL;
  vz_packet_t* it_next = NULL;

  for (it = inPacketList->vz_begin ; it != NULL ;)
  {
    it_next = it->vz_next;
    vz_packet_destroy(it);
    it = it_next;
  }

  assert(inPacketList != NULL);
  free(inPacketList);
}

/**
 * /////////////////////////////////////
 * /////////////////////////////////////
 * Initializes packet list
 */
void vz_packet_list_init(vz_packet_list_t* inPacketList)
{
  assert(inPacketList != NULL);
  inPacketList->vz_begin = NULL;
  inPacketList->vz_end = NULL;
}

/**
 * /////////////////////////////////////
 * /////////////////////////////////////
 * Destroys packet
 */
void vz_packet_destroy(vz_packet_t* inPacket)
{
  assert(inPacket != NULL);

  vz_packet_pool_release_id(&ginfo.packetPool,
                            inPacket->poolIndex);
}

/**
 * /////////////////////////////////////
 * /////////////////////////////////////
   Initializes packet
 */
void vz_packet_init(vz_packet_t* inPacket)
{
  assert(inPacket != NULL);
  memset(inPacket, 0, sizeof(vz_packet_t));
}


/**
 * /////////////////////////////////////
 * /////////////////////////////////////
 * Creates packet
 */
vz_packet_t* vz_packet_create()
{
  vz_packet_t* packet = NULL;

  int poolId = vz_packet_pool_take_id(&ginfo.packetPool);

  if (poolId == -1)
  {
    ERR_EXIT("Max pool size must be readjusted.");
  }

  packet = &ginfo.packetPool.packets[poolId];
  vz_packet_init(packet);
  packet->poolIndex = poolId;

  return packet;
}


/**
 * /////////////////////////////////////
 * /////////////////////////////////////
 * Initialize packet pool
 */
void  vz_packet_pool_initialize(vz_packet_pool_t* inPacketPool)
{
  int i = 0;

  assert(inPacketPool != NULL);

  for (i = 0 ; i < 0x100 ; i++)
  {
    vz_packet_init(&inPacketPool->packets[i]);
    inPacketPool->packets[i].poolIndex = i;
    inPacketPool->used[i] = 0;
  }
}


/**
 * /////////////////////////////////////
 * /////////////////////////////////////
 * Insert packet at the end of list.
 */
void vz_packet_list_push_back(vz_packet_list_t* inPacketList,
                              vz_packet_t* inPacket)
{
  vz_packet_t* lastElement = NULL;

  assert(inPacketList != NULL);
  assert(inPacket != NULL);

  lastElement = inPacketList->vz_end;

  if (lastElement == NULL)
  {
    inPacketList->vz_begin = inPacket;
    inPacketList->vz_end = inPacket;
  }
  else
  {
    inPacketList->vz_end = inPacket;
    lastElement->vz_next = inPacket;
    inPacket->vz_previous = lastElement;
  }
}

/**
 * /////////////////////////////////////
 * /////////////////////////////////////
 * Insert packet at the end of list.
 */
vz_packet_t* vz_packet_list_pop_front(vz_packet_list_t* inPacketList)
{
  vz_packet_t* front = NULL;
  assert(inPacketList != NULL);
  front = inPacketList->vz_begin;

  if (front != NULL)
  {
    vz_packet_t* nextFront = front->vz_next;

    if (nextFront != NULL)
    {
      nextFront->vz_previous = NULL;
    }

    front->vz_next = NULL;
    inPacketList->vz_begin = nextFront;
  }

  return front;
}

/**
 * /////////////////////////////////////
 * /////////////////////////////////////
 * Clears list, and call vz_packet_destroy on every packet.
 */
void vz_packet_list_clear(vz_packet_list_t* inPacketList)
{
  vz_packet_t* current = NULL;
  vz_packet_t* nextCurrent = NULL;

  assert(inPacketList != NULL);

  for (current = inPacketList->vz_begin ; current != NULL ; )
  {
    nextCurrent = current->vz_next;
    current->vz_previous = NULL;
    current->vz_next = NULL;
    vz_packet_destroy(current);
    current = nextCurrent;
  }

  inPacketList->vz_begin = NULL;
  inPacketList->vz_end = NULL;
}

/**
 * /////////////////////////////////////
 * /////////////////////////////////////
    From a line scanned in packet,
    generate a packet_entry_t.
    Separator used here is ' ' (whitespace).
*/
void vz_packet_entry_parse(char* inBuffer,
                           vz_packet_entry_t* rfPacketEntry)
{
  char* retTok = NULL;
  char* strTokSave = NULL;
  int tokenCount = 0;
  int valuesIndex = 0;

  assert(inBuffer != NULL);
  assert(rfPacketEntry != NULL);

  memset(rfPacketEntry, 0, sizeof(vz_packet_entry_t));

  retTok = strtok_r(inBuffer, " ", &strTokSave);


  for ( ; retTok != NULL ; tokenCount++)
  {
    if (tokenCount == 0)
    {
      strcpy(rfPacketEntry->name, retTok);
    }

    if (tokenCount == 1)
    {
      rfPacketEntry->length = atoi(retTok);
    }

    if (tokenCount == 2)
    {
      strcpy(rfPacketEntry->type, retTok);
    }

    if (tokenCount >= 3)
    {
      valuesIndex = tokenCount - 3;
      strcpy(rfPacketEntry->values[valuesIndex], retTok);
    }

    retTok = strtok_r(NULL, " ", &strTokSave);
  }
}

/**
 * /////////////////////////////////////
 * /////////////////////////////////////
 * \brief Decodes packet given into
 *        good 'protocol raw' data.
 */
void vz_packet_decode(vz_packet_t* inPacket,
                      vz_packet_raw_t* rfRawInfo)
{
  int packetEntryCount = 0;
  vz_packet_entry_t packet_entry;
  char* retTok = NULL;
  char* strTokSave = NULL;

  assert(inPacket != NULL);
  assert(rfRawInfo != NULL);

  memset(rfRawInfo, 0, sizeof(vz_packet_raw_t));

  retTok = strtok_r(inPacket->buffer, "\n", &strTokSave);

  while (retTok != NULL)
  {
    vz_packet_entry_parse(retTok,
                          &packet_entry);

    memcpy(&rfRawInfo->entries[packetEntryCount],
           &packet_entry,
           sizeof(vz_packet_entry_t));

    packetEntryCount++;
    rfRawInfo->entryCount = packetEntryCount;

    retTok = strtok_r(NULL, "\n", &strTokSave);
  }
}


