#ifndef OPENFLOW_LOADBALANCER_H
#define OPENFLOW_LOADBALANCER_H

#define OF_DEFAULT_SERVER_NUMBER 4
#define OF_DEFAULT_CLIENT_NUMBER 100

// Tipo de balanceo
enum oflb_type 
{
	OFLB_RANDOM,		// Aleatorio
	OFLB_ROUND_ROBIN,	// Round Robin
  	IP_RANDOM,         	// IP random
	OFLB_ERROR
};

#endif
