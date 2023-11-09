#include <simgrid/actor.h>
#include <simgrid/engine.h>
#include <simgrid/host.h>
#include <simgrid/mailbox.h>
#include <simgrid/plugins/energy.h>
#include <simgrid/zone.h>
#include <simgrid/comm.h>
#include <xbt/dict.h>
#include <xbt/sysdep.h>

// Definición de la estructura de envío de paquetes
typedef struct {
	int op;				//0-connect, 1-disconnect/end, 2-publish, 3-subscribe
	int qos;
	char mbox[128];
	char topic[128];
	char data[128];
} MQTTPackage;

#define TOTAL_EDGE 1

int mqtt_connect(int qos, sg_mailbox_t source, sg_mailbox_t dest);
void mqtt_disconnect(int qos, sg_mailbox_t source, sg_mailbox_t dest);
void mqtt_disconnectAll_b(int id_cluster, int nodes_fog);
int mqtt_publish(int qos, sg_mailbox_t source, sg_mailbox_t dest, char* topic, char* payload);
void mqtt_publish_b(int qos, char* source, char* dest, char* topic, char* payload);
int mqtt_subscribe(int qos, sg_mailbox_t source, sg_mailbox_t dest, char* topic);