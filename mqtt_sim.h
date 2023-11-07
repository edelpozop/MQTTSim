#include <simgrid/actor.h>
#include <simgrid/engine.h>
#include <simgrid/host.h>
#include <simgrid/mailbox.h>
#include "simgrid/zone.h"
#include "simgrid/comm.h"
#include "xbt/dict.h"
#include <xbt/sysdep.h>

// Definición de la estructura de envío de paquetes
typedef struct {
	int op;				//0-connect, 1-disconnect, 2-publish, 3-subscribe, 5-end
	int qos;
	char mbox[128];
	char topic[128];
	char data[128];
} MQTTPackage;



int mqtt_connect(int qos, sg_mailbox_t source, sg_mailbox_t dest);
void mqtt_disconnect(int qos, sg_mailbox_t source, sg_mailbox_t dest);
void mqtt_publish(int qos, sg_mailbox_t source, sg_mailbox_t dest, char* topic, char* payload);
void mqtt_publish_b(int qos, sg_mailbox_t source, char* dest, char* topic, char* payload);
void mqtt_subscribe(int qos, sg_mailbox_t source, char* dest, sg_mailbox_t dest, char* topic);