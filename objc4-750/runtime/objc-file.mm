/*
 * Copyright (c) 1999-2007 Apple Inc.  All Rights Reserved.
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

#if __OBJC2__

#include "objc-private.h"
#include "objc-file.h"

/* Mach-O 类型的文件：是一种用于可执行文件、目标代码、动态库、内核转储的文件格式；
 *
 * Mach-O 文件包含三个主要区域：
 * 1、mach_header : 文件类型, 目标架构
 * 2、Load command: 描述文件在虚拟内存中的逻辑与布局
 * 3、Raw segment date: Load command中定义的原始数据
 *
 *
 * struct mach_header {
 *      uint32_t    magic; //支持设备的CPU位数 :oxFEEDFACE : 表示32位二进制 ;xFEEDFACF : 表示64位二进制
 *      cpu_type_t    cputype; // CPU类型
 *      cpu_subtype_t    cpusubtype; // CPU 子类型
 *      uint32_t    filetype; //Mach-O文件类型
 *      uint32_t    ncmds;  //用于加载器的加载命令的条数
 *      uint32_t    sizeofcmds;//用于加载器的加载命令的大小
 *      uint32_t    flags; // 动态链接器dyld的标志
 * };
 *
 *
 * LC_SEGMENT / LC_SEGMENT_64段的详解（Load command）
 *  常见段：
 *        _PAGEZERO: 空指针陷阱段
 *        _TEXT: 程序代码段
 *        __DATA: 程序数据段
 *        __RODATA: read only程序只读数据段
 *        __LINKEDIT: 链接器使用段
 *   段中区section详解：
 *       __text: 主程序代码
 *       __stubs, __stub_helper: 用于动态链接的桩
 *       __cstring: 程序中c语言字符串
 *       __const: 常量
 *       __RODATA,__objc_methname: OC方法名称
 *       __RODATA,__objc_methntype: OC方法类型
 *       __RODATA,__objc_classname: OC类名
 *       __DATA,__objc_classlist: OC类列表
 *       __DATA,__objc_protollist: OC原型列表
 *       __DATA,__objc_imageinfo: OC镜像信息
 *       __DATA,__objc_const: OC常量
 *       __DATA,__objc_selfrefs: OC类自引用(self)
 *       __DATA,__objc_superrefs: OC类超类引用(super)
 *       __DATA,__objc_protolrefs: OC原型引用
 *       __DATA, __bss: 没有初始化和初始化为0 的全局变量
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 */

/* 获取某个区段数据
 * @param mhp 头信息，文件类型, 目标架构
 * @param segname 段名
 * @param sectname 段中区的名称
 * @param size 函数内部赋值所获取数据的字节数
 * @return uint8_t 返回的数据
uint8_t *getsectiondata(const struct mach_header_64 *mhp,const char *segname,const char *sectname,unsigned long *size);
 */


/* 获取 __DATA区段 或 __DATA_CONST区段 或 __DATA_DIRTY区段 的指定数据
 * @param mhp 头信息，文件类型, 目标架构
 * @param sectname 段中区的名称
 * @param outCount 获取列表中元素的数量
 */
template <typename T> T* getDataSection(const headerType *mhdr, const char *sectname,
                  size_t *outBytes, size_t *outCount){
    unsigned long byteCount = 0;//所获取数据的字节数
    T* data = (T*)getsectiondata(mhdr, "__DATA", sectname, &byteCount);//首先从程序数据段获取数据
    if (!data) {
        data = (T*)getsectiondata(mhdr, "__DATA_CONST", sectname, &byteCount);
    }
    if (!data) {
        data = (T*)getsectiondata(mhdr, "__DATA_DIRTY", sectname, &byteCount);
    }
    if (outBytes) *outBytes = byteCount;
    
    //列表中元素的数量 = 所获取数据的字节数 / sizeof(T)
    if (outCount) *outCount = byteCount / sizeof(T);
    return data;
}

#define GETSECT(name, type, sectname)                                   \
    type *name(const headerType *mhdr, size_t *outCount) {              \
        return getDataSection<type>(mhdr, sectname, nil, outCount);     \
    }                                                                   \
    type *name(const header_info *hi, size_t *outCount) {               \
        return getDataSection<type>(hi->mhdr(), sectname, nil, outCount); \
    }

//          函数名字                 函数返回值类型            section name
GETSECT(_getObjc2SelectorRefs,        SEL,             "__objc_selrefs"); //获取哪些SEL对应的字符串被引用
GETSECT(_getObjc2MessageRefs,         message_ref_t,   "__objc_msgrefs"); //
GETSECT(_getObjc2ClassRefs,           Class,           "__objc_classrefs");//获取被引用的 OC 类
GETSECT(_getObjc2SuperRefs,           Class,           "__objc_superrefs");//获取被引用的 OC 类的父类
GETSECT(_getObjc2ClassList,           classref_t,      "__objc_classlist");//获取所有的Class
GETSECT(_getObjc2NonlazyClassList,    classref_t,      "__objc_nlclslist");//获取非懒加载的所有的类的列表
GETSECT(_getObjc2CategoryList,        category_t *,    "__objc_catlist");//获取所有的 category
GETSECT(_getObjc2NonlazyCategoryList, category_t *,    "__objc_nlcatlist");//获取非懒加载的所有的分类的列表
GETSECT(_getObjc2ProtocolList,        protocol_t *,    "__objc_protolist");//获取所有的 Protocol
GETSECT(_getObjc2ProtocolRefs,        protocol_t *,    "__objc_protorefs");//OC 协议引用
GETSECT(getLibobjcInitializers,       UnsignedInitializer, "__objc_init_func");


objc_image_info *
_getObjcImageInfo(const headerType *mhdr, size_t *outBytes)
{
    //__objc_imageinfo OC镜像信息
    return getDataSection<objc_image_info>(mhdr, "__objc_imageinfo", 
                                           outBytes, nil);
}

// Look for an __objc* section other than __objc_imageinfo
static bool segmentHasObjcContents(const segmentType *seg)
{
    for (uint32_t i = 0; i < seg->nsects; i++) {
        const sectionType *sect = ((const sectionType *)(seg+1))+i;
        if (sectnameStartsWith(sect->sectname, "__objc_")  &&
            !sectnameEquals(sect->sectname, "__objc_imageinfo"))
        {
            return true;
        }
    }

    return false;
}

// Look for an __objc* section other than __objc_imageinfo
bool
_hasObjcContents(const header_info *hi)
{
    bool foundObjC = false;

    foreach_data_segment(hi->mhdr(), [&](const segmentType *seg, intptr_t slide)
    {
        if (segmentHasObjcContents(seg)) foundObjC = true;
    });

    return foundObjC;
    
}


// OBJC2
#endif
