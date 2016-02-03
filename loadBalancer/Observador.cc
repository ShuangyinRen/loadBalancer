/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#include <ns3/core-module.h>
#include "Observador.h"
#include <iostream>
#include <string.h>

#include "ns3/object.h"
#include "ns3/uinteger.h"
#include "ns3/traced-value.h"
#include "ns3/trace-source-accessor.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Observador");

Observador::Observador()
{
	t_rx = 0.0;		// Tiempo de recepción
	t_rx2 = 0.0;	
	flag = 0;
  	//t_tx = 0.0;	// Tiempo de transmisión
	t_delay = 0.0;	// Diferencia (retraso)
	m_PaqTx = 0;	// Num. pkts transmitidos
	m_PaqRx = 0;	// Num. pkts recibidos
}

void Observador::Pqt_Recibido(Ptr <const ns3::Packet> p)
{
  	// Guardamos el instante en el que se recibe un paquete. 
	if (flag == 0) 
	{
		t_rx = Simulator::Now().GetMilliSeconds();
		flag = 1;
	}
	else 
	{
		t_rx2 = Simulator::Now().GetMilliSeconds();
		t_delay = t_rx2-t_rx;
		avg_time.Update(t_delay);
		flag = 0;
	}
	
	m_PaqRx++; // Aumentamos numero de paquetes recibidos.
}

void Observador::Pqt_Enviado(Ptr <const ns3::Packet> p)
{
	// Obtenemos instante en el que se transmite el paquete recibido.
 	// t_tx = Simulator::Now().GetMilliSeconds();
 	// t_delay = t_tx-t_rx;
	// avg_time.Update(t_delay);

	m_PaqTx++; // Aumentamos numero de paquetes transmitidos.
}

void Observador::Pqt_Perdido(Ptr<const Packet> paquete) 
{
	m_perdidos++; // Aumentamos numero de paquetes perdidos
}

// Obtenemos tiempo medio de retardo
double Observador::Get_DelayTime()
{
	return avg_time.Mean();
}

// Devuelve los paquetes perdidos
uint32_t Observador::TotalPerdidos()
{
	return(m_perdidos);
}

// Devuelve los paquetes recibidos
uint32_t Observador::TotalRecibidos()
{
	return(m_PaqRx);
}

// Devuelve el total de paquetes
uint32_t Observador::Total_Paquetes()
{
	return m_PaqTx;
}

double Observador::GETCef(uint32_t pkt, uint32_t tamPkt, double intervalo)
{
	NS_LOG_INFO("Cadencia: " << (double)((pkt/2)*tamPkt)/intervalo);
	return (double)((pkt/2)*tamPkt)/intervalo;
}

double Observador::GETRend(double c, uint32_t v)
{
	NS_LOG_INFO("Rendimiento: " << (c/v)*1000 << "%");
	return (c/v)*1000;
}

void Observador::Reinicio()
{
	m_PaqTx = 0;
	m_PaqRx = 0;
	m_perdidos = 0;
  	//m_reintentos = 0;
  	//acum_reintentos.Reset();
	avg_time.Reset();
}