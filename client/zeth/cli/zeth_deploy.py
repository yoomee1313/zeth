# Copyright (c) 2015-2021 Clearmatics Technologies Ltd
#
# SPDX-License-Identifier: LGPL-3.0+

from zeth.cli.constants import INSTANCE_FILE_DEFAULT
from zeth.cli.utils import \
    open_web3_from_ctx, get_erc20_instance_description, load_eth_address, \
    load_eth_private_key, write_mixer_description, MixerDescription, \
    create_prover_client, load_contract_address
from zeth.core.mixer_client import MixerClient
from click import Context, command, option, pass_context, ClickException
from typing import Optional


@command()
@option("--eth-addr", help="Sender eth address or address filename")
@option("--eth-private-key", help="Sender's eth private key file")
@option(
    "--instance-out",
    default=INSTANCE_FILE_DEFAULT,
    help=f"File to write deployment address to (default={INSTANCE_FILE_DEFAULT})")
@option("--token-address", help="Address of token contract (if used)")
@option(
    "--permitted-dispatcher",
    help="Instance file or address of contract permitted to call dispatch method")
@option("--vk-hash", type=str, help="verification key hash for dispatch calls")
@option("--deploy-gas", type=int, help="Maximum gas, in Wei")
@pass_context
def deploy(
        ctx: Context,
        eth_addr: Optional[str],
        eth_private_key: Optional[str],
        instance_out: str,
        token_address: Optional[str],
        permitted_dispatcher: Optional[str],
        vk_hash: Optional[str],
        deploy_gas: Optional[int]) -> None:
    """
    Deploy the zeth contracts and record the instantiation details.
    """
    eth_address = load_eth_address(eth_addr)
    eth_private_key_data = load_eth_private_key(eth_private_key)
    client_ctx = ctx.obj
    web3 = open_web3_from_ctx(client_ctx)

    if bool(permitted_dispatcher) != bool(vk_hash):
        raise ClickException(
            "Must supply BOTH --permitted-dispatch AND --vk-hash, or NEITHER")

    print(f"deploy: eth_address={eth_address}")
    print(f"deploy: instance_out={instance_out}")
    print(f"deploy: token_address={token_address}")

    if permitted_dispatcher:
        permitted_dispatcher = load_contract_address(permitted_dispatcher)
        print(f"deploy: permitted_dispatcher={permitted_dispatcher}")
        print(f"deploy: vk_hash={vk_hash}")

    token_instance_desc = get_erc20_instance_description(token_address) \
        if token_address else None

    prover_client = create_prover_client(client_ctx)
    _zeth_client, mixer_instance_desc = MixerClient.deploy(
        web3,
        prover_client,
        eth_address,
        eth_private_key_data,
        token_address,
        permitted_dispatcher=permitted_dispatcher,
        vk_hash=vk_hash,
        deploy_gas=deploy_gas)

    mixer_desc = MixerDescription(
        mixer=mixer_instance_desc,
        token=token_instance_desc,
        permitted_dispatcher=permitted_dispatcher,
        vk_hash=vk_hash)
    write_mixer_description(instance_out, mixer_desc)
