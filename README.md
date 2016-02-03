# proyectoPSR
Proyecto de la asignatura "Planificación y Simulación de Redes" de 4º curso del Grado en Ingeniería de las Tecnologías de Telecomunicación de la Universidad de Sevilla. 

## Objetivo
Se pretende desarrollar un Sistema balanceador de carga para servidores haciendo uso del simulador ns3-20, por tanto este proyecto ha sido desarrollado en C++. De esta manera se quieren estudiar diferentes algoritmos de balanceo de carga.

## Introducción
Para dotar al sistema de un Balanceador de Carga se ha decidido hacer uso del concepto de Redes Definidas por Software (SDN, por sus siglas en inglés); este tipo de redes se basan en la idea de separar el plano físico del plano de control, de esta manera se puede dotar de cierta inteligencia a la red. 
En el plano físico destaca la presencia de equipos "tradicionales", principalmente Switches; estos equipos no tienen capacidad para tomar decisiones por sí mismos. Esta capacidad de decisión o inteligencia reside en la figura del Controller, un elemento centralizado desde el cual se puede actuar sobre todos los equipo distribuidos. Una vez introducido el concepto de SDN, hay que mencionar un elemento clave en todo este sistema como es el protocolo OpenFlow. Este protocolo rige el intercambio de mensajes entre los elementos de red y el Controller, para que de esta manera las órdenes y el intercambio de información entre todos los dispositivos se realice de manera adecuada y acorde al estándar.

## Topología
En este proyecto se ha diseñado, mediante el simulador ns3-20, una red de equipos clientes que realizan peticiones a un conjunto de servidores. Estas peticiones pasan a través de un switch, el cual es el encargado de balancearlas hacia el servidor adecuado; para realizar esa función de balanceo se han usado diferentes algoritmos implementados en el Controller.

                                               ___
                            [Cliente1]------->| S |------>[Servidor1]
                            [Cliente2]------->| W |------>[Servidor2]
                            [Cliente3]------->| I |------>[Servidor3]
                                ...           | T |
                            [ClienteN]------->| C |------>[ServidorN]
                                              |_H_|                      ______________
                                                |<-.-.-.-.-.-.-.-.-.-.->|__Controller__|
                                                          OpenFlow

La comunicación, mediante OpenFlow, entre el switch y el Controller se realiza cada vez que llega un paquete al switch, básicamente trabaja en dos fases:
- Primera fase: Información
  Cuando llega un paquete de datos al switch, éste manda un mensaje OpenFlow al Controller. Este mensaje contiene toda la información relativa al flujo de comunicación que ha llegado al switch, entre otros atributos cabe destacar: IP_src, IP_dst, port_src, port_dst, procolo, MAC_src, MAC_dst, etc
- Segunda fase: Respuesta
  Una vez que el Controller tiene información sobre el paquete que ha llegado al siwtch, ejecuta su algoritmo para formar el paquete de respuesta. Tras ejecutar el algoritmo y obtener un resultado satisfactorio, el Controller envía un mensaje OpenFlow de respuesta al switch en el que le dice qué acción debe llevar a cabo con el paquete de datos que ha recibido, principalmente este mensaje OpenFlow contiene dos parámetros: la acción (siempre reenviar) y el puerto del switch por el que reenviar, este puerto es el que va variando en función del algoritmo implementado, de esta manera se produce el balanceo entre los servidores.

## Modos de balanceo                                                    
#### Random
En este modo las peticiones se balancean entre los diferentes servidores de manera aleatoria. Cuando llega una petición al switch este determina a qué servidor la envía de manera totalmente aleatoria, para ello se genera un número aleatorio uniforme entre 0 y el número de servidores.
Tras un número suficientemente grande de iteracciones la carga debe ser uniforme en todos los servidores, puesto que estadísticamente todos los servidores tienen la misma probabilidad de recibir una petición.

                            servidores (0,1,2,3,4,...,N) => Aleatoriamente elige uno

#### Round Robin
En este modo las peticiones se balancean entre los diferentes servidores de mediant el protocolo Round Robin. Cuando llega una petición al switch este determina a qué servidor la envía de manera secuencial, para ello almacena en una lista el último servidor al que envió la petición para en el caso actual mandarla al siguiente, cuando llega al final de la lista vuelve a enviarla al primero.
La carga debe ser uniforme en todos los servidores sin tener que esperar a que se realice un número grande de simulaciones, puesto que no depende de ningún factor aleatorio, simplemente asigna de manera circular las peticiones.

                            serv0 -> serv1 -> serv2 -> ... -> servN
                              ^_________________________________|

#### IP Random
En este modo las peticiones se balancean entre los diferentes servidores en dos fases. Primero teniendo en cuenta la IP origen se determina si ésta es par o impar, en caso de que la IP sea par, la petición será atendida solo por servidores pares. En caso de ser impar se atiende por servidores impares. Una vez determinado esto se genera aleatoriamente el servidor que la atiende, es decir, dentro de los pares o los impares se determina cuál es el que atiende la petición.
Tras un número suficientemente grande de iteracciones la carga debe ser uniforme en todos los servidores, puesto que estadísticamente todos los servidores tienen la misma probabilidad de recibir una petición debido a que el tráfico real tendrá el mismo número de IPs pares que impares, y dentro de cada grupo hay la misma probabilidad de elegir un servidor u otro dentro de la lista de servidores pares o impares.

                             |---> IP src par => servidores (0,2,4,6,...,N) => Aleatoriamente elige uno
                     IP ---->|
                             |---> IP src impar => serviores (1,3,5,7,...,N+1) => Aleatoriamente elige uno

## Instalación de OpenFlow en ns3-20:
Debemos instalar los siguientes paquetes:

    sudo apt-get install libboost-all-dev
    sudo apt-get install mercurial

Posteriormente se descarga el código:

    hg clone http://code.nsnam.org/openflow
    cd openflow

Configuramos e instalamos OpenFlow:

    ./waf configure
    ./waf build

Linkamos nuestro ns3 con OpenFlow:

    cd /ruta/hacia/nuestro/ns3/instalado/
    ./waf configure --enable-examples --enable-tests --with-openflow=/ruta/hacia/nuestro/openflow/bajado/antes

Debemos obtener el atributo OpenFlow marcado como "enabled":

    "NS-3 OpenFlow Integration     : enabled"

Por último, completamos la instalación con:

    ./waf build