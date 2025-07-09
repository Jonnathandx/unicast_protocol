//
// Copyright (C) 2006-2011 Christoph Sommer <christoph.sommer@uibk.ac.at>
// ... [cabecera omitida por brevedad]

#include <set>
#include <utility>

#include "veins/modules/application/traci/TraCIDemo11p.h"
#include "veins/modules/application/traci/TraCIDemo11pMessage_m.h"

using namespace veins;

Define_Module(veins::TraCIDemo11p);

void TraCIDemo11p::initialize(int stage)
{
    DemoBaseApplLayer::initialize(stage);
    if (stage == 0) {
        sentMessage = false;
        lastDroveAt = simTime();
        currentSubscribedServiceId = -1;

        sentWSMs = 0;                   // Variable para el número de mensajes enviados
        receivedWSMs = 0;               // Variable para el número de mensajes recibidos
        intervalWSM = 2;                // Intervalo de tiempo de envio de mensajes WSM
        startWSMTime = 65;              // Tiempo de simulación en donde se inicia el envío de mensajes WSM
        maxWSMs = 5;                    // Número de mensajes WSM a enviar
    }
}

void TraCIDemo11p::onWSA(DemoServiceAdvertisment* wsa)
{
    if (currentSubscribedServiceId == -1) {
        mac->changeServiceChannel(static_cast<Channel>(wsa->getTargetChannel()));
        currentSubscribedServiceId = wsa->getPsid();
        if (currentOfferedServiceId != wsa->getPsid()) {
            stopService();
            startService(static_cast<Channel>(wsa->getTargetChannel()), wsa->getPsid(), "Mirrored Traffic Service");
        }
    }
}

void TraCIDemo11p::onBSM(DemoSafetyMessage* bsm){
    Coord current_Pos = mobility->getPositionAt(simTime());
    int nodeIndex = getParentModule()->getIndex();
    Coord senderPos = bsm->getIni_position();
    double distancia = current_Pos.distance(senderPos);

    if (!forwardMsg && distancia >= 3000) {
        forwardMsg = true;
        bsm->setDistancia(distancia);
        ListBeacon.SortBeacons();
        EV << ListBeacon.TopTable() << endl;
    }

    PreprocessingPreparaBeaconValues(bsm);
    bool Msg_Processed = ListBeacon.SearchBeaconTreeID(Table_Ngh_Msg_TreeID);
    if (!Msg_Processed) {
        UpdateNNT(bsm);
    }
    ListBeacon.SortBeacons();
}

void TraCIDemo11p::onWSM(BaseFrame1609_4* frame)
{
    EV << "WSM recibido.\n";

    if (frame->getRecipientAddress() != myId) {
        EV << "Mensaje WSM no dirigido a este nodo (" << myId << "). Ignorando.\n";
        return;
    }

    int destinoId = frame->par("destinoId");
    int origenId = frame->par("origenId");

    if (myId == destinoId) {
        EV << "Nodo destino (ID " << myId << ") recibió el mensaje.\n";

        double tiempo_envio = frame->par("timestamp");
        double retraso = simTime().dbl() - tiempo_envio;
        int saltos = frame->par("saltos");

        receivedWSMs++;

        EV << "Delay: " << retraso << "s\n";
        EV << "Número total de saltos: " << saltos << "\n";

        return;
    }

    int nextHop = ListBeacon.TopTable();
    int saltos = frame->par("saltos");

    if (nextHop == -1) {
        EV << "Ruta no encontrada hacia el origen.\n";
        return;
    }

    BaseFrame1609_4* reply = new BaseFrame1609_4();
    populateWSM(reply);
    reply->setKind(99);
    reply->addPar("origenId") = origenId;
    reply->addPar("destinoId") = destinoId;
    reply->addPar("timestamp") = frame->par("timestamp");
    reply->addPar("saltos") = saltos + 1;
    reply->setRecipientAddress(nextHop);

    sendDown(reply);
    EV << "Reenviando WSM desde nodo " << myId << " a " << nextHop << "\n";
}

void TraCIDemo11p::handleSelfMsg(cMessage* msg)
{
    if (TraCIDemo11pMessage* wsm = dynamic_cast<TraCIDemo11pMessage*>(msg)) {
        sendDown(wsm->dup());
        wsm->setSerial(wsm->getSerial() + 1);
        if (wsm->getSerial() >= 3) {
            stopService();
            delete (wsm);
        }
        else {
            scheduleAt(simTime() + 1, wsm);
        }
    }
    else {
        DemoBaseApplLayer::handleSelfMsg(msg);
    }
}

void TraCIDemo11p::handlePositionUpdate(cObject* obj)
{
    DemoBaseApplLayer::handlePositionUpdate(obj);

    checkEndOfTransmission(); // Se verifica si se ha terminado de enviar los mensajes, de ser así se muestran los valores estadísticos

    if (myId == 46 && simTime() >= startWSMTime && simTime() < startWSMTime + intervalWSM * maxWSMs) {
        if ((int)((simTime() - startWSMTime).dbl()) % (int)intervalWSM.dbl() == 0 && sentWSMs < maxWSMs) {
            static simtime_t lastSend = -1;
            if (lastSend == simTime()) return;
            lastSend = simTime();

            BaseFrame1609_4* reply = new BaseFrame1609_4();
            populateWSM(reply);
            reply->setKind(99);
            reply->addPar("origenId") = myId;
            reply->addPar("destinoId") = 16;
            reply->addPar("timestamp") = simTime().dbl();
            reply->addPar("saltos") = 1;

            int nextHop = ListBeacon.TopTable();
            reply->setRecipientAddress(nextHop);

            sendDown(reply);
            EV << "Nodo origen (ID " << myId << ") envía WSM ID=" << sentWSMs << " a " << nextHop << " en t=" << simTime() << "\n";

            sentWSMs++;
        }
    }
}

void TraCIDemo11p::PreprocessingPreparaBeaconValues(DemoSafetyMessage* bsm) {
    Table_Ngh_NodeId = bsm->getCurrentNodeAddress();
    Table_Ngh_Msg_TreeID = bsm->getTreeId();
}

void TraCIDemo11p::sendWSM(int origenId, int destinoId) {
    if (myId != origenId) return;

    BaseFrame1609_4* reply = new BaseFrame1609_4();
    populateWSM(reply);
    reply->setKind(99);
    reply->addPar("origenId") = origenId;
    reply->addPar("destinoId") = destinoId;
    reply->addPar("timestamp") = simTime().dbl();
    reply->addPar("saltos") = 1;

    int nextHop = ListBeacon.TopTable();
    reply->setRecipientAddress(nextHop);

    sendDown(reply);
    EV << "Nodo origen (ID " << myId << ") envía WSM a " << nextHop << "\n";
}

void TraCIDemo11p::checkEndOfTransmission() {
    if (sentWSMs >= 5) {  // O el número de mensajes que hayas programado
        double plr = (sentWSMs > 0) ? ((double)(sentWSMs - receivedWSMs) / sentWSMs) : 0;
        EV << "=========================\n";
        EV << "FIN DE TRANSMISIÓN\n";
        EV << "WSMs enviados: " << sentWSMs << "\n";
        EV << "WSMs recibidos: " << receivedWSMs << "\n";
        EV << "PLR: " << plr * 100 << "%\n";
        EV << "=========================\n";
    }
}

