#include <simgrid/actor.h>
#include <simgrid/engine.h>
#include <simgrid/host.h>
#include <simgrid/mailbox.h>
#include <simgrid/plugins/energy.h>
#include <simgrid/zone.h>
#include <simgrid/comm.h>
#include <xbt/dict.h>
#include <xbt/sysdep.h>

#include "subscriptions.h"

#define MAX_PAYLOAD_SIZE (1024) 
#define TOTAL_EDGE0 10
#define TOTAL_EDGE1 10
#define TOTAL_EDGE2 10
#define TOTAL_EDGE3 10
#define TOTAL_FOG0 1
#define TOTAL_FOG1 1
#define TOTAL_FOG2 1
#define TOTAL_FOG3 1
#define TOTAL_MESSAGES 100

// Definición de la estructura de envío de paquetes
typedef struct {
	int op;				//0-connect, 1-disconnect/end, 2-publish, 3-subscribe
	int qos;
	sg_mailbox_t mbox;
	char topic[128];
	char data[128];
	int payloadlen;
} MQTTPackage;



int mqtt_connect 			(int qos, sg_mailbox_t source, sg_mailbox_t dest);
void mqtt_disconnect 		(int qos, sg_mailbox_t source, sg_mailbox_t dest);
void mqtt_disconnectAll_b 	(int id_cluster, int nodes_fog);
int mqtt_publish 			(int qos, sg_mailbox_t source, sg_mailbox_t dest, char* topic, char* payload, int payloadlen);
void mqtt_publish_b 		(int qos, sg_mailbox_t source, char* topic, char* payload, int payloadlen, Sub* subList);
int mqtt_subscribe 			(int qos, sg_mailbox_t source, sg_mailbox_t dest, char* topic);
void broker_run 			(sg_mailbox_t mbox, int id_cluster_fog, int nodes_fog, int active_devices);