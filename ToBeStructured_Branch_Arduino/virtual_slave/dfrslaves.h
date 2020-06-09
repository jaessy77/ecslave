/* Master 0, Slave 0, "LIBIX ORDER"
 * Vendor ID:       0x000001ee
 * Product code:    0x0000000e
 * Revision number: 0x00000012
 */

ec_pdo_entry_info_t slave_0_pdo_entries[] = {
    {0x1600, 0x00, 32}, /* OUT PIN */
    {0x1600, 0x01, 32}, /* OUT PIN */
    {0x1600, 0x02, 32}, /* OUT PIN */
    {0x1600, 0x03, 32}, /* OUT PIN */
    {0x1600, 0x04, 32}, /* OUT PIN */
    {0x1600, 0x05, 32}, /* OUT PIN */
    {0x1600, 0x06, 32}, /* OUT PIN */
    {0x1600, 0x07, 32}, /* OUT PIN */
    {0x1600, 0x08, 32}, /* OUT PIN */
    {0x1600, 0x09, 32}, /* OUT PIN */
    {0x1600, 0x0a, 32}, /* OUT PIN */
    {0x1600, 0x0b, 32}, /* OUT PIN */
    {0x1600, 0x0c, 32}, /* OUT PIN */
    {0x1600, 0x0d, 32}, /* OUT PIN */
    {0x1600, 0x0e, 32}, /* OUT PIN */
    {0x1600, 0x0f, 32}, /* OUT PIN */
    {0x1600, 0x10, 32}, /* OUT PIN */
    {0x1600, 0x11, 32}, /* OUT PIN */
    {0x1600, 0x12, 32}, /* OUT PIN */
    {0x1600, 0x13, 32}, /* OUT PIN */
    {0x1600, 0x14, 32}, /* STATUS */
    {0x1a00, 0x00, 32}, /* IN PIN */
    {0x1a00, 0x01, 32}, /* IN PIN */
    {0x1a00, 0x02, 32}, /* IN PIN */
    {0x1a00, 0x03, 32}, /* IN PIN */
    {0x1a00, 0x04, 32}, /* IN PIN */
    {0x1a00, 0x05, 32}, /* IN PIN */
    {0x1a00, 0x06, 32}, /* IN PIN */
    {0x1a00, 0x07, 32}, /* IN PIN */
    {0x1a00, 0x08, 32}, /* IN PIN */
    {0x1a00, 0x09, 32}, /* IN PIN */
    {0x1a00, 0x0a, 32}, /* IN PIN */
    {0x1a00, 0x0b, 32}, /* IN PIN */
    {0x1a00, 0x0c, 32}, /* IN PIN */
    {0x1a00, 0x0d, 32}, /* IN PIN */
    {0x1a00, 0x0e, 32}, /* IN PIN */
    {0x1a00, 0x0f, 32}, /* IN PIN */
    {0x1a00, 0x10, 32}, /* IN PIN */
    {0x1a00, 0x11, 32}, /* IN PIN */
    {0x1a00, 0x12, 32}, /* IN PIN */
    {0x1a00, 0x13, 32}, /* IN PIN */
    {0x1a00, 0x14, 32}, /* COMMAND */
};

ec_pdo_info_t slave_0_pdos[] = {
    {0x1600, 21, slave_0_pdo_entries + 0}, /* ARDUINO OUTPUT */
    {0x1a00, 21, slave_0_pdo_entries + 21}, /* ARDUINO INPUT */
};

ec_sync_info_t slave_0_syncs[] = {
    {0, EC_DIR_INPUT, 1, slave_0_pdos + 0, EC_WD_DISABLE},
    {1, EC_DIR_OUTPUT, 1, slave_0_pdos + 1, EC_WD_DISABLE},
    {0xff}
};

