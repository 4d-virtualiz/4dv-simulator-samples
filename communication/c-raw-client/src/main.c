#include "packet.h"
#include "utils.h"
#include "global.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>


/**
 * /////////////////////////////////////
 * /////////////////////////////////////
 * \brief Prints usage of program
 */
void usage(char** argv)
{
  fprintf(stdout, "USAGE :      %s IP_ADDR PORT\n", argv[0]);
  exit(EXIT_FAILURE);
}


/**
 * /////////////////////////////////////
 * /////////////////////////////////////
    Do low level look up for
      address required.

 * \brief Retreives addrinfo
   \param inAddr If addr given is empty, then assumed local.
 */
void ll_getaddrinfo(const char* inPort,
                    const char* inAddr,
                    struct sockaddr_in** rfRes,
                    struct addrinfo** rfLookupRes,
                    struct addrinfo** rfAddrInfo)
{
  int gaiErrCode = -1;
  struct addrinfo hints;
  struct addrinfo* res = NULL;
  struct addrinfo* it = NULL;
  const char* addr = NULL;
  const char* port = NULL;

  assert(inPort != NULL);
  assert(inAddr != NULL);
  assert(rfRes != NULL);
  assert(rfLookupRes != NULL);
  assert(rfAddrInfo != NULL);

  *rfRes = NULL;
  *rfLookupRes = NULL;
  *rfAddrInfo = NULL;

  memset(&hints, 0, sizeof(struct addrinfo));

  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = 0;
  hints.ai_protocol = 0;

  if (strlen(inPort) > 0)
    port = inPort;

  if (strlen(inAddr) > 0)
  {
    addr = inAddr;
  }
  else
  {
    hints.ai_flags = AI_PASSIVE;
  }

  if ((gaiErrCode = getaddrinfo(addr,
                                port,
                                &hints,
                                &res))  != 0)
  {
    fprintf(stderr,
            "Error while calling getaddrinfo with %s: %s\n",
            addr,
            gai_strerror(gaiErrCode));
    if (gaiErrCode == EAI_SYSTEM)
    {
      perror("getaddrinfo call");
    }
    ERR_EXIT("Aborting");
  }
  else
  {
    *rfLookupRes = res;
    /* scanning results */
    for (it = res ; it != NULL ; it = it->ai_next)
    {
      if (it->ai_family == AF_INET)
      {
        /* address found, we take the first found */
        *rfRes = (struct sockaddr_in *)it->ai_addr;
        *rfAddrInfo = it;
        break;
      }
    }
  }
}


/**
 * /////////////////////////////////////
 * /////////////////////////////////////
   Do low level socket allocation.
 */
void ll_socket(struct addrinfo* rfAddrInfo,
               int* rfSocketFd)
{
  int sock = -1;

  assert(rfAddrInfo != NULL);
  assert(rfSocketFd != NULL);

  if ((sock = socket(rfAddrInfo->ai_family,
                     rfAddrInfo->ai_socktype,
                     rfAddrInfo->ai_protocol)) == -1)
  {
    perror("socket");
    ERR_EXIT("Aborting");
  }
  else
  {
    *rfSocketFd = sock;
  }
}


/**
 * /////////////////////////////////////
 * /////////////////////////////////////
    Do low level connection to server (simulator).
 */
void ll_connect(struct addrinfo* rfAddrInfo,
                int inSocketFd)
{
  if (connect(inSocketFd,
              rfAddrInfo->ai_addr,
              rfAddrInfo->ai_addrlen) != 0)
  {
    perror("connect");
    ERR_EXIT("Aborting");
  }
}

/**
 * /////////////////////////////////////
 * /////////////////////////////////////
    Parses low level raw packet from
      buffer read on socket.
 */
void receptionParseRawPackets(char* inBuffer,
                               size_t inLength)
{
  int i = 0;
  vz_token_result_t tokenResult;

  vz_strtok_imp(inBuffer, inLength, &tokenResult);

  for (i = 0 ; i < tokenResult.tokenCount ; i++)
  {
    if (ginfo.unfinishedPacket != NULL)
    {
      memcpy(&ginfo.unfinishedPacket->buffer[ginfo.unfinishedPacket->packetLength],
          tokenResult.tokens[i],
          tokenResult.tokenSizes[i]);

      vz_packet_list_push_back(ginfo.packetList,
                               ginfo.unfinishedPacket);
      ginfo.unfinishedPacket = vz_packet_create();
    }
  }

}

/**
 * /////////////////////////////////////
 * /////////////////////////////////////
    Simple helper to execute callback
 */
void packet_decode_execute_callback(vz_packet_raw_t* inRawInfo)
{
  assert(inRawInfo != NULL);

  if (ginfo.packetDecodeCallback != NULL)
  {
    ginfo.packetDecodeCallback(inRawInfo);
  }
}

/**
 * /////////////////////////////////////
 * /////////////////////////////////////
 *  From the list of received packet,
 *  performs decoding to vz_raw_packet_t
 */
void receptionDecodePackets()
{
  vz_packet_t* current = NULL;
  vz_packet_raw_t localPacket;


  for (current = ginfo.packetList->vz_begin ; current != NULL ; current = current->vz_next)
  {
    /* decode packet */
    vz_packet_decode(current,
                     &localPacket);

    packet_decode_execute_callback(&localPacket);
  }

  /* clear list */
  vz_packet_list_clear(ginfo.packetList);
}




/**
 * /////////////////////////////////////
 * /////////////////////////////////////
 *  Executes read operation on socket,
 *  packet parsing, and raw packet decoding.
 *  Executes the callback set in ginfo.packetDecodeCallback
 *  for each packet decoded.
 */
int reception()
{
  int statusCode = 1;
  char buffer[0x1000];
  size_t len = 0;

  /* reset buffer */
  memset(buffer, 0, 0x1000 * sizeof(char));

  /* reception blocking call */
  len = recv(ginfo.socket_fd,
             buffer,
             0x1000 - 8,
             0);

  if (len == (size_t)-1)
  {
    /* error */
    statusCode = -1;
    perror("reception: recv");
  }
  else if (len == 0)
  {
    /* standard shutdown */
    statusCode = 0;
    fprintf(stdout, "Remote side closed connection.\n");
  }
  else
  {
    /* recv done */
    statusCode = 1;

    /* parsing packets */
    receptionParseRawPackets(buffer, len);

    /* decoding packets */
    receptionDecodePackets();

  }


  return statusCode;
}

/**
 * /////////////////////////////////////
 * /////////////////////////////////////
 *  Executes read on socket until connection
 *  is closed by server (simulator) or error occured.
 */
void receptionLoop()
{
  int stopped = 0;
  int receptionStatus = -1;

  while (!stopped)
  {
    receptionStatus = reception();

    if (   receptionStatus == 0
           || receptionStatus == -1)
    {
      stopped = 1;
    }
    else
    {
      ;;
    }
  }
}


/**
 * /////////////////////////////////////
 * /////////////////////////////////////
 *  This is an example of a raw packet reception
 *  callback.
 */
void myCallback(vz_packet_raw_t* inRawInfo)
{
  int i = 0;
  char tstamp[0x100];
  char e0[0x100];

  assert(inRawInfo != NULL);

  memset(tstamp, 0, 0x100);
  memset(e0, 0, 0x100);

  for (i = 0 ; i < inRawInfo->entryCount ; i++)
  {
    if (vz_streq(inRawInfo->entries[i].name, "TimeStamp"))
    {
      /* copy tstamp to string */
      strcpy(tstamp, inRawInfo->entries[i].values[0]);
    }
    else if (vz_streq(inRawInfo->entries[i].name, "e0"))
    {
      /* copy first value of e0 to string */
      strcpy(e0, inRawInfo->entries[i].values[0]);
    }
  }

  /* example of data usage code */
  fprintf(stdout, "New data found ! TimeStamp=%s e0[0]=%s\n", tstamp, e0);
}



/**
 * /////////////////////////////////////
 * /////////////////////////////////////
 */
int myProgram(int argc, char** argv)
{
  memset(&ginfo, 0, sizeof(struct global_info));

  if (argc < 3)
  {
    usage(argv);
  }
  else
  {
    strcpy(ginfo.ipAddrStrRemote, argv[1]);
    strcpy(ginfo.portStrRemote, argv[2]);

    /* retrieving remote addr info */
    ll_getaddrinfo(ginfo.portStrRemote,
                   ginfo.ipAddrStrRemote,
                   &ginfo.remoteSockAddr,
                   &ginfo.remoteLookupRes,
                   &ginfo.remoteAddrInfo);


    /* creating local socket */
    ll_socket(ginfo.remoteAddrInfo,
              &ginfo.socket_fd);

    /* connecting... */
    ll_connect(ginfo.remoteAddrInfo,
               ginfo.socket_fd);

    /* initializing packet pool */
    vz_packet_pool_initialize(&ginfo.packetPool);

    /* initializing packet list  */
    ginfo.packetList = vz_packet_list_create();
    vz_packet_list_init(ginfo.packetList);
    ginfo.unfinishedPacket = vz_packet_create();
    vz_packet_init(ginfo.unfinishedPacket);

    /* setting callback ptr */
    ginfo.packetDecodeCallback = myCallback;

    /* starting reception loop */
    receptionLoop();

  }

  return EXIT_SUCCESS;
}


/**
 * /////////////////////////////////////
 * /////////////////////////////////////
 * \brief Called at exit.
 
 */
void exitRoutine(void)
{
  if (ginfo.socket_fd != 0)
  {
    close(ginfo.socket_fd);
  }

  if (ginfo.remoteLookupRes != NULL)
  {
    freeaddrinfo(ginfo.remoteLookupRes);
  }
}


/**
 * /////////////////////////////////////
 * /////////////////////////////////////
 * \brief Entrypoint
 
 */
int main(int argc, char** argv)
{
  atexit(exitRoutine);
  return myProgram(argc, argv);
}
