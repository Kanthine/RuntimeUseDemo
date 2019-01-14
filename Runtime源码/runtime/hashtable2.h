/*
 * Copyright (c) 1999-2006 Apple Inc.  All Rights Reserved.
 * 
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
/*
    hashtable2.h
    Scalable hash table.
    Copyright 1989-1996 NeXT Software, Inc.
*/

#ifndef _OBJC_LITTLE_HASHTABLE_H_
#define _OBJC_LITTLE_HASHTABLE_H_

#ifndef _OBJC_PRIVATE_H_
#   define OBJC_HASH_AVAILABILITY __OSX_AVAILABLE_BUT_DEPRECATED(__MAC_10_0,__MAC_10_1, __IPHONE_NA,__IPHONE_NA);
#else
#   define OBJC_HASH_AVAILABILITY
#endif

#include <objc/objc.h>
#include <stdint.h>
#include <TargetConditionals.h>

__BEGIN_DECLS

/*************************************************************************
 *  Objective-C 哈希表，也就是 NXHashTable
 *************************************************************************/

/*
 这个模块允许存储任意数据。这些数据必须是指针或整数，客户端负责分配/释放这些数据。提供了一个deallocation回调。
 在处理(键、值)关联时，Objective-C 类 HashTable 是首选的，因为它在这种情况下更容易使用。
 作为表现良好的可扩展数据结构，哈希表在开始变满时会增加一倍，从而保证平均恒定时间访问和线性大小。
 
 NSHashTable 是可变的，它没有不可变版本。
 它可以持有元素的弱引用，而且在对象被销毁后能正确地将其移除。而这一点在NSSet是做不到的。
 它的成员可以在添加时被拷贝。
 它的成员可以使用指针来标识是否相等及做hash检测。
 它可以包含任意指针，其成员没有限制为对象。我们可以配置一个NSHashTable实例来操作任意的指针，而不仅仅是对象。
 
 
 NXHashTable 的应用：在整个 objc/runtime 中，作为私有的数据结构 NXHashTable，直接使用了它的就是存储所有类或者元类的哈希表（在这里会忽略对元类的存储，因为实现几乎完全相同）：
 */


//NXHashTablePrototype 存储了一些构建哈希表必要的函数指针如：hash、isEqual 和 free 的函数指针
typedef struct {
    uintptr_t	(*hash)(const void *info, const void *data);//用于获取数据的哈希的函数地址
    int		(*isEqual)(const void *info, const void *data1, const void *data2);//判断两个数据是否相等的函数地址
    void	(*free)(const void *info, void *data);//释放数据的函数地址
    int		style; /* reserved for future expansion; currently 0 */
    } NXHashTablePrototype;
    
/* the info argument allows a certain generality, such as freeing according to some owner information */
/* invariants assumed by the implementation: 
	1 - data1 = data2 => hash(data1) = hash(data2)
    当数据变化随着时间的推移,散列(数据)必须保持不变；例如,如果数据散列在一个字符串键,字符串不能被改变
    2- isEqual (data1, data2) => data1= data2
 */

typedef struct {
    const NXHashTablePrototype	*prototype OBJC_HASH_AVAILABILITY;
    unsigned			count OBJC_HASH_AVAILABILITY;
    unsigned			nbBuckets OBJC_HASH_AVAILABILITY;
    void			*buckets OBJC_HASH_AVAILABILITY;//真正用来存储数据的数组。
    const void			*info OBJC_HASH_AVAILABILITY;
   } NXHashTable OBJC_HASH_AVAILABILITY;
    /* private data structure; may change */

//哈希表 NXHashTable 使用 NXCreateHashTableFromZone() 函数初始化：
OBJC_EXPORT NXHashTable *NXCreateHashTableFromZone (NXHashTablePrototype prototype, unsigned capacity, const void *info, void *z) OBJC_HASH_AVAILABILITY;
OBJC_EXPORT NXHashTable *NXCreateHashTable (NXHashTablePrototype prototype, unsigned capacity, const void *info) OBJC_HASH_AVAILABILITY;
    /* if hash is 0, pointer hash is assumed */
    /* if isEqual is 0, pointer equality is assumed */
    /* if free is 0, elements are not freed */
    /* capacity is only a hint; 0 creates a small table */
    /* info allows call backs to be very general */

OBJC_EXPORT void NXFreeHashTable (NXHashTable *table) OBJC_HASH_AVAILABILITY;
    /* calls free for each data, and recovers table */
	
OBJC_EXPORT void NXEmptyHashTable (NXHashTable *table) OBJC_HASH_AVAILABILITY;
    /* does not deallocate table nor data; keeps current capacity */

OBJC_EXPORT void NXResetHashTable (NXHashTable *table) OBJC_HASH_AVAILABILITY;
    /* frees each entry; keeps current capacity */

OBJC_EXPORT BOOL NXCompareHashTables (NXHashTable *table1, NXHashTable *table2) OBJC_HASH_AVAILABILITY;
    /* Returns YES if the two sets are equal (each member of table1 in table2, and table have same size) */

OBJC_EXPORT NXHashTable *NXCopyHashTable (NXHashTable *table) OBJC_HASH_AVAILABILITY;
    /* makes a fresh table, copying data pointers, not data itself.  */
	
OBJC_EXPORT unsigned NXCountHashTable (NXHashTable *table) OBJC_HASH_AVAILABILITY;
    /* current number of data in table */
	
OBJC_EXPORT int NXHashMember (NXHashTable *table, const void *data) OBJC_HASH_AVAILABILITY;
    /* returns non-0 iff data is present in table.
    Example of use when the hashed data is a struct containing the key,
    and when the callee only has a key:
	MyStruct	pseudo;
	pseudo.key = myKey;
	return NXHashMember (myTable, &pseudo)
    */
	
OBJC_EXPORT void *NXHashGet (NXHashTable *table, const void *data) OBJC_HASH_AVAILABILITY;
    /* return original table data or NULL.
    Example of use when the hashed data is a struct containing the key,
    and when the callee only has a key:
	MyStruct	pseudo;
	MyStruct	*original;
	pseudo.key = myKey;
	original = NXHashGet (myTable, &pseudo)
    */
	
OBJC_EXPORT void *NXHashInsert (NXHashTable *table, const void *data) OBJC_HASH_AVAILABILITY;
    /* previous data or NULL is returned. */
	
OBJC_EXPORT void *NXHashInsertIfAbsent (NXHashTable *table, const void *data) OBJC_HASH_AVAILABILITY;
    /* If data already in table, returns the one in table
    else adds argument to table and returns argument. */

//移除哈希表中的某个数据
OBJC_EXPORT void *NXHashRemove (NXHashTable *table, const void *data) OBJC_HASH_AVAILABILITY;
    /* previous data or NULL is returned */
	
/* Iteration over all elements of a table consists in setting up an iteration state and then to progress until all entries have been visited.  An example of use for counting elements in a table is:
    unsigned	count = 0;
    MyData	*data;
    NXHashState	state = NXInitHashState(table);
    while (NXNextHashState(table, &state, &data)) {
	count++;
    }
*/

//在将元素重新插入到哈希表中涉及了一个非常奇怪的结构体 NXHashState，这个结构体主要作用是遍历 NXHashTable 中的元素。
typedef struct {int i; int j;} NXHashState OBJC_HASH_AVAILABILITY;
    /* 调用者不应该依赖于结构的实际内容 */

OBJC_EXPORT NXHashState NXInitHashState(NXHashTable *table) OBJC_HASH_AVAILABILITY;

OBJC_EXPORT int NXNextHashState(NXHashTable *table, NXHashState *state, void **data) OBJC_HASH_AVAILABILITY;
    /* returns 0 when all elements have been visited */

/*************************************************************************
 *	Conveniences for writing hash, isEqual and free functions
 *	and common prototypes
 *************************************************************************/

OBJC_EXPORT uintptr_t NXPtrHash(const void *info, const void *data) OBJC_HASH_AVAILABILITY;
    /* scrambles the address bits; info unused */
OBJC_EXPORT uintptr_t NXStrHash(const void *info, const void *data) OBJC_HASH_AVAILABILITY;
    /* string hashing; info unused */
OBJC_EXPORT int NXPtrIsEqual(const void *info, const void *data1, const void *data2) OBJC_HASH_AVAILABILITY;
    /* pointer comparison; info unused */
OBJC_EXPORT int NXStrIsEqual(const void *info, const void *data1, const void *data2) OBJC_HASH_AVAILABILITY;
    /* string comparison; NULL ok; info unused */
OBJC_EXPORT void NXNoEffectFree(const void *info, void *data) OBJC_HASH_AVAILABILITY;
    /* no effect; info unused */
OBJC_EXPORT void NXReallyFree(const void *info, void *data) OBJC_HASH_AVAILABILITY;
    /* frees it; info unused */

/* The two following prototypes are useful for manipulating set of pointers or set of strings; For them free is defined as NXNoEffectFree */
OBJC_EXPORT const NXHashTablePrototype NXPtrPrototype OBJC_HASH_AVAILABILITY;
    /* prototype when data is a pointer (void *) */
OBJC_EXPORT const NXHashTablePrototype NXStrPrototype OBJC_HASH_AVAILABILITY;
    /* prototype when data is a string (char *) */

/* following prototypes help describe mappings where the key is the first element of a struct and is either a pointer or a string.
For example NXStrStructKeyPrototype can be used to hash pointers to Example, where Example is:
	typedef struct {
	    char	*key;
	    int		data1;
	    ...
	    } Example
    
For the following prototypes, free is defined as NXReallyFree.
 */
OBJC_EXPORT const NXHashTablePrototype NXPtrStructKeyPrototype OBJC_HASH_AVAILABILITY;
OBJC_EXPORT const NXHashTablePrototype NXStrStructKeyPrototype OBJC_HASH_AVAILABILITY;


#if !__OBJC2__  &&  !TARGET_OS_WIN32

/*************************************************************************
 *	Unique strings and buffers
 *************************************************************************/

/* Unique strings allows C users to enjoy the benefits of Lisp's atoms:
A unique string is a string that is allocated once for all (never de-allocated) and that has only one representant (thus allowing comparison with == instead of strcmp).  A unique string should never be modified (and in fact some memory protection is done to ensure that).  In order to more explicitly insist on the fact that the string has been uniqued, a synonym of (const char *) has been added, NXAtom. */

typedef const char *NXAtom OBJC_HASH_AVAILABILITY;

OBJC_EXPORT NXAtom NXUniqueString(const char *buffer) OBJC_HASH_AVAILABILITY;
    /* assumes that buffer is \0 terminated, and returns
     a previously created string or a new string that is a copy of buffer.
    If NULL is passed returns NULL.
    Returned string should never be modified.  To ensure this invariant,
    allocations are made in a special read only zone. */
	
OBJC_EXPORT NXAtom NXUniqueStringWithLength(const char *buffer, int length) OBJC_HASH_AVAILABILITY;
    /* assumes that buffer is a non NULL buffer of at least 
    length characters.  Returns a previously created string or 
    a new string that is a copy of buffer. 
    If buffer contains \0, string will be truncated.
    As for NXUniqueString, returned string should never be modified.  */
	
OBJC_EXPORT NXAtom NXUniqueStringNoCopy(const char *string) OBJC_HASH_AVAILABILITY;
    /* If there is already a unique string equal to string, returns the original.  
    Otherwise, string is entered in the table, without making a copy.  Argument should then never be modified.  */
	
OBJC_EXPORT char *NXCopyStringBuffer(const char *buffer) OBJC_HASH_AVAILABILITY;
    /* given a buffer, allocates a new string copy of buffer.  
    Buffer should be \0 terminated; returned string is \0 terminated. */

OBJC_EXPORT char *NXCopyStringBufferFromZone(const char *buffer, void *z) OBJC_HASH_AVAILABILITY;
    /* given a buffer, allocates a new string copy of buffer.  
    Buffer should be \0 terminated; returned string is \0 terminated. */

#endif

__END_DECLS

#endif /* _OBJC_LITTLE_HASHTABLE_H_ */
