# Copyright (c) 2015-2021 Clearmatics Technologies Ltd
#
# SPDX-License-Identifier: LGPL-3.0+

from zeth.cli.utils import create_mixer_client_and_mixer_desc, \
    load_zeth_address, open_wallet, parse_output, do_sync, load_eth_address, \
    load_eth_private_key, zeth_note_short_print, create_prover_client
from zeth.core.mixer_client import MixParameters
from zeth.core.utils import EtherValue, from_zeth_units
from click import command, option, pass_context, ClickException, Context
from typing import Optional
import json


@command()
@option("--eth-addr", help="Sender's eth address or address filename")
@option("--eth-private-key", help="Sender's eth private key file")
@pass_context
def mix_send_tx(
        ctx: Context,
        eth_addr: Optional[str],
        eth_private_key: Optional[str],
        ) -> None:
    """
    Generic mix function
    """
    client_ctx = ctx.obj
    prover_client = create_prover_client(client_ctx)
    zeth_client, mixer_desc = create_mixer_client_and_mixer_desc(
        client_ctx, prover_client)
    zeth_address = load_zeth_address(client_ctx)
    wallet = open_wallet(
        zeth_client.mixer_instance, zeth_address.addr_sk, client_ctx)

    with open("mix_params", "r") as mix_params_f:
        mix_params = MixParameters.from_json(prover_client.get_zksnark_provider(), json.dumps(json.load(mix_params_f)))
    with open("mix_vin", "r") as mix_vin_f:
        vin = mix_vin_f.read()
        tx_value = EtherValue(vin)
    tx_hash = zeth_client.mix(
        mix_params=mix_params,
        sender_eth_address=load_eth_address(eth_addr),
        sender_eth_private_key=load_eth_private_key(eth_private_key),
        tx_value=tx_value)

    do_sync(zeth_client.web3, wallet, prover_client.get_configuration().pairing_parameters, tx_hash, zeth_note_short_print)
