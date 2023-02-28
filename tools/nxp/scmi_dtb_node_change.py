#! /usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
# Copyright 2023 NXP

"""This module can be used to switch from the active device-tree SIUL2 nodes
   to the SCMI ones."""

import argparse
import sys
import pathlib
import fdt

SIUL2_PINCTRL_NAME = "siul2-pinctrl@4009c240"
SIUL2_GPIO_NAME = "siul2-gpio@4009d700"
SCMI_PINCTRL_NAME = "protocol@80"
SCMI_GPIO_NAME = "protocol@81"


def switch_node(dtb, scmi_node_name, siul2_node_name):
    """Perform the actual switch."""

    nodes = dtb.search(siul2_node_name, itype=fdt.ItemType.NODE)
    if len(nodes) != 1:
        print("Can't find siul2 node!")
        sys.exit(1)

    siul2 = nodes[0]
    print("Found siul2 node!")

    nodes = dtb.search(scmi_node_name, itype=fdt.ItemType.NODE)
    if len(nodes) != 1:
        print("Can't find scmi node!")
        sys.exit(1)

    scmi = nodes[0]
    print("Found scmi node!")

    node_names = []
    for node in siul2.nodes:
        print("Copying subnode: ", node.name)
        node_names.append(node.name)
        scmi.append(node.copy())

    for name in node_names:
        siul2.remove_subnode(name)

    if siul2.get_property("phandle") is not None:
        scmi.set_property("phandle", siul2.get_property("phandle").value)
        siul2.remove_property("phandle")
    siul2.set_property("status", "disabled")
    scmi.set_property("status", "okay")


def main():
    """Main script function."""
    description = """Copy SIUL2 nodes' properties and subnodes into the SCMI
                     equivalent nodes. Disable SIUL2 nodes and enable SCMI
                     ones."""
    parser = argparse.ArgumentParser(description=description)
    parser.add_argument('dtb_file', metavar='dtb_file', type=pathlib.Path,
                        nargs=1, help="path to the dtb file to be processed")
    parser.add_argument('--pinctrl', action='store_true', help="use SCMI pinctrl protocol")
    parser.add_argument('--no-pinctrl', dest='pinctrl', action='store_false')
    parser.set_defaults(pinctrl=True)
    parser.add_argument('--gpio', action='store_true', help="use SCMI GPIO protocol")
    parser.add_argument('--no-gpio', dest='gpio', action='store_false')
    parser.set_defaults(gpio=True)
    args = parser.parse_args()

    dtb_file = args.dtb_file[0]
    print("Processing: ", dtb_file)

    data = None
    with open(dtb_file, "rb") as file:
        data = file.read()

    dtb = fdt.parse_dtb(data)

    if args.pinctrl:
        print("Enabling SCMI pinctrl")
        switch_node(dtb, SCMI_PINCTRL_NAME, SIUL2_PINCTRL_NAME)

    if args.gpio:
        print("Enabling SCMI GPIO")
        switch_node(dtb, SCMI_GPIO_NAME, SIUL2_GPIO_NAME)

    with open(dtb_file, "wb") as file:
        file.write(dtb.to_dtb())


if __name__ == "__main__":
    main()
