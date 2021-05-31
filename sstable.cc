#include "sstable.h"

sstable::sstable()
{
    string folder = "data/Level 0";
    list<file> level0;
    filesystem::create_directories(folder);
    size.push_back(0);
    filelist.push_back(level0);
}

sstable::~sstable()
{
}

//transfer navi to string in order to store in disk
string sstable::transfer_nevi(vector<offsetNode> &navi)
{
    string n = "";
    for (int i = 0; i < navi.size(); i++)
    {
        string skey = to_string(navi[i].K);
        while (skey.length() < 8)
            skey = "0" + skey;
        string soffset = to_string(navi[i].O);
        while (soffset.length() < 8)
            soffset = "0" + soffset;
        string stime = to_string(navi[i].time);
        while (stime.length() < 10)
            stime = "0" + stime;
        n += skey + soffset + to_string(navi[i].isdelete) + stime;
    }
    return n;
}

//add new file in level 0
void sstable::addNewFile(string &data, vector<offsetNode> &navi)
{
    string n = "";
    int num = ++size[0];
    n = transfer_nevi(navi);
    file newfile(0, num, navi, data, n);
    filelist[0].push_back(newfile);
    if (filelist[0].size() > 2)
        compaction(0);
}

//Binary search
offsetNode *sstable::binary_search(vector<offsetNode> &navi, int low, int high, key k)
{
    int middle = (low + high) / 2;
    if (low > high)
        return nullptr;
    if (navi[middle].K == k)
        return &navi[middle];
    else if (navi[middle].K > k)
        return binary_search(navi, low, middle - 1, k);
    else
        return binary_search(navi, middle + 1, high, k);
}

//search
value sstable::search(key k)
{
    offsetNode *n = nullptr;
    int i = 0;
    int j = 0;
    list<file>::iterator it;
    if (filelist.size() > 0)
    {
        for (; i < filelist.size(); i++)
        {
            if (filelist[i].size() > 0)
            {
                it = filelist[i].begin();
                j = 0;
                while (j < filelist[i].size())
                {
                    if (i == 0)
                        n = binary_search(it->navi, 0, it->navi.size() - 1, k);
                    else
                    {
                        if (k >= it->navi[0].K && k <= it->Max)
                        {
                            n = binary_search(it->navi, 0, it->navi.size() - 1, k);
                        }
                    }
                    if (n)
                    {
                        break;
                    }
                    else
                    {
                        it++;
                        j++;
                    }
                }
            }
            if (n)
                break;
        }
        if (n && !n->isdelete)
        {
            char *read = new char[9];
            read[8] = '\0';
            value v;
            key readk;
            ifstream ifile(it->file_name, ios::binary | ios::in);
            ifile.seekg(n->O, ios::beg);
            ifile.read(read, 8);
            readk = atoi(read);
            delete[] read;
            ifile.seekg(n->O + 8, ios::beg);
            if (readk == k)
            {
                char *value;
                int len = (n + 1)->O - n->O - 8;

                if (len < 0 || (n + 1)->K == MaxInt)
                    len = it->offset - n->O - 8;
                value = new char[len + 1];
                value[len] = '\0';
                ifile.read(value, len);
                v = value;
                delete[] value;
            }
            ifile.close();
            return v;
        }
    }
    return "";
}

//Merge sort
file *sstable::MergeSort(file &f1, file &f2)
{
    int i = 0, j = 0;
    int len = 0;
    int offset = 0;
    bool f1cover = 0;
    bool f2cover = 0;
    string s1 = "";
    string s2 = "";

    char *readf1 = new char[f1.offset + 1];
    readf1[f1.offset] = '\0';
    ifstream ifile1(f1.file_name, ios::binary | ios::in);
    ifile1.seekg(0, ios::beg);
    ifile1.read(readf1, f1.offset);
    s1 = readf1;
    delete[] readf1;
    ifile1.close();

    char *readf2 = new char[f2.offset + 1];
    readf2[f2.offset] = '\0';
    ifstream ifile(f2.file_name, ios::binary | ios::in);
    ifile.seekg(0, ios::beg);
    ifile.read(readf2, f2.offset);
    s2 = readf2;
    delete[] readf2;
    ifile.close();

    string bigdata = "";
    vector<offsetNode> temp;

    while (i < f1.navi.size() - 1 && j < f2.navi.size() - 1)
    {
        if (f1.navi[i].K == f2.navi[j].K)
        {
            if ((f1.navi[i].time > f2.navi[j].time && f1.navi[i].isdelete) || (f1.navi[i].time < f2.navi[j].time && f2.navi[j].isdelete))
            {
                i++;
                j++;
            }
            else
            {
                if (f1.navi[i].time > f2.navi[j].time)
                {
                    f1cover = 1;
                    j++;
                }
                else
                {
                    f2cover = 1;
                    i++;
                }
            }
        }

        if (f1.navi[i].K < f2.navi[j].K || (f1cover))
        {
            if (f1.navi[i].K != MaxInt)
                len = f1.navi[i + 1].O - f1.navi[i].O;
            else
                len = f1.offset - f1.navi[i].O;
            bigdata += s1.substr(f1.navi[i].O, len);

            f1.navi[i].O = offset;
            temp.push_back(f1.navi[i++]);
            offset = bigdata.length();
            f1cover = 0;
        }

        else if (f1.navi[i].K > f2.navi[j].K || (f2cover))
        {
            if (f2.navi[j].K != MaxInt)
                len = f2.navi[j + 1].O - f2.navi[j].O;
            else
                len = f2.offset - f2.navi[j].O;
            bigdata += s2.substr(f2.navi[j].O, len);
            f2.navi[j].O = offset;
            temp.push_back(f2.navi[j++]);
            offset = bigdata.length();
            f2cover = 0;
        }
    }
    if (i < f1.navi.size() - 1)
        bigdata += s1.substr(f1.navi[i].O, f1.offset - f1.navi[i].O);
    while (i < f1.navi.size() - 1)
    {
        if (f1.navi[i].K != MaxInt)
            len = f1.navi[i + 1].O - f1.navi[i].O;
        else
            len = f1.offset - f1.navi[i].O;
        f1.navi[i].O = offset;
        temp.push_back(f1.navi[i++]);
        offset += len;
    }

    if (j < f2.navi.size() - 1)
        bigdata += s2.substr(f2.navi[j].O, f2.offset - f2.navi[j].O);
    while (j < f2.navi.size() - 1)
    {
        if (f2.navi[j].K != MaxInt)
            len = f2.navi[j + 1].O - f2.navi[j].O;
        else
            len = f2.offset - f2.navi[j].O;
        f2.navi[j].O = offset;
        temp.push_back(f2.navi[j++]);
        offset += len;
    }
    offsetNode endnode(MaxInt, offset, 1, clock());
    temp.push_back(endnode);
    clock_t tmp = clock();
    remove(f1.file_name.c_str());
    remove(f2.file_name.c_str());
    file *tempfile = new file(to_string(tmp), temp, bigdata);
    return tempfile;
}

//Merge unorder files
file *sstable::merge_unordered_file(vector<file> &f)
{
    file *tmp1;
    if (f.size() != 1)
    {
        tmp1 = MergeSort(f[0], f[1]);
        for (int i = 2; i < f.size(); i++)
            tmp1 = MergeSort(*tmp1, f[i]);
    }
    else
        tmp1 = &f[0];
    return tmp1;
}

//Merge ordered files
file *sstable::merge_ordered_file(vector<file> &f)
{

    string bigdata;
    vector<offsetNode> temp;
    int offset = 0;
    for (int i = 0; i < f.size(); i++)
    {
        string s;
        char *read = new char[f[i].offset + 1];
        read[f[i].offset] = '\0';
        ifstream ifile(f[i].file_name, ios::binary | ios::in);
        ifile.seekg(0, ios::beg);
        ifile.read(read, f[i].offset);
        s = read;
        delete[] read;
        ifile.close();

        bigdata += s;
        for (int j = 0; j < f[i].navi.size() - 1; j++)
        {
            f[i].navi[j].O += offset;
            temp.push_back(f[i].navi[j]);
        }
        offset = bigdata.size();
        remove(f[i].file_name.c_str());
    }
    offsetNode endnode(MaxInt, offset, 1, clock());
    temp.push_back(endnode);

    clock_t tmp = clock();
    file *tempfile = new file(to_string(tmp), temp, bigdata);
    return tempfile;
}

//divide a file in certain level
void sstable::divide(int level, file *tmp)
{
    int tot_size = 0;
    int size = 0;
    int add = 0;
    string data = "";
    int blank = 0;
    vector<offsetNode> navi;
    list<file>::iterator it;
    for (int i = 0; i < tmp->navi.size() - 1; i++)
    {
        size = tmp->navi[i].O - blank + add;
        tot_size = tmp->navi[i].O;
        if (i == tmp->navi.size() - 2)
        {
            add += 27;
            size = tmp->offset - blank + add;
            tmp->navi[i].O = tmp->navi[i].O - blank;
            navi.push_back(tmp->navi[i]);
        }
        if (size > MaxSize || i == tmp->navi.size() - 2)
        {
            char *read = new char[size + 1 - add];
            read[size - add] = '\0';
            ifstream ifile(tmp->file_name, ios::binary | ios::in);
            ifile.seekg(blank, ios::beg);
            ifile.read(read, size - add);
            data = read;
            delete[] read;
            ifile.close();
            offsetNode endnode(MaxInt, size, 1, clock());
            navi.push_back(endnode);
            string n = transfer_nevi(navi);
            file newfile(level, 0, navi, data, n);
            if (filelist[level].size() != 0)
            {
                if (newfile.Min > filelist[level].back().Max)
                {
                    filelist[level].push_back(newfile);
                }
                if (newfile.Max < filelist[level].front().Min)
                {
                    filelist[level].push_front(newfile);
                }
                else
                {
                    it = filelist[level].begin();
                    for (int j = 0; j < filelist[level].size() - 1; j++)
                    {
                        if (newfile.Min > it->Max && newfile.Max < (++it)->Min)
                        {
                            filelist[level].insert(it, newfile);
                            break;
                        }
                    }
                }
            }
            else
                filelist[level].push_back(newfile);

            size = 0;
            add = 0;
            blank = tot_size;
            navi.clear();
        }

        add += 27;
        tmp->navi[i].O = tmp->navi[i].O - blank;
        navi.push_back(tmp->navi[i]);
    }
}

//compaction
void sstable::compaction(int level)
{
    vector<file> f;
    times++;
    if (level == 0)
    {
        int size = filelist[0].size();
        for (int i = 0; i < size; i++)
        {
            f.push_back(filelist[0].front());
            filelist[0].pop_front();
        }
        file *temp;
        temp = merge_unordered_file(f);

        //divide temp in empty level1
        if (filelist.size() == 1)
        {
            string folder = "data/Level 1";
            list<file> *level1 = new list<file>;
            filesystem::create_directories(folder);
            filelist.push_back(*level1);
            divide(1, temp);
            remove(temp->file_name.c_str());

            delete temp;
        }

        //compaction with files already in level1
        else
        {
            list<file>::iterator it = filelist[1].begin();
            f.clear();
            int size = filelist[1].size();

            for (int i = 0; i < size; i++)
            {
                if ((temp->Min >= it->Min && temp->Min <= it->Max) || (temp->Min <= it->Min && temp->Max >= it->Max) || (temp->Max >= it->Min && temp->Max <= it->Max))
                {
                    f.push_back(*it);
                    filelist[1].erase(it++);
                }
                else
                    it++;
            }
            if (f.size() == 0)
            {
                divide(1, temp);
                remove(temp->file_name.c_str());
                delete temp;
            }
            else
            {
                file *temp2;
                temp2 = merge_ordered_file(f);
                vector<file> f2;
                f2.push_back(*temp);
                f2.push_back(*temp2);
                temp = merge_unordered_file(f2);
                divide(1, temp);
                remove(temp2->file_name.c_str());
                delete temp2;
                remove(temp->file_name.c_str());
                delete temp;
            }
            if (filelist[1].size() > 4)
                compaction(1);
        }
    }
    //Other levels
    else
    {
        list<file>::iterator it;
        it = filelist[level].begin();
        int i = 0;
        while (i < (level + 1) * 2)
        {
            it++;
            i++;
        }
        int size = filelist[level].size();
        for (; i < size; i++)
        {
            f.push_back(*it);
            filelist[level].erase(it++);
        }
        file *temp;
        temp = merge_ordered_file(f);

        if (filelist.size() == level + 1)
        {
            string folder = "data/Level " + to_string(level + 1);
            list<file> l;
            filesystem::create_directories(folder);
            filelist.push_back(l);
            divide(level + 1, temp);
            remove(temp->file_name.c_str());
            delete temp;
        }
        else
        {
            f.clear();
            list<file>::iterator it = filelist[level + 1].begin();
            int size = filelist[level + 1].size();
            for (int i = 0; i < size; i++)
            {
                if ((temp->Min >= it->Min && temp->Min <= it->Max) || (temp->Min <= it->Min && temp->Max >= it->Max) || (temp->Max >= it->Min && temp->Max <= it->Max))
                {
                    f.push_back(*it);
                    filelist[level + 1].erase(it++);
                }
                else
                    it++;
            }
            if (f.size() == 0)
            {
                divide(level + 1, temp);
                remove(temp->file_name.c_str());
                delete temp;
            }
            else
            {
                file *temp2;
                temp2 = merge_ordered_file(f);
                f.clear();
                f.push_back(*temp);
                f.push_back(*temp2);
                temp = merge_unordered_file(f);
                divide(level + 1, temp);
                remove(temp2->file_name.c_str());
                delete temp2;
                remove(temp->file_name.c_str());
                delete temp;
            }
            if (filelist[level + 1].size() > (level + 2) * 2)

                compaction(level + 1);
        }
    }
}
