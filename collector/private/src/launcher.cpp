/*

*/

#include "log_collector_impl.h"
#include "net_handler_impl.h"
#include "host_context_impl.h"
USING_NAMESPACE_COLLECTOR;

#include "object.h"
USING_NAMESPACE;

#include "file_util.h"
#include "thread.h"
USING_NAMESPACE_BASE;

InterruptImpl g_Interrupt;

//////////////////////////////////////////////////////////////////////////

#define ErrorHandler   TRACE
HANDLE hAppend; 
const char_t*   gStrLockFile = "__collector.lock";

bool_t lockFile() {  

  hAppend = CreateFile(gStrLockFile,   // open TWO.TXT 
    GENERIC_WRITE,                // open for writing 
    0,                            // do not share 
    NULL,                         // no security 
    OPEN_ALWAYS,                  // open or create 
    FILE_ATTRIBUTE_NORMAL,        // normal file 
    NULL);                        // no attr. template 

  if (hAppend == INVALID_HANDLE_VALUE) 
  { 
    ErrorHandler("Could not open TWO.");    // process error 
    return FALSE;
  } 

  // Append the first file to the end of the second file. 
  // Lock the second file to prevent another process from 
  // accessing it while writing to it. Unlock the 
  // file when writing is finished. 

  return LockFile(hAppend, 0, 0, 0, 0);
}

void unlockFile() {

  if (hAppend) { return ; }

  UnlockFile(hAppend, 0, 0, 0, 0);
  CloseHandle(hAppend);
}

// AutoReleaseFile gAutoRelFile(NULL);
// static const char_t* gStrLockFileName   = _STR("__collector.lock");
// 
// bool_t didLockFile() {
// 
//   gAutoRelFile.Set(OpenFile(gStrLockFileName, strAttrOpenRead));
//   if (NULL == gAutoRelFile) { return FALSE; }
//   return LockFile(gAutoRelFile);
// }

rc_t StartService(IInterrupt* pInterrupt, const char_t* strSystemPath) {

  ASSERT(pInterrupt);

//   if (FALSE == didLockFile()) {
//     PRINTF("Lock File %s Failed. may be the collector is running.",gStrLockFileName );
//   }

  HostContextImpl hostContext(strSystemPath);
  NetHandlerImpl netHandler(&hostContext, NULL);
  AutoRelease<LogCollectorImpl*> autoRelCollector(LogCollectorImpl::CreateInstance(&hostContext, &netHandler));
  if (NULL == autoRelCollector) { return RC_E_NOMEM; }
  rc_t rc;

  rc = autoRelCollector->Start();
  if (RC_S_OK != rc) {

    hostContext.ReadLog();
    PRINTF("Start Failed. %u", rc);
    return rc;
  }

  while (FALSE == pInterrupt->didInterrupt()) {

    if (RC_S_OK != autoRelCollector->Run()) {

      PRINTF("collector run failed");
      break;
    }

    hostContext.ReadLog();
    Thread::sleep(1);
  }

  rc = autoRelCollector->Stop();
  hostContext.ReadLog();
  return RC_S_OK;
}

void ctrl_handler(int sig) {

  g_Interrupt.Interrupt((uint32_t)(sig), NULL_STR);
}


#include <signal.h>
#include <direct.h>

int main(int argc, char** argv) {

  char_t strExeFullPath[kMAX_PATH];

  UNUSED_PARAM(argc);
  UNUSED_PARAM(argv);

  if (FALSE == lockFile()) {
    PRINTF("Lock File %s Failed.\n  may be the other collector is running.",gStrLockFile );
    getchar();
    return 0;
  }

  signal(SIGINT,ctrl_handler);

  BZERO_ARR(strExeFullPath);
  if (_getcwd(strExeFullPath, sizeof(strExeFullPath))) {
    STRCAT(strExeFullPath, sizeof(strExeFullPath), STR_DIR_SEP);
  }
  else {  
    STRCPY(strExeFullPath, sizeof(strExeFullPath), NULL_STR);
  }

  PRINTF("%s\n", strExeFullPath);

  StartService(&g_Interrupt, strExeFullPath);

  getchar();
  unlockFile();
  return 0;
}