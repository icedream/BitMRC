/*
 * StorageInterface.h
 *
 *  Created on: 26.05.2016
 *      Author: steady286
 *
 *      This file shall define a generic storage interface for all objects
 *      used in the BitMRC Bitmessage application.
 *      The storage services must provide following services for all objects
 *      - query existance
 *      - store
 *      - retrieve
 *      - invalidate
 *      - delete
 *      The objects are:
 *      - Keys (private/public)
 *      - Messages (private in/out, broadcast)
 *      - Connections (persistant node informations, blacklist etc.)
 *
 */

#ifndef STORAGEINTERFACE_H_
#define STORAGEINTERFACE_H_

using namespace std;

#include <string>
#include <vector>
#include <Storage/Unique_Key.h>

class Storable;

class Storage {
public:
	Storage();
	~Storage();
public:
	virtual bool register_storable(Storable * object) = 0;
	virtual Storage * new_Storage() = 0;// { return NULL ; }
public:
	// methods needed by all Storage objects
	virtual bool open(std::string path, std::string user, std::string password) = 0;
	virtual bool close(Storage &) = 0;
	virtual void flush(Storage &) = 0;
protected:
	static vector<Storable *> storable_list ;
};

class Storable {
public:
	virtual  Unique_Key & calc_key(Storable & object_in);
	virtual  bool query(Unique_Key &uq_key_in, string & data_out);
	virtual  bool store(Storable & object_in, Unique_Key & key_out);
	virtual  bool delete_storable(Storable & object_in);
	virtual  bool delete_storable(Unique_Key & key_in);
};




#endif /* STORAGEINTERFACE_H_ */
