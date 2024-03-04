#include "mqtt_sim.h"
#include "subscriptions.h"

void mqtt_init()
{
	init_subscriptions();
}

void mqtt_end()
{
	end_subscriptions();
}

int mqtt_connect(int qos, sg_mailbox_t source, sg_mailbox_t dest)
{
	/*Broker Connection*/
	MQTTPackage* connectionBroker 		= (MQTTPackage*) xbt_malloc(sizeof(MQTTPackage));
	connectionBroker->op 				= 0;
	connectionBroker->qos 				= qos;
	connectionBroker->mbox				= source;
    sg_mailbox_put_async(dest, connectionBroker, 0);

    //printf("Waiting ACK in %s\n", sg_host_get_name(sg_host_self()));

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
	MQTTPackage* disconnectionBroker 	= (MQTTPackage*) xbt_malloc(sizeof(MQTTPackage));
	disconnectionBroker->op 			= 1;
	disconnectionBroker->qos 			= qos;
	disconnectionBroker->mbox 			= source;
	
    sg_mailbox_put (dest, disconnectionBroker, 0);

    /*char* msgACK_disc 					= sg_mailbox_get(source);
	if (strcmp(msgACK_disc, "Disconnected") != 0)
	{
		printf("Error in %s\n", sg_host_get_name(sg_host_self()));
		return 1;
	}*/
	printf("%s disconnected\n", sg_host_get_name(sg_host_self()));
	return 0;
}


void mqtt_disconnectAll_b (int id_cluster, int nodes_fog)
{
	char destEnd[128];
	for (int i = 0; i < nodes_fog; i++)
	{
		sprintf(destEnd, "fog-%d-%d", id_cluster, i);

		MQTTPackage* payloadBroker 		= (MQTTPackage*) xbt_malloc(sizeof(MQTTPackage));
		payloadBroker->op 				= 1;
		sg_mailbox_t mbox_dest			= sg_mailbox_by_name(destEnd);
    	sg_mailbox_put (mbox_dest, payloadBroker, 0);

	}
}


int mqtt_publish (int qos, sg_mailbox_t source, sg_mailbox_t dest, char* topic, char* payload, int payloadlen)
{
	MQTTPackage* payloadPublish 		= (MQTTPackage*) xbt_malloc(sizeof(MQTTPackage));
	payloadPublish->op 					= 2;
	payloadPublish->qos 				= qos;
	payloadPublish->payloadlen 			= payloadlen;
	payloadPublish->mbox 				= source;
	int sizeofStruct 					= (sizeof(int) * 3) + (sizeof(payloadPublish->mbox) + sizeof(payloadPublish->topic) + payloadlen);

	sprintf(payloadPublish->topic,"%s", topic);
	
	//printf("%d %d\n",payloadlen, sizeofStruct);

	//printf("%s publishing to %s\n", sg_host_get_name(sg_host_self()), dest);
	sg_mailbox_put(dest, payloadPublish, sizeofStruct/1.6);

    //if(qos == 0) sg_actor_sleep_for(1);
    return 0;
}


void mqtt_publish_b (int qos, sg_mailbox_t source, char* topic, char* payload, int payloadlen)
{
	MQTTPackage* payloadBroker 			= (MQTTPackage*) xbt_malloc(sizeof(MQTTPackage));
	payloadBroker->op 					= 2;
	payloadBroker->qos 					= qos;
	payloadBroker->payloadlen 			= payloadlen;
	payloadBroker->mbox 				= source;
	sprintf(payloadBroker->topic,"%s", topic);
	int sizeofStruct 					= (sizeof(int) * 3) + (sizeof(payloadBroker->mbox) + sizeof(payloadBroker->topic) + payloadlen);

    MailboxList dests = findSubs(topic);
    //printf("%d\n", dests.count);

    for (int i = 0; i < dests.count; i++) 
    {
        sg_mailbox_put (dests.mailboxes[i], payloadBroker, sizeofStruct/2);
    }

    free(dests.mailboxes);
}


int mqtt_subscribe (int qos, sg_mailbox_t source, sg_mailbox_t dest, char* topic)
{
	MQTTPackage* subscriptionBroker 	= (MQTTPackage*) xbt_malloc(sizeof(MQTTPackage));
	subscriptionBroker->op 				= 3;
	subscriptionBroker->mbox 			= source;
	sprintf(subscriptionBroker->topic, "%s", topic);
    sg_mailbox_put_async(dest, subscriptionBroker, 0);

    char* msgACK_sub 					= sg_mailbox_get(source);

	if (strcmp(msgACK_sub, "Subscribed") != 0)
	{
		printf("Error in subscription\n");
		return 1;
	}

	//printf("%s subscribed\n", sg_host_get_name(sg_host_self()));
	return 0;
}


void broker_run (sg_mailbox_t mbox, int id_cluster_fog, int nodes_fog, int active_devices)
{
	char ack_broker[128];
	//Sub* localList 					= NULL;
	int listAdded 						= 0;
	int id_cluster_f 					= id_cluster_fog;
	int nodes_f 						= nodes_fog;
	int active_d 						= active_devices;
	int end 							= 0;
	sg_mailbox_t mbroker 				= mbox;
	
	while(!end)
	{
		MQTTPackage* payload 			= sg_mailbox_get(mbroker);
		MQTTPackage package 			= *payload;
		xbt_free(payload);

		switch(package.op)
		{
		case 0:
			strcpy(ack_broker,"Connected");
			//printf("%s connection from %s\n", sg_host_get_name(sg_host_self()), package.mbox);
			sg_mailbox_put(package.mbox, xbt_strdup(ack_broker), strlen(ack_broker));
			break;

		case 1:
			//strcpy(ack_broker,"Disconnected");
			//sg_mailbox_put(package.mbox, xbt_strdup(ack_broker), strlen(ack_broker));
			active_d--;

			if(active_d == 0)
			{
				//printLocalList(localList);
				mqtt_disconnectAll_b(id_cluster_fog, nodes_fog);
				end = 1;
			}
			break;

		case 2:	
			mqtt_publish_b(package.qos, package.mbox, package.topic, package.data, package.payloadlen);
			break;

		case 3:
			//printf("%s subscribed to %s\n", package.mbox, package.topic);

			if(insertSub(package.mbox, package.topic) == 0)
			{
				strcpy(ack_broker,"Subscribed");
				sg_mailbox_put(package.mbox, xbt_strdup(ack_broker), strlen(ack_broker));
			}

			/*if (listAdded == 0) 
			{	
				insertGlobalList(localList);
				printGlobalList();
				listAdded = 1;
			}*/
			break;

		case 4:
		    deleteSub(package.mbox, package.topic);
		    strcpy(ack_broker,"Unsubscribed");
			sg_mailbox_put(package.mbox, xbt_strdup(ack_broker), strlen(ack_broker));
			break;

		default:
		}
	}

	printf("Broker disconnected\n");
}