
NAME := App_mqtt_azure_client

$(NAME)_SOURCES := azure_main.c \
				   mico_main.c \
				   
$(NAME)_INCLUDE := .

$(NAME)_COMPONENTS :=protocols/azure/azure_c_shared_utility \
						protocols/azure/iothub_client \
						protocols/azure/mico \
						protocols/azure/certs \
						protocols/azure/parson \
					 