NAME := Lib_iothub_client_Framework


$(NAME)_SOURCES := blob.c \
				   iothub_client.c \
				   iothub_client_ll.c \
				   iothub_message.c \
				   iothubtransport.c \
				   iothubtransport_mqtt_common.c \
				   iothubtransporthttp.c \
				   iothubtransportmqtt.c \
				   iothubtransportmqtt_websockets.c \
				   iothub_client_ll_uploadtoblob.c \
				   iothub_client_authorization.c \
				   version.c 			   

$(NAME)_COMPONENTS += protocols/azure/azure_c_shared_utility \
					  protocols/azure/umqtt
		   
GLOBAL_INCLUDES += . 