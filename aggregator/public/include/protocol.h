/*

*/

#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#include "aggregator.h"

BEGIN_NAMESPACE_AGGREGATOR

//////////////////////////////////////////////////////////////////////////
// version
static const uint32_t kProtocolVer      = 1;

//////////////////////////////////////////////////////////////////////////
// Setting
static const char_t* kStrCollectorConfig          = _STR("CollectorConfig");
static const char_t* kStrCollectorFile            = _STR("CollectorFile");
static const char_t* kStrCollectRule              = _STR("CollectRule");

static const char_t* kStrCollectLog               = _STR("Log");

static const char_t* kStrSystemSetting            = _STR("SystemSetting");
// 
static const char_t* kStrUpdateFileInfo           = _STR("UpdateFileInfo");
//static const char_t* kStrGetFileID                = _STR("GetFileID");

static const char_t* kStrGetLogTypeRule           = _STR("GetLogTypeRule");
//static const char_t* kStrGetParseFile             = _STR("GetParseFile");
//static const char_t* kStrSetParseFile             = _STR("SetParseFile");

static const char_t* kStrGetModuleSetting         = _STR("GetModuleSetting");
static const char_t* kStrSetModuleSetting         = _STR("SetModuleSetting");

static const char_t* kStrCollectorStatus          = _STR("CollectorStatus");
static const char_t* kStrResourceStatus           = _STR("ResourceStatus");

static const char_t* kStrGetRPCInvoke             = _STR("GetRPCInvoke");
static const char_t* kStrSetRPCInvoke             = _STR("SetRPCInvoke");

static const char_t* kStrCheckSystem              = _STR("CheckSystem");
static const char_t* kStrCheckPoint               = _STR("CheckPoint");

static const char_t* kStrGetTC50LogRule           = _STR("TC50LogRule");

END_NAMESPACE_AGGREGATOR

#endif // PROTOCOL_H_
