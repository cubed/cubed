#include "server.h"

Server::Server() {
    DEBUG("Cubed v%i.%i.%i", CUBED_RELEASE_A, CUBED_RELEASE_B, CUBED_RELEASE_C);

    // Load the SQLite3 Module stuff TODO: move/rename?
    init_db_module();

    // Load the server cvars
    this->loadCvars();
    this->config.load();

    this->db = new DB("server.db");
    if (this->db->is_new) {
        this->setupDatabase();
    }

    this->sv_tickrate->setInt(this->config.tickrate);
    this->sv_name->setString(this->config.name);

    for (auto world_name : this->config.worlds) {
        World *w = new World(world_name);
        w->load();
        this->addWorld(w);
    }

    this->dex.db = this->db;
    this->dex.loadFromPath("vanilla");

    // Create a new TCP server, which will be the main entry point for shit
    this->tcps = new TCPServer(this->config.host_name, this->config.host_port);
    this->tcps->onConnectionOpen = std::bind(&Server::onTCPConnectionOpen, this,
        std::placeholders::_1);
    this->tcps->onConnectionClose = std::bind(&Server::onTCPConnectionClose, this,
        std::placeholders::_1);
    this->tcps->onConnectionData = std::bind(&Server::onTCPConnectionData, this,
        std::placeholders::_1);

    // Dict test;
    // test.setString("string", "test");
    // test.setInt("int", 1);
    // test.setDouble("double", 1.342342);
}

Server::~Server() {
    this->shutdown();
}

void Server::shutdown() {
    for (auto v : this->worlds) {
        v.second->close();
    }

    DEBUG("Joining %i thread-pool threads", THREAD_POOL.size());
    for (auto &t : THREAD_POOL) {
        t->join();
    }
}

void Server::serve_forever() {
    this->active = true;
    this->main_thread = std::thread(&Server::main_loop, this);
}

/*
    It's assumed that everything within this thread has parent access to
    anything in server. Any other threads should use locks if they need to
    access things, or queue things to this thread.
*/
void Server::main_loop() {
    Ticker t = Ticker(this->sv_tickrate->getInt());

    while (this->active) {
        if (!t.next()) {
            WARN("RUNNING SLOW!");
        }

        this->tick();
    }
}

void Server::tick() {
    // this->clients_mutex.lock();
    // for (auto c : this->clients) {
    //     if (c.second->packet_buffer.size()) {
    //         DEBUG("Have a packet :D");
    //     }
    // }
    // this->clients_mutex.unlock();

    for (auto w : this->worlds) {
        w.second->tick();
    }
}

void Server::loadCvars() {
    this->cvars = new CVarDict();

    this->cvars->bind(std::bind(&Server::onCVarChange, this,
        std::placeholders::_1,
        std::placeholders::_2));

    // TODO: implement this
    this->cvars->load("server.json");

    sv_cheats = cvars->create("sv_cheats", "Allow changing of cheat protected variables");
    sv_name = cvars->create("sv_name", "A name for the server");

    sv_tickrate = cvars->create("sv_tickrate", "The servers tickrate");
    sv_tickrate->rmvFlag(FLAG_USER_WRITE)->rmvFlag(FLAG_MOD_WRITE);

    sv_version = cvars->create("sv_version", "The servers version");
    sv_version->setInt(CUBED_VERSION);
    sv_version->rmvFlag(FLAG_USER_WRITE)->rmvFlag(FLAG_MOD_WRITE);

}

void Server::addWorld(World *w) {
    this->worlds[w->wf->name] = w;
}

void Server::setupDatabase() {
    int err = SQLITE_OK;

    // err = sqlite3_exec(db->db, "CREATE TABLE mods ("
    //     "id INTEGER PRIMARY KEY ASC,"
    //     "name TEXT,"
    //     "version INTEGER"
    // ");", 0, 0, 0);

    assert(err == SQLITE_OK);
}

void ServerConfig::load() {
    FILE *fp = fopen("server.json", "r");

    if (!fp) {
        ERROR("Could not open server configuration file `server.json`!");
        throw Exception("Failed to load server configuration file!");
    }

    char readBuffer[65536];
    FileReadStream is(fp, readBuffer, sizeof(readBuffer));
    Document d;

    try {
        d.ParseStream(is);
    } catch (std::string e) {
        ERROR("Error occured while parsing server configuration: %s", e.c_str());
        throw Exception("Error occured while parsing server configuration!");
    }

    this->name = d["name"].GetString();
    this->host_name = d["host"]["name"].GetString();
    this->host_port = d["host"]["port"].GetInt();
    this->tickrate = d["tickrate"].GetInt();

    const Value& wrlds = d["worlds"];
    for (Value::ConstValueIterator itr = wrlds.Begin(); itr != wrlds.End(); ++itr) {
        this->worlds.push_back(itr->GetString());
    }


    fclose(fp);
}

bool Server::onCVarChange(CVar *cv, Container *new_value) {
    DEBUG("onCVarChange!");
    return false;
};

bool Server::onTCPConnectionOpen(TCPClient *c) {
    DEBUG("Adding TCPClient...");
    RemoteClient *rc = new RemoteClient();
    rc->state = STATE_NEW;
    rc->tcp = c;
    rc->id = this->newClientID();
    c->id = rc->id;

    this->clients_mutex.lock();
    this->clients[rc->id] = rc;
    this->clients_mutex.unlock();

    return true;
}

bool Server::onTCPConnectionClose(TCPClient *c) {
    DEBUG("Removing TCPClient...");
    this->clients_mutex.lock();
    this->clients.erase(this->clients.find(c->id));
    this->clients_mutex.unlock();
    delete(this->clients[c->id]);

    return true;
}

bool Server::onTCPConnectionData(TCPClient *c) {
    DEBUG("Running tryparse on client %i", c->id);
    this->clients[c->id]->tryParse();
    return true;
}

ushort Server::newClientID() {
    while (this->clients[client_id_inc]) {
        client_id_inc++; 
    }
    
    return client_id_inc;
}