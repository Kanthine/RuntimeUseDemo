/*	maptable.h 可扩展映射哈希表。
	Bertrand, August 1990
	Copyright 1990-1996 NeXT Software, Inc.
*/

#ifndef _OBJC_MAPTABLE_H_
#define _OBJC_MAPTABLE_H_

#ifndef _OBJC_PRIVATE_H_
#   define OBJC_MAP_AVAILABILITY __OSX_AVAILABLE_BUT_DEPRECATED(__MAC_10_0,__MAC_10_1, __IPHONE_NA,__IPHONE_NA);
#else
#   define OBJC_MAP_AVAILABILITY
#endif

#include <objc/objc.h>

__BEGIN_DECLS

#pragma mark - 结构定义

/* 映射哈希表 NXMapTable
 * 存储任意的键值对：键和值必须是指针或整数，客户端负责分配/释放这些数据。提供了一个deallocation回调。
 * 作为表现良好的可扩展数据结构，哈希表在开始变满时会增加一倍，从而保证了平均固定时间访问和线性大小。
 * 该映射表是可变的；
 * 可以通过弱引用来持有keys和values，所以当key或者value被deallocated的时候，所存储的实体也会被移除；
 * 该映射表可以在添加value的时候对value进行复制；
 * 和 NSHashTable 类似，NSMapTable可以随意的存储指针，并且利用指针的唯一性来进行对比和重复检查。
 */
typedef struct _NXMapTable {
    const struct _NXMapTablePrototype	*prototype;
    unsigned	count;//存储的数据的数量
    unsigned	nbBucketsMinusOne;
    void	*buckets;//真正用来存储数据的数组。
} NXMapTable OBJC_MAP_AVAILABILITY;

//_NXMapTablePrototype 存储了一些构建哈希表必要的函数指针如：hash、isEqual 和 free 的函数指针
typedef struct _NXMapTablePrototype {
    unsigned	(*hash)(NXMapTable *, const void *key);//用于获取数据的哈希的函数地址
    int		(*isEqual)(NXMapTable *, const void *key1, const void *key2);//判断两个数据是否相等的函数地址
    void	(*free)(NXMapTable *, void *key, void *value);//释放数据的函数地址；
    int		style; //预留作日后扩展之用;目前为 0；
} NXMapTablePrototype OBJC_MAP_AVAILABILITY;

    /* invariants assumed by the implementation: 
	A - key != -1
	B - key1 == key2 => hash(key1) == hash(key2)
     当键随时间变化时，哈希(键)必须保持不变；例如:如果是字符串键，就不能改变字符串
	C - isEqual(key1, key2) => key1 == key2
    */

//NX_MAPNOTAKEY(-1)在内部用作标记，因此键必须始终与-1不同。
#define NX_MAPNOTAKEY	((void *)(-1))

#pragma mark - 功能函数

//哈希表 NXMapTable 使用 NXCreateMapTableFromZone() 函数初始化：
OBJC_EXPORT NXMapTable *NXCreateMapTableFromZone(NXMapTablePrototype prototype, unsigned capacity, void *z) OBJC_MAP_AVAILABILITY;
OBJC_EXPORT NXMapTable *NXCreateMapTable(NXMapTablePrototype prototype, unsigned capacity) OBJC_MAP_AVAILABILITY;
    /* capacity is only a hint; 0 creates a small table */

//释放哈希表 NXMapTable
OBJC_EXPORT void NXFreeMapTable(NXMapTable *table) OBJC_MAP_AVAILABILITY;
    /* call free for each pair, and recovers table */
	
OBJC_EXPORT void NXResetMapTable(NXMapTable *table) OBJC_MAP_AVAILABILITY;
    /* free each pair; keep current capacity */

OBJC_EXPORT BOOL NXCompareMapTables(NXMapTable *table1, NXMapTable *table2) OBJC_MAP_AVAILABILITY;
    /* Returns YES if the two sets are equal (each member of table1 in table2, and table have same size) */

OBJC_EXPORT unsigned NXCountMapTable(NXMapTable *table) OBJC_MAP_AVAILABILITY;
    /* current number of data in table */
	
OBJC_EXPORT void *NXMapMember(NXMapTable *table, const void *key, void **value) OBJC_MAP_AVAILABILITY;
    /* return original table key or NX_MAPNOTAKEY.  If key is found, value is set */
	
OBJC_EXPORT void *NXMapGet(NXMapTable *table, const void *key) OBJC_MAP_AVAILABILITY;
    /* return original corresponding value or NULL.  When NULL need be stored as value, NXMapMember can be used to test for presence */
	
OBJC_EXPORT void *NXMapInsert(NXMapTable *table, const void *key, const void *value) OBJC_MAP_AVAILABILITY;
    /* override preexisting pair; Return previous value or NULL. */
	
OBJC_EXPORT void *NXMapRemove(NXMapTable *table, const void *key) OBJC_MAP_AVAILABILITY;
    /* previous value or NULL is returned */
	
/* Iteration over all elements of a table consists in setting up an iteration state and then to progress until all entries have been visited.  An example of use for counting elements in a table is:
    unsigned	count = 0;
    const MyKey	*key;
    const MyValue	*value;
    NXMapState	state = NXInitMapState(table);
    while(NXNextMapState(table, &state, &key, &value)) {
	count++;
    }
*/

typedef struct {int index;} NXMapState OBJC_MAP_AVAILABILITY;
    /* callers should not rely on actual contents of the struct */

OBJC_EXPORT NXMapState NXInitMapState(NXMapTable *table) OBJC_MAP_AVAILABILITY;

OBJC_EXPORT int NXNextMapState(NXMapTable *table, NXMapState *state, const void **key, const void **value) OBJC_MAP_AVAILABILITY;
    /* returns 0 when all elements have been visited */

/***************	Conveniences		***************/

OBJC_EXPORT const NXMapTablePrototype NXPtrValueMapPrototype OBJC_MAP_AVAILABILITY;
    /* hashing is pointer/integer hashing;
      isEqual is identity;
      free is no-op. */
OBJC_EXPORT const NXMapTablePrototype NXStrValueMapPrototype OBJC_MAP_AVAILABILITY;
    /* hashing is string hashing;
      isEqual is strcmp;
      free is no-op. */
OBJC_EXPORT const NXMapTablePrototype NXObjectMapPrototype  OBJC2_UNAVAILABLE;
    /* for objects; uses methods: hash, isEqual:, free, all for key. */

__END_DECLS

#endif /* _OBJC_MAPTABLE_H_ */
