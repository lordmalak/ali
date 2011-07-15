#include "assembler.hpp"

#include <iostream>
#include <fstream>
#include <map>
#include <cstdlib>

#include <boost/assign/std/vector.hpp>


//this assembler makes some assuptions on the size of the datatypes and the total size of the program.
//It might crash or produce random binary garbage without giving any helpful error message, as it is
//just intended as a temporary program used for testing purposes.

template <typename T, typename T2>
inline void cpy_val(T dest, const T2& val)
{
    *(T2*)(dest) = val;
}


assembler::assembler() {
    prg = new char[1024]; //1kB should be enough at first
}

assembler::~assembler() {
    if(prg)
        delete [] prg;
}

void assembler::load(const std::string& filename) {
    using boost::assign::operator+=;

    std::vector<std::string> inst;

    inst += "nop",        //0x00
        "ALLOC",
        "add",        //0x02
        "AND",
        "APPLY",
        "APPLY0",
        "CONS",
        "COPYGLOB",
        "DIV",
        "dup",        //0x09
        "EVAL",
        "EQ",
        "getbasic",   //0x0c
        "GETVEC",
        "GEQ",
        "GR",
        "halt",       //0x10
        "jump",       //0x11
        "jumpz",      //0x12
        "LE",
        "LEQ",
        "loadc",      //0x15
        "MARK",
        "MARK0",
        "MOD",
        "MUL",
        "mkbasic",    //0x1a
        "MKCLOS",
        "MKFUNVAL",
        "MKVEC",
        "MKVEC0",
        "NEG",
        "NEQ",
        "NIL",
        "OR",
        "PUSHGLOB",
        "PUSHLOC",
        "RETURN",
        "REWRITE",
        "POPENV",
        "SLIDE",
        "SUB",
        "store",      //0x2a
        "TARG",
        "UPDATE",
        "WRAP";

    std::ifstream file;
    file.open(filename.c_str(), std::ios_base::in);
    if(!file) {
        std::cerr << "Could not open input file." << std::endl;
        exit(EXIT_FAILURE);
    }

    std::string symbol;
    address = 0;
    std::map<std::string, int> labels;
    std::map<int, std::string> missing_ref;

    while(!file.eof()) {
        file >> symbol;
        char c = *(--symbol.end());
        if(c == ':') //symbol is a label definition
            labels[symbol.substr(0, symbol.length()-1)] = address;
        else if(c > 47 && c < 58) { //symbol is a number
            cpy_val(prg+address, from_string<long long>(symbol));
            address += sizeof(long long);
        }
        else { //symbol is an instruction or label reference
            char i = 0;
            for(;static_cast<unsigned int>(i) < inst.size(); i++) {
                if(symbol == inst[(int)i]) {
                    prg[address] = i;
                    address++;
                    break;
                }
            }

            if(static_cast<unsigned int>(i) == inst.size()) { //symbol must be a label reference
                if(labels.count(symbol) == 1) { //label is already defined
                    cpy_val(prg+address, labels[symbol]);
                    address += sizeof(int);
                }
                else {
                    missing_ref[address] = symbol;
                    address += sizeof(int);
                }
            }

        }
    }

    for(std::map<int, std::string>::iterator i = missing_ref.begin(); i != missing_ref.end(); i++) {
        if(labels.count((*i).second) == 0) {
            std::cerr << "Unknown label or instruction: " << (*i).second << std::endl;
        }
        *(int*)(prg+(*i).first) = labels[(*i).second];
    }

    file.close();
}

void assembler::binary_dump(const std::string& filename) {
    std::ofstream file;
    file.open(filename.c_str(), std::ios_base::out);
    for(int i = 0; i < address; i++) {
        file << prg[i];
    }
    file.close();
}