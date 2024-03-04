

#include "mqtt_sim.h"


// Tamaño máximo (2MB)
#define REPEAT_COUNT 100
#define MQTT_TOPIC "/pingpong"

/*static void edge (int argc, char**argv)
{
	const char* value_broker;
	const char* value_qos;

	const char* edge_name 		= sg_host_get_name(sg_host_self());
	const char* fname 			= sg_host_get_name(sg_host_self());

	value_broker 				= sg_host_get_property_value(sg_host_self(), "mbox_broker");
	value_qos 					= sg_host_get_property_value(sg_host_self(), "qos");

	sg_mailbox_t mbox_edge 		= sg_mailbox_by_name(edge_name);
	sg_mailbox_t mbox_broker 	= sg_mailbox_by_name(value_broker);

	int qos_edge = atoi(value_qos), ret = 0;

	ret = mqtt_connect(qos_edge, mbox_edge, mbox_broker);

	//Sensor data reading

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
    	sprintf(topic, "%s/%s", sg_host_self(), fname);
    	ret = mqtt_publish(qos_edge, mbox_edge, mbox_broker, topic, linea);
    	memset(linea, 0, sizeof(linea));
    	memset(topic, 0, sizeof(topic));   	
    }

    // Closes the file after reading all lines
    fclose(sensor);

    //Broker disconnection

	mqtt_disconnect(qos_edge, mbox_edge, mbox_broker);
}*/


static void edge (int argc, char**argv)
{
	double start_time, end_time, used_time, avg_time, us_rate;

	const char* edge_name 		= sg_host_get_name(sg_host_self());
	const char* fname 			= sg_host_get_name(sg_host_self());
	const char* value_broker	= sg_host_get_property_value(sg_host_self(), "mbox_broker");
	const char* value_qos 		= sg_host_get_property_value(sg_host_self(), "qos");

    sg_mailbox_t mbox_edge 		= sg_mailbox_by_name(sg_host_get_name(sg_host_self()));
    sg_mailbox_t mbox_broker  	= sg_mailbox_by_name(value_broker);

	sg_actor_sleep_for(1);

	int qos_edge = atoi(value_qos), ret = 0;

	ret = mqtt_connect(qos_edge, mbox_edge, mbox_broker);

	// Abrir el archivo CSV y escribir los encabezados
    //FILE *csv_file = fopen("MQTTSimgrid.csv", "w");
    //if (csv_file) {
    //    fprintf(csv_file, "Data Size (B);Time Elapsed (s);Transfer Rate (MB/s)\n");
    //}

	//char message[MAX_PAYLOAD_SIZE];
    // Iniciar el reloj
    avg_time = 0.0;
    start_time = simgrid_get_clock();
    
    for (int repeat = 0; repeat < TOTAL_MESSAGES; repeat++) 
    {
        // Publicar el mensaje
        ret = mqtt_publish(qos_edge, mbox_edge, mbox_broker, MQTT_TOPIC, "Ping", MAX_PAYLOAD_SIZE);
    }
    used_time = (simgrid_get_clock() - start_time);

    avg_time = used_time;

    avg_time = avg_time / (float) TOTAL_MESSAGES;

    if (avg_time > 0) /* rate is megabytes per second */
        us_rate = (double)((MAX_PAYLOAD_SIZE) / (avg_time * (double) 1000000));
    else
        us_rate = 0.0;

    // Escribir los datos en el archivo CSV
    //if (csv_file) 
    //{
    //    fprintf(csv_file, "%d;%.8f;%.8f\n", payload_size, avg_time, us_rate);
    printf("%s\t%d\t\t%.8f\t\t%.8f\n", edge_name, MAX_PAYLOAD_SIZE, avg_time, us_rate);
    //}

    /*if (csv_file) 
    {
        fclose(csv_file);
    }   */ 

    //Broker disconnection

	mqtt_disconnect(qos_edge, mbox_edge, mbox_broker);
}


static void broker (int argc, char** argv)
{
	const char* broker_name 		= sg_host_get_name(sg_host_self());
	sg_mailbox_t mbox_inb 			= sg_mailbox_by_name(broker_name);
	int id_cluster_fog 				= atoi(sg_host_get_property_value(sg_host_self(), "id_cluster_fog"));
	int nodes_fog 					= atoi(sg_host_get_property_value(sg_host_self(), "nodes_fog"));
	int active_devices				= 0;

	switch (id_cluster_fog)
	{
	case 0:
		active_devices 				= TOTAL_EDGE0 + TOTAL_EDGE4;
		break;
	case 1:
		active_devices 				= TOTAL_EDGE1 + TOTAL_EDGE5;
		break;
	case 2:
		active_devices 				= TOTAL_EDGE2 + TOTAL_EDGE6;
		break;
	default:
		active_devices 				= TOTAL_EDGE3 + TOTAL_EDGE7;
		break;
	}

	broker_run(mbox_inb, id_cluster_fog, nodes_fog, active_devices);
}


static void fog (int argc, char** argv)
{
	const char* fog_name 			= sg_host_get_name(sg_host_self());
	const char* value_broker 		= sg_host_get_property_value(sg_host_self(), "mbox_broker");
	const char* value_qos 			= sg_host_get_property_value(sg_host_self(), "qos");
	const char* topic_subs 			= sg_host_get_property_value(sg_host_self(), "subscribe");
	sg_mailbox_t mbox_fog 			= sg_mailbox_by_name(fog_name);
	sg_mailbox_t mbox_broker 		= sg_mailbox_by_name(value_broker);

	int end = 0;
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
		sg_comm_t comm     		= sg_mailbox_get_async(mbox_fog, (void**) &payload);
		sg_error_t retcode 		= sg_comm_wait(comm);

	    if (retcode == SG_OK) 
	    {
			MQTTPackage package = *payload;
			//xbt_free(payload);

			if(package.op == 1) end = 1;
			else
			{
				//printf("Mensaje recibido %s\n", sg_host_get_name(sg_host_self()));
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
	mqtt_init();

	/* load the platform file */
	simgrid_load_platform(argv[1]);
	sg_host_energy_plugin_init();

	int broker_argc           = 0;
	const char* broker_argv0[] = {NULL};
	sg_actor_create_("broker0", sg_host_by_name("broker-0-0"), broker, broker_argc, broker_argv0);

	const char* broker_argv1[] = {NULL};
	sg_actor_create_("broker1", sg_host_by_name("broker-1-0"), broker, broker_argc, broker_argv1);

	const char* broker_argv2[] = {NULL};
	sg_actor_create_("broker2", sg_host_by_name("broker-2-0"), broker, broker_argc, broker_argv2);

	const char* broker_argv3[] = {NULL};
	sg_actor_create_("broker3", sg_host_by_name("broker-3-0"), broker, broker_argc, broker_argv3);

	for(int i = 0; i < TOTAL_EDGE0; i++)
	{
		int edge_argc0           = 0;
		char edge_name0 [128];
		sprintf(edge_name0, "edge-0-%d", i);
		const char* edge_argv0[] = {NULL};
		sg_actor_create_("edge0", sg_host_by_name(edge_name0), edge, edge_argc0, edge_argv0);
	}

	for(int i = 0; i < TOTAL_EDGE1; i++)
	{
		int edge_argc1           = 0;
		char edge_name1 [128];
		sprintf(edge_name1, "edge-1-%d", i);
		const char* edge_argv1[] = {NULL};
		sg_actor_create_("edge1", sg_host_by_name(edge_name1), edge, edge_argc1, edge_argv1);
	}

	for(int i = 0; i < TOTAL_EDGE2; i++)
	{
		int edge_argc2           = 0;
		char edge_name2 [128];
		sprintf(edge_name2, "edge-2-%d", i);
		const char* edge_argv2[] = {NULL};
		sg_actor_create_("edge2", sg_host_by_name(edge_name2), edge, edge_argc2, edge_argv2);
	}

	for(int i = 0; i < TOTAL_EDGE3; i++)
	{
		int edge_argc3           = 0;
		char edge_name3 [128];
		sprintf(edge_name3, "edge-3-%d", i);
		const char* edge_argv3[] = {NULL};
		sg_actor_create_("edge3", sg_host_by_name(edge_name3), edge, edge_argc3, edge_argv3);
	}

	for(int i = 0; i < TOTAL_EDGE4; i++)
	{
		int edge_argc4           = 0;
		char edge_name4 [128];
		sprintf(edge_name4, "edge-4-%d", i);
		const char* edge_argv4[] = {NULL};
		sg_actor_create_("edge4", sg_host_by_name(edge_name4), edge, edge_argc4, edge_argv4);
	}

	for(int i = 0; i < TOTAL_EDGE5; i++)
	{
		int edge_argc5           = 0;
		char edge_name5 [128];
		sprintf(edge_name5, "edge-5-%d", i);
		const char* edge_argv5[] = {NULL};
		sg_actor_create_("edge1", sg_host_by_name(edge_name5), edge, edge_argc5, edge_argv5);
	}

	for(int i = 0; i < TOTAL_EDGE6; i++)
	{
		int edge_argc6           = 0;
		char edge_name6 [128];
		sprintf(edge_name6, "edge-6-%d", i);
		const char* edge_argv6[] = {NULL};
		sg_actor_create_("edge6", sg_host_by_name(edge_name6), edge, edge_argc6, edge_argv6);
	}

	for(int i = 0; i < TOTAL_EDGE7; i++)
	{
		int edge_argc7           = 0;
		char edge_name7 [128];
		sprintf(edge_name7, "edge-7-%d", i);
		const char* edge_argv7[] = {NULL};
		sg_actor_create_("edge7", sg_host_by_name(edge_name7), edge, edge_argc7, edge_argv7);
	}

	for(int i = 0; i < TOTAL_FOG0; i++)
	{
		int fog_argc0           = 0;
		char fog_name0 [128];
		sprintf(fog_name0, "fog-0-%d", i);
		const char* fog_argv0[] = {NULL};
		sg_actor_create_("fog0", sg_host_by_name(fog_name0), fog, fog_argc0, fog_argv0);
	}

	for(int i = 0; i < TOTAL_FOG1; i++)
	{
		int fog_argc1           = 0;
		char fog_name1 [128];
		sprintf(fog_name1, "fog-1-%d", i);
		const char* fog_argv1[] = {NULL};
		sg_actor_create_("fog1", sg_host_by_name(fog_name1), fog, fog_argc1, fog_argv1);
	}

	for(int i = 0; i < TOTAL_FOG2; i++)
	{
		int fog_argc2           = 0;
		char fog_name2 [128];
		sprintf(fog_name2, "fog-2-%d", i);
		const char* fog_argv2[] = {NULL};
		sg_actor_create_("fog2", sg_host_by_name(fog_name2), fog, fog_argc2, fog_argv2);
	}

	for(int i = 0; i < TOTAL_FOG3; i++)
	{
		int fog_argc3           = 0;
		char fog_name3 [128];
		sprintf(fog_name3, "fog-3-%d", i);
		const char* fog_argv3[] = {NULL};
		sg_actor_create_("fog3", sg_host_by_name(fog_name3), fog, fog_argc3, fog_argv3);
	}

	simgrid_register_function("broker", broker);
	simgrid_register_function("fog", fog);
	simgrid_register_function("edge", edge);
	
	simgrid_run();
	printf("Simulation time %g\n", simgrid_get_clock());
	mqtt_end();

	return 0;
}