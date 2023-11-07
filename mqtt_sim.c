#include "mqtt_sim.h"


int mqtt_connect(int qos, sg_mailbox_t source, sg_mailbox_t dest)
{
	/*Broker Connection*/
	MQTTPackage* connectionBroker 	= (MQTTPackage*) xbt_malloc(sizeof(MQTTPackage));
	connectionBroker->op 			= 0;
	connectionBroker->qos 			= qos;
	sprintf(connectionBroker->mbox, "%s", source);
    sg_comm_t comm_send_Conn       	= sg_mailbox_put_async(dest, connectionBroker, 0);

    printf("Waiting ACK in %s\n", sg_host_get_name(sg_host_self()));

    switch (sg_comm_wait_for(comm_send_Conn, 1.0)) 
    {
	case SG_ERROR_NETWORK:
		xbt_free(connectionBroker);
		break;
	case SG_ERROR_TIMEOUT:
		xbt_free(connectionBroker);
		break;
	case SG_OK:
		/* nothing */
		break;
	default:
		printf("Unexpected behavior with '%s'\n", sg_host_get_name(sg_host_self()));
    }

    char* msgACK = sg_mailbox_get(source);
	if (strcmp(msgACK, "Connected") != 0)
	{
		printf("Error in %s\n", sg_host_get_name(sg_host_self()));
		return 1;
	}

	printf("%s connected\n", sg_host_get_name(sg_host_self()));
	return 0;
}


void mqtt_disconnect(int qos, sg_mailbox_t source, sg_mailbox_t dest)
{
	/*Broker Disconnection*/
	MQTTPackage* disconnectionBroker = (MQTTPackage*) xbt_malloc(sizeof(MQTTPackage));
	disconnectionBroker->op = 1;
	disconnectionBroker->qos = qos;
	sprintf(disconnectionBroker->mbox,"%s", source);

    sg_comm_t comm_send_Disc       = sg_mailbox_put_async (dest, disconnectionBroker, 0);

    switch (sg_comm_wait_for(comm_send_Disc, 1.0)) {
      case SG_ERROR_NETWORK:
        xbt_free(disconnectionBroker);
        break;
      case SG_ERROR_TIMEOUT:
        xbt_free(disconnectionBroker);
        break;
      case SG_OK:
        /* nothing */
        break;
      default:
        printf("Unexpected behavior with '%s'\n", sg_host_get_name(sg_host_self()));
    }

	char* msgACK_disc = sg_mailbox_get(source);
	if (strcmp(msgACK_disc, "Disconnected") != 0)
	{
		printf("Error in %s\n", sg_host_get_name(sg_host_self()));
		return 1;
	}

	printf("%s disconnected\n", sg_host_get_name(sg_host_self()));
	return 0;
}


void mqtt_disconnectAll_b(int id_cluster, int nodes_fog)
{
	char destEnd[128];
	for (int i = 0; i < nodes_fog; i++)
	{
		sprintf(destEnd, "fog-%d-%d", id_cluster, i);

		MQTTPackage* payloadBroker = (MQTTPackage*) xbt_malloc(sizeof(MQTTPackage));
		payloadBroker->op = 1;
		sg_mailbox_t mbox_dest			= sg_mailbox_by_name(destEnd);
    	sg_comm_t comm_send_fog       	= sg_mailbox_put_async(mbox_dest, payloadBroker, 0);

    	switch (sg_comm_wait_for(comm_send_fog, 1.0)) {
		case SG_ERROR_NETWORK:
			xbt_free(payloadBroker);
			break;
		case SG_ERROR_TIMEOUT:
			xbt_free(payloadBroker);
			break;
		case SG_OK:
			/* nothing */
			break;
		default:
			printf("Unexpected behavior with '%s'\n", broker_name);
	    }
	}
}


void mqtt_publish(int qos, sg_mailbox_t source, sg_mailbox_t dest, char* topic, char* payload)
{
	MQTTPackage* payload = (MQTTPackage*) xbt_malloc(sizeof(MQTTPackage));
	payload->op = 2;
	payload->qos = qos;
	sprintf(payload->topic,"%s", topic);
	sprintf(payload->data,"%s", payload);
	sprintf(payload->mbox,"%s", source);
	
	printf("publish - %d\t%s\t%s\t%s\n", qos, payload->topic, payload->data, payload->mbox);
    sg_comm_t comm_send_Pub       = sg_mailbox_put_async(dest, payload, 0);

    switch (sg_comm_wait_for(comm_send_Pub, 1.0)) 
    {
      case SG_ERROR_NETWORK:
        xbt_free(payload);
        break;
      case SG_ERROR_TIMEOUT:
        xbt_free(payload);
        break;
      case SG_OK:
        /* nothing */
        break;
      default:
        printf("Unexpected behavior with '%s'\n", sg_host_get_name(sg_host_self()));
    }

    printf("Message sent\n");
}


void mqtt_publish_b(int qos, sg_mailbox_t source, char* dest, char* topic, char* payload)
{
	MQTTPackage* payloadBroker = (MQTTPackage*) xbt_malloc(sizeof(MQTTPackage));
	payloadBroker->op = 2;
	payloadBroker->qos = qos;

	// Buscar aquellos que se han suscrito para reenviarlo

	sprintf(payloadBroker->topic,"%s", topic);
	sprintf(payloadBroker->data,"%s", data);
	sprintf(payloadBroker->mbox,"%s", source);

	sg_mailbox_t mbox_dest			= sg_mailbox_by_name(dest);
    sg_comm_t comm_send_fog       	= sg_mailbox_put_async(mbox_dest, payloadBroker, 0);

    switch (sg_comm_wait_for(comm_send_fog, 1.0)) {
	case SG_ERROR_NETWORK:
		xbt_free(payloadBroker);
		break;
	case SG_ERROR_TIMEOUT:
		xbt_free(payloadBroker);
		break;
	case SG_OK:
		/* nothing */
		break;
	default:
		printf("Unexpected behavior with '%s'\n", broker_name);
    }
}


int mqtt_subscribe(int qos, sg_mailbox_t source, char* dest, sg_mailbox_t dest, char* topic)
{
	MQTTPackage* subscriptionBroker 			= (MQTTPackage*) xbt_malloc(sizeof(MQTTPackage));
	subscriptionBroker->op 						= 3;
	sprintf(subscriptionBroker->mbox, "%s", source);
	sprintf(subscriptionBroker->topic, "%s", topic);
    sg_comm_t comm       						= sg_mailbox_put_async(dest, subscriptionBroker, 0);

    switch (sg_comm_wait_for(comm, 1.0)) 
    {
	case SG_ERROR_NETWORK:
		xbt_free(subscriptionBroker);
		break;
	case SG_ERROR_TIMEOUT:
		xbt_free(subscriptionBroker);
		break;
	case SG_OK:
		/* nothing */
		break;
	default:
		printf("Unexpected behavior with '%s'\n", fog_name);
		return 1;
    }

    char* msgACK_sub = sg_mailbox_get(source);
   
	if (strcmp(msgACK_sub, "Subscribed") != 0)
	{
		printf("Error\n");
		return 1;
	}
	printf("Fog subscribed\n");
	return 0;
}
