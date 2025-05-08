
//          Copyright Oliver Kowalke 2013.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "boost/fiber/mutex.hpp"

#include <algorithm>
#include <functional>
#include <system_error>

#include "boost/fiber/exceptions.hpp"
#include "boost/fiber/scheduler.hpp"
#include "boost/fiber/waker.hpp"

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_PREFIX
#endif

namespace boost {
namespace fibers {

void
mutex::lock() {
    context * calling_context = context::active();
    while ( true) {
        // store this fiber in order to be notified later
        detail::spinlock_lock lk{ wait_queue_splk_ };
        if ( BOOST_UNLIKELY( owner_ == calling_context) ) {
            throw lock_error{
                    std::make_error_code( std::errc::resource_deadlock_would_occur),
                    "boost fiber: a deadlock is detected" };
        }
        if ( owner_ == nullptr) {
            owner_ = calling_context;
            return;
        }

        wait_queue_.suspend_and_wait( lk, calling_context);
    }
}

bool
mutex::try_lock() {
  context * active_ctx = context::active();
  if (!wait_queue_splk_.try_lock()) {
    return false;
  }
  if ( BOOST_UNLIKELY( owner_ == active_ctx) ) {
    throw lock_error{
      std::make_error_code( std::errc::resource_deadlock_would_occur),
      "boost fiber: a deadlock is detected" };
  }
  if ( nullptr == owner_) {
    owner_ = active_ctx;
  }
  return owner_ == active_ctx;
}

void
mutex::unlock() {
    context * active_ctx = context::active();
    detail::spinlock_lock lk{ wait_queue_splk_ };
    if ( BOOST_UNLIKELY( active_ctx != owner_) ) {
        throw lock_error{
                std::make_error_code( std::errc::operation_not_permitted),
                "boost fiber: no  privilege to perform the operation" };
    }
    owner_ = nullptr;

    wait_queue_.notify_one();
}

}}

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_SUFFIX
#endif
