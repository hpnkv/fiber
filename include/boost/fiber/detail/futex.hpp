
//          Copyright Oliver Kowalke 2016.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_FIBERS_DETAIL_FUTEX_H
#define BOOST_FIBERS_DETAIL_FUTEX_H

#include <boost/config.hpp>
#include <boost/predef.h>

#include <boost/fiber/detail/config.hpp>

#ifndef SYS_futex
#define SYS_futex SYS_futex_time64
#endif

#if BOOST_OS_LINUX
extern "C" {
#include <linux/futex.h>
#include <sys/syscall.h>
}
#elif BOOST_OS_BSD_OPEN
extern "C" {
#include <sys/futex.h>
}
#elif BOOST_OS_WINDOWS
#include <windows.h>
#elif (BOOST_OS_MACOS && BOOST_OS_SYNC_WAIT_ON_ADDRESS_AVAILABLE)
#include <os/os_sync_wait_on_address.h>
#endif

namespace boost {
namespace fibers {
namespace detail {

#if BOOST_OS_LINUX || BOOST_OS_BSD_OPEN
BOOST_FORCEINLINE
int sys_futex( void * addr, std::int32_t op, std::int32_t x) {
#if BOOST_OS_BSD_OPEN
    return ::futex
    (
       static_cast< volatile uint32_t* >(addr),
       static_cast< int >(op),
       x,
       nullptr,
       nullptr
    );
#else
    return ::syscall( SYS_futex, addr, op, x, nullptr, nullptr, 0);
#endif
}

BOOST_FORCEINLINE
int futex_wake( std::atomic< std::int32_t > * addr) {
    return 0 <= sys_futex( static_cast< void * >( addr), FUTEX_WAKE_PRIVATE, 1) ? 0 : -1;
}

BOOST_FORCEINLINE
int futex_wait( std::atomic< std::int32_t > * addr, std::int32_t x) {
    return 0 <= sys_futex( static_cast< void * >( addr), FUTEX_WAIT_PRIVATE, x) ? 0 : -1;
}
#elif BOOST_OS_WINDOWS
BOOST_FORCEINLINE
int futex_wake( std::atomic< std::int32_t > * addr) {
    ::WakeByAddressSingle( static_cast< void * >( addr) );
    return 0;
}

BOOST_FORCEINLINE
int futex_wait( std::atomic< std::int32_t > * addr, std::int32_t x) {
    ::WaitOnAddress( static_cast< volatile void * >( addr), & x, sizeof( x), INFINITE);
    return 0;
}
#elif (BOOST_OS_MACOS && BOOST_OS_SYNC_WAIT_ON_ADDRESS_AVAILABLE)
BOOST_FORCEINLINE
int futex_wake( std::atomic< std::int32_t > * addr) {
    return 0 <= os_sync_wake_by_address_any(addr, 4, OS_SYNC_WAKE_BY_ADDRESS_NONE) ? 0 : -1;
}

BOOST_FORCEINLINE
int futex_wait( std::atomic< std::int32_t > * addr, std::int32_t x) {
    return 0 <= os_sync_wait_on_address(static_cast< void * >( addr), x, sizeof(x),
                                  OS_SYNC_WAIT_ON_ADDRESS_NONE) ? 0 : -1;
}
#else
# warn "no futex support on this platform"
#endif

}}}

#endif // BOOST_FIBERS_DETAIL_FUTEX_H
