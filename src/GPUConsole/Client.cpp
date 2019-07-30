#include "Client.h"


//******************************************
Client::Client (rhea::Allocator *allocatorIN, OSSocket &sokIN) :
    cp(allocatorIN, 256)
{
    sok = sokIN;
}

//******************************************
Client::~Client()
{
}

