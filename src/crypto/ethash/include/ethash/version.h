// Copyright (c) 2021-2022 The Raven Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

/** The ethash library version. */
#define KAWPOW_VERSION "0.5.1"

#ifdef __cplusplus
namespace ethash
{
/// The ethash library version.
constexpr auto version = KAWPOW_VERSION;

}  // namespace ethash
#endif
