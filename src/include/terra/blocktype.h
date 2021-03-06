#pragma once

#include "global.h"
#include "terra/terra.h"

namespace Terra {

class Block;

class BlockType {
    public:
        uint32_t id;
        std::string name;

        // 0 = unbreakable, otherwise durability
        virtual unsigned short getDurability(Block *) = 0;

        // 256=opaque, 0=paque
        virtual unsigned char getOpacity(Block *) = 0;

        // True if entities and liquids can pass through it
        virtual bool isPermeable(Block *) = 0;

        // True if can be caught on fire
        virtual bool isFlammable(Block *) = 0;

        std::string key() {
           char buffer[2048];
           sprintf(buffer, "type:%s", this->name.c_str());
           return std::string(buffer);
        }
};

class StaticBlockType : public BlockType {
    public:
        unsigned short durability;
        unsigned char opacity;
        bool permeable;
        bool flammable;

        unsigned short getDurability(Block *) {
            return durability;
        }

        unsigned char getOpacity(Block *) {
            return opacity;
        }

        bool isPermeable(Block *) {
            return permeable;
        }

        bool isFlammable(Block *) {
            return flammable;
        }
};

class AirType : public StaticBlockType {
    public:
        AirType() {
            this->name = "base:air";
            this->opacity = 255;
            this->permeable = true;
            this->flammable = false;
        }
};

class BedRockType : public StaticBlockType {
    public:
        BedRockType() {
            this->name = "base:bedrock";
            this->opacity = 0;
            this->permeable = false;
            this->flammable = false;
        }
};

typedef std::unordered_map<std::string, BlockType*> BlockTypeMapping;
typedef std::unordered_map<uint32_t, BlockType*> BlockTypeCache;

class BlockTypeHolder {
    private:
        uint32_t id_inc = 0;

    public:
        BlockTypeMapping types;
        BlockTypeCache cache;

        void loadBaseTypes();
        void addBlockType(Terra::BlockType*);
        void rmvBlockType(std::string);
        bool haveType(std::string);
        BlockType* getBlockType(std::string);
};

}

