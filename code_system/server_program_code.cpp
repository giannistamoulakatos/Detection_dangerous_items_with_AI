#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <iostream>
#include <iomanip>
#include "httplib.h"
#include <openssl/ssl.h>

using namespace std;
using namespace httplib;


int main(){
    SSLServer srv("server_program_code.ca.cert","server_program_code.ca.key");
    
    srv.Get("/Data_receiving",[](const Request &,Response &res){
        res.set_content("json_message","application/json");

    });
    
    cout << "Server start listen" << endl;
    
    srv.listen("0.0.0.0",5001);

    return 0;

}