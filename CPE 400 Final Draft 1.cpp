class Device{
   public:
      Device() : next(NULL) { }
      virtual string getIP(const string & url) const = 0;
};

class Router : public Device{
   public:
      void setNext(const Device * newNext){
         next = newNext;
      }

      virtual string getIP(const string & url) const {
         cout << "Request has reached router." << endl;

         if(rand() % 100 < 90){
            cout << "Successfully passed through router." << endl;
            return next->getIP(url);
         }
         else{
            cout << "Unsuccesful TCP transfer, request was lost"
            return string("TCP ERROR");
         }
      }
   private:
      Device * next;
};

class LocalDNS : public Device{
   public:
      void addRouteToRoot(const Device * newRoute){
         routeToRootDNS = newRoute;
      }

      virtual string getIP(const string & url) const {
         cout << "Request has reached local DNS Server." << endl;

         //check inside its map if it is there,
         map<string, string>::iterator IPIter = localCache.find(url);
         if( IPIter != localCache.end() ){
            cout << "The IP Address exists within the local cache. Returning IP." << endl;
            return *IPIter;
         }
         else{
            cout << "The IP Address does not exist within the local cache. Relay request to root DNS server." << endl;
            string IP = routeToRootDNS->getIP(url);

            cout << "Inserting IP address for " << url << " into the local DNS Cache." << endl;
            localCache.insert(pair<string,string>(url, IP));

            cout << "Returning IP from local DNS." << endl;
            return IP;
         }
         //If it is then return it
         //else traverse list of routers to get to rootDNS
      }
   private:
      map<string, string> localCache; 
      Device * routeToRootDNS;
};

class RootDNS : public Device{
   public:
      void addRouteToTLD(const Device * newNext, string hostName){
         mapOfRoutesToTLD.insert( pair<string, *Device>(hostName, newNext) );
      }

      virtual string getIP(const string & url) const {
         cout << "Request has reached the Root DNS Server." << endl;

         map<string, *Device>::iterator TLDRoute = mapOfRoutesToTLD.find(url);
         if( TLDRoute != mapOfRoutesToTLD.end() ){
            cout << "Appropriate TLD server exists. Relaying request to TLD DNS server." << endl;
            return TLDRoute->getIP(url);
         }
         else{
            cout << "Appropriate TLD server does not exist." << endl;
            return string("TLD server does not exist");
         }
      }

   private:
      map<string, *Device> mapOfRoutesToTLD;
};

class TLDDNS : public Device{
   public:
      void addRouteToAuthoritative(const Device * newNext, string domainName){
         mapOfRoutesToAuthoritativeDNS.insert( pair<string, *Device>(domainName, newNext) );
      }

      virtual string getIP(const string & url) const {
         cout << "Request has reached the TLD DNS Server." << endl;

         map<string, *Device>::iterator AuthRoute = mapOfRoutesToAuthoritativeDNS.find(url);
         if( AuthRoute != mapOfRoutesToAuthoritativeDNS.end() ){
            cout << "Appropriate authoritative server exists. Relaying request to corresponding authoritative DNS server." << endl;
            return AuthRoute->getIP(url);
         }
         else{
            cout << "Appropriate authoritative server does not exist." << endl;
            return string("Authoritative server does not exist");
         }
      }

   private:
      map<string, *Device> mapOfRoutesToAuthoritativeDNS;
};

class AuthoritativeDNS : public Device{
   public:
      void addIP(const string & IPAddress){
         IP = IPAddress;
      }

      virtual string getIP(const string & url) const {
         cout << "Request has reached the Authoritative DNS Server." << endl;
         cout << "Returning correct IP address" << endl;
         return IP;
      }

   private:
      string IP;
};

Device * intializeNetwork(){
   Device * amazonAuth = new AuthoritativeDNS();
   amazonAuth->addIP(string("1.2.3.4"));

   Device * routeToAmazonAuth = createListOfRouters(amazonAuth, 5);

   Device * comTLD = new TLDDNS();
   comTLD->addRouteToAuthoritative(routeToAmazonAuth, string("amazon"));

   Device * routeToComTLD = createListOfRouters(comTLD, 3);

   Device * root = new RootDNS();
   root->addRouteToTLD(routeToComTLD, "com");

   Device * routeToRoot = createListOfRouters(root, 4);

   Device * local = new LocalDNS();
   local->addRouteToRoot(routeToRoot);

   Device * routeToLocal = createListOfRouters(local, 5);

   return routeToLocal;
}

Device * createListOfRouters(Device * endDevice, int numRouters){
   Device * current = endDevice;
   for(int i = 0; i < numRouters; i++){
      Device * newRouter = new Router();
      newRouter->setNext(current);
      current = newRouter;
   }
   return current;
}

int main(int argc, char ** argv){
   Device * client = intializeNetwork();
   string query;

   while(true){
      cout << "Gimme dat url Batch: ";
      cin >> query;
      cout << endl;

      cout << client->getIP(query);
   }

}






