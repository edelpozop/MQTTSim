

#include "mqtt_sim.h"


// Tamaño máximo (2MB)
#define REPEAT_COUNT 100
#define MQTT_TOPIC "pingpong"

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
	double start_time;
    double end_time;
    double used_time;
    double avg_time;
    double us_rate;

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

	// Abrir el archivo CSV y escribir los encabezados
    FILE *csv_file = fopen("MQTTSimgrid.csv", "w");
    if (csv_file) {
        fprintf(csv_file, "Data Size (B);Time Elapsed (s);Transfer Rate (MB/s)\n");
    }

    int payload_size = 1;

	while (payload_size <= MAX_PAYLOAD_SIZE) 
    {
    	char message[payload_size];
        // Iniciar el reloj
        avg_time = 0.0;
        start_time = simgrid_get_clock();
        
        for (int repeat = 0; repeat < REPEAT_COUNT; repeat++) 
        {
            // Publicar el mensaje
            snprintf(message, payload_size, "%*s", payload_size, "Ping");
            ret = mqtt_publish(qos_edge, mbox_edge, mbox_broker, MQTT_TOPIC, message, payload_size);
        }

        used_time = (simgrid_get_clock() - start_time);
        avg_time = used_time;

        avg_time = avg_time / (float) REPEAT_COUNT;

        if (avg_time > 0) /* rate is megabytes per second */
            us_rate = (double)((payload_size) / (avg_time * (double) 1000000));
        else
            us_rate = 0.0;

        // Escribir los datos en el archivo CSV
        if (csv_file) 
        {
            fprintf(csv_file, "%d;%.8f;%.8f\n", payload_size, avg_time, us_rate);
            printf("%d\t\t%.8f\t\t%.8f\n", payload_size, avg_time, us_rate);
        }

        // Aumentar el tamaño del payload (duplicarlo) para la próxima iteración
        payload_size *= 2;
    }

    if (csv_file) 
    {
        fclose(csv_file);
    }    

    //Broker disconnection

	mqtt_disconnect(qos_edge, mbox_edge, mbox_broker);
}


static void broker (int argc, char** argv)
{
	const char* broker_name 		= sg_host_get_name(sg_host_self());
	sg_mailbox_t mbox_inb 			= sg_mailbox_by_name(broker_name);
	int id_cluster_fog 				= atoi(sg_host_get_property_value(sg_host_self(), "id_cluster_fog"));
	int nodes_fog 					= atoi(sg_host_get_property_value(sg_host_self(), "nodes_fog"));
	int active_devices 				= TOTAL_EDGE;

	broker_run(mbox_inb, id_cluster_fog, nodes_fog, active_devices);
}


static void fog (int argc, char** argv)
{
	const char* value_broker;
	const char* value_qos;
	const char* topic_subs;
	const char* fog_name 		= sg_host_get_name(sg_host_self());
	int end = 0;

	value_broker 				= sg_host_get_property_value(sg_host_self(), "mbox_broker");
	
	value_qos 					= sg_host_get_property_value(sg_host_self(), "qos");
	topic_subs 					= sg_host_get_property_value(sg_host_self(), "subscribe");

	sg_mailbox_t mbox_fog 		= sg_mailbox_by_name(fog_name);
	sg_mailbox_t mbox_broker 	= sg_mailbox_by_name(value_broker);

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
	sg_host_energy_plugin_init();

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