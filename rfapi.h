/*
 * rfapi.h
 *
 */

#ifndef STARTUP_RFAPI_H_
#define STARTUP_RFAPI_H_

#include <stdbool.h>
#include <inttypes.h>

#define NUMBER_OF_SSB_CANDIDATES 4
#define NUMBER_OF_BEST_BEAMS_PER_CANDIDATE  4

typedef struct {
    int id;
    uint32_t arfcnValueNRr15;
    uint8_t periodicityAndOffsetr15;
    uint8_t sf20r15;
    uint8_t ssbDurationr15;
    uint8_t subcarrierSpacingSSBr15;
    uint16_t freqBandIndicatorNRr15;
} SSB_info;

typedef struct {
    uint32_t arfcn;
    int      pss_type;
    int      adc_sel;
    int      spg;
    uint16_t beamID;
    uint32_t energy;
    int      ssbIdx;
    float    rsrp;
    float    snr;
} Beam_info;

typedef struct {
    int     ssbIdx;
    float   rsrp;
    float   snr;
} BestSsbInfo;

typedef enum {
    RELAY_BIDIRECTIONAL,
    RELAY_DL_ONLY,
    RELAY_UL_ONLY,
} Relay_Mode;

typedef enum {
    REPEATER_PD,
    REPEATER_IDLE,
    REPEATER_DL,
    REPEATER_TDD0,
    REPEATER_TDD1,
    REPEATER_TDD2,
    REPEATER_TDD3,
    REPEATER_TDD4,
    REPEATER_TDD5,
    REPEATER_TDD2p1,
    REPEATER_DLr,
} Repeater_Mode;

typedef enum {
    VZ1,    // DDDX, 43/13 D/U split
    VZ2,    // DDDXU, 53/17 D/L split
    KS,     // DDDDU, 56/14 D/U split
} tdd_pattern;

typedef struct {
    bool sync_lost;
    bool tdd_active;
    int ssb_index;
    float rsrp;
    float snr;
    float avg_temp;
    int tssi_ul_b1;
    int tssi_ul_b2;
    int tssi_dl1_b1;
    int tssi_dl1_b2;
    int tssi_dl2_b1;
    int tssi_dl2_b2;
} Diagnostics;

typedef struct __attribute__((packed)) {
    uint8_t bbidx;
    uint8_t spichain;
    uint16_t spiloc;
    uint8_t data[32];
    uint8_t padding[28]; // total 64 bytes
} BeambookEntry;

typedef struct {
    uint16_t addr;
    uint16_t data;
} CsvEntry;

typedef struct {
    uint32_t addr;
    char label[28];
} IndexEntry;

typedef struct __attribute__((packed)) {
    uint8_t spichain;
    uint16_t spiloc;
    uint16_t addr;
    uint16_t data;
    uint8_t padding; // total 8 bytes
} FourColCsvEntry;

// Keeping it generic, write code to handle different types
// label is included so code can handle appropriately if addr isn't sufficient
typedef struct __attribute__((packed)) {
    uint8_t type; // 0 == temperature data, 1 = fpga reg map, 2 = LUT
    uint16_t addr;
    uint8_t data;
    char label[28];
} CalibrationEntry;

typedef struct pss_count_ssb_decode {
    uint32_t pss_pulse_count;
    uint32_t decoded_ssb_count;
    uint32_t decoded_ssb_no_error_count;
} pss_count_ssb_decode_t;

/* Called from main. Init task running in RF library to handle interrupts. */
void rf_createTask(void);

/* RF interrupt handler */
void rf_interrupt_callback(int);

/* This function initializes the FPGA driver which in turn initializes the
 * FPGA. All parameters needed are stored in the external Flash. The parameters
 * can be changed via UART which allows the system to optimize the parameters
 * in field testing without the need to load new image. */
void rf_init(void);

typedef enum {
    EVENT_DUMMY,
} EventType;

void event_callback(EventType e);

/* Function: scan_ssb

Inputs:

    pSSBInfo - A pointer to an array that has NUMBER_OF_SSB_CANDIDATES SSB candidate entries.
               The array must have length equal to NUMBER_OF_SSB_CANDIDATES.
               Each entry has the data type of SSB_info.

    Output: No outputs.
*/
int scan_ssb(SSB_info(*pSSBInfo)[NUMBER_OF_SSB_CANDIDATES]);

/* Function: get_beam_energy_table

Inputs:

    pBeamInfo - A pointer to a 2-D NUMBER_OF_SSB_CANDIDATES-by-NUMBER_OF_BEST_BEAMS_PER_CANDIDATE array.
                The array has NUMBER_OF_SSB_CANDIDATES rows and NUMBER_OF_BEST_BEAMS_PER_CANDIDATE columns.
                Each array element has the data type Beam_info.

                pBeamInfo is used to store the beam information returned by this function.

    Output: Returned output is stored in 2-D array pBeamInfo.
*/
int get_beam_energy_table(Beam_info(*pBeamInfo)[NUMBER_OF_SSB_CANDIDATES][NUMBER_OF_BEST_BEAMS_PER_CANDIDATE]);

/* Function: freeze_beam_ssb

    Inputs:
            beamID - uint8_t type.
            ssb_info - pointer to SSB_info structure data type.

    Output: 0->at least one beam found, -1->no valid beams.

*/
int freeze_beam_ssb(uint8_t beamID, SSB_info *ssbInfo);

/* Function: get_pcid

    Inputs:

    pcid - A pointer to the returned PCID value.
    ssbInfo - A pointer to the returned SSB ID value.

    Output: The returned pcid and ssb id values are stored in the provided
    addresses.

*/
int get_pcid(uint16_t *pcid, uint8_t *ssbID);

/* This function enables the 2 relay boards and donor TX side if set in
 * bidirectional mode. This function can also be used in debugging mode to
 * enable one direction DL only or UL only. */
int start_relay(Relay_Mode mode);

/* This function is called after the Start_Relay function is called and the
 * information is passed to CPE and a certain amount of time is waited to
 * ensure the CPE locks on the best beam and the CPE send the PCID and maybe
 * the beamIndex which match the one returned from the FPGA. After this
 * function is called, the FPGA driver/FPGA enter a monitoring mode of the
 * specific beam and if that specific beam falls below a specific threshold and
 * there is another available beam for the same SSB then the PGA will switch to
 * it and back to the original beam when/if it comes back (hysteresis is
 * implemented so no continuous switch back and force...). */
void lock_beam_monitor(void);

/* This function is called if the uC needs to turn off the repeater relaying
 * the signal. */
int stop_relay();

typedef enum {
    TEST_MODE_PD,
    TEST_MODE_TX,
    TEST_MODE_RX,
    TEST_MODE_UL,
    TEST_MODE_DL,
} TestMode;

typedef enum beambook_mode
{
    DOWNLINK = 0,
    UPLINK,
    RX,
    TX
} beambook_mode_t;

typedef enum {
    SEL_UL = 0,
    SEL_DL1,
    SEL_DL2,
} SelMode;

typedef struct {
    uint16_t b1;
    uint16_t b2;
} tssi_t;


void set_default_gain_DL_UL(void);
int donor_set_ssb(uint32_t arfcnValue, float ppm);

void donor_set_beambook_index(beambook_mode_t mode, int index);
void donor_set_beambook_index_ul_driver(int index);

void repeater_set_mode(Repeater_Mode m);
void enable_tdd_switching(void);
void disable_tdd_switching(void);

void write_adc(uint32_t addr, const uint8_t data);
int read_adc(const uint32_t addr, uint8_t *data);


/* functions to set various gains */
void donor_set_attn_DL(uint8_t idx, uint8_t b1, uint8_t b2);
void donor_set_attn_UL(uint8_t idx, uint8_t b1, uint8_t b2);
void donor_set_ul_2801d_gain(uint8_t gidx, uint8_t b1, uint8_t b2);
void donor_set_ul_2801_fe_gain(uint8_t gidx, uint8_t b1, uint8_t b2);
void donor_set_LNA_gain_AGC_RAM(uint8_t gidx);
void donor_set_dl_2801d_gain(uint8_t gidx, uint8_t b1, uint8_t b2);
void donor_set_mixer_gain_AGC_RAM(TestMode m, uint8_t b1_gidx, uint8_t b2_gidx);
void relay_set_LNA_gain_AGC_RAM(uint8_t gidx);
void relay_set_dl_2801_gain(uint8_t gidx, uint8_t b1, uint8_t b2);

/* diagnostic information functions */
void get_diag_info(Diagnostics *diag);
void rfloop_control(uint8_t enable_byte, int loop_interval_s);
void set_log_level(int log_set);
int temp_comp(float temp_d);
int osc_det_handler(SelMode sel);
float get_brd_temp_sensor(void);

/* config functions */
int lite_set_tdd_pattern(tdd_pattern pat);
void get_best_ssb_rsrp_snr(int duration_in_ms, BestSsbInfo *best_ssb_info);

#endif /* STARTUP_RFAPI_H_ */
