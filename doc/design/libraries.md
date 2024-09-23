# Libraries

| Name                     | Description |
|--------------------------|-------------|
| *libbetgenius_cli*         | RPC client functionality used by *betgenius-cli* executable |
| *libbetgenius_common*      | Home for common functionality shared by different executables and libraries. Similar to *libbetgenius_util*, but higher-level (see [Dependencies](#dependencies)). |
| *libbetgenius_consensus*   | Stable, backwards-compatible consensus functionality used by *libbetgenius_node* and *libbetgenius_wallet* and also exposed as a [shared library](../shared-libraries.md). |
| *libbetgenius_kernel*      | Consensus engine and support library used for validation by *libbetgenius_node* and also exposed as a [shared library](../shared-libraries.md). |
| *libbetgeniusqt*           | GUI functionality used by *betgenius-qt* and *betgenius-gui* executables |
| *libbetgenius_ipc*         | IPC functionality used by *betgenius-node*, *betgenius-wallet*, *betgenius-gui* executables to communicate when [`--enable-multiprocess`](multiprocess.md) is used. |
| *libbetgenius_node*        | P2P and RPC server functionality used by *betgeniusd* and *betgenius-qt* executables. |
| *libbetgenius_util*        | Home for common functionality shared by different executables and libraries. Similar to *libbetgenius_common*, but lower-level (see [Dependencies](#dependencies)). |
| *libbetgenius_wallet*      | Wallet functionality used by *betgeniusd* and *betgenius-wallet* executables. |
| *libbetgenius_wallet_tool* | Lower-level wallet functionality used by *betgenius-wallet* executable. |
| *libbetgenius_zmq*         | [ZeroMQ](../zmq.md) functionality used by *betgeniusd* and *betgenius-qt* executables. |

## Conventions

- Most libraries are internal libraries and have APIs which are completely unstable! There are few or no restrictions on backwards compatibility or rules about external dependencies. Exceptions are *libbetgenius_consensus* and *libbetgenius_kernel* which have external interfaces documented at [../shared-libraries.md](../shared-libraries.md).

- Generally each library should have a corresponding source directory and namespace. Source code organization is a work in progress, so it is true that some namespaces are applied inconsistently, and if you look at [`libbetgenius_*_SOURCES`](../../src/Makefile.am) lists you can see that many libraries pull in files from outside their source directory. But when working with libraries, it is good to follow a consistent pattern like:

  - *libbetgenius_node* code lives in `src/node/` in the `node::` namespace
  - *libbetgenius_wallet* code lives in `src/wallet/` in the `wallet::` namespace
  - *libbetgenius_ipc* code lives in `src/ipc/` in the `ipc::` namespace
  - *libbetgenius_util* code lives in `src/util/` in the `util::` namespace
  - *libbetgenius_consensus* code lives in `src/consensus/` in the `Consensus::` namespace

## Dependencies

- Libraries should minimize what other libraries they depend on, and only reference symbols following the arrows shown in the dependency graph below:

<table><tr><td>

```mermaid

%%{ init : { "flowchart" : { "curve" : "basis" }}}%%

graph TD;

betgenius-cli[betgenius-cli]-->libbetgenius_cli;

betgeniusd[betgeniusd]-->libbetgenius_node;
betgeniusd[betgeniusd]-->libbetgenius_wallet;

betgenius-qt[betgenius-qt]-->libbetgenius_node;
betgenius-qt[betgenius-qt]-->libbetgeniusqt;
betgenius-qt[betgenius-qt]-->libbetgenius_wallet;

betgenius-wallet[betgenius-wallet]-->libbetgenius_wallet;
betgenius-wallet[betgenius-wallet]-->libbetgenius_wallet_tool;

libbetgenius_cli-->libbetgenius_util;
libbetgenius_cli-->libbetgenius_common;

libbetgenius_common-->libbetgenius_consensus;
libbetgenius_common-->libbetgenius_util;

libbetgenius_kernel-->libbetgenius_consensus;
libbetgenius_kernel-->libbetgenius_util;

libbetgenius_node-->libbetgenius_consensus;
libbetgenius_node-->libbetgenius_kernel;
libbetgenius_node-->libbetgenius_common;
libbetgenius_node-->libbetgenius_util;

libbetgeniusqt-->libbetgenius_common;
libbetgeniusqt-->libbetgenius_util;

libbetgenius_wallet-->libbetgenius_common;
libbetgenius_wallet-->libbetgenius_util;

libbetgenius_wallet_tool-->libbetgenius_wallet;
libbetgenius_wallet_tool-->libbetgenius_util;

classDef bold stroke-width:2px, font-weight:bold, font-size: smaller;
class betgenius-qt,betgeniusd,betgenius-cli,betgenius-wallet bold
```
</td></tr><tr><td>

**Dependency graph**. Arrows show linker symbol dependencies. *Consensus* lib depends on nothing. *Util* lib is depended on by everything. *Kernel* lib depends only on consensus and util.

</td></tr></table>

- The graph shows what _linker symbols_ (functions and variables) from each library other libraries can call and reference directly, but it is not a call graph. For example, there is no arrow connecting *libbetgenius_wallet* and *libbetgenius_node* libraries, because these libraries are intended to be modular and not depend on each other's internal implementation details. But wallet code is still able to call node code indirectly through the `interfaces::Chain` abstract class in [`interfaces/chain.h`](../../src/interfaces/chain.h) and node code calls wallet code through the `interfaces::ChainClient` and `interfaces::Chain::Notifications` abstract classes in the same file. In general, defining abstract classes in [`src/interfaces/`](../../src/interfaces/) can be a convenient way of avoiding unwanted direct dependencies or circular dependencies between libraries.

- *libbetgenius_consensus* should be a standalone dependency that any library can depend on, and it should not depend on any other libraries itself.

- *libbetgenius_util* should also be a standalone dependency that any library can depend on, and it should not depend on other internal libraries.

- *libbetgenius_common* should serve a similar function as *libbetgenius_util* and be a place for miscellaneous code used by various daemon, GUI, and CLI applications and libraries to live. It should not depend on anything other than *libbetgenius_util* and *libbetgenius_consensus*. The boundary between _util_ and _common_ is a little fuzzy but historically _util_ has been used for more generic, lower-level things like parsing hex, and _common_ has been used for betgenius-specific, higher-level things like parsing base58. The difference between util and common is mostly important because *libbetgenius_kernel* is not supposed to depend on *libbetgenius_common*, only *libbetgenius_util*. In general, if it is ever unclear whether it is better to add code to *util* or *common*, it is probably better to add it to *common* unless it is very generically useful or useful particularly to include in the kernel.


- *libbetgenius_kernel* should only depend on *libbetgenius_util* and *libbetgenius_consensus*.

- The only thing that should depend on *libbetgenius_kernel* internally should be *libbetgenius_node*. GUI and wallet libraries *libbetgeniusqt* and *libbetgenius_wallet* in particular should not depend on *libbetgenius_kernel* and the unneeded functionality it would pull in, like block validation. To the extent that GUI and wallet code need scripting and signing functionality, they should be get able it from *libbetgenius_consensus*, *libbetgenius_common*, and *libbetgenius_util*, instead of *libbetgenius_kernel*.

- GUI, node, and wallet code internal implementations should all be independent of each other, and the *libbetgeniusqt*, *libbetgenius_node*, *libbetgenius_wallet* libraries should never reference each other's symbols. They should only call each other through [`src/interfaces/`](`../../src/interfaces/`) abstract interfaces.

## Work in progress

- Validation code is moving from *libbetgenius_node* to *libbetgenius_kernel* as part of [The libbetgeniuskernel Project #24303](https://github.com/BetGenius/BetGenius/issues/24303)
- Source code organization is discussed in general in [Library source code organization #15732](https://github.com/BetGenius/BetGenius/issues/15732)
