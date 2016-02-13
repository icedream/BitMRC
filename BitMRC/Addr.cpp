#include "Addr.h"

PubAddr::PubAddr()
{
	this->empty = true;
}

PubAddr::PubAddr(const PubAddr &that)
{
	this->pubSigningKey = that.pubSigningKey;
	this->pubEncryptionKey = that.pubEncryptionKey;
	this->empty = that.empty;

	this->extra_bytes = that.extra_bytes;
	this->nonce_trials = that.nonce_trials;

	this->version = that.version;
	this->stream = that.stream;
	this->ripe = that.ripe;

	this->address = that.address;
	this->tag = that.tag;
	this->tagE = that.tagE;
}

bool PubAddr::loadAddr(ustring address)
{
	this->address = address;

	unsigned int i = 0;
	string BM = address.getString(3, i);
	if (strncmp(BM.c_str(), "BM-", 3) != 0)
	{
		i = 0;
	}

	char retu[100];
	size_t size2 = 50;
	if (b58tobin(retu, &size2, (char *)address.getRest(i).c_str()) != true) 
	{
		return false; //ignoring key not valid
	}

	ustring _buffer;
	retu[size2] = 0x00;
	for (unsigned int j = 0; j < size2; j++)
		_buffer += retu[j];
	i = 0;

	unsigned int checkPos = _buffer.length() - 4;
	
	if (checkPos < (unsigned int)0)
		return false;

	ustring buffer = _buffer.getUstring(checkPos, i);

	byte checksum[4];
	memcpy(checksum, _buffer.getUstring(4, checkPos).c_str(), sizeof checksum);

	CryptoPP::SHA512 hash;
	byte digest[CryptoPP::SHA512::DIGESTSIZE];
	byte digest2[CryptoPP::SHA512::DIGESTSIZE];
	hash.CalculateDigest(digest, (byte*)buffer.c_str(), buffer.length());
	hash.CalculateDigest(digest2, digest, sizeof digest);

	if (memcmp(digest2, checksum, sizeof checksum)!=0)
	{
		return false; //ignoring key not valid
	}

	i = 0;
	
	this->version = (int)buffer.getVarInt_B(i);
	this->stream = (int)buffer.getVarInt_B(i);
	
	ustring tmp_ripe = buffer.getRest(i);

	if (tmp_ripe.length() > 20)
		return false; //too long

	if (tmp_ripe.length() < 4)
		return false; //too short

	while (tmp_ripe.length() != 20) //todo add function prepend
	{
		ustring tmp2;
		tmp2.appendInt8(0);
		tmp2+=tmp_ripe;
		tmp_ripe = tmp2;
	}

	this->ripe.clear();
	this->ripe = tmp_ripe;

	buffer.clear();
	buffer.appendVarInt_B(this->version);
	buffer.appendVarInt_B(this->stream);
	buffer += this->ripe;

	hash.CalculateDigest(digest, (byte*)buffer.c_str(), buffer.length());
	hash.CalculateDigest(digest2, digest, sizeof digest);

	this->tagE.clear();
	this->tagE.append(digest2, 32);
	this->tag.clear();
	this->tag.append(&(digest2[32]), 32);

	this->empty = true;

	return true;
}

bool PubAddr::loadKeys(ustring Skey, ustring Ekey, int nonce, int extra)
{
	std::unique_lock<std::mutex> mlock(this->mutex_);
	this->extra_bytes = extra;
	this->nonce_trials = nonce;

	this->pubEncryptionKey.clear();
	this->pubSigningKey.clear();

	int length = Ekey.length();
	if (length == 64)
	{
		
		this->pubEncryptionKey += 0x04;
		this->pubEncryptionKey += Ekey;
	}
	else if (length == 65)
	{
		if (Ekey.c_str()[0] != 0x04)
		{
			mlock.unlock();
			return false;
		}
		this->pubEncryptionKey += Ekey;
	}

	length = Skey.length();
	if (length == 64)
	{
		

		this->pubSigningKey += 0x04;
		this->pubSigningKey += Skey;

		
	}
	else if (length == 65)
	{
		if (Skey.c_str()[0] != 0x04)
		{
			mlock.unlock();
			return false;
		}
		this->pubSigningKey += Skey;
	}

	this->empty = false;
	mlock.unlock();
	return true;
}

bool PubAddr::decodeFromObj(packet_pubkey pubkey)
{
	ustring data;
	try
	{
		data = this->decode(pubkey.encrypted, this->getTagE());
	}
	catch (...)
	{
		return false;
	}
	unsigned int p = 0;
	int bitfield = data.getInt32(p);
	ustring signingKey;
	signingKey += 0x04;
	signingKey += data.getUstring(64, p);
	ustring encryptionKey;
	encryptionKey += 0x04;
	encryptionKey += data.getUstring(64, p);
	int nonce_trials = (int)data.getVarInt_B(p);
	int extra_bytes = (int)data.getVarInt_B(p);
	int sig_len = (int)data.getVarInt_B(p);
	data.getUstring(sig_len, p);

	//todo check signature

	this->loadKeys(signingKey, encryptionKey, nonce_trials, extra_bytes);

	return true;
}



ustring PubAddr::decode(ustring data, ustring privK)
{
	unsigned int p = 0;

	ustring IV = data.getUstring(16, p);
	unsigned int curveType = data.getInt16(p);
	unsigned int Xlen = data.getInt16(p);
	ustring X = data.getUstring(Xlen, p);
	unsigned int Ylen = data.getInt16(p);
	ustring Y = data.getUstring(Ylen, p);
	ustring cipherText = data.getUstring(data.size() - (p + 32), p);
	ustring MAC = data.getUstring(32, p);

	ustring pubK;
	pubK += 0x04;
	pubK += X;
	pubK += Y;


	OID CURVE = secp256k1();
	AutoSeededRandomPool rng;

	ECDH < ECP >::Domain dhA(CURVE), dhB(CURVE);
	SecByteBlock privA(privK.c_str(), dhA.PrivateKeyLength());
	SecByteBlock pubB(pubK.c_str(), dhB.PublicKeyLength());

	if (dhA.AgreedValueLength() != dhB.AgreedValueLength())
		throw runtime_error("Shared shared size mismatch");

	SecByteBlock sharedA(dhA.AgreedValueLength()), sharedB(dhB.AgreedValueLength());

	if (!dhA.Agree(sharedA, privA, pubB))
		throw runtime_error("Failed to reach shared secret (A)");


	Integer ssa, ssb;

	ssa.Decode(sharedA.BytePtr(), sharedA.SizeInBytes());

	uint8_t H[CryptoPP::SHA512::DIGESTSIZE];
	CryptoPP::SHA512 hash;

	hash.CalculateDigest(H, sharedA.BytePtr(), sharedA.SizeInBytes());



	AutoSeededRandomPool prng;

	byte key[32];

	memcpy(key, H, sizeof(key));

	byte Hkey[32];

	memcpy(Hkey, &(H[32]), sizeof(Hkey));

	byte iv[16];

	memcpy(iv, IV.c_str(), IV.size());

	string cipher = cipherText.toString(), encoded, recovered;

	string HMacPlain;

	p = 0;
	HMacPlain += data.getString(data.size()-32,p);

	string mac;

	try
	{
		HMAC<SHA256> hmac(Hkey, 32);
		StringSource s(HMacPlain, true,
			new HashFilter(hmac,
				new StringSink(mac)
				)
			);
	}
	catch (const CryptoPP::Exception& e)
	{
		throw runtime_error(e.what());
	}

	if (mac != MAC.toString())
		throw runtime_error("mac doesnt match");

	try
	{
		CBC_Mode< AES >::Decryption d;
		d.SetKeyWithIV(key, sizeof(key), iv);

		// The StreamTransformationFilter removes
		//  padding as required.
		StringSource s(cipher, true,
			new StreamTransformationFilter(d,
				new StringSink(recovered)
				) // StreamTransformationFilter
			); // StringSource
	}
	catch (const CryptoPP::Exception& e)
	{
		throw runtime_error(e.what());
	}

	ustring result;
	result.fromString(recovered);
	return result;
}

ustring PubAddr::encode(ustring pubKA, ustring privKB , ustring pubKB ,ustring plain)
{
	OID CURVE = secp256k1();
	AutoSeededRandomPool rng;

	ECDH < ECP >::Domain dhA(CURVE), dhB(CURVE);
	SecByteBlock privB(privKB.c_str(), dhA.PrivateKeyLength());
	SecByteBlock pubA(pubKA.c_str(), dhB.PublicKeyLength());

	if (dhA.AgreedValueLength() != dhB.AgreedValueLength())
		throw runtime_error("Shared shared size mismatch");

	SecByteBlock sharedA(dhA.AgreedValueLength()), sharedB(dhB.AgreedValueLength());

	if (!dhA.Agree(sharedA, privB, pubA))
		throw runtime_error("Failed to reach shared secret (A)");


	Integer ssa, ssb;

	ssa.Decode(sharedA.BytePtr(), sharedA.SizeInBytes());

	uint8_t H[CryptoPP::SHA512::DIGESTSIZE];
	CryptoPP::SHA512 hash;

	hash.CalculateDigest(H, sharedA.BytePtr(), sharedA.SizeInBytes());



	AutoSeededRandomPool prng;

	byte key[32];

	memcpy(key, H, sizeof(key));

	byte Hkey[32];

	memcpy(Hkey, &(H[32]), sizeof(Hkey));

	byte iv[16];
	prng.GenerateBlock(iv, sizeof(iv));

	string SPlain = plain.toString(), encoded;

	try
	{
		CBC_Mode< AES >::Encryption e;
		e.SetKeyWithIV(key, sizeof(key), iv);

		// The StreamTransformationFilter removes
		//  padding as required.
		StringSource s(SPlain, true,
			new StreamTransformationFilter(e,
				new StringSink(encoded)
				) // StreamTransformationFilter
			); // StringSource
	}
	catch (const CryptoPP::Exception& e)
	{
		throw runtime_error(e.what());
	}



	ustring result;
	result.append(iv, 16);
	result.appendInt16(714);
	result.appendInt16(32);
	result.append(pubKB.c_str() + 1, 32);
	result.appendInt16(32);
	result.append(pubKB.c_str() + 33, 32);
	result.fromString(encoded);
	

	
	string HMacPlain;

	HMacPlain += result.toString();

	string mac;

	try
	{
		HMAC<SHA256> hmac(Hkey, 32);
		StringSource s(HMacPlain, true,
			new HashFilter(hmac,
				new StringSink(mac)
				)
			);
	}
	catch (const CryptoPP::Exception& e)
	{
		throw runtime_error(e.what());
	}

	result.fromString(mac);

	return result;
}

ustring PubAddr::buildAddressFromKeys(ustring Skey, ustring Ekey, int stream, int version)
{
	CryptoPP::SHA512 hash;
	byte digest[CryptoPP::SHA512::DIGESTSIZE];
	byte digest1[CryptoPP::SHA512::DIGESTSIZE];

	ustring buffer;
	buffer += Skey;
	buffer += Ekey;

	hash.CalculateDigest(digest, (byte*)buffer.c_str(), buffer.length());

	CryptoPP::RIPEMD160 hash2;
	byte digest2[CryptoPP::RIPEMD160::DIGESTSIZE];
	hash2.CalculateDigest(digest2, digest, sizeof digest);
	
	ustring ripe;
	int i = 0;
	while (digest2[i] == 0x00)
		i++;

	while (i < sizeof digest2)
	{
		ripe += digest2[i];
		i++;
	}

	ustring tmp;
	tmp.appendVarInt_B(version);
	tmp.appendVarInt_B(stream);
	tmp += ripe;

	//generate checksum
	hash.CalculateDigest(digest, (byte*)tmp.c_str(), tmp.length());
	hash.CalculateDigest(digest1, digest, sizeof digest);

	tmp += digest1[0];
	tmp += digest1[1];
	tmp += digest1[2];
	tmp += digest1[3];

	//convert to base58
	char * ret = new char[256];
	size_t size;
	if (!b58enc(ret, &size, tmp.c_str(), tmp.size()))
		throw runtime_error("cannot encode base58");

	ustring addr;
	addr.fromString(string("BM-"));

	addr.append((unsigned char*)ret, size);

	return addr;
}

ustring PubAddr::getPubEncryptionKey()
{
	return this->pubEncryptionKey;
}

ustring PubAddr::getPubSigningKey()
{
	return this->pubSigningKey;
}

ustring PubAddr::getPubOfPriv(ustring priv)
{
	OID CURVE = secp256k1();
	ECIES < ECP >::PrivateKey privK;

	Integer x;
	x.Decode(priv.c_str(), priv.size());
	privK.Initialize(CURVE, x);

	ECIES<ECP>::PublicKey pub;
	privK.MakePublicKey(pub);

	string encoded;
	int len = pub.GetPublicElement().x.MinEncodedSize();
	pub.GetPublicElement().x.Encode(StringSink(encoded).Ref(), len);

	len = pub.GetPublicElement().y.MinEncodedSize();
	pub.GetPublicElement().y.Encode(StringSink(encoded).Ref(), len);

	ustring ret;
	ret += 0x04;
	ret.fromString(encoded);

	return ret;
}

int PubAddr::getNonce()
{
	return this->nonce_trials;
}
int PubAddr::getExtra()
{
	return this->extra_bytes;
}

int PubAddr::getVersion()
{
	return this->version;
}

int PubAddr::getStream()
{
	return this->stream;
}

ustring PubAddr::getRipe()
{
	return this->ripe;
}

ustring PubAddr::getAddress()
{
	return this->address;
}

bool PubAddr::waitingPubKey()
{
	return this->empty;
}

ustring PubAddr::getTag()
{
	return this->tag;
}

ustring PubAddr::getTagE()
{
	return this->tagE;
}





Addr::Addr()
{
	this->empty = true;
}


Addr::Addr(const Addr &that)
{
	this->pubSigningKey = that.pubSigningKey;
	this->privSigningKey = that.privSigningKey;

	this->pubEncryptionKey = that.pubEncryptionKey;
	this->privEncryptionKey = that.privEncryptionKey;

	this->empty = that.empty;

	this->extra_bytes = that.extra_bytes;
	this->nonce_trials = that.nonce_trials;

	this->version = that.version;
	this->stream = that.stream;
	this->ripe = that.ripe;

	this->address = that.address;
	this->tag = that.tag;
	this->tagE = that.tagE;
}

bool Addr::generateRandom()
{
	int stream = 1;
	int version = 4;

	OID CURVE = secp256k1();
	AutoSeededRandomPool rng;

	
	ECIES<ECP>::PrivateKey privE, privS;

	ustring pubSKey;
	ustring pubEKey;

	string encoded;
	size_t len;

	byte digest2[CryptoPP::RIPEMD160::DIGESTSIZE];

	int zeros = 0;
	do
	{
		privE.Initialize(rng, CURVE);
		privS.Initialize(rng, CURVE);

		ECIES<ECP>::PublicKey pubE, pubS;
		privE.MakePublicKey(pubE);
		privS.MakePublicKey(pubS);

		encoded.clear();
		len = pubE.GetPublicElement().x.MinEncodedSize();
		pubE.GetPublicElement().x.Encode(StringSink(encoded).Ref(), len);

		len = pubE.GetPublicElement().y.MinEncodedSize();
		pubE.GetPublicElement().y.Encode(StringSink(encoded).Ref(), len);

		pubEKey.clear();
		pubEKey += 0x04;
		pubEKey.fromString(encoded);


		encoded.clear();
		len = pubS.GetPublicElement().x.MinEncodedSize();
		pubS.GetPublicElement().x.Encode(StringSink(encoded).Ref(), len);

		len = pubS.GetPublicElement().y.MinEncodedSize();
		pubS.GetPublicElement().y.Encode(StringSink(encoded).Ref(), len);

		pubSKey.clear();
		pubSKey += 0x04;
		pubSKey.fromString(encoded);


		CryptoPP::SHA512 hash;
		byte digest[CryptoPP::SHA512::DIGESTSIZE];

		ustring buffer;
		buffer += pubSKey;
		buffer += pubEKey;

		hash.CalculateDigest(digest, (byte*)buffer.c_str(), buffer.length());

		CryptoPP::RIPEMD160 hash2;
		memset(digest2, 0x00, 20);
		hash2.CalculateDigest(digest2, digest, sizeof digest);

		
		while (digest2[zeros] == 0x00)
			zeros++;
	} while (zeros == 0);

	encoded.clear();
	len = privE.GetPrivateExponent().MinEncodedSize();
	privE.GetPrivateExponent().Encode(StringSink(encoded).Ref(), len);

	ustring privEKey;
	privEKey.fromString(encoded);

	encoded.clear();
	len = privS.GetPrivateExponent().MinEncodedSize();
	privS.GetPrivateExponent().Encode(StringSink(encoded).Ref(), len);

	ustring privSKey;
	privSKey.fromString(encoded);

	if (!this->loadKeys(pubEKey, pubSKey, privEKey, privSKey, stream, version))
		return false;

	return true;
}

bool Addr::loadKeys(ustring pubE, ustring pubS, ustring privE, ustring privS, int stream, int version)
{
	std::unique_lock<std::mutex> mlock(this->mutex_);
	this->extra_bytes = 1000;
	this->nonce_trials = 1000;

	this->pubEncryptionKey.clear();
	this->pubSigningKey.clear();
	this->privEncryptionKey.clear();
	this->privSigningKey.clear();

	int length = pubE.length();
	if (length == 64)
	{
		this->pubEncryptionKey += 0x04;
		this->pubEncryptionKey += pubE;
	}
	else if (length == 65)
	{
		if (pubE.c_str()[0] != 0x04)
		{
			mlock.unlock();
			return false;
		}
		this->pubEncryptionKey += pubE;
	}

	length = pubS.length();
	if (length == 64)
	{
		this->pubSigningKey += 0x04;
		this->pubSigningKey += pubS;
	}
	else if (length == 65)
	{
		if (pubS.c_str()[0] != 0x04)
		{
			mlock.unlock();
			return false;
		}
		this->pubSigningKey += pubS;
	}

	length = privE.length();
	if (length == 32)
	{
		this->privEncryptionKey += privE;
	}
	else
	{
		mlock.unlock();
		return false;
	}

	length = privS.length();
	if (length == 32)
	{
		this->privSigningKey += privS;
	}
	else
	{
		mlock.unlock();
		return false;
	}

	this->version = version;
	this->stream = stream;

	this->empty = false;

	this->address = this->buildAddressFromKeys(this->pubSigningKey, this->pubEncryptionKey, stream, version);

	if (!this->loadAddr(this->address))
		return false;

	mlock.unlock();
	return true;
}

packet_pubkey Addr::encodePubKey()
{
	packet_pubkey pubkey;
	time_t ltime = std::time(nullptr);
	time_t TTL = 60 * 60;
	ltime = ltime + TTL;

	pubkey.objectType = 1;
	pubkey.Time = ltime;
	pubkey.stream = this->getStream();
	pubkey.version = 4;

	pubkey.encodePayload();

	ustring plain;

	plain.appendInt32(1); //bitfiled 1 not yet integrated
	plain.append(this->pubSigningKey.c_str() + 1, 64);
	plain.append(this->pubEncryptionKey.c_str() + 1, 64);
	plain.appendVarInt_B(this->nonce_trials);
	plain.appendVarInt_B(this->extra_bytes);




	OID CURVE = secp256k1();
	AutoSeededRandomPool prng;

	ECDSA<ECP, SHA1>::PrivateKey privateKey;

	Integer x;
	x.Decode(this->getPrivSigningKey().c_str(), this->getPrivSigningKey().size());
	privateKey.Initialize(CURVE, x);

	ECDSA<ECP, SHA1>::Signer signer(privateKey);

	string signature;
	string mess;
	ustring mess1;
	unsigned int i = 8;
	mess1.appendInt64(pubkey.message_payload.getInt64(i));
	mess1.appendInt32(pubkey.message_payload.getInt32(i));
	mess1.appendVarInt_B(pubkey.message_payload.getVarInt_B(i));
	mess1.appendVarInt_B(pubkey.message_payload.getVarInt_B(i));
	mess1 += this->getTag();

	mess += mess1.toString();
	mess += plain.toString();

	StringSource ss(mess, true /*pump all*/,
		new SignerFilter(prng,
			signer,
			new StringSink(signature)
			) // SignerFilter
		); // StringSource

		   //DER encoding
	Integer r, s;
	StringStore store(signature);
	r.Decode(store, signature.size() / 2);
	s.Decode(store, signature.size() / 2);
	string sign;
	StringSink sink(sign);
	DERSequenceEncoder seq(sink);
	r.DEREncode(seq);
	s.DEREncode(seq);
	seq.MessageEnd();
	//end conversion

	plain.appendVarInt_B(sign.size());
	plain.append((unsigned char*)sign.c_str(), sign.size());
	
	ECIES<ECP>::PrivateKey priv;
	Integer e;
	e.Decode(this->getTagE().c_str(), 32);
	priv.Initialize(CURVE, e);
	ECIES<ECP>::PublicKey pub;
	priv.MakePublicKey(pub);
	const ECP::Point& qq = pub.GetPublicElement();
	string pubS;
	StringSink sinkK(pubS);
	qq.x.Encode(sinkK, 32);
	qq.y.Encode(sinkK, 32);

	ustring pubK;
	pubK += 0x04;
	pubK.fromString(pubS);

	ustring encoded = this->encode(pubK, this->getPrivEncryptionKey(), this->getPubEncryptionKey(), plain);
	pubkey.tag = this->getTag();
	pubkey.encrypted = encoded;

	pubkey.encodeObject();
	return pubkey;
}

ustring Addr::getPrivEncryptionKey()
{
	return this->privEncryptionKey;
}

ustring Addr::getPrivSigningKey()
{
	return this->privSigningKey;
}

ustring Addr::getPubEncryptionKey()
{
	return this->pubEncryptionKey;
}

ustring Addr::getPubSigningKey()
{
	return this->pubSigningKey;
}