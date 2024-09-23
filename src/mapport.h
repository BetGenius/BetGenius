// Copyright (c) 2011-2020 The Bitcoin Core developers
// Copyright (c) 2024 The Betgenius Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BETGENIUS_MAPPORT_H
#define BETGENIUS_MAPPORT_H

static constexpr bool DEFAULT_UPNP = false;

static constexpr bool DEFAULT_NATPMP = false;

enum MapPortProtoFlag : unsigned int {
    NONE = 0x00,
    UPNP = 0x01,
    NAT_PMP = 0x02,
};

void StartMapPort(bool use_upnp, bool use_natpmp);
void InterruptMapPort();
void StopMapPort();

#endif // BETGENIUS_MAPPORT_H
