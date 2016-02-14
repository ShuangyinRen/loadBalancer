![Build Status](https://api.travis-ci.org/stamparm/maltrail.svg?branch=master)
[![C++ 2.6|2.7](https://img.shields.io/badge/C++-14-yellow.svg)](http://www.cplusplus.com/) 
[![License](https://img.shields.io/badge/license-CreativeCommons | NC-red.svg)](http://es.creativecommons.org/blog/licencias/) 
[![Twitter](https://img.shields.io/badge/LinkedIn-Carlos-blue.svg)](https://es.linkedin.com/in/carlosrodriguezhernandez)
[![social](https://img.shields.io/badge/Twitter-carrodher-blue.svg)](https://twitter.com/carrodher)

# Load Balancer
Proyecto de la asignatura "Planificación y Simulación de Redes" de 4º curso del Grado en Ingeniería de las Tecnologías de Telecomunicación de la Universidad de Sevilla. 

## Contenido
- [Objetivo](#objetivo)
- [Introduccion](#introduccion)
- [Topologia](#topologia)
- [Modos de balanceo](#modos-de-balanceo)
 - [Random](#random)
 - [Round Robin](#round-robin)
 - [IP Random](#ip-random)
- [Instalacion de OpenFlow en ns3-20](#instalacion-de-openflow-en-ns3-20)
- [Archivos y ejecucion](#archivos-y-ejecucion)
 - [Ficheros](#ficheros)
 - [Uso](#uso)

## Objetivo
Se pretende desarrollar un Sistema balanceador de carga para servidores haciendo uso del simulador ns3-20, por tanto este proyecto ha sido desarrollado en C++. De esta manera se quieren estudiar diferentes algoritmos de balanceo de carga.

## Introducción
Para dotar al sistema de un Balanceador de Carga se ha decidido hacer uso del concepto de Redes Definidas por Software (SDN, por sus siglas en inglés); este tipo de redes se basan en la idea de separar el plano físico del plano de control, de esta manera se puede dotar de cierta inteligencia a la red. 
En el plano físico destaca la presencia de equipos "tradicionales", principalmente Switches; estos equipos no tienen capacidad para tomar decisiones por sí mismos. Esta capacidad de decisión o inteligencia reside en la figura del Controller, un elemento centralizado desde el cual se puede actuar sobre todos los equipo distribuidos. Una vez introducido el concepto de SDN, hay que mencionar un elemento clave en todo este sistema como es el protocolo OpenFlow. Este protocolo rige el intercambio de mensajes entre los elementos de red y el Controller, para que de esta manera las órdenes y el intercambio de información entre todos los dispositivos se realice de manera adecuada y acorde al estándar.

## Topología
En este proyecto se ha diseñado, mediante el simulador ns3-20, una red de equipos clientes que realizan peticiones a un conjunto de servidores. Estas peticiones pasan a través de un switch, el cual es el encargado de balancearlas hacia el servidor adecuado; para realizar esa función de balanceo se han usado diferentes algoritmos implementados en el Controller.
![Mensajes](http://imgur.com/D07cQ91.png)
La comunicación, mediante OpenFlow, entre el switch y el Controller se realiza cada vez que llega un paquete al switch, básicamente trabaja en dos fases:
- **Primera fase: Información**
  Cuando llega un paquete de datos al switch, éste manda un mensaje OpenFlow al Controller. Este mensaje contiene toda la información relativa al flujo de comunicación que ha llegado al switch, entre otros atributos cabe destacar: IP_src, IP_dst, port_src, port_dst, procolo, MAC_src, MAC_dst, etc
- **Segunda fase: Respuesta**
  Una vez que el Controller tiene información sobre el paquete que ha llegado al siwtch, ejecuta su algoritmo para formar el paquete de respuesta. Tras ejecutar el algoritmo y obtener un resultado satisfactorio, el Controller envía un mensaje OpenFlow de respuesta al switch en el que le dice qué acción debe llevar a cabo con el paquete de datos que ha recibido, principalmente este mensaje OpenFlow contiene dos parámetros: la acción (siempre reenviar) y el puerto del switch por el que reenviar, este puerto es el que va variando en función del algoritmo implementado, de esta manera se produce el balanceo entre los servidores.

## Modos de balanceo
![Flujo](http://imgur.com/QuE5eiW.png)
#### Random
En este modo las peticiones se balancean entre los diferentes servidores de manera aleatoria. Cuando llega una petición al switch este determina a qué servidor la envía de manera totalmente aleatoria, para ello se genera un número aleatorio uniforme entre 0 y el número de servidores.
Tras un número suficientemente grande de iteracciones la carga debe ser uniforme en todos los servidores, puesto que estadísticamente todos los servidores tienen la misma probabilidad de recibir una petición.

![R](http://imgur.com/V6AG0V0.png)
#### Round Robin
En este modo las peticiones se balancean entre los diferentes servidores de mediant el protocolo Round Robin. Cuando llega una petición al switch este determina a qué servidor la envía de manera secuencial, para ello almacena en una lista el último servidor al que envió la petición para en el caso actual mandarla al siguiente, cuando llega al final de la lista vuelve a enviarla al primero.
La carga debe ser uniforme en todos los servidores sin tener que esperar a que se realice un número grande de simulaciones, puesto que no depende de ningún factor aleatorio, simplemente asigna de manera circular las peticiones.

![RR](http://imgur.com/XDLHlI9.png)
#### IP Random
En este modo las peticiones se balancean entre los diferentes servidores en dos fases. Primero teniendo en cuenta la IP origen se determina si ésta es par o impar, en caso de que la IP sea par, la petición será atendida solo por servidores pares. En caso de ser impar se atiende por servidores impares. Una vez determinado esto se genera aleatoriamente el servidor que la atiende, es decir, dentro de los pares o los impares se determina cuál es el que atiende la petición.
Tras un número suficientemente grande de iteracciones la carga debe ser uniforme en todos los servidores, puesto que estadísticamente todos los servidores tienen la misma probabilidad de recibir una petición debido a que el tráfico real tendrá el mismo número de IPs pares que impares, y dentro de cada grupo hay la misma probabilidad de elegir un servidor u otro dentro de la lista de servidores pares o impares.

![IR](http://imgur.com/gtE8xIR.png)
## Instalacion de OpenFlow en ns3-20
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

## Archivos y ejecucion
#### Ficheros

    controller.h => Librería con las definiciones necesarias en los Controllers
    iprandom_controller.cc  => Controller IP random
    loadbalancer.h => Librería con las definiciones necesarias en el escenario
    Observador.cc => Implementación del observador para obtener datos
    Observador.h  => Librería con las definiciones necesarias en el observador
    random_controller.cc => Controller random
    roundrobin_controller.cc => Controller round-robin
    topologia_ServGraf.cc => Escenario y simulación pintando gráficas en función del tipo de algoritmo introducido
    topologia_TipoGraf.cc => Escenario y simulación pintando gráficas comparando los tres tipos

NOTA: Para ejecutar se deben incluir estos ficheros en la carpeta ns-3.20/scratch/ en un directorio que se llame con el nombre que se desee ejecutar, por ejemplo, "loadBalancer". En este directorio sólo debe aparecer uno de los ficheros topologia_xxxx.cc,según el tipo de simulación que se desee realizar.

#### Uso

    NS_LOG="Topologia" ./waf --run "loadBalancer --number=2 --type=random --dataRate=100Mbps --delay=5us"
              |                          |            |            |                |           |-> Retraso del canal
              |                          |            |            |                |-> Tasa del canal
              |                          |            |            |-> Tipo de balanceo (random, round-robin ó ip-random)
              |                          |            |-> Número inicial de servidores
              |                          |-> Nombre del directorio en ns-3.20/scratch/
              |-> Activación de las trazas:
                        |-> Topologia para el escenario
                        |-> random para el random Controller
                        |-> roundrobin para el round-robin Controller
                        |-> ip-random para el IP random Controller
