#include "client.h"
#include "acctserver.h"
#include "packet.h"
#include "lin_time.h"

VegaConfig * vs_config;
//VegaConfig * acct_config;
string acctdir;

AccountServer::AccountServer()
{
	cout<<"AccountServer init"<<endl;
	Network = new TCPNetUI();
	newaccounts = 0;
	UpdateTime();
	srand( (int) getNewTime());
	serial_seed = (ObjSerial) (rand()*(500./(((double)(RAND_MAX))+1)));
}

/**************************************************************/
/**** Authenticate a connected client                      ****/
/**************************************************************/

ObjSerial	AccountServer::getUniqueSerial()
{
	// MAYBE CHANGE TO SOMETHING MORE "RANDOM"
	serial_seed = (serial_seed+3)%MAXSERIAL;
	return serial_seed;
}

AccountServer::~AccountServer()
{
	delete Network;
}

/**************************************************************/
/**** Display info on the server at startup                ****/
/**************************************************************/

void	AccountServer::startMsg()
{
	cout<<endl<<"Vegastrike Account Server version 0.0.1"<<endl;
	cout<<"Written by Stephane Vaxelaire"<<endl<<endl;
}

void	AccountServer::start()
{
	string strperiod;
	keeprun = 1;
	double	savetime;
	double	curtime;

	TCPSOCKET	comsock;

	startMsg();

	cout<<"Loading config file...";
	vs_config = new VegaConfig( ACCTCONFIGFILE);
	cout<<" done."<<endl;
	InitTime();
	UpdateTime();
	acctdir = vs_config->getVariable( "server", "accounts_dir", "");
	if( acctdir=="")
		acctdir = "./accounts/";
	strperiod = vs_config->getVariable( "server", "saveperiod", "");
	int period;
	if( strperiod=="")
		period = 7200;
	else
		period = atoi( strperiod.c_str());
	savetime = getNewTime()+period;

	cout<<"Loading accounts data... ";
	LoadAccounts( "accounts.xml");
	// Gets hashtable accounts elements and put them in vector Cltacct
	Cltacct = getAllAccounts();
	if(Cltacct.size()<=0)
	{
		cout<<"No account found in accounts.xml"<<endl;
		cleanup();
	}
	cout<<Cltacct.size()<<" accounts loaded."<<endl;

	// Create and bind socket
	cout<<"Initializing network..."<<endl;
	unsigned short tmpport;
	if( vs_config->getVariable( "network", "accountsrvport", "")=="")
		tmpport = ACCT_PORT;
	else
		tmpport = atoi((vs_config->getVariable( "network", "accountsrvport", "")).c_str());
	Network->createSocket( "127.0.0.1", tmpport, 1);
	cout<<"done."<<endl;
	while( keeprun)
	{
		//cout<<"Loop"<<endl;
		// Check for incoming connections
		comsock = Network->acceptNewConn( NULL);
		if( comsock)
		{
			Socks.push_back( comsock);
			cout<<"New connection - socket allocated : "<<comsock<<endl;
		}
		// Check for messages
		this->checkMsg();

		for (LS j=DeadSocks.begin(); j!=DeadSocks.end(); j++)
		{
			cout<<"Closing socket number "<<(*j)<<endl;
			Network->closeSocket( (*j));
			Socks.remove( (*j));
		}
		DeadSocks.clear();

		// Check for automatic server status save time
		curtime = getNewTime();
		if( curtime - savetime > period)
		{
			// Not implemented
			this->save();
			savetime += period;
		}

		micro_sleep(40000);
	}

	delete vs_config;
	Network->disconnect( "Shutting down.");
}

void	AccountServer::checkMsg()
{
	// Check sockets to be watched
	Network->resetSets();
	for( LS i=Socks.begin(); i!=Socks.end(); i++)
	{
		Network->watchSocket( (*i));
	}
	// Get the number of active clients
	int nb = Network->activeSockets();
	if( nb)
	{
		//cout<<"Net activity !"<<endl;
		// Loop for each active client and process request
		for( LS i=Socks.begin(); i!=Socks.end(); i++)
		{
			if( Network->isActive( (*i)))
			{
				this->recvMsg( (*i));
			}
		}
	}
}

void	AccountServer::recvMsg( TCPSOCKET sock)
{
	char			name[NAMELEN+1];
	char			passwd[NAMELEN+1];
	AddressIP		ipadr;
	unsigned int	len=0;
	int				recvcount=0;
	unsigned char	cmd;
	Account *		elem = NULL;
	int 			found=0, connected=0;

	// Receive data from sock
	//cout<<"Receiving on socket "<<sock<<endl;
	if( (recvcount = Network->recvbuf( sock, (char *)&packet, len, &ipadr))>0)
	{
		// Check the command of the packet
		packet.received();
		//packet.display();
		cmd = packet.getCommand();
		const char * buf = packet.getData();
		cout<<"Buffer => "<<buf<<endl;
		VI j;
		switch( cmd)
		{
			case CMD_LOGIN :
				strcpy( name, buf);
				strcpy( passwd, buf+NAMELEN);
				cout<<">>> LOGIN REQUEST =( "<<name<<":"<<passwd<<" )= --------------------------------------"<<endl;

				for (  j=Cltacct.begin(); j!=Cltacct.end() && !found && !connected; j++)
				{
					elem = *j;
					if( !elem->compareName( name) && !elem->comparePass( passwd))
					{
						if( elem->isConnected())
							connected = 1;
						else
							found = 1;
					}
				}
				if( !found)
				{
					if( connected)
					{
						cout<<"Inconsistency found !"<<endl;
						exit( 1);
					}
					else
					{
						cout<<"Login/passwd not found"<<endl;
						elem = new Account( name, passwd);
						this->sendUnauthorized( sock, elem);
						delete elem;
					}
				}
				else
				{
					if( connected)
					{
						cout<<"Login already connected !"<<endl;
						this->sendAlreadyConnected( sock, elem);
					}
					else
					{
						cout<<"Login accepted !"<<endl;
						this->sendAuthorized( sock, elem);
						elem->setConnected( true);
					}
				}
				cout<<"<<< LOGIN REQUEST ------------------------------------------"<<endl;
			break;
			case CMD_LOGOUT :
				strcpy( name, buf);
				cout<<">>> LOGOUT REQUEST =( "<<name<<" )= --------------------------------------"<<endl;
				strcpy( passwd, buf+NAMELEN);
				// Receive logout request containing name of player
				for (  j=Cltacct.begin(); j!=Cltacct.end() && !found && !connected; j++)
				{
					elem = *j;
					if( !elem->compareName( name) && !elem->comparePass( passwd))
					{
						found = 1;
						if( elem->isConnected())
							connected = 1;
						else
							connected = 0;
					}
				}
				if( !found)
				{
					cout<<"ERROR LOGOUT -> didn't find player to disconnect = <"<<name<<">:<"<<passwd<<">"<<endl;
				}
				else
				{
					if( connected)
					{
						elem->setConnected( false);
						cout<<"-= "<<name<<" =- Disconnected"<<endl;
					}
					else
					{
						cout<<"ERROR LOGOUT -> player exists but wasn't connected ?!?!"<<endl;
					}
				}
				cout<<"<<< LOGOUT REQUEST ---------------------------------------"<<endl;
			break;
			case CMD_NEWCHAR :
				cout<<">>> NEW CHAR REQUEST =( "<<name<<" )= --------------------------------------"<<endl;
				// Should receive the result of the creation of a new char/ship
				cout<<"<<< NEW CHAR REQUEST -------------------------------------------------------"<<endl;
			break;
			case CMD_NEWSUBSCRIBE :
				cout<<">>> SUBSRIBE REQUEST =( "<<name<<" )= --------------------------------------"<<endl;
				// Should receive a new subscription
				cout<<"<<< SUBSRIBE REQUEST --------------------------------------"<<endl;
			break;
			default:
				cout<<">>> UNKNOWN command =( "<<cmd<<" )= ---------------------------------";
		}
		//cout<<"end received"<<endl;
	}
	/*
	else if( recvcount==0)
	{
		// SOCKET should have been closed by that game server
		cout<<"Received 0 data on socket "<<sock<<endl;
	}
	*/
	else
	{
		cout<<"Received failed or socket closed"<<endl;
		DeadSocks.push_back( sock);
	}
}

void	AccountServer::sendAuthorized( TCPSOCKET sock, Account * acct)
{
	Packet	packet2;
	// Get a serial for client
	ObjSerial serial = getUniqueSerial();
	cout<<"\tLOGIN REQUEST SUCCESS for <"<<acct->name<<">:<"<<acct->passwd<<">"<<endl;
	// Verify that client already has a ship or if it is a new account
	if( acct->isNew())
	{
		// Send a command to make the client create a new character/ship
		packet2.create( LOGIN_NEW, serial, packet.getData(), packet.getLength(), 1);
		packet2.tosend();
		if( Network->sendbuf( sock, (char *) &packet2, packet2.getSendLength(), NULL) == -1)
		{
			cout<<"ERROR sending authorization"<<endl;
			exit( 1);
		}
		// Should receive an answer from game server that contains ship's creation info to
		// be saved on the account server
	}
	else
	{
		// Should get the data about the player state and data so they can be sent with ACCEPT
		char	buf[MAXBUFFER];
		memcpy( buf, packet.getData(), NAMELEN*2);
		unsigned int maxsave = MAXBUFFER - Packet::getHeaderLength() - 2*NAMELEN - 2*sizeof( unsigned int);
		unsigned int readsize=0, readsize2=0;

		// Read the XML unit file
		string acctfile = acctdir+acct->name+".xml";
		cout<<"Trying to open : "<<acctfile<<endl;
		FILE *fp = fopen( acctfile.c_str(), "r");
		if( fp == NULL)
		{
			cout<<"Account file does not exists... sending default one to game server"<<endl;
			acctfile = acctdir+"default.xml";
			cout<<"Trying to open : "<<acctfile<<endl;
			fp = fopen( acctfile.c_str(), "r");
		}
		if( fp!=NULL)
		{
			readsize = fread( (buf+2*NAMELEN+sizeof( unsigned int)), sizeof( char), maxsave, fp);
			if( readsize>=maxsave)
			{
				cout<<"Error : account file is bigger than "<<maxsave<<" ("<<readsize<<")"<<endl;
				exit( 1);
			}
			fclose( fp);
		}
		else
		{
			cout<<"Error, default xml save not found"<<endl;
			cleanup();
		}
		// Put the size of the first save file in the buffer to send
		unsigned int xmlsize = htonl( readsize);
		memcpy( buf+2*NAMELEN, &xmlsize, sizeof( unsigned int));
		//unsigned int xml_size = ntohl( *( (unsigned int *)(buf+NAMELEN*2)));
		//cout<<"XML reversed = "<<xml_size<<endl;

		// Read the save file
		string acctsave = acctdir+acct->name+".save";
		fp = fopen( acctsave.c_str(), "r");
		if( fp == NULL)
		{
			cout<<"Save file does not exists... sending default one to game server"<<endl;
			acctsave = acctdir+"default.save";
			cout<<"Trying to open : "<<acctsave<<endl;
			fp = fopen( acctsave.c_str(), "r");
		}
		if( fp!=NULL)
		{
			// Read the XML unit file
			readsize2 = fread( (buf+readsize+2*NAMELEN+2*sizeof( unsigned int)), sizeof( char), maxsave, fp);
			if( (readsize2+readsize) >= maxsave)
			{
				cout<<"Error : save file is bigger than "<<maxsave<<" ("<<readsize2<<")"<<endl;
				exit( 1);
			}
			fclose( fp);
		}
		else
		{
			cout<<"Error, default save not found"<<endl;
			cleanup();
		}
		// Put the size of the second save file in the buffer to send
		unsigned int savesize = htonl( readsize2);
		//cout<<"NETWORK FORMAT : XML size = "<<xmlsize<<" --- SAVE size = "<<savesize<<endl;
		//cout<<"HOST FORMAT : XML size = "<<ntohl(xmlsize)<<" --- SAVE size = "<<ntohl(savesize)<<endl;
		memcpy( buf+2*NAMELEN+sizeof( unsigned int)+readsize, &savesize, sizeof( unsigned int));
		cout<<"Loaded -= "<<acct->name<<" =- save files ("<<(readsize+readsize2)<<")"<<endl;
		unsigned int total_size = readsize+readsize2+2*NAMELEN+2*sizeof( unsigned int);
		//cout<<"Login packet size = "<<total_size<<endl;

		// ??? memcpy( buf, packet.getData(), packet.getLength());

		// For now saves are really limited to maxsave bytes
		packet2.create( LOGIN_ACCEPT, serial, buf, total_size, 1);
		cout<<"Login packet size = "<<packet2.getLength()<<endl;
		packet2.tosend();
		if( Network->sendbuf( sock, (char *) &packet2, packet2.getSendLength(), NULL) == -1)
		{
			cout<<"ERROR sending authorization"<<endl;
			exit( 1);
		}
	}
}

void	AccountServer::sendUnauthorized( TCPSOCKET sock, Account * acct)
{
	Packet	packet2;

	packet2.create( LOGIN_ERROR, 0, packet.getData(), packet.getLength(), 1);
	packet2.tosend();
	//packet2.displayHex();
	//cout<<" done."<<endl;
	Network->sendbuf( sock, (char *) &packet2, packet2.getSendLength(), NULL);
	cout<<"\tLOGIN REQUEST FAILED for <"<<acct->name<<">:<"<<acct->passwd<<">"<<endl;
}

void	AccountServer::sendAlreadyConnected( TCPSOCKET sock, Account * acct)
{
	Packet	packet2;

	packet2.create( LOGIN_ALREADY, 0, packet.getData(), packet.getLength(), 1);
	packet2.tosend();
	//packet2.displayHex();
	//cout<<" done."<<endl;
	Network->sendbuf( sock, (char *) &packet2, packet2.getSendLength(), NULL);
	cout<<"\tLOGIN REQUEST FAILED for <"<<acct->name<<">:<"<<acct->passwd<<"> -> ALREADY LOGGED IN"<<endl;
}

void	AccountServer::save()
{
}
