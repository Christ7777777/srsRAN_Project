/*
 *
 * Copyright 2021-2023 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#pragma once

#include "f1ap_cu_ue_transaction_manager.h"
#include "srsran/f1ap/common/f1ap_ue_id.h"
#include "srsran/f1ap/cu_cp/f1ap_cu.h"
#include <unordered_map>

namespace srsran {
namespace srs_cu_cp {

struct f1ap_ue_context {
  const ue_index_t           ue_index           = ue_index_t::invalid;
  const gnb_cu_ue_f1ap_id_t  cu_ue_f1ap_id      = gnb_cu_ue_f1ap_id_t::invalid;
  gnb_du_ue_f1ap_id_t        du_ue_f1ap_id      = gnb_du_ue_f1ap_id_t::invalid;
  f1ap_rrc_message_notifier* rrc_notifier       = nullptr;
  bool                       marked_for_release = false;
  /// Whether the old gNB-DU UE F1AP UE ID IE needs to be notified back to the DU, due to reestablishment.
  optional<gnb_du_ue_f1ap_id_t> pending_old_ue_id;

  f1ap_ue_transaction_manager ev_mng;

  f1ap_ue_context(ue_index_t ue_idx_, gnb_cu_ue_f1ap_id_t cu_ue_f1ap_id_, timer_factory timers_) :
    ue_index(ue_idx_), cu_ue_f1ap_id(cu_ue_f1ap_id_), ev_mng(timers_)
  {
  }
};

class f1ap_ue_context_list
{
public:
  f1ap_ue_context_list(timer_factory timers_, srslog::basic_logger& logger_) : timers(timers_), logger(logger_) {}

  bool contains(gnb_cu_ue_f1ap_id_t cu_ue_id) const { return ues.find(cu_ue_id) != ues.end(); }

  /// \brief Checks whether a UE with the given UE index exists.
  /// \param[in] ue_index The UE index used to find the UE.
  /// \return The CU UE ID.
  bool contains(ue_index_t ue_idx) const
  {
    if (ue_index_to_ue_f1ap_id.find(ue_idx) == ue_index_to_ue_f1ap_id.end()) {
      return false;
    }
    if (ues.find(ue_index_to_ue_f1ap_id.at(ue_idx)) == ues.end()) {
      return false;
    }
    return true;
  }

  f1ap_ue_context& operator[](gnb_cu_ue_f1ap_id_t cu_ue_id)
  {
    srsran_assert(ues.find(cu_ue_id) != ues.end(), "cu_ue_f1ap_id={}: F1AP UE context not found", cu_ue_id);
    return ues.at(cu_ue_id);
  }
  f1ap_ue_context& operator[](ue_index_t ue_idx)
  {
    srsran_assert(ue_index_to_ue_f1ap_id.find(ue_idx) != ue_index_to_ue_f1ap_id.end(),
                  "ue={} gNB-CU-UE-F1AP-ID not found",
                  ue_idx);
    srsran_assert(ues.find(ue_index_to_ue_f1ap_id.at(ue_idx)) != ues.end(),
                  "cu_ue_f1ap_id={}: F1AP UE context not found",
                  ue_index_to_ue_f1ap_id.at(ue_idx));
    return ues.at(ue_index_to_ue_f1ap_id.at(ue_idx));
  }

  const f1ap_ue_context* find(gnb_du_ue_f1ap_id_t du_ue_id) const
  {
    auto it = std::find_if(
        ues.begin(), ues.end(), [du_ue_id](const std::pair<const gnb_cu_ue_f1ap_id_t, f1ap_ue_context>& u) {
          return u.second.du_ue_f1ap_id == du_ue_id;
        });
    return it != ues.end() ? &it->second : nullptr;
  }

  f1ap_ue_context& add_ue(ue_index_t ue_idx, gnb_cu_ue_f1ap_id_t cu_ue_id)
  {
    logger.debug("ue={} cu_ue_f1ap_id={}: Adding F1AP UE context", ue_idx, cu_ue_id);
    ues.emplace(
        std::piecewise_construct, std::forward_as_tuple(cu_ue_id), std::forward_as_tuple(ue_idx, cu_ue_id, timers));
    ue_index_to_ue_f1ap_id.emplace(ue_idx, cu_ue_id);
    return ues.at(cu_ue_id);
  }

  void remove_ue(ue_index_t ue_index)
  {
    srsran_assert(ue_index_to_ue_f1ap_id.find(ue_index) != ue_index_to_ue_f1ap_id.end(),
                  "ue={}: gNB-CU-UE-F1AP-ID not found",
                  ue_index);

    gnb_cu_ue_f1ap_id_t cu_ue_id = ue_index_to_ue_f1ap_id.at(ue_index);

    srsran_assert(ues.find(cu_ue_id) != ues.end(), "cu_ue_f1ap_id={}: F1AP UE context not found", cu_ue_id);
    logger.debug("ue={} cu_ue_f1ap_id={}: Removing F1AP UE context", ue_index, cu_ue_id);
    ue_index_to_ue_f1ap_id.erase(ue_index);
    ues.erase(cu_ue_id);
  }

  void add_rrc_notifier(ue_index_t ue_index, f1ap_rrc_message_notifier* notifier)
  {
    srsran_assert(ue_index_to_ue_f1ap_id.find(ue_index) != ue_index_to_ue_f1ap_id.end(),
                  "ue={}: gNB-CU-UE-F1AP-ID not found",
                  ue_index);
    srsran_assert(ues.find(ue_index_to_ue_f1ap_id.at(ue_index)) != ues.end(),
                  "cu_ue_f1ap_id={}: F1AP UE context not found",
                  ue_index_to_ue_f1ap_id.at(ue_index));
    ues.at(ue_index_to_ue_f1ap_id.at(ue_index)).rrc_notifier = notifier;
  }

  size_t size() const { return ues.size(); }

  /// \brief Get the next available GNB-CU-F1AP-UE-ID.
  gnb_cu_ue_f1ap_id_t next_gnb_cu_ue_f1ap_id()
  {
    // return invalid when no cu ue f1ap id is available
    if (ue_index_to_ue_f1ap_id.size() == MAX_NOF_UES_PER_DU) {
      return gnb_cu_ue_f1ap_id_t::invalid;
    }

    // iterate over all ids starting with the next_cu_ue_f1ap_id to find the available id
    while (true) {
      // Only iterate over ue_index_to_ue_f1ap_id (size=MAX_NOF_UES_PER_DU)
      // to avoid iterating over all possible values of gnb_cu_ue_f1ap_id_t (size=2^32-1)
      auto it = std::find_if(ue_index_to_ue_f1ap_id.begin(), ue_index_to_ue_f1ap_id.end(), [this](auto& u) {
        return u.second == next_cu_ue_f1ap_id;
      });

      // return the id if it is not already used
      if (it == ue_index_to_ue_f1ap_id.end()) {
        gnb_cu_ue_f1ap_id_t ret = next_cu_ue_f1ap_id;
        // increase the next cu ue f1ap id
        increase_next_cu_ue_f1ap_id();
        return ret;
      }

      // increase the next cu ue f1ap id and try again
      increase_next_cu_ue_f1ap_id();
    }

    return gnb_cu_ue_f1ap_id_t::invalid;
  }

protected:
  gnb_cu_ue_f1ap_id_t next_cu_ue_f1ap_id = gnb_cu_ue_f1ap_id_t::min;

private:
  timer_factory         timers;
  srslog::basic_logger& logger;

  inline void increase_next_cu_ue_f1ap_id()
  {
    if (next_cu_ue_f1ap_id == gnb_cu_ue_f1ap_id_t::max) {
      // reset cu ue f1ap id counter
      next_cu_ue_f1ap_id = gnb_cu_ue_f1ap_id_t::min;
    } else {
      // increase cu ue f1ap id counter
      next_cu_ue_f1ap_id = int_to_gnb_cu_ue_f1ap_id(gnb_cu_ue_f1ap_id_to_uint(next_cu_ue_f1ap_id) + 1);
    }
  }

  // Note: Given that UEs will self-remove from the map, we don't want to destructor to clear the lookups beforehand.
  std::unordered_map<ue_index_t, gnb_cu_ue_f1ap_id_t>      ue_index_to_ue_f1ap_id; // indexed by ue_index
  std::unordered_map<gnb_cu_ue_f1ap_id_t, f1ap_ue_context> ues;                    // indexed by gnb_cu_ue_f1ap_id
};

} // namespace srs_cu_cp
} // namespace srsran