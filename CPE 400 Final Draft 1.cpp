class Device{
	public:
		virtual string getIP(const string & url) const = 0;
};

class Router : public Device{
	public:
		virtual string getIP(const string & url){
			//Error Stuff!
			return next->getIP(url);
		}

	private:
		Device * next;
};

class LocalDNS : public Device{
	public:
		virtual string getIP(const string & url){
			//check inside its map if it is there,
			map<string, string>::iterator IPIter = localCache.find(url);
			if( IPIter != localCache.end() ){
				return *IPIter;
			}
			else{
				string IP = routeToRootDNS->getIP(url);
				localCache.insert(pair<string,string>(url, IP));
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
		virtual string getIP(const string & url){
			map<string, *Device>::iterator TLDRoute = mapOfRoutesToTLD.find(url);
			if( TLDRoute != mapOfRoutesToTLD.end() ){
				return TLDRoute->getIP(url);
			}
			else{
				return -1;
			}
		}

	private:
		map<string, *Device> mapOfRoutesToTLD;
};

class TLDDNS : public Device{
	public:
		virtual string getIP(const string & url){
			map<string, *Device>::iterator AuthRoute = mapOfRoutesToAuthoritativeDNS.find(url);
			if( AuthRoute != mapOfRoutesToAuthoritativeDNS.end() ){
				return AuthRoute->getIP(url);
			}
			else{
				return -1;
			}
		}

	private:
		map<string, *Device> mapOfRoutesToAuthoritativeDNS;
};

class AuthoritativeDNS : public Device{
	public:
		virtual string getIP(const string & url){
			return IP;
		}

	private:
		string IP;
}