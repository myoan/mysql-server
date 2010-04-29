/* Copyright (C) 2003 MySQL AB

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */

#ifndef NDB_QUERY_TREE_HPP
#define NDB_QUERY_TREE_HPP

#include <ndb_global.h>
#include <ndb_types.h>

struct QueryNode  // Effectively used as a base class for QN_xxxNode
{
  Uint32 len;
  Uint32 requestInfo;
  Uint32 tableId;      // 16-bit
  Uint32 tableVersion;

  enum OpType
  {
    QN_LOOKUP    = 0x1,
    QN_SCAN_FRAG = 0x2,
    QN_END = 0
  };

  static Uint32 getOpType(Uint32 op_len) { return op_len & 0xFFFF;}
  static Uint32 getLength(Uint32 op_len) { return op_len >> 16;}

  static void setOpLen(Uint32 &d, Uint32 o, Uint32 l) { d = (l << 16) | o;}

  // If possible we should change the above static methods to non-static:
//Uint32 getOpType() const { return len & 0xFFFF;}
//Uint32 getLength() const { return len >> 16;}
//void setOpLen(Uint32 o, Uint32 l) { len = (l << 16) | o;}
};

struct QueryNodeParameters  // Effectively used as a base class for QN_xxxParameters
{
  Uint32 len;
  Uint32 requestInfo;
  Uint32 resultData;   // Api connect ptr

  enum OpType
  {
    QN_LOOKUP    = 0x1,
    QN_SCAN_FRAG = 0x2,
    QN_END = 0
  };

  static Uint32 getOpType(Uint32 op_len) { return op_len & 0xFFFF;}
  static Uint32 getLength(Uint32 op_len) { return op_len >> 16;}

  static void setOpLen(Uint32 &d, Uint32 o, Uint32 l) { d = (l << 16) | o;}

  // If possible we should change the above static methods to non-static:
//Uint32 getOpType() const { return len & 0xFFFF;}
//Uint32 getLength() const { return len >> 16;}
//void setOpLen(Uint32 o, Uint32 l) { len = (l << 16) | o;}
};

struct DABits
{
  /**
   * List of bits shared for QN_LookupNode & QN_ScanFragNode
   */
  enum NodeInfoBits
  {
    NI_HAS_PARENT     = 0x01,

    NI_KEY_LINKED     = 0x02,  // Does keyinfo contain linked values
    NI_KEY_PARAMS     = 0x04,  // Does keyinfo contain parameters
    NI_KEY_CONSTS     = 0x08,  // Does keyinfo contain const values

    NI_LINKED_ATTR    = 0x10,  // List of attributes to be used by children

    NI_ATTR_INTERPRET = 0x20,  // Is attr-info a interpreted program
    NI_ATTR_PARAMS    = 0x40,  // Does attrinfo contain parameters
    NI_ATTR_LINKED    = 0x80,  // Does attrinfo contain linked values

    /**
     * Iff this flag is set, then this operation has a child operation with a 
     * linked value that refes to a disk column of this operation. For example
     * SELECT * FROM t1, t2 WHERE t1.disk_att = t2.primary_key;  
     */
    NI_LINKED_DISK    = 0x100, 

    NI_END = 0
  };

  enum ParamInfoBits
  {
    PI_ATTR_LIST   = 0x1, // "user" projection list

    /**
     * These 2 must match their resp. QueryNode-definitions
     */
    PI_ATTR_PARAMS = 0x2, // attr-info parameters (NI_ATTR_PARAMS)
    PI_KEY_PARAMS  = 0x4, // key-info parameters  (NI_KEY_PARAMS)

    /** 
     * The parameter object contains a program that will be interpreted 
     * before reading the attributes (i.e. a scan filter).
     * NOTE: Can/should not be used if QueryNode contains interpreted program
     */
    PI_ATTR_INTERPRET = 0x8,

    /**
     * Iff this flag is set, then the user projection for this operation 
     * contains at least one disk column.   
     */
    PI_DISK_ATTR = 0x10, 
    PI_END = 0
  };
};


/**
 * This node describes a pk-lookup
 */
struct QN_LookupNode // Is a QueryNode subclass
{
  Uint32 len;
  Uint32 requestInfo;
  Uint32 tableId;      // 16-bit
  Uint32 tableVersion;
  STATIC_CONST ( NodeSize = 4 );

  /**
   * See DABits::NodeInfoBits
   */
  Uint32 optional[1];

//Uint32 getLength() const { return len >> 16;}
//void setOpLen(Uint32 o, Uint32 l) { len = (l << 16) | o;}
};

/**
 * This struct describes parameters that are associated with
 *  a QN_LookupNode
 */
struct QN_LookupParameters // Is a QueryNodeParameters subclass
{
  Uint32 len;
  Uint32 requestInfo;
  Uint32 resultData;   // Api connect ptr
  STATIC_CONST ( NodeSize = 3 );

  /**
   * See DABits::ParamInfoBits
   */
  Uint32 optional[1];
};

/**
 * This node describes a table/index-fragment scan
 */
struct QN_ScanFragNode // Is a QueryNode subclass
{
  Uint32 len;
  Uint32 requestInfo;
  Uint32 tableId;      // 16-bit
  Uint32 tableVersion;
  STATIC_CONST ( NodeSize = 4 );

  /**
   * See DABits::NodeInfoBits
   */
  Uint32 optional[1];
};

/**
 * This struct describes parameters that are associated with
 *  a QN_ScanFragNode
 */
struct QN_ScanFragParameters // Is a QueryNodeParameters subclass
{
  Uint32 len;
  Uint32 requestInfo;
  Uint32 resultData;   // Api connect ptr
  STATIC_CONST ( NodeSize = 3 );

  /**
   * See DABits::ParamInfoBits
   */
  Uint32 optional[1];
};

/**
 * This is the definition of a QueryTree
 */
struct QueryTree
{
  Uint32 cnt_len;  // Length in words describing full tree + #nodes
  Uint32 nodes[1]; // The nodes

  static Uint32 getNodeCnt(Uint32 cnt_len) { return cnt_len & 0xFFFF;}
  static Uint32 getLength(Uint32 cnt_len) { return cnt_len >> 16;}
  static void setCntLen(Uint32 &d, Uint32 c, Uint32 l) { d=(l << 16) | c;}
};

/**
 * This is description of *one* entry in a QueryPattern
 *   (used by various QueryNodes)
 */
struct QueryPattern
{
  Uint32 m_info;
  enum
  {
    P_DATA   = 0x1,  // Raw data of len-words (constants)
    P_COL    = 0x2,  // Get column value from RowRef
    P_UNQ_PK = 0x3,  // NDB$PK column from a unique index
    P_PARAM  = 0x4,  // User specified parameter value
    P_PARAM_HEADER = 0x6, // User specified param val including AttributeHeader
    P_ATTRINFO = 0x7,// Get column including header from RowRef
    P_END    = 0
  };

  static Uint32 getType(const Uint32 info) { return info >> 16;}

  /**
   * If type == DATA, get len here
   */
  static Uint32 getLength(Uint32 info) { return info & 0xFFFF;}
  static Uint32 data(Uint32 length) 
  { 
    assert(length <= 0xFFFF);
    return (P_DATA << 16) | length;
  }

  /**
   * If type == COL, get col-no here (index in row)
   */
  static Uint32 getColNo(Uint32 info) { return info & 0xFFFF;}
  static Uint32 col(Uint32 no) { return (P_COL << 16) | no; }

  /**
   * If type == P_UNQ_PK, get PK value from composite NDB$PK col.
   */
  static Uint32 colPk(Uint32 no) {  return (P_UNQ_PK << 16) | no; }

  /**
   * If type == PARAM, get param-no here (index in param list)
   */
  static Uint32 getParamNo(Uint32 info) { return info & 0xFFFF;}
  static Uint32 param(Uint32 no) { return (P_PARAM << 16) | no; }
};

#endif
