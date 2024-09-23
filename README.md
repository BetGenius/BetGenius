BetGenius Core integration/staging tree
=====================================

https://betgenius.cc

What is BetGenius Core?
---------------------

BetGenius Core (BETG) is the native token powering the BetGenius platform, designed to enhance user experience and engagement. BETG serves multiple functions, including:

- Placing Bets: Use BETG to wager on crypto price predictions and other betting features within the platform.
- Rewards & Payouts: Earn BETG as rewards for winning bets, with instant payouts directly to your wallet.
- Exclusive Features: Access premium features, higher-tier rewards, and special promotions using BETG.
- Staking & Benefits: Stake BETG to unlock additional benefits, such as enhanced rewards and participation in governance decisions.

Important Parameters
---------------------
- Algorithm: KAWPoW
- Block time: 60 seconds
- Retargeting: DGW V3
- Maximum supply: 21B BETG
- Premine: 21M BETG (~0.1%)
- Block reward: 5000 BETG

The block reward employs a halving mechanism every 2,102,400 blocks, reducing by a factor of two at each interval. After the fifth halving, the reward will stabilize at 156.25 BETG
per block, remaining constant until the maximum supply is fully mined.

Further information about BetGenius Core is available in the [doc folder](/doc).

License
-------

BetGenius Core is released under the terms of the MIT license. See [COPYING](COPYING) for more
information or see https://opensource.org/licenses/MIT.

Development Process
-------------------

The `master` branch is regularly built (see `doc/build-*.md` for instructions) and tested, but it is not guaranteed to be
completely stable. [Tags](https://github.com/BetGenius/BetGenius/tags) are created
regularly from release branches to indicate new official, stable release versions of BetGenius Core.

The contribution workflow is described in [CONTRIBUTING.md](CONTRIBUTING.md)
and useful hints for developers can be found in [doc/developer-notes.md](doc/developer-notes.md).

Testing
-------

Testing and code review is the bottleneck for development; we get more pull
requests than we can review and test on short notice. Please be patient and help out by testing
other people's pull requests, and remember this is a security-critical project where any mistake might cost people
lots of money.

### Automated Testing

Developers are strongly encouraged to write [unit tests](src/test/README.md) for new code, and to
submit new unit tests for old code. Unit tests can be compiled and run
(assuming they weren't disabled in configure) with: `make check`. Further details on running
and extending unit tests can be found in [/src/test/README.md](/src/test/README.md).

There are also [regression and integration tests](/test), written
in Python.
These tests can be run (if the [test dependencies](/test) are installed) with: `test/functional/test_runner.py`

The CI (Continuous Integration) systems make sure that every pull request is built for Windows, Linux, and macOS,
and that unit/sanity tests are run automatically.

### Manual Quality Assurance (QA) Testing

Changes should be tested by somebody other than the developer who wrote the
code. This is especially important for large or high-risk changes. It is useful
to add a test plan to the pull request description if testing the changes is
not straightforward.
