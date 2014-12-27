/* Master 0, Slave 0, "LIBIX ORDER"
 * Vendor ID:       0x000001ee
 * Product code:    0x0000000e
 * Revision number: 0x00000012
 */

ec_pdo_entry_info_t slave_0_pdo_entries[] = {
    {0x1600, 0x00, 8}, /* IO */
    {0x1600, 0x01, 8}, /* IO */
    {0x1600, 0x02, 8}, /* IO */
    {0x1600, 0x03, 8}, /* IO */
    {0x1600, 0x04, 8}, /* IO */
    {0x1600, 0x05, 8}, /* IO */
    {0x1600, 0x06, 8}, /* IO */
    {0x1600, 0x07, 8}, /* IO */
    {0x1600, 0x08, 8}, /* IO */
    {0x1600, 0x09, 8}, /* IO */
    {0x1600, 0x0a, 8}, /* IO */
    {0x1600, 0x0b, 8}, /* IO */
    {0x1600, 0x0c, 8}, /* IO */
    {0x1600, 0x0d, 8}, /* IO */
    {0x1600, 0x0e, 8}, /* IO */
    {0x1600, 0x0f, 8}, /* IO */
    {0x1600, 0x10, 8}, /* IO */
    {0x1600, 0x11, 8}, /* IO */
    {0x1600, 0x12, 8}, /* IO */
    {0x1600, 0x13, 8}, /* IO */
    {0x1600, 0x14, 32}, /* CMD CONTROL */
};

ec_pdo_info_t slave_0_pdos[] = {
    {0x1600, 21, slave_0_pdo_entries + 0}, /* ARDUINO 14 I/O PINS AND 6 ANALOG INPUTS */
};

ec_sync_info_t slave_0_syncs[] = {
    {0, EC_DIR_INPUT, 1, slave_0_pdos + 0, EC_WD_DISABLE},
    {1, EC_DIR_OUTPUT, 0, NULL, EC_WD_DISABLE},
    {0xff}
};

