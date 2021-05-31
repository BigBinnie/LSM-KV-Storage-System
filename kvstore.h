#pragma once
#include<filesystem>
#include "kvstore_api.h"
#include "memtable.h"
#include "sstable.h"

using namespace std::filesystem;
class KVStore : public KVStoreAPI {
	// You can add your implementation here
private:
	SkipList *memtable;
	sstable *disk;
public:
	
	KVStore(const std::string &dir);

	~KVStore();

	void put(uint64_t key, const std::string &s) override;

	std::string get(uint64_t key) override;

	bool del(uint64_t key) override;

	void reset() override;

};
