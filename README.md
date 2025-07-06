# VANET Unicast Multi-Hop Routing Protocol in Veins

Este repositorio contiene la implementación de un protocolo de enrutamiento unicast multi-salto para redes vehiculares ad hoc (VANETs) utilizando el simulador Veins (Vehicles in Network Simulation).

## 📌 Descripción General

El protocolo implementado permite que un nodo origen envíe un mensaje WSM (Wave Short Message) a un nodo destino a través de múltiples saltos intermedios. La selección de cada nodo de reenvío se basa en una **tabla de vecinos** (Neighbor Node Table, NNT) construida mediante la recepción periódica de mensajes beacon enriquecidos.

Cada nodo toma decisiones de reenvío localmente, priorizando el vecino con mejor **puntaje global (GS)** basado en criterios como distancia, densidad vehicular y antigüedad de la información.

## 📁 Estructura del Repositorio

```bash
.
├── beacon.cc                # Módulo para manejo de la tabla de vecinos (NNT)
├── DemoBaseApplLayer.cc     # Lógica común de aplicación: beacons, WSM, WSA, movilidad
├── TraCIDemo11p.cc          # Implementación específica del protocolo unicast multisalto
├── README.md                # Este archivo

⚙️ Requisitos
OMNeT++ 5.6.2

Veins 5.3.1

SUMO (Simulation of Urban MObility)

🚦 Funcionamiento del Protocolo
Beacons periódicos
Cada nodo envía beacons que incluyen:

ID del nodo y del mensaje (TreeID)

Coordenadas actuales e iniciales

Tipo de nodo (vehículo o RSU)

Densidad local de vecinos

Construcción de la NNT
Los nodos que reciben beacons actualizan su tabla de vecinos (beacon.cc), purgan entradas antiguas y ordenan la lista según una métrica de reenvío.

Transmisión unicast WSM

El nodo origen envía un WSM al nodo mejor posicionado (top de la NNT).

Cada nodo intermedio reenvía el mensaje al siguiente mejor vecino.

El nodo destino calcula el retardo y el número de saltos.

Respuesta al origen
El nodo destino puede enviar una respuesta de vuelta al origen utilizando la misma lógica de reenvío.

🧮 Métrica de Selección (GS)
El puntaje global (Global Score) de cada vecino se calcula con la fórmula:

ini
Copy
Edit
GS = distancia + 2 × densidad + 0.4 × antigüedad del beacon
Esto favorece a nodos cercanos al destino, con buena conectividad y datos recientes.

📊 Lógica de Archivos
TraCIDemo11p.cc
Maneja la lógica principal del protocolo: envío/recepción de WSMs, reenvío según la NNT, y procesamiento de beacons.

DemoBaseApplLayer.cc
Gestiona movilidad, generación de beacons, WSAs, y suscripciones a eventos.

beacon.cc
Implementa la estructura de lista enlazada para la NNT: agregar, actualizar, ordenar, eliminar y seleccionar vecinos.

🔍 Cómo Probarlo
Crea un escenario en SUMO con al menos 3 nodos móviles y sin RSUs.

Asigna IDs fijos (por ejemplo, origen = 16, destino = 40).

Ajusta el archivo .ini para controlar los tiempos de simulación y beaconing.

Ejecuta la simulación y observa en el log el retardo y número de saltos.

📈 Futuras Mejoras
Inclusión de calidad de enlace como parámetro de reenvío.

ACKs y retransmisiones para mayor confiabilidad.

Integración de topologías con RSUs y escenarios urbanos más complejos.

Comparación con protocolos estándar como GPSR o DSR.

👨‍💻 Autor
Desarrollado por [Tu Nombre] como parte de un proyecto académico/investigación sobre protocolos de comunicación en VANETs usando Veins.

