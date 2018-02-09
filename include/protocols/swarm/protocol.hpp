#ifndef SWARM_PROTOCOL_HPP
#define SWARM_PROTOCOL_HPP

#include "service/function.hpp"
#include "protocols/swarm/commands.hpp"
#include "protocols/swarm/manager.hpp"
#include "http/module.hpp"

namespace fetch
{
namespace protocols
{
class SwarmProtocol : public SwarmManager,
                      public fetch::service::Protocol,
                      public fetch::http::HTTPModule { 
public:
  SwarmProtocol(network::ThreadManager *thread_manager, uint64_t const &protocol, NodeDetails &details) :
    SwarmManager(protocol, thread_manager, details),
    fetch::service::Protocol()
  {
    using namespace fetch::service;

    // RPC Protocol
    auto ping = new CallableClassMember<SwarmProtocol, uint64_t()>(this, &SwarmProtocol::Ping);
    auto hello = new CallableClassMember<SwarmProtocol, NodeDetails(uint64_t, NodeDetails)>(Callable::CLIENT_ID_ARG, this, &SwarmProtocol::Hello);
    auto suggest_peers = new CallableClassMember<SwarmProtocol, std::vector< NodeDetails >()>(this, &SwarmProtocol::SuggestPeers);
    auto req_conn = new CallableClassMember<SwarmProtocol, void(NodeDetails)>(this, &SwarmProtocol::RequestPeerConnections);
    auto myip = new CallableClassMember<SwarmProtocol, std::string(uint64_t)>(Callable::CLIENT_ID_ARG, this, &SwarmProtocol::GetAddress);    
    
    Protocol::Expose(SwarmRPC::PING, ping);
    Protocol::Expose(SwarmRPC::HELLO, hello);
    Protocol::Expose(SwarmRPC::SUGGEST_PEERS, suggest_peers); 
    Protocol::Expose(SwarmRPC::REQUEST_PEER_CONNECTIONS, req_conn);
    Protocol::Expose(SwarmRPC::WHATS_MY_IP, myip);    

    
    // Using the event feed that
    Protocol::RegisterFeed(SwarmFeed::FEED_REQUEST_CONNECTIONS, this);
    Protocol::RegisterFeed(SwarmFeed::FEED_ENOUGH_CONNECTIONS, this);
    Protocol::RegisterFeed(SwarmFeed::FEED_ANNOUNCE_NEW_COMER, this);

    
    // Web interface
    auto http_bootstrap = [this](fetch::http::ViewParameters const &params, fetch::http::HTTPRequest const &req) {
        this->Bootstrap( params["ip"] , params["port"].AsInt() );
        return fetch::http::HTTPResponse("{\"status\":\"ok\"}");
        
    };
    HTTPModule::Get("/bootstrap/(ip=\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3})/(port=\\d+)",  http_bootstrap);
    HTTPModule::Get("/connect-to/(ip=\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3})/(port=\\d+)",  http_bootstrap);


    auto shard_connect = [this](fetch::http::ViewParameters const &params, fetch::http::HTTPRequest const &req) {
        this->ConnectShard( params["ip"] , params["port"].AsInt() );
        return fetch::http::HTTPResponse("{\"status\":\"ok\"}");
        
    };    
    HTTPModule::Get("/connect-shard/(ip=\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3})/(port=\\d+)",  shard_connect);    

    auto list_shards = [this](fetch::http::ViewParameters const &params, fetch::http::HTTPRequest const &req) {
      std::stringstream response;
      response << "{ \"shards\": [";
      
      this->with_shard_details_do([this, &response](std::vector< ShardDetails > const &detail_list) {
          bool first = true;
          
          for(auto &d: detail_list)
          {
            if(!first) response << ",";
            response << "{ \"host\": \""  << d.entry_for_swarm.host << "\",";
            response << " \"port\": "  << d.entry_for_swarm.port << "}";              
            first = false;            
          }
          
        });
      response << "] }";
      
      return fetch::http::HTTPResponse(response.str());
        
    };    
    HTTPModule::Get("/list/shards",  list_shards);    
    

    auto list_outgoing =  [this](fetch::http::ViewParameters const &params, fetch::http::HTTPRequest const &req) {
      std::stringstream response;
      
      response << "{\"outgoing\": [";  
      
      this->with_server_details_do([&response](std::map< uint64_t, NodeDetails > const &peers) {
          bool first = true;          
          for(auto &ppair: peers)
          {
            auto &p = ppair.second;            
            if(!first) response << ", \n";            
            response << "{\n";
            response << "\"public_key\": \"" + p.public_key() + "\",";
            response << "\"entry_points\": [";
            bool sfirst = true;
            
            for(auto &e: p.entry_points())
            {
              if(!sfirst) response << ",\n";              
              response << "{";
              response << "\"shard\": " << e.shard  <<",";  
              response << "\"host\": \"" << e.host  <<"\",";
              response << "\"port\": " << e.port  << ",";
              response << "\"http_port\": " << e.http_port  << ",";
              response << "\"configuration\": " << e.configuration  << "}";              
              sfirst = false;              
            }
            
            response << "]";
            response << "}";
            first = false;            
          }
          
        });
      
      response << "]}";
      
      std::cout << response.str() << std::endl;
            
      return fetch::http::HTTPResponse(response.str());
    };            

    HTTPModule::Get("/list/outgoing",  list_outgoing);


    auto list_incoming = [this](fetch::http::ViewParameters const &params, fetch::http::HTTPRequest const &req) {
      std::stringstream response;
      
      response << "{\"incoming\": [";  
      
      this->with_client_details_do([&response](std::map< uint64_t, NodeDetails > const &peers) {
          bool first = true;          
          for(auto &ppair: peers)
          {
            auto &p = ppair.second;            
            if(!first) response << ", \n";            
            response << "{\n";
            response << "\"public_key\": \"" + p.public_key() + "\",";
            response << "\"entry_points\": [";
            bool sfirst = true;
            
            for(auto &e: p.entry_points())
            {
              if(!sfirst) response << ",\n";              
              response << "{";
              response << "\"shard\": " << e.shard  <<",";  
              response << "\"host\": \"" << e.host  <<"\",";
              response << "\"port\": " << e.port  << ",";
              response << "\"http_port\": " << e.http_port  << ",";
              response << "\"configuration\": " << e.configuration  << "}";              
              sfirst = false;              
            }
            
            response << "]";
            response << "}";
            first = false;            
          }
          
        });
      
      response << "]}";
      std::cout << response.str() << std::endl;
            
      return fetch::http::HTTPResponse(response.str());
    };            

    HTTPModule::Get("/list/incoming",  list_incoming);
    

    
    auto list_suggestions = [this](fetch::http::ViewParameters const &params, fetch::http::HTTPRequest const &req) {
      std::stringstream response;
      
      response << "{\"suggestions\": [";  
      
      this->with_suggestions_do([&response](std::vector< NodeDetails > const &peers) {
          bool first = true;          
          for(auto &p: peers)
          {
            if(!first) response << ", \n";            
            response << "{\n";
            response << "\"public_key\": \"" + p.public_key() + "\",";
            response << "\"entry_points\": [";
            bool sfirst = true;
            
            for(auto &e: p.entry_points())
            {
              if(!sfirst) response << ",\n";              
              response << "{";
              response << "\"shard\": " << e.shard  <<",";  
              response << "\"host\": \"" << e.host  <<"\",";
              response << "\"port\": " << e.port  << ",";
              response << "\"http_port\": " << e.http_port  << ",";
              response << "\"configuration\": " << e.configuration  << "}";              
              sfirst = false;              
            }
            
            response << "]";
            response << "}";
            first = false;            
          }
          
        });
      
      response << "]}";
      std::cout << response.str() << std::endl;
            
      return fetch::http::HTTPResponse(response.str());
    };        
    HTTPModule::Get("/list/suggestions",  list_suggestions);
    

    
    auto node_details = [this](fetch::http::ViewParameters const &params, fetch::http::HTTPRequest const &req) {
      std::string response = "{\"name\": \"hello world\"";
      
      response += "}";      
      return fetch::http::HTTPResponse(response);
    };    
    HTTPModule::Get("/node-details",  node_details);         

    
  }
  

};
}; // namespace protocols
}; // namespace fetch

#endif // SWARM_PROTOCOL_HPP