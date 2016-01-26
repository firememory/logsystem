/*

*/
#include "net_util.h"

#include "memory_pool.h"
#include "code_util.h"

//////////////////////////////////////////////////////////////////////////
typedef uint8_t             uint8;
typedef uint16_t            uint16;
typedef uint32_t            uint32;
typedef uint64_t            uint64;

#include "sys_byteorder.h"

#define DCHECK    ASSERT
#define WARN_UNUSED_RESULT
USING_NAMESPACE_NET;


#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable: 4244)
#endif // _MSC_VER

#include "ip_endpoint.cc"

#ifdef _MSC_VER
# pragma warning(default: 4244)
#pragma warning(pop)
#endif // _MSC_VER

//////////////////////////////////////////////////////////////////////////
BEGIN_NAMESPACE_NET

NetworkInterface::NetworkInterface() {
}

NetworkInterface::NetworkInterface(const std::string& name,
                                   const IPAddressNumber& address,
                                   const IPAddressNumber& mac)
    : name(name), address(address), mac(mac) {
}

NetworkInterface::NetworkInterface(const std::string& name,
                                   const IPAddressNumber& address)
    : name(name), address(address) {
}

NetworkInterface::~NetworkInterface() {
}

//////////////////////////////////////////////////////////////////////////

// Extracts the address and port portions of a sockaddr.
bool GetIPAddressFromSockAddr(const struct sockaddr* sock_addr,
                              socklen_t sock_addr_len,
                              const uint8** address,
                              size_t* address_len,
                              uint16* port) {
  if (sock_addr->sa_family == AF_INET) {
    if (sock_addr_len < static_cast<socklen_t>(sizeof(struct sockaddr_in)))
      return false;
    const struct sockaddr_in* addr =
        reinterpret_cast<const struct sockaddr_in*>(sock_addr);
    *address = reinterpret_cast<const uint8*>(&addr->sin_addr);
    *address_len = kIPv4AddressSize;
    if (port)
      *port = ::base::NetToHost16(addr->sin_port);
    return true;
  }

  if (sock_addr->sa_family == AF_INET6) {
    if (sock_addr_len < static_cast<socklen_t>(sizeof(struct sockaddr_in6)))
      return false;
    const struct sockaddr_in6* addr =
        reinterpret_cast<const struct sockaddr_in6*>(sock_addr);
    *address = reinterpret_cast<const unsigned char*>(&addr->sin6_addr);
    *address_len = kIPv6AddressSize;
    if (port)
      *port = ::base::NetToHost16(addr->sin6_port);
    return true;
  }

  return false;  // Unrecognized |sa_family|.
}
//////////////////////////////////////////////////////////////////////////
uint32_t IPAddressToString(char_t* strIP, const uint8_t* address, size_t len) {

  //const uint8_t* ptr;
  uint8_t val;
  uint32_t pos = 0;
  while (len--) {

    val = *address++;
    if (val > 100) { strIP[pos++] = Base16Encode[(val / 100)]; val = val % 100; }
    if (val > 10) {   strIP[pos++] = Base16Encode[val / 10]; }
    strIP[pos++] = Base16Encode[val % 10];
    strIP[pos++] = _CHAR('.');
  }
  return pos - 1;
}


std::string IPAddressToString(const uint8_t* address,
                              size_t address_len) {
  UNUSED_PARAM(address);
  UNUSED_PARAM(address_len);
  std::string str;
//   url_canon::StdStringCanonOutput output(&str);
// 
//   if (address_len == kIPv4AddressSize) {
//     url_canon::AppendIPv4Address(address, &output);
//   } else if (address_len == kIPv6AddressSize) {
//     url_canon::AppendIPv6Address(address, &output);
//   } else {
//     CHECK(false) << "Invalid IP address with length: " << address_len;
//   }
// 
//   output.Complete();
  return str;
}

std::string IPAddressToStringWithPort(const uint8* address,
                                      size_t address_len,
                                      uint16 port) {
  std::string address_str = IPAddressToString(address, address_len);

  /*
  if (address_len == kIPv6AddressSize) {
    // Need to bracket IPv6 addresses since they contain colons.    
    return base::StringPrintf("[%s]:%d", address_str.c_str(), port);
  }
  return base::StringPrintf("%s:%d", address_str.c_str(), port);
  */
  port = port;
  return address_str;
}

std::string NetAddressToStringWithPort(const struct sockaddr* sa,
                                       socklen_t sock_addr_len) {
  const uint8* address;
  size_t address_len;
  uint16 port;
  if (!GetIPAddressFromSockAddr(sa, sock_addr_len, &address,
                                &address_len, &port)) {
    //NOTREACHED();
    return "";
  }
  return IPAddressToStringWithPort(address, address_len, port);
}

std::string IPAddressToString(const IPAddressNumber& addr) {
  return IPAddressToString(&addr.front(), addr.size());
}

std::string IPAddressToStringWithPort(const IPAddressNumber& addr,
                                      uint16 port) {
  return IPAddressToStringWithPort(&addr.front(), addr.size(), port);
}

std::string GetHostName() {
// #if defined(OS_WIN)
//   EnsureWinsockInit();
// #endif

  // Host names are limited to 255 bytes.
  char buffer[256];
  int result = gethostname(buffer, sizeof(buffer));
  if (result != 0) {
    //DVLOG(1) << "gethostname() failed with " << result;
    buffer[0] = '\0';
  }
  return std::string(buffer);
}

#if defined(PLATFORM_API_WINDOWS)

#include <winsock2.h>
#include <iphlpapi.h>
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")
#define HAVE_W2K
// GetAdaptersAddresses win2000 unsupport
#ifndef HAVE_W2K
bool_t GetNetworkList(NetworkInterfaceList* networks) {
  // GetAdaptersAddresses() may require IO operations.
  //base::ThreadRestrictions::AssertIOAllowed();

  IP_ADAPTER_ADDRESSES info_temp;
  ULONG len = 0;

  // First get number of networks.
  ULONG result = GetAdaptersAddresses(AF_UNSPEC, 0, NULL, &info_temp, &len);
  if (result != ERROR_BUFFER_OVERFLOW) {
    // There are 0 networks.
    return TRUE;
  }

  //scoped_array<char> buf(new char[len]);
  AutoReleaseMemoryBase autoRelMem((uint8_t*)malloc(len));
  IP_ADAPTER_ADDRESSES *adapters =
      reinterpret_cast<IP_ADAPTER_ADDRESSES *>((uint8_t*)autoRelMem/*buf.get()*/);
  result = GetAdaptersAddresses(AF_UNSPEC, 0, NULL, adapters, &len);
  if (result != NO_ERROR) {
    //LOG(ERROR) << "GetAdaptersAddresses failed: " << result;
    return FALSE;
  }

  for (IP_ADAPTER_ADDRESSES *adapter = adapters; adapter != NULL;
       adapter = adapter->Next) {
    // Ignore the loopback device.
    if (adapter->IfType == IF_TYPE_SOFTWARE_LOOPBACK) {
      continue;
    }

    if (adapter->OperStatus != IfOperStatusUp) {
      continue;
    }

    IP_ADAPTER_UNICAST_ADDRESS* address;
    for (address = adapter->FirstUnicastAddress; address != NULL;
         address = address->Next) {
      int family = address->Address.lpSockaddr->sa_family;
      if (family == AF_INET || family == AF_INET6) {
        ::net::IPEndPoint endpoint;
        if (endpoint.FromSockAddr(address->Address.lpSockaddr,
                                  address->Address.iSockaddrLength)) {
          std::string name = adapter->AdapterName;
          //networks->push_back(NetworkInterface(name, endpoint.address()));

          // make mac
          MACAddressNumber mac(adapters->PhysicalAddressLength, 0);
          mac.assign(adapters->PhysicalAddress, adapters->PhysicalAddress + adapters->PhysicalAddressLength);
          
          networks->push_back(NetworkInterface(name, endpoint.address(), mac));
        }
      }
    }
  }

  return TRUE;
}

#else

#define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x))
#define FREE(x) HeapFree(GetProcessHeap(), 0, (x))

bool_t GetNetworkList(NetworkInterfaceList* networks) {
  // GetAdaptersAddresses() may require IO operations.
  //base::ThreadRestrictions::AssertIOAllowed();

  PIP_ADAPTER_INFO pAdapterInfo;
  PIP_ADAPTER_INFO adapter = NULL;

  ULONG ulOutBufLen = sizeof (IP_ADAPTER_INFO);
  pAdapterInfo = (IP_ADAPTER_INFO *) MALLOC(sizeof (IP_ADAPTER_INFO));
  if (pAdapterInfo == NULL) { return FALSE; }

  if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW) {
    FREE(pAdapterInfo);

    pAdapterInfo = (IP_ADAPTER_INFO *) MALLOC(ulOutBufLen);
    if (pAdapterInfo == NULL) {
      return FALSE;
    }
  }

  if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == NO_ERROR) {
    adapter = pAdapterInfo;
    while (adapter) {

      // Ignore the loopback device.
      if (adapter->Type != MIB_IF_TYPE_ETHERNET) { continue; }

      std::string name = adapter->AdapterName;

      MACAddressNumber mac(adapter->AddressLength, 0);
      mac.assign(adapter->Address, adapter->Address + adapter->AddressLength);

      PIP_ADDR_STRING address;
      for (address = &(adapter->IpAddressList); address != NULL;
        address = address->Next) {

          BYTE ip_data[sizeof(address->IpAddress.String)] = {0};
          //size_t len = strlen(address->IpAddress.String);

          size_t idx = 0;
          const char* pSrc = address->IpAddress.String;
          const char* pDot = NULL;
          while (NULL != pSrc && NULL != (pDot = strchr(pSrc, '.'))) {

            ip_data[idx++] = (BYTE)ATOI(pSrc);
            pSrc = pDot + 1;
          }
          if(pSrc) { ip_data[idx++] = (BYTE)ATOI(pSrc); }

          IPAddressNumber ip(idx, 0);
          ip.assign(ip_data, ip_data + idx);

          networks->push_back(NetworkInterface(name, ip, mac));
          if (networks->size()) { break; }
      }
      if (networks->size()) { break; }
      adapter = adapter->Next;
    }
  }

  if (pAdapterInfo) { FREE(pAdapterInfo); }
  return TRUE;
}
#endif // HAVE_W2K

#else
# error "Net is unknown"
#endif

END_NAMESPACE_NET


