#ifndef OPENFLOW_CONTROLLER_H
#define OPENFLOW_CONTROLLER_H

#include "ns3/openflow-interface.h"
#include "ns3/openflow-switch-net-device.h"

namespace ns3 {

namespace ofi {

// Controlador con algoritmo aleatorio
class RandomizeController : public Controller {
  public:
    RandomizeController(int server_number);
    void ReceiveFromSwitch (Ptr<OpenFlowSwitchNetDevice> swtch, ofpbuf* buffer);
 
  protected:
    int num_servers; 
    struct LearnedState
    {
     uint32_t port;                      // Puerto aprendido
    };

    typedef std::map<Mac48Address, LearnedState> LearnState_t;  // Relación MAC/Puerto
    LearnState_t m_learnState;            // Datos aprendidos
};

// Controlador con algoritmo Round Robin
class RoundRobinController : public Controller 
{
	public:
    	RoundRobinController(int server_number);
    	void ReceiveFromSwitch (Ptr<OpenFlowSwitchNetDevice> swtch, ofpbuf* buffer);
  
	protected:
    	int num_servers;

    	// Tabla MAC del switch
    	struct LearnedState
    	{
     		uint32_t port;                      // Puerto aprendido
    	};

    	typedef std::map<Mac48Address, LearnedState> LearnState_t;  // Relación MAC/Puerto
    	LearnState_t m_learnState;            // Datos aprendidos

    	// Almacena la lista de puertos para realizar el balanceo Round Robin
    	struct RoundRobinState
    	{
	   		uint32_t port;             // Último puerto usado
    	};

    	typedef std::map<Ipv4Address, RoundRobinState> RoundRobinState_t; // Relación IP/rr
    	RoundRobinState_t m_lastState;    
};

// Controlador con algoritmo aleatorio basado en IP par o impar
class IpRandomController : public Controller 
{
	public:
		IpRandomController(int server_number);
    	void ReceiveFromSwitch(Ptr<OpenFlowSwitchNetDevice> swtch, ofpbuf* buffer);
  
  	protected:
    	int num_servers;

    	// Tabla MAC del switch
    	struct LearnedState
    	{
     		uint32_t port;                      // Puerto aprendido
    	};
    
    	typedef std::map<Mac48Address, LearnedState> LearnState_t;  // Relación MAC/Puerto
    	LearnState_t m_learnState;            // Datos aprendidos
};
}
}
#endif
