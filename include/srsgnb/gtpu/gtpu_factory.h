/*
 *
 * Copyright 2013-2022 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#pragma once

#include "srsgnb/gtpu/gtpu_config.h"
#include "srsgnb/gtpu/gtpu_entity.h"
#include <memory>

/// This factory header file depends on the GTP-U entity interfaces (see above include list). It is kept separate as
/// clients of the GTP-U entity interfaces do not need to call factory methods.
namespace srsgnb {

struct gtpu_entity_creation_message {
  gtpu_config                   cfg;
  gtpu_rx_lower_layer_notifier* rx_lower;
  gtpu_tx_upper_layer_notifier* tx_upper;
};

/// Creates an instance of a GTP-U entity.
std::unique_ptr<gtpu_entity> create_gtpu_entity(gtpu_entity_creation_message& msg);

} // namespace srsgnb
