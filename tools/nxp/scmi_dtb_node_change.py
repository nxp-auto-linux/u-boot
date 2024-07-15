#! /usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
# Copyright 2023-2024 NXP

"""This script does the required DT changes for switching to SCMI-based GPIO,
   Pinctrl and NVMEM drivers."""

import argparse
import sys
import pathlib
import fdt

SIUL2_PINCTRL_NAME = "siul2-pinctrl@4009c240"
SIUL2_GPIO_NAME = "siul2-gpio@4009d700"
SCMI_PINCTRL_NAME = "protocol@80"
SCMI_GPIO_NAME = "protocol@81"
SCMI_NVMEM_NAME = "protocol@82"

""" Disable list containing readable and DT name node pairs"""
disabled_dt_nodes = [
    ("MMIO MUX", "mux-controller@f0"),
    ("I2C2", "i2c@401ec000"),
    ("I2C4", "i2c@402dc000"),
    ("SPI1", "spi@401d8000"),
]

def log_step(msg):
    """Log an indented message (an execution step of the script)."""
    print("\t{0}".format(msg))

def switch_node(dtb, scmi_node_name, siul2_node_name):
    """Perform the actual switch."""

    nodes = dtb.search(siul2_node_name, itype=fdt.ItemType.NODE)
    if len(nodes) != 1:
        log_step("Can't find siul2 node!")
        sys.exit(1)

    siul2 = nodes[0]
    log_step("Found siul2 node!")

    nodes = dtb.search(scmi_node_name, itype=fdt.ItemType.NODE)
    if len(nodes) != 1:
        log_step("Can't find scmi node!")
        sys.exit(1)

    scmi = nodes[0]
    log_step("Found scmi node!")

    node_names = []
    for node in siul2.nodes:
        log_step("Copying subnode: " + node.name)
        node_names.append(node.name)
        scmi.append(node.copy())

    for name in node_names:
        siul2.remove_subnode(name)

    if siul2.get_property("phandle") is not None:
        scmi.set_property("phandle", siul2.get_property("phandle").value)
        siul2.remove_property("phandle")
    siul2.set_property("status", "disabled")
    scmi.set_property("status", "okay")

def update_nvmem_consumers(dtb, scmi_node_name):
    """ Update the NVMEM consumers nodes' to use the SCMI NVMEM node. """

    scmi_node = dtb.search(scmi_node_name, itype=fdt.ItemType.NODE)
    if len(scmi_node) != 1:
        log_step("Can't find SCMI NVMEM node!")
        sys.exit(1)

    # Get the SCMI NVMEM node and its NVMEM cells
    scmi_node = scmi_node[0]
    log_step("Found scmi node!")
    scmi_nvmem_cells = scmi_node.get_property("nvmem-cells")
    scmi_nvmem_cell_names = scmi_node.get_property("nvmem-cell-names")
    if len(scmi_nvmem_cells) != len(scmi_nvmem_cell_names):
        log_step("SCMI NVMEM Node: Number of 'nvmem-cells' and 'nvmem-cell-names' does not match!")
        sys.exit(1)

    scmi_nvmem_phandles = dict(zip(scmi_nvmem_cell_names, scmi_nvmem_cells))

    # Get the NVMEM consumers nodes (all nodes which contain "nvmem-cells")
    nvmem_consumers = dtb.search(name="nvmem-cells",
                                 itype=fdt.ItemType.PROP_WORDS)
    nvmem_consumers = [prop.parent for prop in nvmem_consumers
                       if prop.parent.name != scmi_node_name]

    # Update the NVMEM consumers nodes
    for nvmem_consumer in nvmem_consumers:
        log_step("Updating \'" + nvmem_consumer.name + "\' to use SCMI NVMEM where possible")

        nvmem_cells = nvmem_consumer.get_property("nvmem-cells")
        nvmem_cell_names = nvmem_consumer.get_property("nvmem-cell-names")
        if len(nvmem_cells) != len(nvmem_cell_names):
            log_step(nvmem_consumer.name +
                     ": Number of 'nvmem-cells' and 'nvmem-cell-names' does not match!")
            sys.exit(1)

        # Replace the phandles of the NVMEM cells if corresponding
        # ones exist in the SCMI NVMEM node
        new_phandles = [scmi_nvmem_phandles[cell_name] if cell_name in scmi_nvmem_phandles
                        else ph for (cell_name, ph) in zip(nvmem_cell_names, nvmem_cells)]

        nvmem_consumer.set_property("nvmem-cells", new_phandles)
        log_step("\t" + str(nvmem_cells) + " -> " + str(new_phandles))

    scmi_node.set_property("status", "okay")

def disable_generic_node(dtb, in_node):
    """ Disable a given device tree node.
        Node description must be aded to log_names dictionary."""

    log_name, dt_name = in_node
    print("Disabling " + log_name + " node")

    nodes = dtb.search(dt_name, itype=fdt.ItemType.NODE)
    if len(nodes) != 1:
        log_step("Error: can't find " + log_name + " node!")
        return

    node = nodes[0]
    log_step("Found " + log_name + " node: " + node.path + "/" + node.name)

    node.set_property("status", "disabled")
    log_step("Disabled " + log_name + " node!")

def main():
    """Main script function."""
    description = """This script does the required DT changes for switching to
                     SCMI-based GPIO, Pinctrl and NVMEM drivers:
                     \t- GPIO and Pinctrl: Copy SIUL2 nodes' properties and subnodes
                     into the SCMI equivalent nodes. Disable SIUL2 nodes and enable SCMI
                     ones.
                     \t- NVMEM: Update the phandles of NVMEM cells in NVMEM consumer
                     nodes to corresponding phandles of NVMEM cells from SCMI NVMEM node.
                     Enable the SCMI NVMEM node.
                     """
    parser = argparse.ArgumentParser(description=description)
    parser.add_argument('dtb_file', metavar='dtb_file', type=pathlib.Path,
                        nargs=1, help="path to the dtb file to be processed")
    parser.add_argument('--pinctrl', action='store_true', help="use SCMI pinctrl protocol")
    parser.add_argument('--no-pinctrl', dest='pinctrl', action='store_false')
    parser.set_defaults(pinctrl=True)
    parser.add_argument('--gpio', action='store_true', help="use SCMI GPIO protocol")
    parser.add_argument('--no-gpio', dest='gpio', action='store_false')
    parser.set_defaults(gpio=True)
    parser.add_argument('--nvmem', action='store_true', help="use SCMI NVMEM protocol")
    parser.add_argument('--no-nvmem', dest='nvmem', action='store_false')
    parser.set_defaults(nvmem=True)
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

    if args.nvmem:
        print("Enabling SCMI NVMEM")
        update_nvmem_consumers(dtb, SCMI_NVMEM_NAME)

    for node in disabled_dt_nodes:
        disable_generic_node(dtb, node)

    with open(dtb_file, "wb") as file:
        file.write(dtb.to_dtb())


if __name__ == "__main__":
    main()
