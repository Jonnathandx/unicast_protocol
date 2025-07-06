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
