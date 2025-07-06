//
// Copyright (C) 2006-2011 Christoph Sommer <christoph.sommer@uibk.ac.at>
//
// Documentation for these modules is at http://veins.car2x.org/
//
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
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

    //Datos del nodo actual
    Coord current_Pos = mobility->getPositionAt(simTime());
    int current_node = myId;
    int nodeIndex = getParentModule()->getIndex();

    //Datos del beacon
    Coord senderPos = bsm->getIni_position();

    //Calculo distancia
    double distancia = current_Pos.distance(senderPos);

    if (!forwardMsg && distancia >= 3000) {
        forwardMsg = true;

        bsm->setDistancia(distancia);
        ListBeacon.SortBeacons();
        EV <<  ListBeacon.TopTable()<< endl;
        //sendDown(bsm->dup());     // Aquí su lógica de reenvio del beacon
    }


    //===========  Actualiza Tabla de vecinos ================
    // Actualiza variable locales con campos del beacon para decidir si procesa el beacon recibido
    PreprocessingPreparaBeaconValues(bsm);

    //VERIFICA SI YA HE PROCESADO ESTE BEACON TREEID. EVITA GUARDAR BEACONS FORWARDED
    bool Msg_Processed = ListBeacon.SearchBeaconTreeID(Table_Ngh_Msg_TreeID);

    //================== ACTUALIZA NNT  -- NODE  ===================
    if(!Msg_Processed){
       UpdateNNT(bsm);
    }
    ListBeacon.SortBeacons();
    //EV <<  ListBeacon.TopTable()<< endl;



    if(simTime() > 65 and forwardMsg){
        forwardMsg = false;
        sendWSM(40, 16);
    }
}

void TraCIDemo11p::onWSM(BaseFrame1609_4* frame)
{
    EV << "WSM recibido.\n";

    // Solo procesar si este nodo es el destinatario
    if (frame->getRecipientAddress() != myId) {
        EV << "Mensaje WSM no dirigido a este nodo (" << myId << "). Ignorando.\n";
        return;
    }

    int destinoId = frame->par("destinoId");
    int origenId = frame->par("origenId");
    //int saltos = frame->par("saltos");



    if (myId == destinoId) {
        EV << "Nodo destino (ID " << myId << ") recibió el mensaje.\n";

        double tiempo_envio = frame->par("timestamp");
        double retraso = simTime().dbl() - tiempo_envio;
        int saltos = frame->par("saltos");

        EV << "Delay: " << retraso << "s\n";
        EV << "Número total de saltos: " << saltos << "\n";

        return;
    }

    int nextHop = ListBeacon.TopTable();
    int saltos = frame->par("saltos");

    //EV << ds << endl;
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

        // send this message on the service channel until the counter is 3 or higher.
        // this code only runs when channel switching is enabled
        sendDown(wsm->dup());
        wsm->setSerial(wsm->getSerial() + 1);
        if (wsm->getSerial() >= 3) {
            // stop service advertisements
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

    // stopped for for at least 10s?
    if (mobility->getSpeed() < 1) {
        if (simTime() - lastDroveAt >= 10 && sentMessage == false) {
            findHost()->getDisplayString().setTagArg("i", 1, "red");
            sentMessage = true;

            TraCIDemo11pMessage* wsm = new TraCIDemo11pMessage();
            populateWSM(wsm);
            wsm->setDemoData(mobility->getRoadId().c_str());

            // host is standing still due to crash
            if (dataOnSch) {
                startService(Channel::sch2, 42, "Traffic Information Service");
                // started service and server advertising, schedule message to self to send later
                scheduleAt(computeAsynchronousSendingTime(1, ChannelType::service), wsm);
            }
            else {
                // send right away on CCH, because channel switching is disabled
                sendDown(wsm);
            }
        }
    }
    else {
        lastDroveAt = simTime();
    }
}


void TraCIDemo11p::PreprocessingPreparaBeaconValues(DemoSafetyMessage* bsm) {
    //Obtengo del beacon ->
    Table_Ngh_NodeId = bsm->getCurrentNodeAddress();                   //MAC identificdor del beacon recibido
    Table_Ngh_Msg_TreeID = bsm->getTreeId();                           //busca el msg en la tabla de beacons
    //Msg_RecipienAddress = bsm->getRecipientAddress();                  //cual es el destino del MSG????
    //Msg_DestinationAddress = bsm->getDestinationAddress();             // verifica la posible direccion de destino
}

void TraCIDemo11p::sendWSM(int origenId, int destinoId) {

    if (myId != origenId) {
        return;
    }

    BaseFrame1609_4* reply = new BaseFrame1609_4();
    populateWSM(reply);  // Llena los campos básicos
    reply->setKind(99);  // Tipo personalizado

    reply->addPar("origenId") = origenId;
    reply->addPar("destinoId") = destinoId;
    reply->addPar("timestamp") = simTime().dbl();
    reply->addPar("saltos") = 1;

    int nextHop = ListBeacon.TopTable();
    reply->setRecipientAddress(nextHop);

    sendDown(reply);
    EV << "Nodo origen (ID " << myId << ") envía WSM a " << nextHop << "\n";
}
