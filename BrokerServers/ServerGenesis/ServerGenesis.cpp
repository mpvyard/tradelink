#include "stdafx.h"
#include "ServerGenesis.h"
#include <fstream>

using namespace TradeLibFast;

const char* CONFIGFILE = "GenesisServer.Config.txt";
const char* AUTOFILE = "GenesisServer.Login.txt";
const int MINSERVERS = 3;
enum GENESISSERVERTYPE
{
	GTEXEC,
	GTQUOTE,
	GTLEVEL2,
};
bool ServerGenesis::LoadConfig()
{
	std::ifstream file;
	file.open(CONFIGFILE);
	std::vector<CString> servers;
	std::vector<int> ports;
	if (file.is_open())
	{
		char skip[100];
		char data[100];
		int i = 0;
		while (i++<MINSERVERS)
		{
			file.getline(skip,100);
			file.getline(data,100);
			CString tmp = CString(data);
			std::vector<CString> r;
			gsplit(tmp,CString(","),r);
			servers.push_back(r[0]);
			int port = 15805;
			port = atoi(r[1].GetBuffer());
			ports.push_back(port);
		}
		file.close();
	}

	if (servers.size()<MINSERVERS)
	{
		servers.clear();
		ports.clear();
		servers.push_back("69.64.202.155");
		ports.push_back(15805);
		servers.push_back("69.64.202.156");
		ports.push_back(17811);
		servers.push_back("69.64.202.156");
		ports.push_back(17810);
	}

	gtw->m_setting.SetExecAddress(servers[GTEXEC], ports[GTEXEC]);
	gtw->m_setting.SetQuoteAddress(servers[GTQUOTE], ports[GTQUOTE]);
	gtw->m_setting.SetLevel2Address(servers[GTLEVEL2], ports[GTLEVEL2]);
	return true;
}

	int ServerGenesis::AccountResponse(CString clientname)
	{
		CString s = gjoin(m_accts,","); // join em back together
		TLSend(ACCOUNTRESPONSE,s,clientname); // send the whole list
		return OK;
	}

ServerGenesis::ServerGenesis()
{
	_depth = 0;
	autoattempt = false;
	gtw = new GTWrap();
	LoadConfig();
	gtw->_sg = this;
	
}

ServerGenesis::~ServerGenesis()
{
	gtw->_sg = NULL;
	delete gtw;
}

bool ServerGenesis::Autologin()
{
	if (autoattempt) return false;
	autoattempt = true;
	std::ifstream file;
	file.open(AUTOFILE);
	if (file.is_open())
	{
		char skip[100];
		char data[100];
		file.getline(skip,100);
		file.getline(data,100);
		un = CString(data);
		file.getline(skip,100);
		file.getline(data,100);
		pw = CString(data);
		file.close();
		if (pw.GetLength()*un.GetLength()>0)
		{
			Start(un,pw);
			return true;
		}
	}
	return false;
}

void ServerGenesis::Start()
{
	TLServer_WM::Start();
	Autologin();
}

bool ServerGenesis::subscribed(CString sym)
{
	for (uint i = 0; i<m_stk.size(); i++)
		if (m_stk[i]->GetSymbolName()==sym) 
			return true;
	return false;
}

int ServerGenesis::RegisterStocks(CString client)
{
	TLServer_WM::RegisterStocks(client);

	// get client id
	int cid = FindClient(client);

	// loop through every stock for this client
	for (unsigned int i = 0; i<stocks[cid].size(); i++)
	{
		CString sym = stocks[cid][i];
		// make sure we have subscription
		if (!subscribed(sym))
		{
			StkWrap* stk = (StkWrap*)gtw->CreateStock(sym);
			//stk->tl = this;
			m_stk.push_back(stk);
			CString m;
			m.Format("Added subscription for: %s",sym);
			D(m);
		}
	}
	return OK;
}

void ServerGenesis::SendPosition(int cid,GTOpenPosition& pos)
{
	CString sym = CString(pos.szStock);
	int len = sym.GetLength();
	if ((len>10) || (len==0)) return;
	TLPosition p;
	p.Symbol = CString(pos.szStock);
	p.Size = abs(pos.nOpenShares)*(pos.chOpenSide=='B' ? 1 : -1);
	p.AvgPrice = pos.dblOpenPrice;
	TLSend(POSITIONRESPONSE,p.Serialize(),cid);
}

int ServerGenesis::PositionResponse(CString account, CString client)
{
	GTOpenPosition pos;
	int cid = FindClient(client);
	if (cid==-1) return CLIENTNOTREGISTERED;
	const char * acct = (account=="") ? NULL : account.GetBuffer();
	void *it = gtw->GetFirstOpenPosition(acct,pos);
	SendPosition(cid,pos);
	while (it!=NULL)
	{
		it = gtw->GetNextOpenPosition(acct,it,pos);
		SendPosition(cid,pos);
	}
	return OK;
}

int ServerGenesis::BrokerName()
{
	return Genesis;
}

#define DEFAULTMETHOD METHOD_HELF

MMID getexchange(CString exch)
{
	if (exch=="ISLD")
		return METHOD_ISLD;
	if (exch=="ARCA")
		return METHOD_ARCA;
	if (exch=="BRUT")
		return METHOD_BRUT;
	if (exch=="BATS")
		return METHOD_BATS;
	if (exch=="AUTO")
		return METHOD_AUTO;
	if (exch=="HELF")
		return METHOD_HELF;
	MMID_NYSE;
	return DEFAULTMETHOD;

}

MMID getplace(CString exch)
{
	if (exch=="ISLD")
		return MMID_ISLD;
	if (exch=="ARCA")
		return MMID_ARCA;
	if (exch=="MSE")
		return MMID_MSE;
	if (exch=="BATS")
		return MMID_BATS;
	if (exch=="NASD")
		return MMID_NASD;
	if (exch=="NYSE")
		return MMID_NYSE;
	return MMID_NYSE;

}

long getTIF(CString tif)
{
	if (tif=="DAY")
		return TIF_DAY;
	if (tif=="IOC")
		return TIF_IOC;
	if (tif=="GTC")
		return TIF_MGTC;
	return TIF_DAY;
}

char getPI(TLOrder o)
{
	if (o.isLimit() && o.isStop())
		return PRICE_INDICATOR_STOPLIMIT;
	else if (o.isLimit())
		return PRICE_INDICATOR_LIMIT;
	else if (o.isStop())
		return PRICE_INDICATOR_STOP;
	return PRICE_INDICATOR_MARKET;

}

int ServerGenesis::DOMRequest(int depth)
{
	_depth = depth;
	return OK;
}

std::vector<int> ServerGenesis::GetFeatures()
{
	std::vector<int> f;
	f.push_back(SENDORDER);
	f.push_back(BROKERNAME);
	f.push_back(REGISTERSTOCK);
	f.push_back(ACCOUNTREQUEST);
	f.push_back(ACCOUNTRESPONSE);
	f.push_back(ORDERCANCELREQUEST);
	f.push_back(ORDERCANCELRESPONSE);
	f.push_back(ORDERNOTIFY);
	f.push_back(EXECUTENOTIFY);
	f.push_back(TICKNOTIFY);
	f.push_back(SENDORDERMARKET);
	f.push_back(SENDORDERLIMIT);
	f.push_back(SENDORDERSTOP);
	f.push_back(DOMREQUEST);
	//f.push_back(SENDORDERTRAIL);
	f.push_back(LIVEDATA);
	f.push_back(LIVETRADING);
	f.push_back(SIMTRADING);
	f.push_back(POSITIONRESPONSE);
	f.push_back(POSITIONREQUEST);
	return f;
}


int ServerGenesis::SendOrder(TradeLibFast::TLOrder o)
{
	// ensure logged in
	BOOL valid = gtw->IsLoggedIn();
	if(valid == FALSE)
	{
		D("Must login before sending orders.");
		return BROKERSERVER_NOT_FOUND;
	}

	// make sure symbol is loaded
	GTStock *pStock;
	pStock = gtw->GetStock(o.symbol);
	if(pStock == NULL)
	{
		gtw->CreateStock(o.symbol);
		pStock = gtw->GetStock(o.symbol);
		if (pStock==NULL)
		{
			CString m;
			m.Format("Symbol %s could not be loaded.",o.symbol);
			D(m);
			return SYMBOL_NOT_LOADED;
		}
	}
	// ensure id is unique
	if (o.id==0) // if id is zero, we auto-assign the id
	{
		vector<int> now;
		int id = GetTickCount();
		while (!IdIsUnique(id))
		{
			if (id<2) id = 4000000000;
			id--;
		}
		o.id = id;
	}

	int err = OK;

	// place order
	GTOrder go;
	go = pStock->m_defOrder;
	go.dwUserData = o.id;
	if(pStock->PlaceOrder(go, (o.side ? 'B' : 'S'), o.size, o.price, getexchange(o.exchange), getTIF(o.TIF))==0)
	{
		go.chPriceIndicator = getPI(o);
		go.dblStopLimitPrice = o.stop;
		go.place = getplace(o.exchange);
		err = pStock->PlaceOrder(go);
	}
	
	// save order so it can be canceled later
	if (err==OK)
	{
		m_order.push_back(go);
		orderids.push_back(o.id);
	}

	return err;
}

bool ServerGenesis::IdIsUnique(uint id)
{
	for (uint i = 0; i<orderids.size(); i++)
		if (orderids[i]==id) 
			return false;
	return true;
}

int ServerGenesis::CancelRequest(long id)
{
	gtw->CancelOrder(id);
	return OK;
}

void ServerGenesis::accounttest()
{
	CString m;
	std::list<std::string> lstAccounts;
	gtw->GetAllAccountName(lstAccounts);
	
	std::list<std::string>::iterator s;
	for (s = lstAccounts.begin(); s != lstAccounts.end(); s++)
		m_accts.push_back((*s).c_str());

	if (m_accts.size()>0)
	{
		CString js = gjoin(m_accts,CString(","));
		m.Format("User %s [ID: %s] has %i accounts: %s",gtw->m_user.szUserName,gtw->m_user.szUserID,lstAccounts.size(),js);
		gtw->SetCurrentAccount(m_accts[0]);
	}
	else
		m.Format("No accounts found.");
	D(m);
}

void ServerGenesis::Start(LPCSTR user, LPCSTR pw)
{
	Start();
	int err;
	err = gtw->Login(user,pw);
	CString m = (err==0 ? CString("Login succeeded, wait for connection turn-up...") : CString("Login failed.  Check information."));
	D(m);
}

void ServerGenesis::Stop()
{
	for (uint i = 0; i<m_stk.size(); i++)
		m_stk[i] = NULL;
	m_stk.clear();
	gtw->DeleteAllStocks();
	gtw->Logout();
	D("Logged out.");
	while(!gtw->CanClose()){
		gtw->TryClose();
		Sleep(0);
	}
	D("Connection closed.");
}



