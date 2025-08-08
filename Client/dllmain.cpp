#include <memory>
#include <vector>
#include <string>
#include <array>
#include <algorithm>
#include <sstream>

#include "defines.h"

#include <d3d9.h>
#include <d3dx9.h>

#include "pugixml.hpp"

#include "X3Classes.h"
#include "X3Functions.h"
#include "X3Util.h"
#include "Console.h"
#include "Renderer.h"
#include "Chatbox.h"
#include "directx.h"
#include "client.h"
#include "settings.h"
#include "mem.h"

#include "net_packets.h"


Renderer* renderer{ nullptr };
Client client;
std::unique_ptr<Chatbox> chatbox;

std::array<x3::Entity*, MAX_ENTITIES> entities{ nullptr };

void* d3d9Device[119];
tEndScene oEndScene = nullptr;
extern LPDIRECT3DDEVICE9 pDevice = nullptr;

void StartClient(const char* ip, const unsigned short port) {
    client.Run(ip, port);
}

std::map<int, int> calledMap;
std::map<int, int> savedCalledMap;

/*int hook_SetSimulatorParam(int param_1, int param_2, int param_3, char* param_4, int param_5, int param_6)
{
    std::cout << "SetSimParam: " << param_4 << std::endl;
    return x3::SetSimulatorParam(param_1, param_2, param_3, param_4, param_5, param_6);
}*/

int __cdecl hook_SomeUpdate(int param_1, int param_2, int updateType, int param_4, x3::struct_1* updateData)
{
    std::map<int, int>::iterator it = calledMap.find(updateType);
    if (it != calledMap.end())
        it->second++;
    else
    {
        std::map<int, int>::iterator it2 = savedCalledMap.find(updateType);
        if (it2 == savedCalledMap.end())
            calledMap.insert(std::make_pair(updateType, 1));
    }
    
    return x3::SomeUpdate(param_1, param_2, updateType, param_4, updateData);
}

void APIENTRY hook_EndScene(LPDIRECT3DDEVICE9 o_pDevice) {
    if (!pDevice)
        pDevice = o_pDevice;
    if (renderer == nullptr)
        renderer = new Renderer();

    /*/uintptr_t moduleBase = (uintptr_t)GetModuleHandle(L"X3AP.exe");
    moduleBase = (uintptr_t)GetModuleHandle(NULL);
    x3::Base2* stuff = (x3::Base2*)((BYTE*)moduleBase + 0x0020850C);
    */

    //Hooked D3D9 :D
    //DrawFilledRect(25, 25, 100, 100, D3DCOLOR_ARGB(255,255,255,255
    //x3::Entity* target = stuff->N000010EA->Sector->N00000BAF->N000014A4->N000014ED->N00001552->Target;
    //DrawTextString(100, 100, "X3:MP Modification");
    //if(CheckShipPointer(target))
    //    DrawTextString(100, 120, std::string(target->ShipType));

    chatbox->Draw(*renderer);

    oEndScene(o_pDevice);
}


DWORD WINAPI ModThread(HMODULE hModule)
{
    x3::Console console = x3::Console(); //TODO: Make or singleton or something
    chatbox = std::make_unique<Chatbox>();

    uintptr_t moduleBase = (uintptr_t)GetModuleHandle(L"X3AP.exe");
    moduleBase = (uintptr_t)GetModuleHandle(NULL);
    x3::InitFunctionPointer(moduleBase);

    //Hook
    x3::HookFunctions((BYTE*)hook_SomeUpdate, (BYTE*)x3::util::hook_DeleteEntityLoop);
    //SetSimulatorParam = (tSetSimulatorParam)0x0049f4c0;
    //SetSimulatorParam = (tSetSimulatorParam)mem::TrampHook32((BYTE*)SetSimulatorParam, (BYTE*)hook_SetSimulatorParam, 7);

    //Hook D3D
    if (GetD3D9Device(d3d9Device, sizeof(d3d9Device)))
    {
        oEndScene = (tEndScene)mem::TrampHook32((BYTE*)d3d9Device[42], (BYTE*)hook_EndScene, 7);
    }

    //Create Console

    x3::Base* basePtr = (x3::Base*)((BYTE*)moduleBase + 0x0017B0F8);
    x3::Base* sectorBasePtr = (x3::Base*)((BYTE*)moduleBase + 0x0020850C);

    chatbox->SendChatMessage("==================================================================", 255, 180, 180, 180);
    chatbox->SendChatMessage("      X3MP started...", 255, 180, 180, 180);
    chatbox->SendChatMessage("==================================================================", 255, 180, 180, 180);

    auto xmlSettings = std::make_unique<x3mp::Settings>();
    if (!xmlSettings->Load() || xmlSettings->username.length() > 64)
    {
        //close the client
        std::cout << "  [ERR] Configuration \"x3mp.xml\" is faulty, please start the mod via the launcher to generate it. This window selfdestructs in 10 seconds.";

        Sleep(10000);
        console.Close();
        FreeLibraryAndExitThread(hModule, 0);
    }

    std::stringstream stream;

    stream << "      Your username is : " << xmlSettings->username;
    chatbox->SendChatMessage(stream.str(), 255, 180, 180, 180);
    stream.str(std::string());

    stream << "      Connecting to server : " << xmlSettings->ip << std::string(":") << xmlSettings->port << "...";
    chatbox->SendChatMessage(stream.str(), 255, 180, 180, 180);

    while ((uint32_t)basePtr->EntityManager == (uint32_t)0x57B0FC) // If an Entity Manager has not been found, wait for it...
        Sleep(10);

    std::thread clientThread(StartClient, xmlSettings->ip.c_str(), xmlSettings->port);
    int clientID = -1;
    int ownShipID = -1;

    auto sectorPtr = (x3::Sector*)(uintptr_t)sectorBasePtr->EntityManager->EntityList; // Has to be executed before ship spawn and after sector creation

    while (!client.isConnected)
        Sleep(100);

    chatbox->SendChatMessage("Connected successfully.", 255, 180, 180, 180);

    Connect connectPacket;
    connectPacket.type = PacketType::Connect;
    memcpy(connectPacket.Name, &xmlSettings->username, xmlSettings->username.length());
    connectPacket.size = sizeof(Connect);
    connectPacket.Model = basePtr->EntityManager->EntityList->ShipTypeID;

    for (size_t i = 0; i < USHRT_MAX; i++)
    {
        entities[i] = nullptr;
    }

    if (client.isConnected)
        client.SendPacket(&connectPacket);

    bool pRun = true;
    while (pRun)
    {
        /*if (GetAsyncKeyState(VK_DELETE) & 1)
        {
            std::cout << "VK_DELETE pressed" << std::endl;
            savedCalledMap = calledMap;
            calledMap.clear();
        }

        if (GetAsyncKeyState(VK_END) & 1)
        {
            std::cout << "VK_END pressed" << std::endl;
            for (auto& e : calledMap)
            {
                std::cout << "Address: " << e.first << " | Called: " << e.second << std::endl;
            }
        }*/

        //CreateInSectorEntity(entity, 0x1); //PBK Schuss
        //SetSimulatorParam(0, 0, 0x96, "NotifyIntoLaserRange", 1, 0); //Ziel ist nun in Feuerreichweite
        //SetSimulatorParam(0, 0, 0x96, "NotifyOutofLaserRange", 1, 0); //Ziel verlässt Feuerreichweite
        //SetSimulatorParam(0, 0, 0x96, "NotifyTargetLock", 1, 2); //
        //SetSimulatorParam(8, 0, 0x96, "NotifyDockingAbort", 1, 1); //Andockvorgang abgebrochen
        //SetSimulatorParam(8, 0, SomeAddressOrCounter, "CanWarp", 1, 1); //Called when entering jumpgate
        //SetSimulatorParam(0, 0, 0x96, "InputAction", 1, 1); //Called when shooting
        //SetSimulatorParam(0xA, 0, 0, "WriteGalaxyMapSectorName", 1, 4); //Called when shooting
        //SetSimulatorParam(0, 0, 0, "_Init", 1, 1); // Crash
        //SetSimulatorParam(0, 0, 0x96, "InputAction", 1, 1); //Does nothing?
        //SetSimulatorParam(0, 0, 0x96, "StopFastForward", 1, 0); // Disables SETA?
        //SetSimulatorParam(8, 0, SomeAddressOrCounter, "MissileLauncher", 1, 1);
        //SetSimulatorParam(8, 0, SomeAddressOrCounter, "MissileAlert", 1, 2);
        //SetSimulatorParam(8, 0, SomeAddressOrCounter, "AttackedBy", 1, 2);
        //SetSimulatorParam(0, 0, 0x96, "InputMouseDelta", 1, 2);
        //SetSimulatorParam(0, 0, 0x96, "Stop", 1, 0); //Called when leaving game
        //SetSimulatorParam(8, 0, 0xFFFF8913, "CollisionWarn", 1, 0);
        //SetSimulatorParam(0, 0, 0x96, "NotifyLaserLow", 1, 0); //Warnung, Energielevel niedrig
        //SetSimulatorParam(0, 0, ppiVar2[0x25], "SetKnown", 1, 1);
        //SetSimulatorParam(*(int*)&(NullEntity.NextLink)->field_0x60, 0, this_00[0x25], "SelfDestruct", 1, 2);
        //SetSimulatorParam(0, 0, 0x96, "SetAutopilot", 1, 2); //Kein Ziel //Crash if target is there
        //SetSimulatorParam(0, 0, 0x96, "NotifyAlert", 1, 1); //Makes music uncomfortable
        //std::cout << x3::GetSomeText(0x79152C0, 0x39) << std::endl;
        //Crashes the game, sadly:
        //void* background;
        //x3::LoadThisObject(&background, "environments\\nebulae\\uranus\\nebula_uranus_stars_01");
        //x3::SetSectorBackground(&background);

        x3::Entity* ownShip = basePtr->EntityManager->EntityList;

        auto a = *(DWORD*)(sectorBasePtr);
        auto b = *(DWORD*)(a + 0x38); 

        if (b != 0x0 && ownShipID != -1 && x3::util::CheckShipPointer(ownShip, entities))
        {
            ShipUpdate packet;
            packet.type = PacketType::ShipUpdate;
            packet.PosX = ownShip->WorldData->PosX;
            packet.PosY = ownShip->WorldData->PosY;
            packet.PosZ = ownShip->WorldData->PosZ;
            packet.RotX = ownShip->WorldData->RotQuaternionX;
            packet.RotY = ownShip->WorldData->RotQuaternionY;
            packet.RotZ = ownShip->WorldData->RotQuaternionZ;
            packet.RotW = ownShip->WorldData->RotQuaternionW;
            packet.UpX = ownShip->WorldData->UpQuaternionX;
            packet.UpY = ownShip->WorldData->UpQuaternionY;
            packet.UpZ = ownShip->WorldData->UpQuaternionZ;
            packet.UpW = ownShip->WorldData->UpQuaternionW;
            packet.LookAtX = ownShip->WorldData->LookAtX;
            packet.LookAtY = ownShip->WorldData->LookAtY;
            packet.LookAtZ = ownShip->WorldData->LookAtZ;

            for (size_t i = 0; i < 65535; i++)
            {
                if (entities[i] != nullptr && entities[i] == ownShip)
                {
                    packet.ShipID = i;
                    break;
                }
            }

            packet.size = sizeof(ShipUpdate);

            if (client.isConnected)
                client.SendPacket(&packet);
        }

        while (client.isConnected && !client.receivedPackets.empty())
        {
            Packet* packet = client.receivedPackets.front();
            client.receivedPackets.pop();

            if (packet->type == PacketType::ShipUpdate)
            {

                ShipUpdate* updatePacket = (x3::net::ShipUpdate*)packet;
                if (entities[updatePacket->ShipID] != nullptr && x3::util::CheckShipPointer(entities[updatePacket->ShipID], entities))
                {
                    entities[updatePacket->ShipID]->WorldData->PosX = updatePacket->PosX;
                    entities[updatePacket->ShipID]->WorldData->PosY = updatePacket->PosY;
                    entities[updatePacket->ShipID]->WorldData->PosZ = updatePacket->PosZ;
                    entities[updatePacket->ShipID]->WorldData->RotQuaternionX = updatePacket->RotX;
                    entities[updatePacket->ShipID]->WorldData->RotQuaternionY = updatePacket->RotY;
                    entities[updatePacket->ShipID]->WorldData->RotQuaternionZ = updatePacket->RotZ;
                    entities[updatePacket->ShipID]->WorldData->RotQuaternionW = updatePacket->RotW;
                    entities[updatePacket->ShipID]->WorldData->UpQuaternionX = updatePacket->UpX;
                    entities[updatePacket->ShipID]->WorldData->UpQuaternionY = updatePacket->UpY;
                    entities[updatePacket->ShipID]->WorldData->UpQuaternionZ = updatePacket->UpZ;
                    entities[updatePacket->ShipID]->WorldData->UpQuaternionW = updatePacket->UpW;
                    entities[updatePacket->ShipID]->WorldData->LookAtX = updatePacket->LookAtX;
                    entities[updatePacket->ShipID]->WorldData->LookAtY = updatePacket->LookAtY;
                    entities[updatePacket->ShipID]->WorldData->LookAtZ = updatePacket->LookAtZ;
                }
                else
                    console.Log(std::string("Ship Update for invalid ship! ShipID: ") + std::to_string(updatePacket->ShipID), x3::MessageLevel::Error);
            }
            else if (packet->type == PacketType::CreateShip)
            {
                if (ownShipID == -1)
                {
                    delete packet;
                    continue;
                }
                CreateShip* createPacket = (x3::net::CreateShip*)packet;
                sectorPtr = (x3::Sector*)(uintptr_t)sectorBasePtr->EntityManager->EntityList; // Has to be executed before ship spawn and after sector creation
                x3::Entity* entity = x3::AllocateEntitySpace(0x130);
                x3::CreateInSectorEntity(entity, 0x70000 + createPacket->Model);
                x3::SetEntityInSector(entity, sectorPtr);
                entities[createPacket->ShipID] = entity;
                entity->WorldData->PosX = createPacket->PosX;
                entity->WorldData->PosY = createPacket->PosY;
                entity->WorldData->PosZ = createPacket->PosZ;
                /*entity->WorldData->RotQuaternionX = createPacket->RotX; // TODO: Fix
                entity->WorldData->RotQuaternionY = createPacket->RotY;
                entity->WorldData->RotQuaternionZ = createPacket->RotZ;
                entity->WorldData->RotQuaternionW = createPacket->RotW;
                entity->WorldData->UpQuaternionX = createPacket->UpX;
                entity->WorldData->UpQuaternionY = createPacket->UpY;
                entity->WorldData->UpQuaternionZ = createPacket->UpZ;
                entity->WorldData->UpQuaternionW = createPacket->UpW;
                entity->WorldData->LookAtX = createPacket->LookAtX;
                entity->WorldData->LookAtY = createPacket->LookAtY;
                entity->WorldData->LookAtZ = createPacket->LookAtZ;*/
                console.Log(std::string("Creating ship at position: ") + std::to_string(createPacket->PosX) + std::string("|..."), x3::MessageLevel::Debug);
            }

            else if (packet->type == PacketType::DeleteShip)
            {
                DeleteShip* deletePacket = (x3::net::DeleteShip*)packet;
                if (entities[deletePacket->ShipID] != nullptr && x3::util::CheckShipPointer(entities[deletePacket->ShipID], entities))
                {
                    x3::util::DeleteEntity(entities[deletePacket->ShipID]);
                    console.Log(std::string("Deleted ship ") + std::to_string(deletePacket->ShipID), x3::MessageLevel::Debug);
                    entities[deletePacket->ShipID] = nullptr;
                }
            }
            else if (packet->type == PacketType::CreateStar)
            {
                CreateStar* createPacket = (x3::net::CreateStar*)packet;
                sectorPtr = (x3::Sector*)(uintptr_t)sectorBasePtr->EntityManager->EntityList; // Has to be executed before ship spawn and after sector creation
                x3::Entity* entity = x3::AllocateEntitySpace(0x130);
                x3::CreateInSectorEntity(entity, 0x30000 + createPacket->Model);
                x3::SetEntityInSector(entity, sectorPtr);
                entity->WorldData->PosX = createPacket->PosX;
                entity->WorldData->PosY = createPacket->PosY;
                entity->WorldData->PosZ = createPacket->PosZ;
            }
            else if (packet->type == PacketType::ConnectAcknowledge)
            {
                ConnectAcknowledge* ackPacket = (x3::net::ConnectAcknowledge*)packet;
                clientID = ackPacket->ClientID;
                std::cout << std::hex; // TODO: Shift into log class
                console.Log(std::string("Your ShipID: ") + std::to_string((int)ackPacket->ShipID), x3::MessageLevel::Info);
                console.Log(std::string("Your ShipAdress: ") + std::to_string((int)ownShip), x3::MessageLevel::Info);
                std::cout << std::dec;
                ownShipID = ackPacket->ShipID;
                entities[ackPacket->ShipID] = ownShip;
            }
            else if (packet->type == PacketType::ChatMessage)
            {
                x3::net::ChatMessage* chatPacket = (x3::net::ChatMessage*)packet;
                chatbox->SendChatMessage(chatPacket->Message, chatPacket->A, chatPacket->R, chatPacket->G, chatPacket->B);
            }

            delete packet;
        }

        Sleep(20);
    }


    clientThread.join();
    ShutdownSteamDatagramConnectionSockets();
    console.Close();

    delete(renderer);

    FreeLibraryAndExitThread(hModule, 0);
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    {
        HANDLE hProcess = nullptr;
        hProcess = CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)ModThread, hModule, 0, nullptr);
        if (hProcess)
            CloseHandle(hProcess);
        break;
    }
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}