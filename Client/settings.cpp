#include "Settings.h"

bool x3mp::Settings::Load()
{
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file("x3mp.xml");


    this->username = doc.child("config").child("server").child_value("username");
    this->ip = doc.child("config").child("server").child_value("ip");
    port = std::stoi(doc.child("config").child("server").child_value("port"));
    localmode = std::stoi(doc.child("config").child_value("local")) == 1;
    debug = std::stoi(doc.child("config").child_value("debug")) == 1;

    //if xml had a problem...
    if (result.status != pugi::xml_parse_status::status_ok)
    {
        return false;
    }
    else {
        return true;
    }
}
