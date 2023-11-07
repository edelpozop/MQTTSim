

#include "mqtt_sim.h"

static void edge (int argc, char**argv)
{
	const char* value_broker;
	const char* value_qos;

	const char* edge_name 	= sg_host_get_name(sg_host_self());
	const char* fname 		= sg_host_get_name(sg_host_self());

	value_broker 	= sg_zone_get_property_value(sg_zone_get_by_name("zone1"), "mbox_broker");
	value_qos 		= sg_zone_get_property_value(sg_zone_get_by_name("zone1"), "qos");

	sg_mailbox_t mbox_edge = sg_mailbox_by_name(sg_host_self());
	sg_mailbox_t mbox_broker = sg_mailbox_by_name(value_broker);

	int qos_edge = atoi(value_qos), ret = 0;

	ret = mqtt_connect(qos_edge, mbox_edge, mbox_broker);

	/*Sensor data reading*/

	FILE *sensor;
    char linea[128]; // Maximum line size
    char topic[128];
    memset(linea, 0, sizeof(linea));
    memset(topic, 0, sizeof(topic));

    // Opens the file in read mode
    sensor = fopen(fname, "r");

    // Checks if the file was opened correctly
    if (sensor == NULL) {
        printf("Cannot open file\n");
    }

    // Reading the file line by line
    while (fgets(linea, sizeof(linea), sensor) != NULL) 
    {
    	sprintf(topic, "%s/%s", edge_name, fname);
    	ret = mqtt_publish(qos_edge, mbox_edge, mbox_broker, topic, linea);
    	memset(linea, 0, sizeof(linea));
    	memset(topic, 0, sizeof(topic));   	
    }

    // Closes the file after reading all lines
    fclose(sensor);

    /*Broker disconnection*/

	mqtt_disconnect(qos_edge, mbox_edge, mbox_broker);
}

static void broker (int argc, char** argv)
{
	const char* broker_name = sg_host_get_name(sg_host_self());
	sg_mailbox_t mbox_inb = sg_mailbox_by_name(broker_name);
	int nodes_fog = atoi(sg_zone_get_property_value(sg_zone_get_by_name("zone2"), "nodes_fog"));
	int id_cluster_fog = atoi(sg_zone_get_property_value(sg_zone_get_by_name("zone2"), "id_cluster_fog"));
	char ack_broker[128], destEnd[128], dest[128];
	int active_devices = TOTAL_EDGE;
	int end = 0;

	while(!end)
	{
		MQTTPackage* payload;
		sg_comm_t comm     = sg_mailbox_get_async(mbox_inb, (void**)&payload);
		sg_error_t retcode = sg_comm_wait(comm);

	    if (retcode == SG_OK) 
	    {
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
				active_devices--;

				if(active_devices == 0)
				{
					mqtt_disconnectAll_b(id_cluster_fog, nodes_fog);
					end = 1;
				}
				break;

			case 2:	
				for (int i = 0; i < nodes_fog; i++)
				{
					sprintf(dest, "fog-0-%d", i);
					mqtt_publish_b(package.qos, package.mbox, dest, package.topic, package.data);
				}
				break;
			case 3:
				/*TODO-You can save in a file for each mbox.fog the topic to which you subscribe.*/
				printf("%s subscribed to %s\n", package.mbox, package.topic);
				strcpy(ack_broker,"Subscribed");
				sg_mailbox_put(package.mbox, xbt_strdup(ack_broker), strlen(ack_broker));
				break;

			default:
			}
		} 
		else if (retcode == SG_ERROR_NETWORK) 
		{
			printf("Mmh. Something went wrong. Nevermind. Let's keep going!\n");
		}
	}
	
	printf("Broker disconnected\n");
}

static void fog (int argc, char** argv)
{
	const char* value_broker;
	const char* value_qos;
	const char* topic_subs;
	const char* fog_name 	= sg_host_get_name(sg_host_self());
	const char* fname 		= sg_host_get_name(sg_host_self());
	int end = 0;

	value_broker 	= sg_zone_get_property_value(sg_zone_get_by_name("zone3"), "mbox_broker");
	value_qos 		= sg_zone_get_property_value(sg_zone_get_by_name("zone3"), "qos");
	topic_subs 		= sg_zone_get_property_value(sg_zone_get_by_name("zone3"), "subscribe");

	sg_mailbox_t mbox_fog = sg_mailbox_by_name(fog_name);
	sg_mailbox_t mbox_broker = sg_mailbox_by_name(value_broker);

	int qos_fog = atoi(value_qos);
	
	/*Fog Connection*/

	end = mqtt_connect(qos_fog, mbox_fog, mbox_broker);
	
	/*Subscription*/

	if (!end)
	{
		end = mqtt_subscribe(qos_fog, mbox_fog, mbox_broker, topic_subs);
	}

	while(!end)
	{
		MQTTPackage* payload;
		sg_comm_t comm     = sg_mailbox_get_async(mbox_fog, (void**) &payload);
		sg_error_t retcode = sg_comm_wait(comm);

	    if (retcode == SG_OK) 
	    {
			MQTTPackage package = *payload;
			xbt_free(payload);

			if(package.op == 1) end = 1;
			else
			{
				printf("FOG\t%s\t%s\n", package.topic, package.data);
			}
			
		} 
		else if (retcode == SG_ERROR_NETWORK) 
		{
			printf("Mmh. Something went wrong. Nevermind. Let's keep going!\n");
		}
	}

	printf("Fog disconnected\n");
}



int main(int argc, char* argv[])
{
	/* Get the arguments */
	simgrid_init(&argc, argv);

	/* load the platform file */
	simgrid_load_platform(argv[1]);

	int broker_argc           = 0;
	const char* broker_argv[] = {NULL};
	sg_actor_create_("broker0", sg_host_by_name("broker-0-0"), broker, broker_argc, broker_argv);

	int fog_argc           = 0;
	const char* fog_argv[] = {NULL};
	sg_actor_create_("fog0", sg_host_by_name("fog-0-0"), fog, fog_argc, fog_argv);

	int edge_argc           = 0;
	const char* edge_argv[] = {NULL};
	sg_actor_create_("edge0", sg_host_by_name("edge-0-0"), edge, edge_argc, edge_argv);

	

	simgrid_register_function("broker", broker);
	simgrid_register_function("fog", fog);
	simgrid_register_function("edge", edge);
	
	simgrid_run();
	printf("Simulation time %g\n", simgrid_get_clock());

	return 0;
}