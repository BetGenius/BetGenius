// Copyright (c) 2011-2020 The Bitcoin Core developers
// Copyright (c) 2024 The Betgenius Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BETGENIUS_QT_BETGENIUSADDRESSVALIDATOR_H
#define BETGENIUS_QT_BETGENIUSADDRESSVALIDATOR_H

#include <QValidator>

/** Base58 entry widget validator, checks for valid characters and
 * removes some whitespace.
 */
class BetGeniusAddressEntryValidator : public QValidator
{
    Q_OBJECT

public:
    explicit BetGeniusAddressEntryValidator(QObject *parent);

    State validate(QString &input, int &pos) const override;
};

/** BetGenius address widget validator, checks for a valid betgenius address.
 */
class BetGeniusAddressCheckValidator : public QValidator
{
    Q_OBJECT

public:
    explicit BetGeniusAddressCheckValidator(QObject *parent);

    State validate(QString &input, int &pos) const override;
};

#endif // BETGENIUS_QT_BETGENIUSADDRESSVALIDATOR_H
