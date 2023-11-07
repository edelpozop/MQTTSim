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
			printf("Unexpected behavior with '%s'\n", sg_host_get_name(sg_host_self()));
	    }
	}
}


int mqtt_publish(int qos, sg_mailbox_t source, sg_mailbox_t dest, char* topic, char* payload)
{
	MQTTPackage* payloadPublish = (MQTTPackage*) xbt_malloc(sizeof(MQTTPackage));
	payloadPublish->op = 2;
	payloadPublish->qos = qos;
	sprintf(payloadPublish->topic,"%s", topic);
	sprintf(payloadPublish->data,"%s", payload);
	sprintf(payloadPublish->mbox,"%s", source);
	
	printf("publish - %d\t%s\t%s\t%s\n", qos, payloadPublish->topic, payloadPublish->data, payloadPublish->mbox);
    sg_comm_t comm_send_Pub       = sg_mailbox_put_async(dest, payloadPublish, 0);

    switch (sg_comm_wait_for(comm_send_Pub, 1.0)) 
    {
      case SG_ERROR_NETWORK:
        xbt_free(payloadPublish);
        break;
      case SG_ERROR_TIMEOUT:
        xbt_free(payloadPublish);
        break;
      case SG_OK:
        /* nothing */
        break;
      default:
        printf("Unexpected behavior with '%s'\n", sg_host_get_name(sg_host_self()));
        return 1;
    }
    printf("Message sent\n");
    if(qos == 0) sg_actor_sleep_for(1);
    return 0;
}


void mqtt_publish_b(int qos, char* source, char* dest, char* topic, char* payload)
{
	MQTTPackage* payloadBroker = (MQTTPackage*) xbt_malloc(sizeof(MQTTPackage));
	payloadBroker->op = 2;
	payloadBroker->qos = qos;

	// Buscar aquellos que se han suscrito para reenviarlo

	sprintf(payloadBroker->topic,"%s", topic);
	sprintf(payloadBroker->data,"%s", payload);
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
		printf("Unexpected behavior with '%s'\n", sg_host_get_name(sg_host_self()));
    }
}


int mqtt_subscribe(int qos, sg_mailbox_t source, sg_mailbox_t dest, char* topic)
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
		printf("Unexpected behavior with '%s'\n", sg_host_get_name(sg_host_self()));
		return 1;
    }

    char* msgACK_sub = sg_mailbox_get(source);
   
	if (strcmp(msgACK_sub, "Subscribed") != 0)
	{
		printf("Error\n");
		return 1;
	}
	printf("%s subscribed\n", sg_host_get_name(sg_host_self()));
	return 0;
}
