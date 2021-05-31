# <center>LSM Tree 键值存储系统<center>

## 1.设计

​		LSM Tree 主要通过将键值对存储系统划分为内存和硬盘两个部分，并利用数据结构和算法知识，对增删查改的方式进行优化，我的实现主要包括以下两个部分。

- `memtable.h` 和`memtable.cc`: 借助跳表实现在内存中的增删查改，主要包括了四个函数

  `void  insert（key k,value v,isdelete)` 	插入，支持在memtable中插入在sstable中删除的值

  `Node*	search(key k)`	查找

  `bool	del(key k)` 删除

  `void	transfersstable(vetor<offset>&navi,string &data)`

  将memtable转换成sstable, 索引和数据分别存在navi和data中

- `sstable.h` 和 `sstable.cc`：实现文件的分层存储，将文件在内存中组织成`vector<list<file>>`的形式

  主要包括三个函数

  `void	addnewfile(vector<offsetNode>&navi,string&data)`

  将从memtable中获得的索引和数据写到文件中

  `value	search(key k)` 查找

  `void compaction（int level)`递归函数实现文件层的溢出文件的合并

  

#### 一、代码结构

`Node`:

- `Node`主要存储在跳表中，记录key、value、插入时间、以及是否是待删除节点
- `offsetNode`主要存储在sstable中，记录key、offset、插入时间以及是否是待删除节点

`vector<offsetNode>` :

- 将索引区组织成vector的形式，方便查找和转换成字符串存储在文件中

`file`：

- 类，将文件的名字，索引组织成类`file`,方便将索引缓存在内存中进行查找

`vector<list<file>> filelist`:

- sstable的主要形式，将文件在内存中组织起来

`memtable`:

- 主要由`node`组成，在内存中缓存小规模数据

`sstable:`

- 由`vector<list<file>> filelist`进行组织，实现数据在硬盘的写入和其索引在内存中的组织



#### 二、性能优化函数

`offsetNode binary_search(vector<offsetNode>&navi,int low,int high,key k)`

- 对sstable文件的索引区二分查找，有效地优化了查找性能

`file* MergeSort(file &arr1,file &arr2)`

- 归并排序：对两个文件进行归并排序，并且将合并的结果写入一个临时文件，存在`file`类型的指针中

`file* merge_unordered_file(vector<file> &f)`

` file* merge_ordered_file(vector<file> &f)`

- 考虑到`level 1`以下的层都是有序的文件，以上两个函数对无序的文件列表和有序的文件列表进行了分别的处理，前者通过调用`MergeSort`函数实现无序文件的归并，后者则直接生成有序文件的归并



#### 三、程序设计亮点

1. `file`以及`vector<list<file>> filelist`对sstable的缓存的索引的结构进行封装，既方便了在sstable中进行查找，也有利于再次利用该结构缓存已有文件的索引区
2. 根据不同的增删查改需要，采用`vector`和`list`存储，利用其自带的函数简化功能的实现
3. 采用经典的查找算法和排序算法，优化查找和合并的性能



## 2.测试

#### 一、测试环境

系统：MacOS Catalina 版本 10.15.4 

内存：8GB 2133 MHz LPDDR3



#### 二、测试

- 时延测试：

  - 方法分别测试计算了仅在memtable中进行插入、查找、删除512组数据，和同时在memtable和sstable中进行插入、查找、删除3200组数据的总时间，以获得单次操作的平均值

  - 结果（单位为秒）

    ![图片1](/Users/yaobinwei/Downloads/图片1.png)

    1、2、3分布代指Put、Get和Delete操作，time1反映了在memtable中操作的时延，time2反映了同时在memtable和sstable进行操作的时延

  - 分析：
    可以看出，当数据需要写入到sstable时，时延明显增加。

    

- 吞吐量测试

  通过插入1024*64组数据测试吞吐量的变化情况

  ![吞吐量](/Users/yaobinwei/Downloads/吞吐量.jpg)

  ​											蓝色曲线反映了吞吐量的变化，红色反映了compaction的次数

  - 分析：

    1. 从图中可以看出5秒内，put的次数急剧下滑，这是因为随着数据量的增多，调用compaction函数的次数从14次提高至24次，由于compaction伴随着硬盘中文件的读写，所以put需要的时间增多。

    2. 之后吞吐量的下滑速度逐渐减缓，这是因为compaction的次数趋于稳定，这个阶段sstable的层数增多，深度越大的单层可以储存的文件数目也随之增多，所以compaction主要集中在对较浅的层进行所以合并的次数趋于稳定。

    3. 可以看到在蓝色曲线的末尾，有周期性上涨和下跌的趋势。这是因为插入数据的规模随着插入次数的增加而增加，到这一阶段时，相邻两次插入直接如果需要合并，其插入速度就会明显减缓

       

## 3.备注

为减小存储空间，默认时间戳为10位，若`clock()`函数的返回值累计大于10位时，需要重新编译运行再进行测试。

