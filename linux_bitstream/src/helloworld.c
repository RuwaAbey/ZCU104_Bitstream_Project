#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>

#define FW_DIR "/lib/firmware"
#define GPIO_LABEL "80000000.gpio"
#define GPIO_COUNT 4

// Run a system command and check for errors
void run_cmd(const char *cmd) {
    printf(">>> %s\n", cmd);
    int ret = system(cmd);
    if (ret != 0) {
        fprintf(stderr, "Command failed: %s\n", cmd);
        exit(1);
    }
}

// Detect PL GPIO base dynamically
int get_pl_gpio_base() {
    DIR *d;
    struct dirent *dir;
    d = opendir("/sys/class/gpio/");
    if (!d) return -1;

    char path[256], label[64];
    int base = -1;
    while ((dir = readdir(d)) != NULL) {
        if (strstr(dir->d_name, "gpiochip")) {
            snprintf(path, sizeof(path), "/sys/class/gpio/%s/label", dir->d_name);
            FILE *f = fopen(path, "r");
            if (f) {
                if (fgets(label, sizeof(label), f)) {
                    if (strstr(label, GPIO_LABEL)) {
                        sscanf(dir->d_name, "gpiochip%d", &base);
                        fclose(f);
                        closedir(d);
                        return base;
                    }
                }
                fclose(f);
            }
        }
    }
    closedir(d);
    return -1;
}

// Export GPIOs if not already exported
void export_gpio(int gpio) {
    char path[64];
    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d", gpio);
    if (access(path, F_OK) != 0) {
        char cmd[128];
        snprintf(cmd, sizeof(cmd), "echo %d > /sys/class/gpio/export", gpio);
        run_cmd(cmd);
    }
}

// Read GPIO value
int read_gpio_value(int gpio) {
    char path[128], val;
    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/value", gpio);
    FILE *f = fopen(path, "r");
    if (!f) return -1;
    fread(&val, 1, 1, f);
    fclose(f);
    return val - '0';
}

// Print 4 GPIO values as binary (MSB = highest GPIO)
void print_gpio_binary(int base) {
    printf("GPIO Values (binary): ");
    for (int i = GPIO_COUNT - 1; i >= 0; i--) {
        int gpio_num = base + i;
        int v = read_gpio_value(gpio_num);
        if (v < 0) v = 0;
        printf("%d", v);
    }
    printf("  [");
    for (int i = GPIO_COUNT - 1; i >= 0; i--) {
        int gpio_num = base + i;
        int v = read_gpio_value(gpio_num);
        if (v < 0) v = 0;
        printf("GPIO%d=%d%s", gpio_num, v, (i > 0 ? " " : ""));
    }
    printf("]\n");
}

// Load bitstream using fpgautil
void load_bitstream(const char *bitfile, const char *dtbo_file) {
    char cmd[512];
    if (dtbo_file)
        snprintf(cmd, sizeof(cmd), "fpgautil -b %s -o %s/%s", bitfile, FW_DIR, dtbo_file);
    else
        snprintf(cmd, sizeof(cmd), "fpgautil -b %s", bitfile);
    run_cmd(cmd);
}

// Copy a file to firmware folder
void copy_file_to_fw(const char *src, const char *dest_name) {
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "cp %s %s/%s", src, FW_DIR, dest_name);
    run_cmd(cmd);
}

int main(int argc, char *argv[]) {
    if (argc != 5) {
        printf("Usage: sudo %s <wrapper_bin> <dtbo_file> <bitstream1> <bitstream2>\n", argv[0]);
        return 1;
    }

    char *wrapper_bin = argv[1];
    char *dtbo_file = argv[2];
    char *bit1 = argv[3];
    char *bit2 = argv[4];

    printf("==========================================================\n");
    printf("  DEMO: Dynamic PL Bitstream Upload and GPIO Read\n");
    printf("==========================================================\n");

    // Step 0: Reset FPGA before starting
    printf("\nStep 0: Resetting FPGA configuration ...\n");
    run_cmd("fpgautil -R");
    printf("FPGA reset complete. Starting fresh configuration.\n");

    // Step 1: Copy all files
    printf("\nStep 1: Copying wrapper and bitstreams to %s ...\n", FW_DIR);
    copy_file_to_fw(wrapper_bin, wrapper_bin);
    copy_file_to_fw(dtbo_file, dtbo_file);
    copy_file_to_fw(bit1, bit1);
    copy_file_to_fw(bit2, bit2);
    printf("Files copied successfully!\n");

    // Step 2: Load wrapper + DTBO
    printf("\nStep 2: Loading wrapper and DTBO into PL ...\n");
    char wrapper_fw[256];
    snprintf(wrapper_fw, sizeof(wrapper_fw), "%s/%s", FW_DIR, wrapper_bin);
    load_bitstream(wrapper_fw, dtbo_file);
    printf("Wrapper and overlay loaded successfully!\n");

    // Step 3: Detect GPIO base
    printf("\nStep 3: Detecting PL GPIO pins ...\n");
    int base_gpio = get_pl_gpio_base();
    if (base_gpio < 0) {
        fprintf(stderr, "Error: Could not detect PL GPIO base\n");
        return 1;
    }
    printf("Detected base GPIO: %d\n", base_gpio);

    // Step 4: Export GPIOs
    printf("\nStep 4: Exporting 4 consecutive GPIOs ...\n");
    for (int i = 0; i < GPIO_COUNT; i++) {
        export_gpio(base_gpio + i);
    }
    printf("GPIOs ready for reading.\n");

    // Step 5: Load Bitstream 1 and read GPIOs
    printf("\nStep 5: Loading Bitstream 1 (%s) ...\n", bit1);
    char bit1_fw[256];
    snprintf(bit1_fw, sizeof(bit1_fw), "%s/%s", FW_DIR, bit1);
    load_bitstream(bit1_fw, NULL);
    print_gpio_binary(base_gpio);

    // Step 6: Load Bitstream 2 and read GPIOs
    printf("\nStep 6: Loading Bitstream 2 (%s) ...\n", bit2);
    char bit2_fw[256];
    snprintf(bit2_fw, sizeof(bit2_fw), "%s/%s", FW_DIR, bit2);
    load_bitstream(bit2_fw, NULL);
    print_gpio_binary(base_gpio);

    printf("\n==========================================================\n");
    printf(" DEMO COMPLETE: Bitstreams uploaded and GPIO values read!\n");
    printf("==========================================================\n");

    return 0;
}
