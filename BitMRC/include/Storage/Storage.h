/*
 * Storage.h
 *
 *  Created on: 07.06.2016
 *      Author: steady286
 */

#ifndef STORAGE_H_
#define STORAGE_H_

using namespace std;

#include <string.h>
#include <vector>
#include <Storage/Unique_Key.h>
#include <Storage/Storable.h>
typedef struct {
	uint32_t query_type;
	void * data_struct_ptr;
} query_config_t;

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
	static std::vector<Storable *> storable_list ;
};

#endif /* STORAGE_H_ */
