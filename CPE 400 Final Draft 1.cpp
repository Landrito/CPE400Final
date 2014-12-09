/*
CPE 400 Final Project

Renee Iinuma, Kyle Ernest Lee, Ernest Landrito, Wesley Kepke.
*/

// Header files
#include <iostream>
#include <string>
#include <cstring>
#include <map>
#include <cstdlib>
#include <vector>
#include <climits>
#include <sys/time.h>
#include <cstdio>
using namespace std;

//Basic Timer Class
class Timer{
   public:
      void start()
      {
         gettimeofday( &startTime, NULL);
      }
      double stop()
      {
         gettimeofday( &stopTime, NULL);
         duration = ( (double) stopTime.tv_sec + 
                      (double) stopTime.tv_usec*0.000001 ) * 1000 -
                  ( (double) startTime.tv_sec + 
                   (double) startTime.tv_usec*0.000001 ) * 1000;
         return duration;
      }
   private:
      struct timeval startTime;
      struct timeval stopTime;
      double duration;
};

// Base class - "Device"
class Device{
   public:
      virtual string getIP( const string & url)  = 0;
      virtual int getCost() = 0;

};
Device * createListOfRouters(Device * endDevice, int numRouters);

// Derived class - "Router"
class Client : public Device{
   public:
      // Assign next node in linked list of routers
      void setNext(Device * newNext) {
         next = newNext;
      }

      virtual int getCost() {
         return -1;
      }

      // Function to obtain IP address from router
      virtual string getIP( const string & url)  {
         return next->getIP(url);
      }

      // Devices will be linked to other devices
      Device * next;
};

// Derived class - "Router"
class Router : public Device{
   public:
      // Assign next node in linked list of routers
      void setNext(Device * newNext) {
         next = newNext;
      }
      virtual int getCost() {
         return rand() % 100 + 1;
      }
      // Function to obtain IP address from router
      virtual string getIP( const string & url)  {
         cout << "Request has reached router." << endl;

         // No error, continue query 
         cout << "Successfully passed through router." << endl;
         string IP = next->getIP(url);
         cout << "Coming back through router." << endl;

         // IP address found
         return IP;  
      }

      // Devices will be linked to other devices
      Device * next;
};

class Routes : public Device{
public:
   //Gets the IP based on the best route
   virtual string getIP(const string & url){
      cout << "Calculating Best path" << endl;
      Device * route = getShortestRoute();
      cout << "Proceeding through shortest route" << endl;
      return route->getIP(url);
   }

   //creates a random amount of routers between two main devices
   void createRoutes(Device * endLocation,int numRoutes){
      for(int i = 0; i < numRoutes; i++){
         Routers.push_back(createListOfRouters(endLocation, rand() % 6 + 1));
      }
   }

   //returns a stopping value
   virtual int getCost() {
         return -1;
   }

   //calculates the shortest route
   Device * getShortestRoute(){
      int shortestIndex = 0;
      int shortestRouteLength = INT_MAX;
      for(int i = 0; i < Routers.size(); i++){
         int routeLength = calculateRoute(Routers[i]);
         cout << "Route: " << i << " has cost of: " << routeLength << endl;
         if( routeLength < shortestRouteLength ){
            shortestRouteLength = routeLength;
            shortestIndex = i;
         }
      }
      cout << "Shortest Route has a cost of: " << shortestRouteLength << endl;
      return Routers[shortestIndex];
   }

   //calculates the route to a specific device.
   int calculateRoute( Device * Node ){
      Device * current = Node;
      int nodeCost = current->getCost();
      int totalCost = 0;
      while( nodeCost != -1 ){
         totalCost += nodeCost;
         current = ((Router*)current)->next;
         nodeCost = current->getCost();
      }

      return totalCost;
   }

private:
   vector<Device *> Routers;
};

// Derived class - "LocalDNS"
class LocalDNS : public Device{
   public:
      // Function to add a route from local DNS to root DNS
      void addRouteToRoot(Device * newRoute){
         routeToRootDNS = newRoute;
      }

      // Obtain the IP address from the local DNS server
      virtual string getIP( const string & url){
         // Output statements - request has reached local DNS server
         cout << "Request has reached local DNS Server." << endl;

         // Check inside local DNS's map to see if the request is cached
         map<string, string>::iterator IPIter = localCache.find(url);

         // Request is cached - query can be stopped here
         if( IPIter != localCache.end() ){
            cout << "The IP Address exists within the local cache. Returning IP." << endl;
            return IPIter->second;
         }

         // Request is not cached - query must be continued
         else{
            // Output statements - request is not cached - continue to root 
            cout << "The IP Address does not exist within the local cache.";
            cout << " Relay request to root DNS server." << endl;

            // Attempt to obtain IP address 
            string IP = routeToRootDNS->getIP(url);

            // IP address has been successfully located
            if( strcmp(IP.c_str(), "ERROR") != 0 ) {
               // Output statements - request has now been cached in local DNS
               cout << "Inserting IP address for " << url;
               cout << " into the local DNS Cache." << endl;
               localCache.insert( pair<string,string>(url, IP) );

               // IP address has been successfully returned
               cout << "Returning IP from local DNS." << endl;
               return IP;
            }

            // IP address has been NOT successfully located
            else{
               // Query for IP address has failed
               cout << "Returning error. Query failed." << endl;
               return IP;  
            }
         }
      }

   virtual int getCost() {
         return -1;
   }

   // Local DNS contains a map and a pointer to the routers that lead to the root
   private:
      map<string, string> localCache; 
      Device * routeToRootDNS;
};

// Derived class - "RootDNS"
class RootDNS : public Device{
   public:
      // Function to add a route from the root DNS to a TLD DNS
      void addRouteToTLD( Device * newNext, string hostName){
         mapOfRoutesToTLD.insert( pair<string, Device*>(hostName, newNext) );
      }

      // Obtain the IP address from the local DNS server
      virtual string getIP( const string & url)  {
         // Output statements - request has reached the DNS server
         cout << "Request has reached the Root DNS Server." << endl;

         // Parse url for website type (.com, .org, etc.)
         string websiteType;
         bool passedDot = false; 
         for(int i = 0, size = url.size(); i < size; i++){
            if(url[i] == '.' && passedDot == false){
               passedDot = true;
               i++;  
            }
            if(passedDot == true){
               websiteType.push_back(url[i]);
            }
         }

         // Check to see if a route to the appropriate TLD server exists 
         map<string, Device*>::iterator TLDRoute = mapOfRoutesToTLD.find(websiteType);

         // The route to a TLD server exists
         if( TLDRoute != mapOfRoutesToTLD.end() ){
            // Output statements - TLD server exists 
            cout << "Appropriate TLD server exists. Relaying request to TLD DNS server." << endl;
            string IP = TLDRoute->second->getIP(url);
            cout << "Going back through root DNS." << endl; 
            return IP; 
         }

         // The route to a TLD server does not exist
         else{
            cout << "Appropriate TLD server does not exist." << endl;
            return string("ERROR");
         }
      }

   virtual int getCost() {
         return -1;
   }

   // Root DNS contains a map and a pointer to the routers that lead 
   // to a TLD server
   private:
      map<string, Device*> mapOfRoutesToTLD;
};

class TLDDNS : public Device{
   public:
      // Function to add a route from the TLD DNS to a authoritative DNS
      void addRouteToAuthoritative( Device * newNext, string domainName){
         mapOfRoutesToAuthoritativeDNS.insert( pair<string, Device*>(domainName, newNext) );
      }

      // Obtain the IP address from the authoritative DNS server
      virtual string getIP( const string & url)  {
         cout << "Request has reached the TLD DNS Server." << endl;

         // Parse url for domain name
         string domainName;
         for(int i = 0, size = url.size(); url[i] != '.' && i < size; i++){
            domainName.push_back(url[i]);
         }

         // Check to see if a route to the appropriate authoritative server exists          
         map<string, Device*>::iterator AuthRoute = mapOfRoutesToAuthoritativeDNS.find(domainName);

         // The route to an authoritative server exists 
         if( AuthRoute != mapOfRoutesToAuthoritativeDNS.end() ){
            // Output statements - authoritative server exists 
            cout << "Appropriate authoritative server exists. ";
            cout << "Relaying request to corresponding authoritative DNS server." << endl;
            string IP = AuthRoute->second->getIP(url);
            cout << "Going back through the TLD DNS." << endl; 
            return IP; 
         }

         // The route to an authoritative server exists
         else{
            cout << "Appropriate authoritative server does not exist." << endl;
            return string("ERROR");
         }
      }

   virtual int getCost() {
         return -1;
   }

   // TLD DNS contains a map and a pointer to the routers that lead 
   // to a authoritative server
   private:
      map<string, Device*> mapOfRoutesToAuthoritativeDNS;
};

class AuthoritativeDNS : public Device{
   public:
      // Function to add an IP address to the authoritative DNS's map
      void addIP( const string & IPAddress){
         IP = IPAddress;
      }

      // Function for acquiring the IP address from the authoritative DNS
      virtual string getIP( const string & url)  {
         cout << "Request has reached the Authoritative DNS Server." << endl;
         cout << "Returning correct IP address." << endl;
         return IP;
      }

   virtual int getCost() {
         return -1;
   }

   // Authoritatie DNS will only hold IP adresses
   private:
      string IP;
};

Device * createListOfRouters(Device * endDevice, int numRouters){
   Device * current = endDevice;

   // Create a new router and add to previously allocated node
   // If no such node exists already, the linked list will need to be created
   for(int i = 0; i < numRouters; i++){
      Router * newRouter = new Router();
      newRouter->setNext(current);
      current = newRouter;
   }

   // Return a pointer to the head of the newly allocated linked list
   return current;
}

// This function will initialize our network
Device * intializeNetwork(){
   TLDDNS * comTLD = new TLDDNS();
   TLDDNS * eduTLD = new TLDDNS();
   TLDDNS * netTLD = new TLDDNS();

   // amazon
   AuthoritativeDNS * amazonAuth = new AuthoritativeDNS();
   amazonAuth->addIP(string("1.2.3.4"));
   Device * routeToAmazonAuth = new Routes();
   ((Routes*)routeToAmazonAuth)->createRoutes(amazonAuth, rand() % 7 + 1);
   comTLD->addRouteToAuthoritative(routeToAmazonAuth, string("amazon"));

   // google
   AuthoritativeDNS * googleAuth = new AuthoritativeDNS();
   googleAuth->addIP(string("11.22.33.44"));
   Device * routetoGoogleAuth = new Routes();
   ((Routes*)routetoGoogleAuth)->createRoutes(googleAuth, rand() % 6 + 1);
   comTLD->addRouteToAuthoritative(routetoGoogleAuth, string("google"));

   // unr
   AuthoritativeDNS * unrAuth = new AuthoritativeDNS();
   unrAuth->addIP(string("123.22.455.23"));
   Device * routetoUnrAuth = new Routes();
   ((Routes*)routetoUnrAuth)->createRoutes(unrAuth, rand() % 6 + 1);
   eduTLD->addRouteToAuthoritative(routetoUnrAuth, string("unr"));

   // mit
   AuthoritativeDNS * mitAuth = new AuthoritativeDNS();
   mitAuth->addIP(string("122.44.683.35"));
   Device * routetoMitAuth = new Routes();
   ((Routes*)routetoMitAuth)->createRoutes(mitAuth, rand() % 6 + 1);
   eduTLD->addRouteToAuthoritative(routetoMitAuth, string("mit"));

   // nexon
   AuthoritativeDNS * nexonAuth = new AuthoritativeDNS();
   nexonAuth->addIP(string("13.654.542.33"));
   Device * routetoNexonAuth = new Routes();
   ((Routes*)routetoNexonAuth)->createRoutes(nexonAuth, rand() % 6 + 1);
   netTLD->addRouteToAuthoritative(routetoNexonAuth, string("nexon"));

   // speedtest
   AuthoritativeDNS * speedTestAuth = new AuthoritativeDNS();
   speedTestAuth->addIP(string("15.934.932.90"));
   Device * routetoSpeedTestAuth = new Routes();
   ((Routes*)routetoSpeedTestAuth)->createRoutes(speedTestAuth, rand() % 6 + 1);
   netTLD->addRouteToAuthoritative(routetoSpeedTestAuth, string("speedtest"));

   Device * routeToComTLD = new Routes();
   ((Routes*)routeToComTLD)->createRoutes(comTLD, rand() % 6 + 1);
   Device * routeToEduTLD = new Routes();
   ((Routes*)routeToEduTLD)->createRoutes(eduTLD, rand() % 6 + 1);
   Device * routeToNetTLD = new Routes();
   ((Routes*)routeToNetTLD)->createRoutes(netTLD, rand() % 6 + 1);

   RootDNS * root = new RootDNS();
   root->addRouteToTLD(routeToComTLD, "com");
   root->addRouteToTLD(routeToEduTLD, "edu");
   root->addRouteToTLD(routeToNetTLD, "net");

   Device * routeToRoot = new Routes();
   ((Routes*)routeToRoot)->createRoutes(root, rand() % 6 + 1);

   LocalDNS * local = new LocalDNS();
   local->addRouteToRoot(routeToRoot);

   Device * routeToLocal = new Routes();
   ((Routes*)routeToLocal)->createRoutes(local, rand() % 6 + 1);;

   Client * client = new Client();
   client->setNext(routeToLocal);

   return ((Device*)client);
}

bool determineIfNodeWantsToSend(){
   // This function will determine whether or not a node wants to send
   // something in our token ring network
   if(rand() % 100 < 50){
      return false;
   }
   else{
      return true; 
   }
}

void tokenPassingMacProtocol(Device *arrayOfClients[]){
   string query;  
   Timer timer;

   // Simulate "taking turns" MAC protocol
   for(int i = 0; true; i++){
      // Check if a node wants to send a query 
      if(determineIfNodeWantsToSend() == true){
         cout << endl <<"Node " << (i % 4) << " has token and wants to send." << endl;
         cout << "Enter url (q quits the program): " << endl;

         // Obtain input from the user 
         cin >> query; 

         if(strcmp(query.c_str(), "q") == 0){
            break;
         }
         timer.start();
         string IP = arrayOfClients[i % 4]->getIP(query);
         cout << endl << "IP Address: " << IP;
         cout << endl;
         printf( "Query TIME = %f milliseconds.\n", timer.stop() );
      }

      else{
         cout << endl << "Node " << (i % 4) << " has token but does not want";
         cout << " to send." << endl;
      } 
   }

   // Simulation has finished 
   cout << endl << "Token passing protocol is finished." << endl;
}

int main(int argc, char ** argv){
   // Initialize program with 4 clients 
   Device *client[4];
   client[0] = intializeNetwork();
   client[1] = client[0];
   client[2] = client[0];
   client[3] = client[0]; 

   // Perform the token passing protocol
   tokenPassingMacProtocol(client); 
}