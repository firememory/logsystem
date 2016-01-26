/*

*/

#ifndef TC50_DICT_H_
#define TC50_DICT_H_

//////////////////////////////////////////////////////////////////////////
#include "aggregator_base.h"

#include "parse_ix.h"

#include "std_list"
#include "hash_tables.h"
#include <set>

BEGIN_NAMESPACE_AGGREGATOR
//////////////////////////////////////////////////////////////////////////
USING_BASE(hash_set);
USING_BASE(char_t_hash);

//////////////////////////////////////////////////////////////////////////
// func
// #include "prot_ix/IXStdProt_WT.h"

#define NO_QSIDS
#include "prot_ix/IXStdProt_WT.h"

//////////////////////////////////////////////////////////////////////////
class TC50Dict {
public:
   // sec
  rc_t AddDict(const char_t* strCollector, const char_t* strDir, file_id_t, uint64_t);
  IDictIX* FindNearDict_Simple(IFileIteratorBase*, const char_t* strCollector, const char_t* strDir, uint64_t);
  IDictIX* FindNearDict_Common(IFileIteratorBase*, const char_t* strCollector, const char_t* strDir, uint64_t);
  IDictIX* FindNearDict_Scntr(IFileIteratorBase*, const char_t* strCollector, const char_t* strDir, uint64_t);
  
  IDictIX* GetDefDict_Simple();
  IDictIX* GetDefDict_Common();
  IDictIX* GetDefDict_Scntr();
  rc_t LoadDefaultDict(const char_t* path);
public:
  TC50Dict();
  ~TC50Dict();

private:
  class DictNode;

  void clearDictMap();
  DictNode* findDict(const char_t* strCollector, const char_t* strDir, uint64_t);
  DictNode* getNearDict(IFileIteratorBase* pIFileIterator, const char_t* strCollector, const char_t* strDir, uint64_t);


private:
  typedef std::size_t       hash_value_t;
  class DictNode {
  public:
    rc_t LoadDictFromFile(uint8_t* buffer, uint32_t, IFileIteratorBase* pIFileIterator);
    rc_t LoadDictFromFile(const uint8_t* buffer, uint32_t);

    // hash
  public:
    hash_value_t GetHashValue() const { return m_hv; }
    uint64_t GetFileTime() const { return m_file_create_time; }

  public:
    DictNode(const char_t* strCollector, const char_t* strDir, file_id_t file_id, uint64_t create_time)
      : m_file_create_time(create_time)
      , m_autoRelDictIXSimple(IDictIX::CreateInstance(kStrWtDict2, NULL, 0))
      , m_autoRelDictIXCommon(IDictIX::CreateInstance(kStrWtDict2, NULL, 0))
      , m_autoRelDictIXScntr(IDictIX::CreateInstance(kStrWtDict2, NULL, 0))
      //, m_hv((hash_value_t)(char_t_hash()(strCollector) + char_t_hash()(strDir)))
      , m_file_id(file_id)
    {
      char_t strRealDir[kMAX_PATH] = {0};
      STRCPY(strRealDir, sizeof(strRealDir), strDir);

      const char_t* strFind = STRCHR(strRealDir, STRLEN(strRealDir), ':');
      if (strFind) { strRealDir[strFind - strRealDir] = '_'; }
      size_t nDirLen = STRLEN(strRealDir) - 1;
      while (nDirLen && '\\' == strRealDir[nDirLen] || '/' == strRealDir[nDirLen]) { --nDirLen; }
      strRealDir[nDirLen + 1] = 0;
      m_hv = char_t_hash()(strCollector) + char_t_hash()(strRealDir);
    }

    ~DictNode() {}

  public:
    file_id_t FileID() const { return m_file_id; }

  private:
    hash_value_t    m_hv;

    uint64_t        m_file_create_time;   // sec
    file_id_t       m_file_id;

  public:
    // 格式是：登录字典长度（4个字节）+登录字典内存+标准字典长度（4个字节）+标准字典内存
    //  吴火生(16888) 16:17:09
    //  文件名：dll名称_券商ID_YYYYMMDDHHMMSS.sto
    AutoRelease<IDictIX*>     m_autoRelDictIXSimple;
    AutoRelease<IDictIX*>     m_autoRelDictIXCommon;
    AutoRelease<IDictIX*>     m_autoRelDictIXScntr;
  };

  struct dict_node_hash {
    enum
    {	// parameters for hash table
      bucket_size = 4,	// 0 < bucket_size
      min_buckets = 8};	// min_buckets = 2 ^^ N, 0 < N

      hash_value_t operator()(const DictNode* pDictNode) const {
        return pDictNode->GetHashValue();
      }

      bool operator()(const DictNode* s1,const DictNode* s2) const {

        // VC2005
        // strict weak ordering
        // http://support.microsoft.com/kb/949171
        if (s1->GetHashValue() == s2->GetHashValue()) {
          return s1->GetFileTime() <= s2->GetFileTime() ? false : true;
        }
        else {
          return s1->GetHashValue() <= s2->GetHashValue() ? false : true;
        }
      }
  };

  typedef std::set<DictNode*, dict_node_hash>           dict_set_t;
  dict_set_t       m_setDict;

  DictNode*         m_pDictNodeDef;

private:  
  DISALLOW_COPY_AND_ASSIGN(TC50Dict);
}; // TC50Dict

END_NAMESPACE_AGGREGATOR
#endif // TC50_DICT_H_

