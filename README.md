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
```
## ⚙️ Requisitos

Para compilar y ejecutar correctamente este protocolo, se requiere el siguiente entorno:

- **OMNeT++ 5.6.2**
- **Veins 5.3.1**
- **SUMO (Simulation of Urban MObility)**
- **INET Framework** (opcional, solo si se usa para modelos de red adicionales)
- Sistema operativo: **Ubuntu 20.04+** (recomendado)
- Paquetes adicionales: `make`, `g++`, `libgdal`, `libxml2`, `libxerces-c-dev`, entre otros requeridos por Veins y SUMO.

## 🚦 Funcionamiento del Protocolo

1. **Generación de beacons:**  
   Cada nodo genera periódicamente un mensaje tipo beacon que contiene su posición, velocidad, tipo (vehículo o RSU), densidad de vecinos y otros datos relevantes.

2. **Construcción de la NNT:**  
   Los nodos que reciben beacons almacenan esta información en una tabla de vecinos (Neighbor Node Table - NNT), implementada como una lista enlazada en `beacon.cc`.

3. **Selección del siguiente salto:**  
   Antes de reenviar un mensaje WSM, el nodo selecciona el mejor candidato de su NNT basado en un **puntaje global (GS)**, que pondera factores como la distancia hacia el destino, la densidad local y la antigüedad del beacon.

4. **Reenvío unicast:**  
   El mensaje se transmite nodo a nodo hasta llegar al destino. Cada salto actualiza la información y reevalúa a su mejor vecino antes de reenviar.

5. **Nodo destino:**  
   Una vez recibido el mensaje, el nodo destino registra métricas como el número total de saltos y el retardo total, y puede enviar una respuesta si se requiere.

## 🧠 Métrica de Selección (Global Score - GS)

El protocolo utiliza una métrica compuesta para clasificar a los vecinos en la NNT:

