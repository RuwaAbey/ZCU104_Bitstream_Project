# ZCU104 FPGA Bitstream & GPIO Demo

A Vivado/Vitis 2024.2 project for experimenting with FPGA bitstream loading and GPIO observation on the **AMD Zynq UltraScale+ ZCU104** development board. The repository includes the Vivado hardware project, Vitis-related sources, a device-tree overlay source, and scripts for transferring files and running a dynamic PL reconfiguration demonstration.

## What the demo does

The included `Automate_scripts/loading.sh` script is designed to run on a compatible Linux image on the ZCU104. It:

1. Copies a wrapper configuration binary, a device-tree overlay, and two additional bitstream files into `/lib/firmware`.
2. Loads the wrapper binary and overlay using `fpgautil`.
3. Finds a GPIO controller whose sysfs label contains `80000000.gpio`.
4. Exports four GPIO lines and reads their values.
5. Loads two subsequent bitstream files, reading the four GPIO values after each load.

**Compatibility note:** The script assumes `fpgautil`, legacy `/sys/class/gpio` access, and the expected GPIO controller are available. Reconfiguration behavior depends on the board's hardware design and Linux image. Test on your target setup before relying on the script.

## Development environment

- Hardware: ZCU104 development board
- FPGA tools: Vivado / Vitis **2024.2**
- Target runtime: compatible Linux image with `fpgautil`
- Host tools for release verification: `unzip`, `sha256sum`

## Repository layout

```text
ZCU104_Bitstream_Project/
├── Automate_scripts/     # Loading and SCP helper scripts
├── GPIO_Project/         # Vivado hardware project (GPIO_Project.xpr)
├── Linux_GPIO/           # Vitis-related GPIO platform/application files
├── linux_bitstream/      # Vitis-related bitstream application files
├── Overlay/              # Device-tree overlay source (pl.dtsi)
├── Bin_gen_folder/       # Bitstream generation inputs and local outputs
├── Bitstream_files/      # Local bitstream artifacts (release assets)
├── Demo_files/           # Local demo binaries (release assets)
└── dist/                 # Local release staging area (gitignored)
```

Generated Vivado build directories and compiled artifacts are intentionally excluded from normal Git commits. **Prebuilt binaries are distributed through GitHub Releases.**

## Download a prebuilt release

Open the repository's **Releases** page and download both:

- `ZCU104-GPIO-v1.0.0.zip`
- `SHA256SUMS.txt`

Verify the archive before extracting:

```bash
sha256sum -c SHA256SUMS.txt
unzip ZCU104-GPIO-v1.0.0.zip
```

Expected verification output:

```text
ZCU104-GPIO-v1.0.0.zip: OK
```

The planned release archive contains:

```text
ZCU104-GPIO-v1.0.0/
├── bitstreams/
│   ├── Bitstream_1.bit
│   ├── Bitstream_2.bit
│   └── Design_GPIO_wrapper.bin
├── applications/
│   └── linux_bit.elf
└── overlays/
    └── pl.dtbo
```

**Release contents may vary:** Check the actual ZIP before using it. No `BOOT.BIN`, `image.ub`, or `boot.scr` is included in this release; it is **not a complete bootable image**.

## Running the GPIO demonstration

The existing `loading.sh` takes four file paths:

```bash
sudo bash Automate_scripts/loading.sh \
  /path/to/Design_GPIO_wrapper.bin \
  /path/to/pl.dtbo \
  /path/to/Bitstream_1.bin \
  /path/to/Bitstream_2.bin
```

**Important:** The script expects `Bitstream_1.bin` and `Bitstream_2.bin`, while the planned release bundle currently lists `Bitstream_1.bit` and `Bitstream_2.bit`. These are not interchangeable merely by renaming them. Produce and include the correct `.bin` files for the target `fpgautil` flow before using this example. The source archive contains candidate `.bin` outputs under `Bin_gen_folder/` and `Demo_files/`; verify that they match the intended hardware configuration.

The script writes to `/lib/firmware` and interacts with FPGA hardware and sysfs GPIO, so it requires suitable privileges and should be run only on a compatible target.

## Rebuilding from source

1. Install Vivado/Vitis 2024.2 and the board support required for ZCU104.
2. Open `GPIO_Project/GPIO_Project.xpr` in Vivado.
3. Inspect the block design and constraints, then generate the bitstream for your intended configuration.
4. Use the appropriate Vitis and/or bitstream conversion workflow to produce deployment artifacts.
5. Build or validate the matching device-tree overlay before loading the design on Linux.

The precise build/export commands depend on your local toolchain and target Linux image; no fully automated reproducible build is claimed here.

## Creating a release bundle

After placing the verified files in `dist/ZCU104-GPIO-v1.0.0/`, run from the repository root:

```bash
cd dist
zip -r ZCU104-GPIO-v1.0.0.zip ZCU104-GPIO-v1.0.0/
sha256sum ZCU104-GPIO-v1.0.0.zip > SHA256SUMS.txt
sha256sum -c SHA256SUMS.txt
```

Upload **both** the ZIP and `SHA256SUMS.txt` to the same GitHub Release. Regenerate the checksum whenever the ZIP changes. The checksum detects accidental changes or corruption; it does not by itself authenticate the publisher.

