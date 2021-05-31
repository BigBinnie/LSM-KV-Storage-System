#include "memtable.h"

//construct the skiplist
SkipList::SkipList(int maxlevel, value init) : maxlevel(maxlevel)
{
    size = 0;
    head = new Node(MinInt, init, maxlevel, 0);
    tail = new Node(MaxInt, init, maxlevel, 0);
    for (int i = 0; i < maxlevel; i++)
        head->next[i] = tail;
}

//destruct the skiplist
SkipList::~SkipList()
{
    delete head;
    delete tail;
}

//get the size of skiplist
int SkipList::getSize()
{
    return size;
}

//insert into skiplist
void SkipList::insert(key k, value v, bool isdelete)
{
    int level = randomLevel();
    Node *newnode = nullptr;
    Node *tmp = head;

    newnode = new Node(k, v, level, isdelete);
    Node *findnode = search(k);
    if (findnode != nullptr)
    {
        size = size - findnode->size + newnode->size;
        findnode = newnode;
        delete newnode;
    }
    else
    {
        for (int i = 0; i < level; i++)
        {
            while (tmp->next[i] && tmp->next[i]->K < k)
                tmp = tmp->next[i];
            newnode->next[i] = tmp->next[i];
            tmp->next[i] = newnode;
            tmp = head;
        }
        size += newnode->size;
    }
}

//search the skiplist
Node *SkipList::search(key k)
{
    Node *tmp = head;
    int current_level = tmp->next.size();

    for (int i = (current_level - 1); i > -1; i--)
    {
        while (tmp->next[i] != nullptr && tmp->next[i]->K < k)
            tmp = tmp->next[i];
        if (tmp->next[i]->K == k)
        {
            tmp = tmp->next[i];
            return tmp;
        }
    }
    return nullptr;
}

//delete the skiplist
bool SkipList::del(key k)
{
    Node *node = search(k);
    if (node != nullptr)
    {
        size -= node->size;
        Node *tmp = head;
        int level = node->next.size();
        for (int i = level - 1; i > -1; i--)
        {
            while (tmp->next[i] != nullptr && tmp->next[i]->K < k)
                tmp = tmp->next[i];
            tmp->next[i] = tmp->next[i]->next[i];
            tmp = head;
        }
        return true;
    }
    return false;
}

//transfer memtable to sstable
void SkipList::tranfersstable(vector<offsetNode> &navi, string &data)
{
    Node *tmp = head->next[0];
    int i = 0;
    int offset = 0;
    while (tmp->K < MaxInt)
    {
        offsetNode node(tmp->K, offset, tmp->isDelete, tmp->time);
        navi.push_back(node);
        i++;
        string skey = to_string(tmp->K);
        while (skey.length() < 8)
            skey = "0" + skey;
        data += skey + tmp->V;
        offset = data.length();
        tmp = tmp->next[0];
    }
    offsetNode endnode(MaxInt, offset, tmp->isDelete, tmp->time);
    navi.push_back(endnode);
}

//randomly generate the level
int SkipList::randomLevel()
{
    int random_level = 1;
    int seed = time(NULL);
    static default_random_engine e(seed);
    static uniform_int_distribution<int> u(0, 1);

    while (u(e) && random_level < maxlevel)
        random_level++;

    return random_level;
}
