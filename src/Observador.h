/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */                  
#include <ns3/packet.h>
#include <ns3/average.h>

using namespace ns3;

class Observador
{
public:
  Observador();
  void Pqt_Enviado(Ptr<const Packet> paquete);
  void Pqt_Recibido(Ptr<const Packet> paquete);
  void Pqt_Perdido(Ptr<const Packet> paquete);
  //void Pqt_Reintentado (Ptr<const Packet> paquete);

  uint32_t TotalEnviados();
  //uint32_t TotalReintentos();
  uint32_t TotalPerdidos();
  uint32_t TotalRecibidos();
  uint32_t Total_Paquetes();
  double Get_DelayTime();

  //double Media_Reintentos_Pqt();
  //double Var_Reintentos_Pqt();
  void Reinicio();
  
  double GETRend(double c, uint32_t v);
  double GETCef(uint32_t pkt, uint32_t tamPkt, double intervalo);

private:
  //Average<double> acum_reintentos;
  uint32_t m_PaqTx;
  uint32_t m_PaqRx;
  //uint32_t m_reintentos;
  uint32_t m_perdidos;
  
  //int64_t t_tx;
  int64_t t_rx;
  int64_t t_rx2;
  int flag;
  int64_t t_delay;
  Average<double> avg_time;
};