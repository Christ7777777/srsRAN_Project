/*
 *
 * Copyright 2021-2023 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#include "srsran/fapi_adaptor/precoding_matrix_repository.h"

using namespace srsran;
using namespace fapi_adaptor;

const precoding_configuration& precoding_matrix_repository::get_precoding_configuration(unsigned index) const
{
  srsran_assert(index < repo.size(), "Invalid precoding matrix index={}, repository size={}", index, repo.size());

  return repo[index];
}
