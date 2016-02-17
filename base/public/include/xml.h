/*

*/

#ifndef XML_H_
#define XML_H_

#include "base.h"
#include "base_export.h"

//////////////////////////////////////////////////////////////////////////
#include "expat.h"


//////////////////////////////////////////////////////////////////////////
BEGIN_NAMESPACE

typedef AutoRelease<XML_Parser>         AutoReleaseXML;

template<>
void AutoReleaseXML::Release() { if (m_handle) { XML_ParserFree(m_handle); } }

END_NAMESPACE

#endif // XML_H_
