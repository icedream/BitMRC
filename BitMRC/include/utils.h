#ifndef LINUX
#pragma once
#endif

#ifndef _UTILS 
#define _UTILS
using namespace std;

#ifdef LINUX
//#include <pthread.h>
#include <string>


#else

#endif
#include <types.h>
#include <shared_mutex>
#include <cstdio>
#include <cstring>
#include <vector>


//unsigned string with stream function for protocol bytes array
class ustring : public basic_string<unsigned char, char_traits<unsigned char>, allocator<unsigned char> >
{
public:
	//convert to string
	string toString();

	//append from string
	void fromString(string from);

	//append a unsigned char or int8
	void appendInt8(unsigned char i);

	//big endian
	void appendInt16(__int16 i);
	
	//big endian
	void appendInt32(__int32 i);
	
	//big endian
	void appendInt64(__int64 i);

	//append varint that use 1-3-5-9 byte with the structures:
	//	1	byte
	//  3   0xfd + short
	//  5	0xfe + uint32
	//	9	0xff + uint64
	void appendVarInt_B(unsigned __int64 i);

	//append varint 32, little endian
	void appendVarInt32(unsigned __int32 i);

	//append varint 64, little endian
	void appendVarInt64(unsigned __int64 i);

	//append a string prefixed by a varint representing the length
	void appendVarString(string str);

	//append a unsigned string prefixed by a varint representing the length
	void appendVarUstring(ustring str);

	//append a string prefixed by a varint_B representing the length
	void appendVarString_B(string str);

	//append a unsigned string prefixed by a varint_B representing the length
	void appendVarUstring_B(ustring str);

	//get a signed byte
	unsigned char getInt8(unsigned int& i);

	//get a signed short
	__int16 getInt16(unsigned int& i);

	//get a signed int32
	__int32 getInt32(unsigned int& i);

	//get a signed int64
	__int64 getInt64(unsigned int& i);

	//get varint that use 1-3-5-9 byte with the structures:
	//	1	byte
	//  3   0xfd + short
	//  5	0xfe + uint32
	//	9	0xff + uint64
	__int64 getVarInt_B(unsigned int& i);

	//get varint 32, little endian
	__int32 getVarInt32(unsigned int& i);

	//get varint 32, little endian
	__int64 getVarInt64(unsigned int& i);

	//get a string segment from i for len bytes
	string getString(int len, unsigned int& i);

	//get a ustring segment from i for len bytes
	ustring getUstring(int len, unsigned int& i);

	//get a string prefixed by a varint representing the lenght
	string getVarString(unsigned int& i);

	//get a unsigned string prefixed by a varint representing the lenght
	ustring getVarUstring(unsigned int& i);

	//get a string prefixed by a varint representing the lenght
	string getVarString_B(unsigned int& i);

	//get a unsigned string prefixed by a varint representing the lenght
	ustring getVarUstring_B(unsigned int& i);

	//get the rest in a ustring
	ustring getRest(unsigned int& i);

	//sum two ustring
	//ustring operator+(ustring adder);
};
#ifdef LINUX
/*\todo: move this to proper location -steady-
 *
 */
#else

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#pragma comment (lib, "Ws2_32.lib")
#endif

#define CONNECTION_CLOSED	1
#define CONNECTION_ERROR	2

//this allow to write and read from a socket
class socket_ustring
{
public:
	public:
	//send a unsigned char or int8
	void sendByte(unsigned char i);

	//big endian
	void sendInt16(__int16 i);
	
	//big endian
	void sendInt32(__int32 i);
	
	//big endian
	void sendInt64(__int64 i);

	//send varint that use 1-3-5-9 byte with the structures:
	//	1	byte
	//  3   0xfd + short
	//  5	0xfe + uint32
	//	9	0xff + uint64
	void sendVarInt_B(unsigned __int64 i);

	//send varint 32, little endian
	void sendVarInt32(unsigned __int32 i);

	//send varint 64, little endian
	void sendVarInt64(unsigned __int64 i);

	//send a string without prefix
	void sendString(string str, int len);

	//send an ustring without prefix
	void sendUstring(ustring str, int len);

	//send a string without prefix
	void sendString(string str);

	//send an ustring without prefix
	void sendUstring(ustring str);

	//send a string prefixed by a varint representing the lenght
	void sendVarString(string str);

	//send an unsigned string prefixed by a varint representing the lenght
	void sendVarUstring(ustring str);

	//send a string prefixed by a varint_B representing the lenght
	void sendVarString_B(string str);

	//send a unsigned string prefixed by a varint_B representing the lenght
	void sendVarUstring_B(ustring str);

	//get a signed byte
	unsigned char getInt8();

	//get a signed short
	__int16 getInt16();

	//get a signed int32
	__int32 getInt32();

	//get a signed int64
	__int64 getInt64();

	//get varint that use 1-3-5-9 byte with the structures:
	//	1	byte
	//  3   0xfd + short
	//  5	0xfe + uint32
	//	9	0xff + uint64
	__int64 getVarInt_B();

	//get varint 32, little endian
	__int32 getVarInt32();

	//get varint 32, little endian
	__int64 getVarInt64();

	//get a string segment from i for len bytes
	string getString(int len);

	//get a ustring segment from i for len bytes
	ustring getUstring(int len);

	//get a string prefixed by a varint representing the lenght
	string getVarString();

	//get a unsigned string prefixed by a varint representing the lenght
	ustring getVarUstring();

	//get a string prefixed by a varint representing the lenght
	string getVarString_B();

	//get a unsigned string prefixed by a varint representing the lenght
	ustring getVarUstring_B();

	//set socket
	void setSocket(SOCK_TYPE sock);

	//holds the socket pointer
	SOCK_TYPE socket;

};

//rewrite ustring for file pointer
class file_ustring
{
public:
public:
	//write a unsigned char or int8
	void writeByte(unsigned char i);

	//big endian
	void writeInt16(__int16 i);

	//big endian
	void writeInt32(__int32 i);

	//big endian
	void writeInt64(__int64 i);

	//write varint that use 1-3-5-9 byte with the structures:
	//	1	byte
	//  3   0xfd + short
	//  5	0xfe + uint32
	//	9	0xff + uint64
	void writeVarInt_B(unsigned __int64 i);

	//write varint 32, little endian
	void writeVarInt32(unsigned __int32 i);

	//write varint 64, little endian
	void writeVarInt64(unsigned __int64 i);

	//write a string without prefix
	void writeString(string str, int len);

	//write an ustring without prefix
	void writeUstring(ustring str, int len);

	//write a string without prefix
	void writeString(string str);

	//write an ustring without prefix
	void writeUstring(ustring str);

	//write a string prefixed by a varint representing the lenght
	void writeVarString(string str);

	//write an unsigned string prefixed by a varint representing the lenght
	void writeVarUstring(ustring str);

	//write a string prefixed by a varint_B representing the lenght
	void writeVarString_B(string str);

	//write a unsigned string prefixed by a varint_B representing the lenght
	void writeVarUstring_B(ustring str);

	//get a signed byte
	unsigned char getInt8();

	//get a signed short
	__int16 getInt16();

	//get a signed int32
	__int32 getInt32();

	//get a signed int64
	__int64 getInt64();

	//get varint that use 1-3-5-9 byte with the structures:
	//	1	byte
	//  3   0xfd + short
	//  5	0xfe + uint32
	//	9	0xff + uint64
	__int64 getVarInt_B();

	//get varint 32, little endian
	__int32 getVarInt32();

	//get varint 32, little endian
	__int64 getVarInt64();

	//get a string segment from i for len bytes
	string getString(int len);

	//get a ustring segment from i for len bytes
	ustring getUstring(int len);

	//get a string prefixed by a varint representing the lenght
	string getVarString();

	//get a unsigned string prefixed by a varint representing the lenght
	ustring getVarUstring();

	//get a string prefixed by a varint representing the lenght
	string getVarString_B();

	//get a unsigned string prefixed by a varint representing the lenght
	ustring getVarUstring_B();

	//set socket
	void setFile(FILE * file);

	//holds the file pointer
	FILE * pFile;

};

#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
 
template <typename T>
class Queue
{
 public:
 
  T pop()
  {
    std::unique_lock<std::mutex> mlock(mutex_);
    while (queue_.empty())
    {
      cond_.wait(mlock);
    }
    auto item = queue_.front();
    queue_.pop();
    return item;
  }
 
  void pop(T& item)
  {
    std::unique_lock<std::mutex> mlock(mutex_);
    while (queue_.empty())
    {
      cond_.wait(mlock);
    }
    item = queue_.front();
    queue_.pop();
  }
 
  void push(const T& item)
  {
    std::unique_lock<std::mutex> mlock(mutex_);
    queue_.push(item);
    mlock.unlock();
    cond_.notify_one();
  }
 
  void push(T&& item)
  {
    std::unique_lock<std::mutex> mlock(mutex_);
    queue_.push(std::move(item));
    mlock.unlock();
    cond_.notify_one();
  }

  int size()
  {
	  std::unique_lock<std::mutex> mlock(mutex_);
	  int ret = queue_.size();
	  mlock.unlock();
	  return ret;
  }
 
 private:
  std::queue<T> queue_;
  std::mutex mutex_;
  std::condition_variable cond_;
};

template<typename T>
class hash_table
{
public:
	
	std::shared_timed_mutex mutex_;

	typedef struct linked_node_struct
	{
		ustring hash;
		T info;
		struct linked_node_struct *next;
	}linked_node;

	linked_node **Table;
	int Dim;
	hash_table()
	{
		std::unique_lock<shared_timed_mutex> mlock(this->mutex_);
		Dim = 100771;
		this->Table = new linked_node*[this->Dim];
		memset(this->Table, NULL, sizeof(linked_node*) * this->Dim);
		mlock.unlock();
	}

	~hash_table()
	{
		std::unique_lock<shared_timed_mutex> mlock(this->mutex_);
		for (int i = 0; i < this->Dim; i++)
		{
			hash_table::linked_node * cur = this->Table[i];
			hash_table::linked_node * tmp;
			while (cur != NULL)
			{
				tmp = cur->next;
				delete cur;
				cur = tmp;
			}
		}
		delete[] this->Table;
	}
	//insert in the table
	// 0 if present so not insert
	// 1 if first with this hash
	// 2 if not first
	int insert(T info, ustring hash)
	{
		int i = hash_f(hash);

		std::unique_lock<std::shared_timed_mutex> mlock(this->mutex_);
		linked_node * cur = this->Table[i];
		if (cur == NULL)
		{
			this->Table[i] = new linked_node;
			this->Table[i]->info = info;
			this->Table[i]->hash = hash;
			this->Table[i]->next = NULL;
			mlock.unlock();
			return 1;
		}
		else
		{
			while (cur->next != NULL)
			{
				if (cur->info == info)
				{
					mlock.unlock();
					return 0;
				}
				cur = cur->next;
			}
			if (cur->info == info)
			{
				mlock.unlock();
				return 0;
			}
			else
			{
				cur->next = new linked_node;
				cur->next->info = info;
				cur->next->hash = hash;
				cur->next->next = NULL;
				mlock.unlock();
				return 2;
			}
		}
	}

	//search the hash
	//return:	1 if present
	//			0 if not
	int present(ustring hash)
	{
		int i = hash_f(hash);
		std::unique_lock<std::shared_timed_mutex> mlock(this->mutex_);
		linked_node * cur = this->Table[i];
		while (cur != NULL)
		{
			if (cur->hash == hash)
			{
				mlock.unlock();
				return 1;
			}
			cur = cur->next;
		}
		mlock.unlock();
		return false;
	}

	T searchByHash(ustring hash)
	{
		int i = hash_f(hash);
		std::unique_lock<std::shared_timed_mutex> mlock(this->mutex_);
		linked_node * cur = this->Table[i];
		while (cur != NULL)
		{
			if (cur->hash == hash)
			{
				mlock.unlock();
				return cur->info;
			}
			cur = cur->next;
		}
		mlock.unlock();
		T ret;
		return ret;
	}

	//update an element
	//return:	1 if present
	//			0 else
	int updateByHash(ustring hash, T info)
	{
		int i = hash_f(hash);
		std::unique_lock<std::shared_timed_mutex> mlock(this->mutex_);
		linked_node * cur = this->Table[i];
		linked_node * prec = NULL;
		while (cur != NULL)
		{
			if (cur->hash == hash)
			{
				cur->info = info;
				mlock.unlock();
				return 1;
			}
			prec = cur;
			cur = cur->next;
			
		}

		prec->next = new linked_node;
		prec->next->info = info;
		prec->next->hash = hash;
		prec->next->next = NULL;

		mlock.unlock();
		return 0;
	}

	int hash_f(ustring hash)
	{
		int intLength = hash.length() / 4;
		long sum = 0;
		char c[4];
		for (int j = 0; j < intLength; j++) {
			strncpy(c,(char*)hash.substr(j * 4, 4).c_str(),4);
			long mult = 1;
			for (int k = 0; k < 4; k++) {
				sum += c[k] * mult;
				mult *= 256;
			}
		}

		strcpy(c,(char*)hash.substr(intLength * 4).c_str());
		long mult = 1;
		for (unsigned int k = 0; k < (hash.length()-(intLength * 4)); k++) {
			sum += c[k] * mult;
			mult *= 256;
		}

		return(abs(sum) % this->Dim);
	}
};

#endif 
