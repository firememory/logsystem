/*

*/

#ifndef FILE_COLLECTOR_IMPL_H_
#define FILE_COLLECTOR_IMPL_H_

#include "file_collector.h"

#include "aggregator_base.h"
//USING_NAMESPACE_AGGREGATOR

#include "file_util.h"
#include "std_list"
#include "hash_tables.h"

#include "regex.hpp"

BEGIN_NAMESPACE_COLLECTOR
//////////////////////////////////////////////////////////////////////////
USING_BASE(isDirChar);

USING_BASE(OpenFile);
USING_BASE(ReadFile);
USING_BASE(strAttrOpenRead);
USING_BASE(PlatformFileInfo);
USING_BASE(ChangeDirStype);

USING_BASE(hash_map);
USING_BASE(char_t_hash);

USING_BASE(DayTime);

USING_AGGREGATOR(kCloseFileTimeOut);

//////////////////////////////////////////////////////////////////////////
class FileNode;

class FileCollectorImpl : public IFileCollector {

  // IFileCollector
public:
  void Release() { delete this; }

public:
  rc_t AddCollectRule(const char_t* strDir, const char_t* strExclude, const char_t* strInclude);
  void ClearCollectRule();

  IFileNode* AddFileNode(const file_id_t&, file_size_t pos, bool_t bFinish, const char_t* strDir, const char_t* strName);
  IFileNode* FindFileNode(const char_t* strDir, const char_t* strName);
  IFileNode* FindFileNode(const file_id_t&);
  void ClearFileNode();

  //void ConfirmSend();
  void SetTimeControl(bool_t bAfterToDay) { m_bAfterToDay = bAfterToDay; }

public:
  /*
    RC_S_OK   ==> continue
  */
  rc_t Collector(collector_log_type_e, file_collect_callback_t, void* context);

  // collect rule

public:
  void TickProc();

public:
  FileCollectorImpl();
  ~FileCollectorImpl();

private:

  typedef std::size_t       hash_value_t;

  class CollectRule {
  public:
    CollectRule(const char_t* strDir, const char_t* strExclude, const char_t* strInclude)
      : regExExclude(0, strExclude)
      , regExInclude(0, strInclude)
    {

      ASSERT(strDir);
      ASSERT(strExclude);
      ASSERT(strInclude);

      size_t len = STRLEN(strDir);

      if (FALSE == isDirChar(strDir[len - 1])) {
        // add dir char
        SNPRINTF(dir, kMAX_PATH, kMAX_PATH, "%s/", strDir);  
      }
      else {
        STRCPY(dir, kMAX_PATH, strDir);
      }

      SNPRINTF(find_dir, kMAX_PATH, kMAX_PATH, "%s/*", dir);

#if defined(DEBUG_ON)
      STRCPY(strExclude__, kMAX_PATH, strExclude);
      STRCPY(strInclude__, kMAX_PATH, strInclude);
#endif
      // calc hash
      hash = char_t_hash()(dir);

      // 
      SNPRINTF(mml_dir, sizeof(mml_dir), sizeof(mml_dir)
        ,"%s%s", dir, strMemMiniLog);

      mml_hash = char_t_hash()(mml_dir);
      // compile regex
    }

    ~CollectRule() {}

  public:
    char_t    find_dir[kMAX_PATH + 1];
    char_t    dir[kMAX_PATH + 1];

    RegEx     regExExclude;
    RegEx     regExInclude;

#if defined(DEBUG_ON)
    char_t    strExclude__[kMAX_PATH + 1];
    char_t    strInclude__[kMAX_PATH + 1];
#endif // DEBUG_ON

    hash_value_t  hash;

    // memory mini log
  public:
    const char_t* GetMMLDir() const { return mml_dir; }

  public:
    char_t    mml_dir[kMAX_PATH + 1];
    hash_value_t  mml_hash;
  }; // CollectRule

  typedef std::list<CollectRule*>     collect_rule_list_t;

  collect_rule_list_t         m_listCollectRule;

  bool_t searchAllFile(collector_log_type_e, CollectRule*, file_collect_callback_t pfn_cb, void* context);

  // file node
private:
  class FileNode : public IFileNode {
    // IFileNode
  public:
    void Release() {}

  public:
    file_id_t ID() const { return m_id; }
    file_size_t Size() const { return m_info.size; }
    STATUS_e Status() const { return UNKNOWN;}
    const char_t* CheckSum() const { return NULL_STR; }
    const char_t* LocalPath() const { return m_strFullName; }
    const char_t* RemotePath() const { return NULL_STR; }
    const char_t* Collector() const { return NULL_STR; }
    const uint64_t CreateTime() const { return m_info.creation_time; }

  public:
    file_size_t GetNFPos() const { return m_posConfirm; }
    void SetID(file_id_t id) { m_id = id; }

    // 
    void SetSize(file_size_t size) { SetConfirm(size); }

  public:
    const char_t* LocalDir() const { return m_strDir; }
    const char_t* LocalName() const { return m_strFullName + m_nNamePos;}
    
  public:
    // encode 
    uint32_t GetMaxEncodeSize() { return m_nMaxEncodeSize;}
    void SetMaxEncodeSize(uint32_t size) { m_nMaxEncodeSize = size > kMAX_READ_SIZE ? kMAX_READ_SIZE : size; }

  public:
    const char_t* GetMMLDir() const { return m_mml_dir;}

  public:
    FileNode(const char_t* strDir, const char_t* strMMLName, const char_t* strName
      , file_id_t id = IFileNode::INVALID_FILE_ID, file_size_t pos = 0, bool_t bFinish = FALSE)
      : m_id(id)

      , m_bSendReqFileID(FALSE)

      , m_posConfirm(pos)
      , m_posSended(pos)

      , m_bSended(FALSE)

      , m_bFinish(bFinish)
      , m_nNamePos(0)
      , m_autoRelFile(NULL)

      , m_nMemMiniLogData(0)

      , m_nMaxEncodeSize(kMAX_READ_SIZE)

      , m_mml_dir(strMMLName)
      
    {
      ASSERT(strDir);
      ASSERT(strName);
      BZERO_ARR(m_strFullName);
      BZERO_ARR(m_strDir);

      STRCPY(m_strDir, kMAX_PATH, strDir);
      ChangeDirStype(m_strDir);
      
      // memory mini log
      STRCPY(m_strFullName, kMAX_PATH, m_strDir);

      if (NULL == m_mml_dir) {
        size_t nDirLen = STRLEN(m_strDir);
        if (kstrMemMiniLogLen < nDirLen) {
          if (0 == STRNCMP(m_strDir + nDirLen - kstrMemMiniLogLen, strMemMiniLog, kstrMemMiniLogLen)) {
            STRNCPY(m_strFullName, sizeof(m_strFullName), m_strDir, nDirLen - kstrMemMiniLogLen - 1);
            m_strFullName[nDirLen - kstrMemMiniLogLen - 1] = 0;
          }
        }
      }

      size_t nNameLen = STRNLEN(m_strFullName, sizeof(m_strFullName));
      if (FALSE == isDirChar(m_strFullName[nNameLen - 1])) {
        //STRCAT(m_strFullName, kMAX_PATH, STR_DIR_SEP);
        m_strFullName[nNameLen] = '/';
        m_strFullName[nNameLen + 1] = 0;
        m_nNamePos = (uint32_t)nNameLen + 1;
      }
      else {
        m_nNamePos = (uint32_t)nNameLen;
      }
      
      STRCAT(m_strFullName, kMAX_PATH, strName);

      //ChangeDirStype(m_strFullName);
      

      /*
      const char_t* strFind = STRSTR(m_strFullName, strName);
      if (strFind) { m_nNamePos = (uint32_t)(strFind - m_strFullName); }
      */
      ReSetTick();
    }

    ~FileNode() {}

  public:
    PlatformFileInfo& FileInfo() { return m_info; }
    const file_size_t GetSendSize() { return m_posSended; }
    const file_size_t GetConfirmSize() { return m_posConfirm; }

    void SetConfirm(file_size_t size) {
      if (FALSE == m_bSended) { m_bSended = TRUE; }
      m_posConfirm = size; m_posSended = size;
    }

    void UpdateSendSize(file_size_t size) { m_posSended += size; }

    bool_t DidSend() {
      if (FALSE == m_bSended) { return TRUE; }
      //if (m_nCountConfirm != m_nCountSend) { return TRUE; }
      return m_posConfirm == m_posSended && m_posSended < m_info.size ? TRUE : FALSE;
    }

  public:
    rc_t GetData(uint8_t* data, file_size_t* read_size, file_size_t pos) {

      openFile();
      if (NULL == m_autoRelFile) { return RC_S_FAILED;}
      ReSetTick();

      return ReadFile(data, read_size, m_autoRelFile, pos);
    }

    void TickProc() {

      uint32_t now = DayTime::now();
      if (now - m_nTimeLastRead > kCloseFileTimeOut) { m_autoRelFile.Release(); m_nTimeLastRead = now; }
    }

  private:
    void openFile() {

      if (NULL == m_autoRelFile) {
        m_autoRelFile.Set(OpenFile(m_strFullName, strAttrOpenRead));
      }
    }

    void ReSetTick() { m_nTimeLastRead  = DayTime::now(); }

  private:
    PlatformFileInfo          m_info;
    char_t                    m_strFullName[kMAX_PATH + 1];
    char_t                    m_strDir[kMAX_PATH + 1];
    uint32_t                  m_nNamePos;
    file_id_t                 m_id;
    bool_t                    m_bFinish;

    bool_t                    m_bSendReqFileID;
    file_size_t               m_posConfirm;
    file_size_t               m_posSended;

    bool_t                    m_bSended;
    /*const CollectRule*        m_pCollectRule;*/
    uint32_t                  m_nTimeLastRead;
    AutoReleaseFile           m_autoRelFile;

  public:
    uint32_t                  m_nMemMiniLogData;

  private:
    uint32_t                  m_nMaxEncodeSize;
    const char_t*             m_mml_dir;

    DISALLOW_COPY_AND_ASSIGN(FileNode);
  }; // FileNode

  friend class LogCollectorImpl;

  struct two_string_key_t {
    hash_value_t    hash;
    const char_t*   one;
  };

  struct two_string_hash {
    enum
    {	// parameters for hash table
      bucket_size = 4,	// 0 < bucket_size
      min_buckets = 8};	// min_buckets = 2 ^^ N, 0 < N

      std::size_t operator()(const two_string_key_t& twoString) const {
        std::size_t result = 0;
        const char_t* s = twoString.one;
        while (*s) { result = (result * 131) + *s++;}
        return result + twoString.hash;
      }

      bool operator()(const two_string_key_t& s1,const two_string_key_t& s2) const {
        return (s1.hash == s2.hash && 0 == STRCMP(s1.one, s2.one)) ? false : true;
      }
  };


  typedef hash_map<const two_string_key_t, FileNode*, two_string_hash>    file_node_map_t;

  file_node_map_t             m_mapFileNode;


  void clearFileNode();
  FileNode* findFileNode(hash_value_t, const char_t*);
  rc_t addFileNode(hash_value_t hash, const char_t* strOne, FileNode* pFileNode);

  typedef hash_map<file_id_t, FileNode*>    file_node_id_map_t;
  file_node_id_map_t          m_mapFileNodeID;
  rc_t addFileNode(file_id_t, FileNode* pFileNode);


private:
  bool_t  m_bAfterToDay;

  DISALLOW_COPY_AND_ASSIGN(FileCollectorImpl);
}; // FileCollectorImpl

END_NAMESPACE_COLLECTOR
#endif // FILE_COLLECTOR_IMPL_H_
