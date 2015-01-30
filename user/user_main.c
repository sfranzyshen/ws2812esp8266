#include "mem.h"
#include "user_interface.h"
#include "espconn.h"

#define procTaskPrio        0
#define procTaskQueueLen    1


static volatile os_timer_t some_timer;
static struct espconn *pUdpServer;

//Tasks that happen all the time.
os_event_t procTaskQueue[procTaskQueueLen];
static void ICACHE_FLASH_ATTR procTask(os_event_t *events)
{
	system_os_post(procTaskPrio, 0, 0 );
	if( events->sig == 0 && events->par == 0 )
	{
		//Idle Event.
	}
}

//Called when new packet comes in.
static void ICACHE_FLASH_ATTR udpserver_recv(void *arg, char *pusrdata, unsigned short len)
{
	struct espconn *pespconn = (struct espconn *)arg;
	WS2812OutBuffer( pusrdata, len );
}

void user_init(void)
{
	int wifiMode = wifi_get_opmode();

	wifi_set_opmode( 2 ); //We broadcast our ESSID, wait for peopel to join.

        pUdpServer = (struct espconn *)os_zalloc(sizeof(struct espconn));
	ets_memset( pUdpServer, 0, sizeof( struct espconn ) );
	espconn_create( pUdpServer );
	pUdpServer->type = ESPCONN_UDP;
	pUdpServer->proto.udp = (esp_udp *)os_zalloc(sizeof(esp_udp));
	pUdpServer->proto.udp->local_port = 7777;
	espconn_regist_recvcb(pUdpServer, udpserver_recv);

	wifi_station_dhcpc_start();

	if( espconn_create( pUdpServer ) )
	{
		while(1) ;
	}

	ets_wdt_disable();
	char outbuffer[] = { 0x00, 0x00, 0x00 };
	WS2812OutBuffer( outbuffer, 1 ); //Initialize the output.

	//Add a process
	system_os_task(procTask, procTaskPrio, procTaskQueue, procTaskQueueLen);
	WS2812OutBuffer( outbuffer, sizeof(outbuffer) );
	system_os_post(procTaskPrio, 0, 0 );
}


