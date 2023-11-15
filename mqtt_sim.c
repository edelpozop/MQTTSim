#include "mqtt_sim.h"


int mqtt_connect(int qos, sg_mailbox_t source, sg_mailbox_t dest)
{
	/*Broker Connection*/
	MQTTPackage* connectionBroker 	= (MQTTPackage*) xbt_malloc(sizeof(MQTTPackage));
	connectionBroker->op 			= 0;
	connectionBroker->qos 			= qos;
	sprintf(connectionBroker->mbox, "%s", source);
    sg_comm_t comm_send_Conn       	= sg_mailbox_put_async(dest, connectionBroker, 0);

    //printf("Waiting ACK in %s\n", sg_host_get_name(sg_host_self()));

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

	//printf("%s connected\n", sg_host_get_name(sg_host_self()));
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
	//printf("%s disconnected\n", sg_host_get_name(sg_host_self()));
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


int mqtt_publish(int qos, sg_mailbox_t source, sg_mailbox_t dest, char* topic, char* payload, int payloadlen)
{
	MQTTPackage* payloadPublish = (MQTTPackage*) xbt_malloc(sizeof(MQTTPackage));
	payloadPublish->op = 2;
	payloadPublish->qos = qos;
	payloadPublish->payloadlen = payloadlen;
	int sizeofStruct = (sizeof(int) * 3) + (sizeof(payloadPublish->mbox) + sizeof(payloadPublish->topic) + payloadlen);

	sprintf(payloadPublish->topic,"%s", topic);
	sprintf(payloadPublish->mbox,"%s", source);
	//printf("%d %d\n",payloadlen, sizeofStruct);

	//printf("publish - %d\t%s\t%s\t%s\n", qos, payloadPublish->topic, payloadPublish->data, payloadPublish->mbox);
    //sg_comm_t comm_send_Pub       = sg_mailbox_put_async(dest, payloadPublish, sizeofStruct);

    sg_mailbox_put(dest, payloadPublish, sizeofStruct/1.6);

    /*switch (sg_comm_wait_for(comm_send_Pub, 1.0)) 
    {
      case SG_ERROR_NETWORK:
        xbt_free(payloadPublish);
        break;
      case SG_ERROR_TIMEOUT:
        xbt_free(payloadPublish);
        break;
      case SG_OK:
        break;
      default:
        printf("Unexpected behavior with '%s'\n", sg_host_get_name(sg_host_self()));
        return 1;
    }*/
    //printf("Message sent\n");
    //if(qos == 0) sg_actor_sleep_for(1);
    return 0;
}


void mqtt_publish_b(int qos, char* source, char* dest, char* topic, char* payload, int payloadlen)
{
	MQTTPackage* payloadBroker = (MQTTPackage*) xbt_malloc(sizeof(MQTTPackage));
	payloadBroker->op = 2;
	payloadBroker->qos = qos;
	//payloadBroker->data = (char*)malloc(payloadlen);
	payloadBroker->payloadlen = payloadlen;
	// Buscar aquellos que se han suscrito para reenviarlo

	sprintf(payloadBroker->topic,"%s", topic);
	sprintf(payloadBroker->data,"%s", payload);
	sprintf(payloadBroker->mbox,"%s", source);

	sg_mailbox_t mbox_dest			= sg_mailbox_by_name(dest);
    sg_comm_t comm_send_fog       	= sg_mailbox_put_async(mbox_dest, payloadBroker, sizeof(*payloadBroker));

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
	//printf("%s subscribed\n", sg_host_get_name(sg_host_self()));
	return 0;
}


void broker_run(sg_mailbox_t mbox, int id_cluster_fog, int nodes_fog, int active_devices)
{
	sg_mailbox_t mbroker 				= mbox;
	int id_cluster_f 					= id_cluster_fog;
	int nodes_f 						= nodes_fog;
	int active_d 						= active_devices;
	int end 							= 0;

	char ack_broker[128], destEnd[128], dest[128];
	
	while(!end)
	{
		MQTTPackage* payload = sg_mailbox_get(mbroker);
		//sg_comm_t comm     = sg_mailbox_get_async(mbroker, (void**)&payload);
		//sg_error_t retcode = sg_comm_wait(comm);
		
	    //if (retcode == SG_OK) 
	    //{
			MQTTPackage package = *payload;
			xbt_free(payload);
			switch(package.op)
			{
			case 0:
				strcpy(ack_broker,"Connected");
				sg_mailbox_put(package.mbox, xbt_strdup(ack_broker), strlen(ack_broker));
				break;

			case 1:
				strcpy(ack_broker,"Disconnected");
				sg_mailbox_put(package.mbox, xbt_strdup(ack_broker), strlen(ack_broker));
				active_d--;

				if(active_d == 0)
				{
					mqtt_disconnectAll_b(id_cluster_fog, nodes_fog);
					end = 1;
				}
				break;

			case 2:	
				/*for (int i = 0; i < nodes_f; i++)
				{
					sprintf(dest, "fog-%d-%d", id_cluster_f, i);
					mqtt_publish_b(package.qos, package.mbox, dest, package.topic, package.data, package.payloadlen);
				}*/
				break;
			case 3:
				/*TODO-You can save in a file for each mbox.fog the topic to which you subscribe.*/
				//printf("%s subscribed to %s\n", package.mbox, package.topic);
				strcpy(ack_broker,"Subscribed");
				sg_mailbox_put(package.mbox, xbt_strdup(ack_broker), strlen(ack_broker));
				break;

			default:
			}
		//} 
		//else if (retcode == SG_ERROR_NETWORK) 
		//{
		//	printf("Mmh. Something went wrong. Nevermind. Let's keep going!\n");
		//}
	}
	
	printf("Broker disconnected\n");
}