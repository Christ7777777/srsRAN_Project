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

#include "srsran/fapi/slot_last_message_notifier.h"
#include "srsran/fapi/slot_message_gateway.h"
#include "srsran/fapi_adaptor/mac/mac_fapi_adaptor.h"
#include "srsran/fapi_adaptor/precoding_matrix_mapper.h"
#include "srsran/ran/subcarrier_spacing.h"
#include <memory>

namespace srsran {
namespace fapi_adaptor {

/// Configuration parameters for the MAC&ndash;FAPI adaptor factory.
struct mac_fapi_adaptor_factory_config {
  mac_fapi_adaptor_factory_config(unsigned                                 sector_id_,
                                  subcarrier_spacing                       scs_,
                                  fapi::slot_message_gateway&              gateway_,
                                  fapi::slot_last_message_notifier&        last_msg_notifier_,
                                  std::unique_ptr<precoding_matrix_mapper> pm_mapper_,
                                  unsigned                                 cell_nof_prbs_) :
    sector_id(sector_id_),
    gateway(gateway_),
    last_msg_notifier(last_msg_notifier_),
    scs(scs_),
    pm_mapper(std::move(pm_mapper_)),
    cell_nof_prbs(cell_nof_prbs_)
  {
  }

  /// Base station sector identifier.
  unsigned sector_id;
  /// Slot-specific FAPI message gateway.
  std::reference_wrapper<fapi::slot_message_gateway> gateway;
  /// Slot-specific last message notifier.
  std::reference_wrapper<fapi::slot_last_message_notifier> last_msg_notifier;
  /// Subcarrier spacing as per TS38.331 Section 6.2.2.
  subcarrier_spacing scs;
  /// Precoding matrix mapper.
  std::unique_ptr<precoding_matrix_mapper> pm_mapper;
  /// Cell number of resource blocks.
  unsigned cell_nof_prbs;
};

/// Factory to create \c mac_fapi_adaptor objects.
class mac_fapi_adaptor_factory
{
public:
  virtual ~mac_fapi_adaptor_factory() = default;

  /// Creates a \c mac_fapi_adaptor object using the given configuration.
  virtual std::unique_ptr<mac_fapi_adaptor> create(mac_fapi_adaptor_factory_config&& config) = 0;
};

/// Creates a \c mac_fapi_adaptor_factory object.
std::unique_ptr<mac_fapi_adaptor_factory> create_mac_fapi_adaptor_factory();

} // namespace fapi_adaptor
} // namespace srsran
