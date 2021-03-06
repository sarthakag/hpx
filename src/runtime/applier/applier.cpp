//  Copyright (c) 2007-2008 Anshul Tandon
//  Copyright (c) 2007-2017 Hartmut Kaiser
//  Copyright (c) 2011      Bryce Lelbach
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <hpx/config.hpp>
#include <hpx/assertion.hpp>
#include <hpx/basic_execution/register_locks.hpp>
#include <hpx/errors.hpp>
#include <hpx/runtime.hpp>
#include <hpx/runtime/actions/continuation.hpp>
#include <hpx/runtime/agas/interface.hpp>
#include <hpx/runtime/applier/applier.hpp>
#include <hpx/runtime/components/pinned_ptr.hpp>
#include <hpx/runtime/naming/resolver_client.hpp>
#include <hpx/runtime/parcelset/parcel.hpp>
#include <hpx/runtime/parcelset/parcelhandler.hpp>
#include <hpx/threading_base/thread_helpers.hpp>
#include <hpx/runtime/threads/threadmanager.hpp>
#include <hpx/threading_base/thread_description.hpp>

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <sstream>
#include <utility>
#include <vector>

namespace hpx { namespace applier {
#if defined(HPX_HAVE_NETWORKING)
    applier::applier(parcelset::parcelhandler& ph, threads::threadmanager& tm)
      : parcel_handler_(ph)
      , thread_manager_(tm)
    {
    }
#else
    applier::applier(threads::threadmanager& tm)
      : thread_manager_(tm)
    {
    }
#endif

    void applier::initialize(std::uint64_t rts, std::uint64_t mem)
    {
        naming::resolver_client& agas_client = get_agas_client();
        runtime_support_id_ =
            naming::id_type(agas_client.get_local_locality().get_msb(), rts,
                naming::id_type::unmanaged);
        memory_id_ = naming::id_type(agas_client.get_local_locality().get_msb(),
            mem, naming::id_type::unmanaged);
    }

    naming::resolver_client& applier::get_agas_client()
    {
        return hpx::naming::get_agas_client();
    }

#if defined(HPX_HAVE_NETWORKING)
    parcelset::parcelhandler& applier::get_parcel_handler()
    {
        return parcel_handler_;
    }
#endif

    threads::threadmanager& applier::get_thread_manager()
    {
        return thread_manager_;
    }

    naming::gid_type const& applier::get_raw_locality(error_code& ec) const
    {
        return hpx::naming::get_agas_client().get_local_locality(ec);
    }

    std::uint32_t applier::get_locality_id(error_code& ec) const
    {
        return naming::get_locality_id_from_gid(get_raw_locality(ec));
    }

    bool applier::get_raw_remote_localities(
        std::vector<naming::gid_type>& prefixes,
        components::component_type type, error_code& ec) const
    {
#if defined(HPX_HAVE_NETWORKING)
        return parcel_handler_.get_raw_remote_localities(prefixes, type, ec);
#else
        return true;
#endif
    }

    bool applier::get_remote_localities(std::vector<naming::id_type>& prefixes,
        components::component_type type, error_code& ec) const
    {
#if defined(HPX_HAVE_NETWORKING)
        std::vector<naming::gid_type> raw_prefixes;
        if (!parcel_handler_.get_raw_remote_localities(raw_prefixes, type, ec))
            return false;

        for (naming::gid_type& gid : raw_prefixes)
            prefixes.emplace_back(gid, naming::id_type::unmanaged);
#endif
        return true;
    }

    bool applier::get_raw_localities(std::vector<naming::gid_type>& prefixes,
        components::component_type type) const
    {
#if defined(HPX_HAVE_NETWORKING)
        return parcel_handler_.get_raw_localities(prefixes, type);
#else
        naming::gid_type id;
        naming::get_agas_client().get_console_locality(id);
        prefixes.emplace_back(id);
        return true;
#endif
    }

    bool applier::get_localities(
        std::vector<naming::id_type>& prefixes, error_code& ec) const
    {
        std::vector<naming::gid_type> raw_prefixes;
#if defined(HPX_HAVE_NETWORKING)
        if (!parcel_handler_.get_raw_localities(
                raw_prefixes, components::component_invalid, ec))
            return false;

        for (naming::gid_type& gid : raw_prefixes)
            prefixes.emplace_back(gid, naming::id_type::unmanaged);
#else
        prefixes.emplace_back(agas::get_console_locality());
#endif
        return true;
    }

    bool applier::get_localities(std::vector<naming::id_type>& prefixes,
        components::component_type type, error_code& ec) const
    {
#if defined(HPX_HAVE_NETWORKING)
        std::vector<naming::gid_type> raw_prefixes;
        if (!parcel_handler_.get_raw_localities(raw_prefixes, type, ec))
            return false;

        for (naming::gid_type& gid : raw_prefixes)
            prefixes.emplace_back(gid, naming::id_type::unmanaged);
#else
        prefixes.emplace_back(agas::get_console_locality());
#endif
        return true;
    }

    applier& get_applier()
    {
        return hpx::get_runtime().get_applier();
    }

    applier* get_applier_ptr()
    {
        return &hpx::get_runtime().get_applier();
    }
}}    // namespace hpx::applier
