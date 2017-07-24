/*********************************************
HAL API
**********************************************/

void vibr_Enable_HW(void);
void vibr_Disable_HW(void);
void vibr_power_set(void);
void vib_enabled(int enable);
struct vibrator_hw *mt_get_cust_vibrator_hw(void);
