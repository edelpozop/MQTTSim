#include "mqtt_sim.h"



int mqtt_connect(char* qos, sg_mailbox_t source, sg_mailbox_t dest)
{
	sg_mailbox_put(dest, "0", CONNECT_OP_SIZE);
	sg_mailbox_put(dest, qos, CONNECT_QOS_SIZE);
	sg_mailbox_put(dest, "AAAAAAA", sizeof(sg_mailbox_t));
    
    printf("Waiting ACK in %s\n", sg_host_get_name(sg_host_self()));

    char* msgACK = sg_mailbox_get(source);
	if (strcmp(msgACK, "Connected") != 0)
	{
		printf("Error in %s\n", sg_host_get_name(sg_host_self()));
		return 1;
	}

	printf("%s connected\n", sg_host_get_name(sg_host_self()));
	return 0;
}


void mqtt_disconnect(char* qos, sg_mailbox_t source, sg_mailbox_t dest)
{
    sg_mailbox_put(dest, "1", DISCONNECT_OP_SIZE);
	sg_mailbox_put(dest, qos, DISCONNECT_QOS_SIZE);
	sg_mailbox_put(dest, source, DISCONNECT_MBOX_SIZE);
  
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
		sg_mailbox_t mbox_dest			= sg_mailbox_by_name(destEnd);
    	sg_mailbox_put(mbox_dest, 1, DISCONNECTB_OP_SIZE);

	}
}


int mqtt_publish(char* qos, sg_mailbox_t source, sg_mailbox_t dest, char* topic, char* payload, int payloadlen)
{
	sg_mailbox_put(dest, "2", PUBLISH_OP_SIZE);
	sg_mailbox_put(dest, qos, PUBLISH_QOS_SIZE);
	sg_mailbox_put(dest, source, sizeof(sg_mailbox_t));
	sg_mailbox_put(dest, topic, PUBLISH_TOPIC_SIZE);
	sg_mailbox_put(dest, payload, payloadlen);
	
	//printf("publish - %d\t%s\t%s\t%s\n", qos, payloadPublish->topic, payloadPublish->data, payloadPublish->mbox);
    
    //printf("Message sent\n");
    //if(qos == 0) sg_actor_sleep_for(1);
    return 0;
}


void mqtt_publish_b(char* qos, char* source, char* dest, char* topic, char* payload, int payloadlen)
{
	// Buscar aquellos que se han suscrito para reenviarlo

	sg_mailbox_t mbox_dest			= sg_mailbox_by_name(dest);

    sg_mailbox_put(mbox_dest, "2", PUBLISH_OP_SIZE);
	sg_mailbox_put(mbox_dest, qos, PUBLISH_QOS_SIZE);
	sg_mailbox_put(mbox_dest, source, sizeof(sg_mailbox_t));
	sg_mailbox_put(mbox_dest, topic, PUBLISH_TOPIC_SIZE);
	sg_mailbox_put(mbox_dest, payload, payloadlen);

}


int mqtt_subscribe(char* qos, sg_mailbox_t source, sg_mailbox_t dest, char* topic)
{
	sg_mailbox_put(dest, "3", SUBSCRIBE_OP_SIZE);
	sg_mailbox_put(dest, qos, SUBSCRIBE_QOS_SIZE);
	sg_mailbox_put(dest, topic, SUBSCRIBE_TOPIC_SIZE);
	sg_mailbox_put(dest, source, sizeof(sg_mailbox_t));

	printf("A\n");

    char* msgACK_sub = sg_mailbox_get(source);
   
	if (strcmp(msgACK_sub, "Subscribed") != 0)
	{
		printf("Error\n");
		return 1;
	}
	printf("%s subscribed\n", sg_host_get_name(sg_host_self()));
	return 0;
}


void broker_run(sg_mailbox_t mbox, int id_cluster_fog, int nodes_fog, int active_devices)
{
	sg_mailbox_t mbroker 				= mbox;
	int id_cluster_f 					= id_cluster_fog;
	int nodes_f 						= nodes_fog;
	int active_d 						= active_devices;
	int end 							= 0;

	char ack_broker[128], msg_qos[128], msg_mbox[128], msg_topic[128], msg_source_broker[128];
	
	while(!end)
	{
		char* msg_op = (char *)sg_mailbox_get(mbroker);
		int op = atoi (msg_op);

		switch(op)
		{
		case 0:
			strcpy(msg_qos, sg_mailbox_get(mbroker));
			strcpy(msg_mbox, sg_mailbox_get(mbroker));
			printf("%s %s\n", msg_qos, msg_mbox);
			strcpy(ack_broker,"Connected");
			sg_mailbox_put(msg_mbox, xbt_strdup(ack_broker), strlen(ack_broker));
			break;

		case 1:
			strcpy(msg_qos, sg_mailbox_get(mbroker));
			strcpy(msg_mbox, sg_mailbox_get(mbroker));
			strcpy(ack_broker, "Disconnected");
			sg_mailbox_put(msg_mbox, xbt_strdup(ack_broker), strlen(ack_broker));
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
				mqtt_publish_b(package.qos, package.mbox, dest, package.topic, package.data);
			}*/
			break;
		case 3:
			/*TODO-You can save in a file for each mbox.fog the topic to which you subscribe.*/
			strcpy(msg_qos, sg_mailbox_get(mbroker));
			strcpy(msg_topic, sg_mailbox_get(mbroker));
			strcpy(msg_source_broker, sg_mailbox_get(mbroker));
			strcpy(ack_broker,"Subscribed");
			sg_mailbox_put(msg_source_broker, xbt_strdup(ack_broker), strlen(ack_broker));
			break;

		default:
		}
		
	}
	
	printf("Broker disconnected\n");
}