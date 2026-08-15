// SPDX-FileCopyrightText: 2026 NVIDIA CORPORATION
// SPDX-License-Identifier: Apache-2.0

// VulkanHpp Tests : CpptraceExceptions
//                   Test stack traces on Vulkan system exceptions

#include "../test_macros.hpp"
#include <cpptrace/cpptrace.hpp>
#include <string>
#include <type_traits>

#ifdef VULKAN_HPP_USE_CXX_MODULE
#  include <vulkan/vulkan_hpp_macros.hpp>
import vulkan;
#else
#  include <vulkan/vulkan.hpp>
#endif

static_assert( std::is_base_of<std::system_error, vk::SystemError>::value, "vk::SystemError must remain a std::system_error" );
static_assert( std::is_base_of<vk::SystemError, vk::DeviceLostError>::value, "result-specific exceptions must retain their hierarchy" );

void throwDeviceLost()
{
  vk::detail::resultCheck( vk::Result::eErrorDeviceLost, "vkTest" );
}

int main()
{
  bool caughtTyped = false;
  try
  {
    throwDeviceLost();
  }
  catch ( vk::DeviceLostError const & error )
  {
    caughtTyped = true;
    release_assert( error.code() == vk::make_error_code( vk::Result::eErrorDeviceLost ) );
    release_assert( std::string( error.message() ).find( "vkTest" ) != std::string::npos );
    release_assert( std::string( error.what() ).find( "vkTest" ) != std::string::npos );
    release_assert( !error.trace().frames.empty() );
    bool hasResolvedSymbols = false;
    bool foundCallSite = false;
    for ( auto const & frame : error.trace().frames )
    {
      hasResolvedSymbols |= !frame.symbol.empty();
      foundCallSite |= frame.symbol == "main";
    }
    release_assert( !hasResolvedSymbols || foundCallSite );
  }
  release_assert( caughtTyped );

  bool caughtStandard = false;
  try
  {
    throwDeviceLost();
  }
  catch ( std::system_error const & )
  {
    caughtStandard = true;
  }
  release_assert( caughtStandard );
}
