#pragma once

#include "global.h"
#include "net.h"
#include "cvar.h"
#include "mod.h"
#include "db.h"
#include "loginserver.h"

#include "terra/terra.h"
#include "terra/blocktype.h"

#include "util/util.h"
#include "util/crypto.h"

class ServerConfig {
    public:
        std::string name;
        std::string host_name;
        short host_port;

        std::vector<std::string> worlds;
        short tickrate;

        std::vector<std::string> login_servers;
        std::vector<std::string> mods;

        void load();
};

class Server {
    public:
        // This represents the global client ID counter
        ushort client_id_inc = 1;

        // This holds all the worlds loaded by the server
        std::map<std::string, Terra::World*> worlds;

        // Holds all registered block typeis
        Terra::BlockTypeCache types;

        // A mapping of connected clients
        std::mutex clients_mutex;
        std::map<ushort, RemoteClient*> clients;

        // List of all our login servers
        std::vector<LoginServer*> login_servers;

        // The main thread
        std::thread main_thread;

        // Whether the server is active
        bool active;

        // The TCP server
        TCPServer *tcps;

        // The servers keypair
        KeyPair keypair = KeyPair("keys");

        // The mod index
        ModDex dex;

        // The servers cvar dictionary
        CVarDict *cvars;

        // The servers config
        ServerConfig config;

        // The servers database file
        DB *db;

        // Server cvar handles
        CVar *sv_cheats;
        CVar *sv_name;
        CVar *sv_tickrate;
        CVar *sv_version;
        CVar *sv_motd;

        Server();
        ~Server();

        // Adds a world to the server
        void addWorld(Terra::World *);

        // Loads cvars, should be called early (startup)
        void loadCvars();

        // Loads the server-database
        void loadDatabase();

        // Loads the base types
        void loadBaseTypes();

        // Shuts the server down
        void shutdown();

        // Runs the server (this is blocking)
        void serveForever();

        // Fires a server tick, should be run by the main loop
        void tick();

        // The main server loop
        void mainLoop();

        // Fired when CVarDict (this.cvars) has a change
        bool onCVarChange(CVar *, Container *);

        // Generates a new client-id
        ushort newClientID();

        bool verifyLoginServer(std::string&);

        // Manage block types
        void addBlockType(Terra::BlockType*);
        void rmvBlockType(std::string);
        Terra::BlockType* getBlockType(std::string);

        // Hooks for the TCP-server
        bool onTCPConnectionClose(TCPRemoteClient *c);
        bool onTCPConnectionOpen(TCPRemoteClient *c);
        bool onTCPConnectionData(TCPRemoteClient *c);

        // Hooks for packets
        void handlePacket(cubednet::Packet *pk, RemoteClient *c);
        void handlePacketHandshake(cubednet::PacketHandshake, RemoteClient *c);
        void handlePacketStatusRequest(cubednet::PacketStatusRequest pk, RemoteClient *c);
};
