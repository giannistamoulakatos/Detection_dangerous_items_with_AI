#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <iostream>
#include <iomanip>
#include "httplib.h"
#include <openssl/ssl.h>

using namespace std;
using namespace httplib;


int main(){
    SSLServer srv("server_program_code.ca.cert","server_program_code.ca.key");
    
    srv.Post("/Data_receiving",[](const Request &req ,Response &res){
        string received_json = req.body;

        cout << "Received JSON: " << received_json << std::endl;

        
        res.set_content("{\"status\":\"ok\"}", "application/json");

    });    

    cout << "Server start listen" << endl;
   
    
    srv.listen("0.0.0.0",8443);

    return 0;

}