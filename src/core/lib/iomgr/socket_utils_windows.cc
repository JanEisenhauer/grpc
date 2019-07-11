/*
 *
 * Copyright 2016 gRPC authors.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <grpc/support/port_platform.h>

#include "src/core/lib/iomgr/port.h"

#ifdef GRPC_WINDOWS_SOCKETUTILS

#include "src/core/lib/iomgr/sockaddr.h"
#include "src/core/lib/iomgr/socket_utils.h"

#include <grpc/support/log.h>

#include <errno.h>

uint16_t grpc_htons(uint16_t hostshort) { return htons(hostshort); }

uint16_t grpc_ntohs(uint16_t netshort) { return ntohs(netshort); }

uint32_t grpc_htonl(uint32_t hostlong) { return htonl(hostlong); }

uint32_t grpc_ntohl(uint32_t netlong) { return ntohl(netlong); }

int grpc_inet_pton(int af, const char* src, void* dst) {
#if !defined(GPR_BACKWARDS_COMPATIBILITY_MODE) || _WIN32_WINNT >= 0x0600
  return inet_pton(af, src, dst);
#else
  switch(af) {
    case AF_INET: // IPv4
    {
      struct sockaddr_in sa{};
      sa.sin_family = AF_INET;
      INT sz = sizeof(sa);
      /* Windows WSAStringToAddress wants a mutable ip pointer */
      if(WSAStringToAddress((char*)src, AF_INET, nullptr, (struct sockaddr*)&sa, &sz) == SOCKET_ERROR) {
        if(WSAGetLastError() == WSAEINVAL) {
          return 0;
        } else {
          return -1;
        }
      } else {
        return 1;
      }
    }
    case AF_INET6: // Ipv6
      // TODO(Jan) Konvertierung fuer IPv6 implementieren.
    default:
      errno = EAFNOSUPPORT;
      return -1;
  }
#endif
}

const char* grpc_inet_ntop(int af, const void* src, char* dst, size_t size) {
#if !defined(GPR_BACKWARDS_COMPATIBILITY_MODE) || _WIN32_WINNT >= 0x0600
  /* Windows InetNtopA wants a mutable ip pointer */
  return InetNtopA(af, (void*)src, dst, size);
#else
  switch(af) {
    case AF_INET: // IPv4
    {
      struct in_addr const * addr = (struct in_addr const *)src;
      struct sockaddr_in sa{};
      sa.sin_family = AF_INET;
      sa.sin_addr = *addr;
      DWORD sz = size;

      if(WSAAddressToString((struct sockaddr *)&sa, sizeof(sa), nullptr, dst, &sz) == SOCKET_ERROR) {
        errno = ENOSPC;
        return nullptr;
      } else {
        return dst;
      }
    }
    case AF_INET6: // IPv6
      // TODO(Jan) Konvertierung fuer IPv6 implementieren.
    default:
      errno = EAFNOSUPPORT;
      return nullptr;
  }
#endif
}

#endif /* GRPC_WINDOWS_SOCKETUTILS */
