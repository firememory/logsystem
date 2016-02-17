/*
net unittest
*/
#include "test.h"

#ifdef DO_UNITTEST
#include "net_util.h"
USING_NAMESPACE_NET;

TEST_BEGIN(net, interface_list) {

  NetworkInterfaceList NI;

  GetNetworkList(&NI);

  const NetworkInterface& ni = NI[0];

  if (ni.name.size()) { printf("\n%s", ni.name); }

  size_t nIPLen = ni.address.size();
  const uint8_t* pIP = &(ni.address.front());
  char_t strIP[128] = {0};
  size_t len = IPAddressToString(strIP, pIP, nIPLen);
  printf("|%s", strIP);

  size_t nMACLen = ni.mac.size();
  const uint8_t* pMAC = &(ni.mac.front());
  char_t strMAC[128] = {0};
  len = IPAddressToString(strMAC, pMAC, nMACLen);
  printf("|%s", strMAC);

} TEST_END

#endif // DO_UNITTEST
