/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#include <string.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <ns3/applications-module.h>
#include <ns3/random-variable.h>
#include <ns3/ipv4-global-routing-helper.h>
#include <ns3/onoff-application.h>
#include <ns3/gnuplot.h>
#include <ns3/core-module.h>
#include <ns3/network-module.h>
#include <ns3/csma-module.h>
#include <ns3/internet-module.h>
#include <ns3/applications-module.h>
#include <ns3/openflow-module.h>
#include <ns3/log.h>
#include "ns3/point-to-point-module.h"

#include "loadbalancer.h"
#include "controller.h"
#include "Observador.h"

using namespace ns3;
NS_LOG_COMPONENT_DEFINE ("Topologia");

// Se tienen client_number dispositivos que actúan como clientes y server_number dispostivos
// que actúan como servidores. Todos ellos se conectan a un switch, el cual está asociado
// a un dispositivo controlador, el cual mediante OpenFlow es capaz de gestionar el comportamiento del switch.
// En la implementación final: switch+servidores en la misma red y clientes en internet

// Topología
//				 172.16.0.0/16
//                clientes
//                   |
//       ------------------------		 	  ---------------
//       |        switch        |<----------->|	Controller	|
//       ------------------------		      ---------------
//        |      |      |      |
//      serv0  serv1   ...  servN
//             172.16.0.1
//

// En esta primera versión está todo desarrollado en una red local, es decir, clientes, servidores y sitch se encuentran
// en la misma red, por eso no se ha usado IP para encaminar

// To Do:
//	- Usar enrutamiento IP
//	- Jerarquizar clientes mediantes más switchs fuera de la red local
// 	- Parámetros por línea de comando en el lanzamiento:
//		- Tipo de balanceo
//		- Atributos enlaces
//		- Nº Servidores
//		- ...

// Controla la entrada del tipo de balanceo
oflb_type ParseType(char *lb_type_str)
{
    if (strcmp(lb_type_str,"round-robin") == 0)
    {
        return OFLB_ROUND_ROBIN;
    }
    else if (strcmp(lb_type_str,"ip-random") == 0)
    {
        return OFLB_IP_RANDOM;
    }
    else
    {
        return OFLB_RANDOM;
    }
}

void simulacion(int client_number, int server_number, DataRate dataRate, Time delay, oflb_type lb_type, Observador *observadores)
{
    // Se crean los contenedores de nodos...
    // ... clientes
    NS_LOG_INFO ("Creando clientes...");
    NodeContainer clients;
    clients.Create(client_number);

    // ... servidores
    NS_LOG_INFO ("Creando servidores...");
    NodeContainer servers;
    servers.Create(server_number);

    // ... switch
    NS_LOG_INFO ("Creando switch...");
    NodeContainer csmaSwitch;
    csmaSwitch.Create(1);

    // Características del canal
    CsmaHelper csma;
    csma.SetChannelAttribute("DataRate", DataRateValue(dataRate));
    csma.SetChannelAttribute("Delay", TimeValue(delay));

    // Creamos los enlaces entre los equipos y el switch
    NetDeviceContainer clientDevices;
    NetDeviceContainer serverDevices;
    NetDeviceContainer switchDevices;

    // --- Servidores
    for (int i = 0; i < server_number; i++)
    {
        // Primero se crea un NodeContainer provisional con el servidor i y el NodeContainer del switch
        // Se crea un NetDevice csma, se añade el device al nodo y se añade un canal al device.
        // link es un container con todos los devices añadidos
        NetDeviceContainer link = csma.Install(NodeContainer(servers.Get(i), csmaSwitch));
        // Separa por un lado los devices de servidores y por otro los del switch
        serverDevices.Add(link.Get(0));		// Añade este device del servidor i al container de devices de servidores
        switchDevices.Add(link.Get(1));		// Añade el device del switch al container de devices del switch
    }

    // --- Clientess
    for (int i = 0; i < client_number; i++)
    {
        // Primero se crea un NodeContainer provisional con el cliente i y el NodeContainer del switch
        // Se crea un NetDevice csma, se añade el device al nodo y se añade un canal al device.
        // link es un container con todos los devices añadidos
        NetDeviceContainer link = csma.Install(NodeContainer(clients.Get(i), csmaSwitch));
        // Separa por un lado los devices de clientes y por otro los del switch
        clientDevices.Add(link.Get(0));		// Añade este device del cliente i al container de devices de clientes
        switchDevices.Add(link.Get(1));		// Añade el device del switch al container de devices del switch
    }

    // Creamos el nodo switch con OpenFlow, el cual realizará las tareas de reenvío
    Ptr<Node> switchNode = csmaSwitch.Get(0);
    OpenFlowSwitchHelper swtch;

    // Creamos el controlador vacío, en función del modo elegido se asignará posteriormente una u otra implementación
    Ptr<ns3::ofi::Controller> controller = NULL;

    switch (lb_type)
    {
        // Modo aleatorio
        case OFLB_RANDOM:
        {
            NS_LOG_INFO("RANDOM (Usando balanceo de carga aleatorio)");
            // Ahora si creamos el controlador de este tipo
            controller = CreateObject<ns3::ofi::RandomizeController>(server_number);
            // Instala switch:
            //    - Añade el dispositivo swtch al nodo switchNode
            //    - Añade los devices switchDevices como puertos del switch
            //    - Establece el controlador para este modo de balanceo
            swtch.Install(switchNode, switchDevices, controller);
            break;
        }
        // Modo Round Robin
        case OFLB_ROUND_ROBIN:
        {
            NS_LOG_INFO("ROUND_ROBIN (Usando balanceo de carga Round Robin)");
            // Ahora si creamos el controlador de este tipo
            controller = CreateObject<ns3::ofi::RoundRobinController>(server_number);
            // Instala switch:
            //    - Añade el dispositivo swtch al nodo switchNode
            //    - Añade los devices switchDevices como puertos del switch
            //    - Establece el controlador para este modo de balanceo
            swtch.Install(switchNode, switchDevices, controller);
            break;
        }
        case OFLB_IP_RANDOM:
        {
            NS_LOG_INFO("IP_RANDOM (Usando balanceo de carga basado en IP par o impar)");
            // Ahora si creamos el controlador de este tipo
            controller = CreateObject<ns3::ofi::IpRandomController>(server_number);
            // Instala switch:
            //    - Añade el dispositivo swtch al nodo switchNode
            //    - Añade los devices switchDevices como puertos del switch
            //    - Establece el controlador para este modo de balanceo
            swtch.Install(switchNode, switchDevices, controller);
            break;
        }
        default:
        {
            // No tipo de balanceo, no party
            break;
        }
    }

    // Añadimos la pila de internet a los equipos
    InternetStackHelper internet;
    internet.Install(servers);
    internet.Install(clients);

    // Asigna dirección IP a los servidores
    NS_LOG_INFO ("Asignando dirección IP a los servidores...");
    for (int i = 0; i < server_number; i++)
    {
        Ptr<NetDevice> device = serverDevices.Get(i);	             // device = device del servidor i
        Ptr<Node> node = device->GetNode();				                 // node = nodo servidor i
        Ptr<Ipv4> ipv4 = node->GetObject<Ipv4>();		               // ipv4 = objeto IP del nodo servidor i

        int32_t interface = ipv4->GetInterfaceForDevice(device); 	 // interface = interfaz del servidor i
        // Si no tiene interfaz, se le asigna
        if (interface == -1)
        {
            interface = ipv4->AddInterface(device);
        }
        // Todos los servidores con una única IP/máscara (172.16.0.1/24)
        // De cara al exterior no se dan detalles del tipo de balanceo, nº servidores, etc; se accede a una IP
        // e internamente se realiza el balanceo de forma transparente al usuario
        Ipv4InterfaceAddress ipv4Addr = Ipv4InterfaceAddress("172.16.0.1", "255.255.255.0");
        ipv4->AddAddress(interface, ipv4Addr);
        ipv4->SetMetric(interface, 1);
        ipv4->SetUp(interface);
    }

    // Asigna dirección IP a los clientes
    NS_LOG_INFO ("Asignando dirección IP a los clientes...");
    // Todos los clientes con IPs de la subred 172.16.0.0/16
    Ipv4AddressHelper ipv4;
    ipv4.SetBase("172.16.0.0", "255.255.0.0", "0.0.0.2");
    ipv4.Assign(clientDevices);

    uint16_t port= 9;   // Puerto para la conexión
    ApplicationContainer appClients;	// Contenedor para las aplicaciones de cliente
    ApplicationContainer appServers;  // Contenedor para las aplicaciones de servidor

    // Creamos aplicación para los clientes
    NS_LOG_INFO ("Creando aplicación para los clientes...");

    // Aplicación on/off
    double ton = 3.0;
    double toff = 1.2;
    OnOffHelper onoff("ns3::UdpSocketFactory", Address(InetSocketAddress(Ipv4Address("172.16.0.1"), port)));
    onoff.SetConstantRate(DataRate("500kbps"));

    std::ostringstream str_ton;
    std::ostringstream str_toff;
    str_ton << "ns3::ExponentialRandomVariable[Mean=" << ton <<"]";
    str_toff << "ns3::ExponentialRandomVariable[Mean=" << toff <<"]";
    onoff.SetAttribute("OnTime",StringValue(str_ton.str()));
    onoff.SetAttribute("OffTime",StringValue(str_toff.str()));

    // Tiempo de simulacion
    int stop = 4;

    // Instala la aplicación en todos los clientes
    appClients = onoff.Install(clients);
    appClients.Start(Seconds(2.0));
    appClients.Stop(Seconds(stop));

    // Creamos sumideros para los servidores
    NS_LOG_INFO ("Creando sumidero para los servidores...");

    // Sumidero
    PacketSinkHelper sink("ns3::UdpSocketFactory",Address(InetSocketAddress(Ipv4Address::GetAny(), port)));

    // Instala el sumidero en los servidores

    appServers = sink.Install(servers);
    appServers.Start(Seconds(1.0));
    appServers.Stop(Seconds(stop));

    // Suscribimos cada server a un observador.
    for (int i = 0; i < server_number; i++)
    {
        serverDevices.Get(i)->TraceConnectWithoutContext ("PhyRxDrop", MakeCallback(&Observador::Pqt_Perdido, &(observadores[i])));
        serverDevices.Get(i)->TraceConnectWithoutContext ("MacTx", MakeCallback(&Observador::Pqt_Enviado, &(observadores[i])));
        serverDevices.Get(i)->TraceConnectWithoutContext ("MacRx", MakeCallback(&Observador::Pqt_Recibido, &(observadores[i])));
    }

    NS_LOG_INFO ("Lanzando simulación");
    Simulator::Run();
    Simulator::Destroy();
    NS_LOG_INFO ("Hecho");
}

int main (int argc, char *argv[])
{
    GlobalValue::Bind("ChecksumEnabled", BooleanValue(true));
    Time::SetResolution(Time::US);

    int client_number = OF_DEFAULT_CLIENT_NUMBER;  // Número de clientes
    int server_number = OF_DEFAULT_SERVER_NUMBER;  // Número de servidores
    oflb_type lb_type = OFLB_RANDOM;               // Tipo de balanceo
    DataRate dataRate = DataRate("0Mbps");		   // Tasa bits
    Time delay = Time("0us");         		       // Retardo de propagación en el bus (delay)

    char server_number_str[1024];       // Para convertir entrada
    char lb_type_str[1024];             // Para convertir entrada

    memset(server_number_str,0,1024);
    memset(lb_type_str,0,1024);

    CommandLine cmd;
    cmd.AddValue ("number", "Número de servidores", server_number_str);				// Con callback para parsear
    cmd.AddValue ("type", "Algoritmo de balanceo [random,round-robin o ip-random]", lb_type_str);	// Con callback para convertir
    cmd.AddValue("dataRate","Tasa de bit",dataRate);												// Sin callback porque se puede pasar string a DataRateValue()
    cmd.AddValue("delay","Retardo de propagación en el bus",delay);									// Sin callback porque se puede pasar string a TimeValue()
    cmd.Parse(argc,argv);

    // Comprobamos valores por línea de comandos, si no hay se usan por defecto.
    // Si las cadenas de server_number y lb_type estan vacías no se hace nada,
    // puesto que los valores enteros, ya estaban establecidos por defecto.
    // En caso contrario, se sobreescriben los valores con los introducios por cmd.
    if(strcmp(server_number_str,"") != 0) {
        server_number = atoi(server_number_str);
    }
    if(strcmp(lb_type_str,"") != 0) {
        lb_type = ParseType(lb_type_str);
    }

    // Con dataRate y delay se establecen cero al inicio.
    // Si tras leer los valores de cmd siguen siendo cero, se inicializan a unos
    // valores por defecto.
    if(dataRate.GetBitRate() == 0) {
        dataRate = DataRate("100Mbps");
    }
    if(delay.GetMicroSeconds() == 0) {
        delay = Time("6.56us");
    }


    // Parámetros elegidos:
    NS_LOG_INFO ("				Numero clientes : " << client_number);
    NS_LOG_INFO ("				Numero servidores : " << server_number);
    switch (lb_type)
    {
        case OFLB_RANDOM:
        {
            NS_LOG_INFO("				Algoritmo: RANDOM");
            break;
        }
        case OFLB_ROUND_ROBIN:
        {
            NS_LOG_INFO("				Algoritmo: ROUND_ROBIN");
            break;
        }
        case OFLB_IP_RANDOM:
        {
            NS_LOG_INFO("				Algoritmo: IP_RANDOM");
            break;
        }
        default:
        {
            break;
        }
    }

    // Gráficas
    Gnuplot plot1;
    plot1.SetTitle ("Gráfica 1: Paquetes recibidos aumentando el nº clientes en IP-random");
    plot1.SetLegend("Número de clientes/servidor","Paquetes recibidos");
    Gnuplot2dDataset datos11("Servidores: 2");
    datos11.SetStyle (Gnuplot2dDataset::LINES_POINTS);
    Gnuplot2dDataset datos12("Servidores: 4");
    datos12.SetStyle (Gnuplot2dDataset::LINES_POINTS);
    Gnuplot2dDataset datos13("Servidores: 6");
    datos13.SetStyle (Gnuplot2dDataset::LINES_POINTS);

    Gnuplot plot2;
    plot2.SetTitle ("Gráfica 2: Tiempo entre paquetes aumentando el nº clientes en IP-random");
    plot2.SetLegend("Número de clientes/servidor","Tiempo entre paquetes (ms)");
    plot2.AppendExtra ("set xrange [0:+200]");
    Gnuplot2dDataset datos21("Servidores: 2");
    datos21.SetStyle (Gnuplot2dDataset::LINES_POINTS);
    Gnuplot2dDataset datos22("Servidores: 4");
    datos22.SetStyle (Gnuplot2dDataset::LINES_POINTS);
    Gnuplot2dDataset datos23("Servidores: 6");
    datos23.SetStyle (Gnuplot2dDataset::LINES_POINTS);


    int stepClient = 50;		// Incremento de clientes en cada iteracción
    int clients, servers; 	// Número inicial de servidores y clientes desde el que se empieza a sumar
    int relation = 0; 		// Relación clientes/servidores

    for (int k=0; k < 3; k++)
    {
        servers = server_number + (2*k);	// Servidores aumentando
        clients = client_number;			// Valor inicial del número de clientes

        while(relation<200)
        {
            relation = clients/servers; 		// Calcula la relación cliente/servidor

            NS_LOG_INFO ("Número clientes : " << clients);
            NS_LOG_INFO ("Número servidores : " << servers);
            NS_LOG_INFO ("Relación clientes/servidor : " << relation);

            Average<double> avg_total_received;
            Average<double> avg_total_lost;
            Average<double> avg_time_delay;

            // Creamos observadores
            Observador observadores[servers];

            simulacion(clients, servers, dataRate, delay, lb_type, observadores);

            for(int i=0;i < servers;i++)
            {
                // Para calcular porcentajes
                avg_total_received.Update(observadores[i].TotalRecibidos());
                avg_time_delay.Update(observadores[i].Get_DelayTime());
                avg_total_lost.Update(observadores[i].TotalPerdidos());
            }
            if (k==0)
            {
                datos11.Add(relation,avg_total_received.Mean());
                datos21.Add(relation,avg_time_delay.Mean());
            }
            else if (k==1)
            {
                datos12.Add(relation,avg_total_received.Mean());
                datos22.Add(relation,avg_time_delay.Mean());
            }
            else if (k==2)
            {
                datos13.Add(relation,avg_total_received.Mean());
                datos23.Add(relation,avg_time_delay.Mean());
            }
            // Aumenta clientes. Servidores constantes
            clients = clients + stepClient;
        }
        relation=0;
    }
    // Asigna valores a las gráficas
    plot1.AddDataset(datos11);
    plot1.AddDataset(datos12);
    plot1.AddDataset(datos13);
    plot2.AddDataset(datos21);
    plot2.AddDataset(datos22);
    plot2.AddDataset(datos23);

    std::ofstream fichero1 ("proyectopsr-gf1.plt");
    plot1.GenerateOutput (fichero1);
    fichero1 << "pause -1" << std::endl;
    fichero1.close();

    std::ofstream fichero2("proyectopsr-gf2.plt");
    plot2.GenerateOutput (fichero2);
    fichero2 << "pause -1" << std::endl;
    fichero2.close();

    return 0;
}
