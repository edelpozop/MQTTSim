

#include "mqtt_sim.h"

#define TOTAL_EDGE 1


/* Los edge recibiran como parametros: mbox_in, mbox_broker, qos, fname */
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

	int qos = atoi(value_qos);

	/*Conexion con broker*/
	MQTTPackage* connectionBroker 	= (MQTTPackage*) xbt_malloc(sizeof(MQTTPackage));
	connectionBroker->op 			= 0;
	connectionBroker->qos 			= qos;
	sprintf(connectionBroker->mbox, "%s", mbox_edge);
    sg_comm_t comm_send_Conn       	= sg_mailbox_put_async(mbox_broker, connectionBroker, 0);

    printf("Esperando ACK en %s\n", edge_name);

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
		printf("Unexpected behavior with '%s'\n", edge_name);
    }

	char* msgACK = sg_mailbox_get(mbox_edge);
	if (strcmp(msgACK, "Connected") != 0)
	{
		printf("Error\n");
		exit(0);
	}

	printf("Edge Connected\n");

	/*Lectura sensor*/

	FILE *sensor;
    char linea[128]; // Tamaño máximo de una línea
    memset(linea, 0, sizeof(linea));

    // Abre el archivo en modo de lectura
    sensor = fopen(fname, "r");

    // Verifica si el archivo se abrió correctamente
    if (sensor == NULL) {
        printf("No se pudo abrir el archivo.\n");
    }

    // Leer el archivo línea por línea
    while (fgets(linea, sizeof(linea), sensor) != NULL) 
    {

    	MQTTPackage* payload = (MQTTPackage*) xbt_malloc(sizeof(MQTTPackage));
		payload->op = 2;
		payload->qos = qos;
		sprintf(payload->topic,"%s/%s", edge_name, fname);
		sprintf(payload->data,"%s", linea);
		sprintf(payload->mbox,"%s", mbox_edge);
		
		printf("EDGE\t%s\t%s\t%s\n", payload->topic, payload->data, payload->mbox);
	    sg_comm_t comm_send_Pub       = sg_mailbox_put_async(mbox_broker, payload, 0);

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
	        printf("Unexpected behavior with '%s'\n", edge_name);;
	    }

    	if(qos == 0) sg_actor_sleep_for(1);
    	memset(linea, 0, sizeof(linea));
    }

    // Cierra el archivo después de leer todas las líneas
    fclose(sensor);

    /*Desconexion con broker*/
	MQTTPackage* disconnectionBroker = (MQTTPackage*) xbt_malloc(sizeof(MQTTPackage));
	disconnectionBroker->op = 1;
	disconnectionBroker->qos = qos;
	sprintf(disconnectionBroker->mbox,"%s", mbox_edge);
    sg_comm_t comm_send_Disc       = sg_mailbox_put_async (mbox_broker, disconnectionBroker, 0);

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
        printf("Unexpected behavior with '%s'\n", edge_name);
    }

	char* msgACK_disc = sg_mailbox_get(mbox_edge);

	if (strcmp(msgACK_disc, "Disconnected") != 0)
	{
		printf("Error Edge\n");
	}
	printf("Edge Disconnected\n");
}


/* El broker recibe como parametro: mbox_inbroker */
static void broker (int argc, char** argv)
{
	const char* broker_name = sg_host_get_name(sg_host_self());
	sg_mailbox_t mbox_inb = sg_mailbox_by_name(broker_name);
	int nodes_fog = atoi(sg_zone_get_property_value(sg_zone_get_by_name("zone2"), "nodes_fog"));
	int id_cluster_fog = atoi(sg_zone_get_property_value(sg_zone_get_by_name("zone2"), "id_cluster_fog"));
	char ack_broker[128];
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
					char destEnd[128];
					for (int i = 0; i < nodes_fog; i++)
					{
						sprintf(destEnd, "fog-%d-%d", id_cluster_fog, i);

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
					end = 1;
				}
				break;

			case 2:
				char dest[128];
				
				for (int i = 0; i < nodes_fog; i++)
				{
					sprintf(dest, "fog-0-%d", i);

					MQTTPackage* payloadBroker = (MQTTPackage*) xbt_malloc(sizeof(MQTTPackage));
					payloadBroker->op = package.op;
					payloadBroker->qos = package.qos;
					sprintf(payloadBroker->topic,"%s", package.topic);
					sprintf(payloadBroker->data,"%s", package.data);
					sprintf(payloadBroker->mbox,"%s", package.mbox);

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


/* El fog recibe como parametro: mbox_fog, mbox_broker, qos_fog */
static void fog (int argc, char** argv)
{
	const char* value_broker;
	const char* value_qos;
	const char* fog_name 	= sg_host_get_name(sg_host_self());
	const char* fname 		= sg_host_get_name(sg_host_self());
	int end = 0;

	value_broker 	= sg_zone_get_property_value(sg_zone_get_by_name("zone3"), "mbox_broker");
	value_qos 		= sg_zone_get_property_value(sg_zone_get_by_name("zone3"), "qos");

	sg_mailbox_t mbox_fog = sg_mailbox_by_name(fog_name);
	sg_mailbox_t mbox_broker = sg_mailbox_by_name(value_broker);

	int qos_fog = atoi(value_qos);
	
	/*Conexion con broker*/
	MQTTPackage* connectionBroker 			= (MQTTPackage*) xbt_malloc(sizeof(MQTTPackage));
	connectionBroker->op 					= 0;
	connectionBroker->qos 					= qos_fog;
	sprintf(connectionBroker->mbox, "%s", mbox_fog);

    sg_comm_t comm       					= sg_mailbox_put_async(mbox_broker, connectionBroker, 0);

    switch (sg_comm_wait_for(comm, 1.0)) 
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
		printf("Unexpected behavior with '%s'\n", fog_name);
    }

    char* msgACK = sg_mailbox_get(mbox_fog);
    printf("Fog Connected\n");

	if (strcmp(msgACK, "Connected") != 0)
	{
		printf("Error\n");
		end = 1;
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

	int edge_argc           = 0;
	const char* edge_argv[] = {NULL};
	sg_actor_create_("edge0", sg_host_by_name("edge-0-0"), edge, edge_argc, edge_argv);

	int broker_argc           = 0;
	const char* broker_argv[] = {NULL};
	sg_actor_create_("broker0", sg_host_by_name("broker-0-0"), broker, broker_argc, broker_argv);

	int fog_argc           = 0;
	const char* fog_argv[] = {NULL};
	sg_actor_create_("fog0", sg_host_by_name("fog-0-0"), fog, fog_argc, fog_argv);

	simgrid_register_function("edge", edge);
	simgrid_register_function("broker", broker);
	simgrid_register_function("fog", fog);
	

	simgrid_run();
	printf("Simulation time %g\n", simgrid_get_clock());

	return 0;
}