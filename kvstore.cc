#include "kvstore.h"

#define MaxSize 2097152
KVStore::KVStore(const std::string &dir) : KVStoreAPI(dir)
{
	//construct the object based on previous files
	memtable = new SkipList(16, "");
	disk = new sstable();
	//if dir has already existed
	if (exists(dir))
	{
		//travel all levels
		int i = 0;
		string level = dir + "/Level ";
		string level_num = "";

		while (true)
		{
			level_num = level + to_string(i);
			if (exists(level_num))
			{
				if (i != 0)
				{
					list<file> files;
					disk->filelist.push_back(files);
				}
				vector<clock_t> filename;
				directory_iterator l(level_num);

				for (auto &it : l)
				{
					string tmp = it.path().filename();
					clock_t num = atoi(tmp.substr(4).c_str());
					filename.push_back(num);
				}
				sort(filename.begin(), filename.end());

				for (int j = 0; j < filename.size(); j++)
				{
					vector<offsetNode> navi;
					string file_name = level_num + "/Data" + to_string(filename[j]) + ".dat";
					char *read = new char[9];
					read[8] = '\0';
					int off;
					ifstream ifile(file_name, ios::binary | ios::in);
					ifile.seekg(0, ios::end);
					int length = ifile.tellg();
					ifile.seekg(length - 8, ios::beg);
					ifile.read(read, 8);
					off = atoi(read);
					delete[] read;
					ifile.seekg(off, ios::beg);
					char *readoffset = new char[length - off - 7];
					readoffset[length - off - 8] = '\0';
					ifile.read(readoffset, length - off - 8);
					string o = readoffset;
					delete[] readoffset;
					int m = 0;
					while (m < o.length() - 40)
					{
						key k = atoi(o.substr(m, 8).c_str());
						m += 8;
						int offset = atoi(o.substr(m, 8).c_str());
						m += 8;
						bool isdelete = atoi(o.substr(m, 1).c_str());
						m += 1;
						clock_t time = atoi(o.substr(m, 10).c_str());
						m += 10;
						offsetNode newnode(k, offset, isdelete, time);
						navi.push_back(newnode);
					}
					offsetNode endnode(MaxInt, off, 1, clock());
					navi.push_back(endnode);
					file newfile(file_name, navi, off);
					disk->filelist[i].push_back(newfile);
				}

				i++;
			}
			else
				break;
		}
	}
}

KVStore::~KVStore()
{
	delete memtable;
	delete disk;
}

/**
 * Insert/Update the key-value pair.
 * No return values for simplicity.
 */
void KVStore::put(uint64_t key, const std::string &s)
{

	if (memtable->getSize() < MaxSize)
		memtable->insert(key, s, 0);
	else
	{
		vector<offsetNode> navi;
		string data;
		memtable->tranfersstable(navi, data);
		disk->addNewFile(data, navi);
		delete memtable;
		memtable = new SkipList(16, "");
		memtable->insert(key, s, 0);
	}
}
/**
 * Returns the (string) value of the given key.
 * An empty string indicates not found.
 */
std::string KVStore::get(uint64_t key)
{
	Node *tmp = memtable->search(key);
	if (tmp)
	{
		if (tmp->isDelete == 1)
			return "";
		return tmp->V;
	}
	else
	{
		value v = disk->search(key);
		return v;
	}
}
/**
 * Delete the given key-value pair if it exists.
 * Returns false iff the key is not found.
 */
bool KVStore::del(uint64_t key)
{
	value v = get(key);
	if (v != "")
	{
		if (!memtable->del(key))
		{
			if (memtable->getSize() < MaxSize)
				memtable->insert(key, v, 1);
			else
			{
				vector<offsetNode> navi;
				string data;
				memtable->tranfersstable(navi, data);
				disk->addNewFile(data, navi);
				delete memtable;
				memtable = new SkipList(16, "");
				memtable->insert(key, v, 1);
			}
			return true;
		}

		else
			return true;
	}
	return false;
}

/**
 * This resets the kvstore. All key-value pairs should be removed,
 * including memtable and all sstables files.
 */
void KVStore::reset()
{
	if (disk->filelist.size() != 0)
	{
		list<file>::iterator it;
		for (int i = 0; i < disk->filelist.size(); i++)
		{
			if (disk->filelist[i].size() != 0)
			{
				it = disk->filelist[i].begin();
				for (int j = 0; j < disk->filelist[i].size(); j++)
				{
					remove(it->file_name.c_str());
					it++;
				}
			}
			string level_name = "data/Level " + to_string(i);
			remove(level_name.c_str());
		}
	}
	delete memtable;
	delete disk;
	memtable = new SkipList(16, "");
	disk = new sstable();
}
