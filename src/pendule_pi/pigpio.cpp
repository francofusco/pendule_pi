#include "pendule_pi/pigpio.hpp"
#include "pendule_pi/debug.hpp"
#include <csignal>


namespace pigpio {

Exception::Exception(
  const std::string& caller,
  int return_code
)
: std::runtime_error(makeMessage(caller, return_code))
{
  // nothing else to do here!
}


std::string Exception::makeMessage(
  const std::string& caller,
  int return_code
)
{
  return "pigpio::Exception encountered while running '" + caller + "'. Return "
         "code: " + error2msg(return_code);
}


std::string Exception::error2msg(
  int error_code
)
{
  auto code_it = ERROR_CODES.find(error_code);
  if(code_it == ERROR_CODES.end())
    return "UNKNOWN ERROR CODE " + std::to_string(error_code);
  else
    return code_it->second;
}


ActivationToken* ActivationToken::active_token = nullptr;


ActivationToken::ActivationToken(bool register_sigint) {
  PENDULE_PI_DBG("ActivationToken: checking that no other token exists");
  // Make sure that no other active token exists
  if(active_token != nullptr) {
    throw MultpleTokensCreated(true);
  }
  // "register" this object as the active token
  active_token = this;
  // Initialize the pigpio library
  PENDULE_PI_DBG("ActivationToken: initializing");
  PiGPIO_RUN_VOID(gpioInitialise);
  // NOTE: it is VERY important that the signal handlers are assigned after
  // attempting to call gpioInitialise()!
  if(register_sigint)
    std::signal(SIGINT, ActivationToken::pleaseStop);
  std::signal(SIGABRT, ActivationToken::abort);
}


ActivationToken::~ActivationToken() {
  PENDULE_PI_DBG("ActivationToken: calling resetPins() upon token destruction");
  resetPins();
  // If there is no registered token, or another one is registered, bad things might happen!
  if(active_token != this) {
    throw MultpleTokensCreated(false);
  }
  // "unregister" this token.
  active_token = nullptr;
}


void ActivationToken::resetPins() {
  PENDULE_PI_DBG("ActivationToken: putting all pins into high impedance mode");
  for(int pin=0; pin<=26; pin++) {
    auto retval = gpioSetPullUpDown(pin, PI_PUD_OFF);
    if(retval < 0) {
      PENDULE_PI_WRN("ActivationToken: while disabling resistors on pin " << \
        pin << ", gpioSetPullUpDown() returned " << Exception::error2msg(retval));
    }
    retval = gpioSetMode(pin, PI_INPUT);
    if(retval < 0) {
      PENDULE_PI_WRN("ActivationToken: while setting pin " << pin << " as " \
        "input, gpioSetMode() returned " << Exception::error2msg(retval));
    }
  }
  #warning "Calling gpioTerminate() while handling a signal seems to kill the process, which is NOT a good thing. Check if there is a fix for this."
  // PENDULE_PI_DBG("ActivationToken: calling gpioTerminate()");
  // gpioTerminate();
  PENDULE_PI_DBG("ActivationToken: GPIO reset completed");
}


void ActivationToken::pleaseStop(int s) {
  PENDULE_PI_DBG("ActivationToken: executing 'pleaseStop(" << (s==SIGINT?std::string("SIGINT"):std::to_string(s)) << ")'");
  throw PleaseStop();
}


void ActivationToken::abort(int s) {
  PENDULE_PI_DBG("ActivationToken: executing 'abort(" << (s==SIGABRT?std::string("SIGABRT"):std::to_string(s)) << ")'");
  resetPins();
  PENDULE_PI_DBG("ActivationToken: calling std::_Exit(EXIT_FAILURE)");
  std::_Exit(EXIT_FAILURE);
}


Rate::Rate(
  unsigned int period_us
)
: period_(period_us)
, tnow_(0)
, tpast_(0)
{
  // nothing else to do here!
}


unsigned int Rate::sleep() {
  do {
    tnow_ = gpioTick();
  }
  while(tnow_-tpast_ < period_);
  return tpast_ = tnow_;
}


#define MAKE_CODE_ENTRY(CODE,DESCR) {CODE, #CODE " (" + std::to_string(CODE) + "): '" DESCR "'"}
const std::map<int,std::string> Exception::ERROR_CODES {
  MAKE_CODE_ENTRY(PI_INIT_FAILED, "gpioInitialise failed"),
  MAKE_CODE_ENTRY(PI_BAD_USER_GPIO, "GPIO not 0-31"),
  MAKE_CODE_ENTRY(PI_BAD_GPIO, "GPIO not 0-53"),
  MAKE_CODE_ENTRY(PI_BAD_MODE, "mode not 0-7"),
  MAKE_CODE_ENTRY(PI_BAD_LEVEL, "level not 0-1"),
  MAKE_CODE_ENTRY(PI_BAD_PUD, "pud not 0-2"),
  MAKE_CODE_ENTRY(PI_BAD_PULSEWIDTH, "pulsewidth not 0 or 500-2500"),
  MAKE_CODE_ENTRY(PI_BAD_DUTYCYCLE, "dutycycle outside set range"),
  MAKE_CODE_ENTRY(PI_BAD_TIMER, "timer not 0-9"),
  MAKE_CODE_ENTRY(PI_BAD_MS, "ms not 10-60000"),
  MAKE_CODE_ENTRY(PI_BAD_TIMETYPE, "timetype not 0-1"),
  MAKE_CODE_ENTRY(PI_BAD_SECONDS, "seconds < 0"),
  MAKE_CODE_ENTRY(PI_BAD_MICROS, "micros not 0-999999"),
  MAKE_CODE_ENTRY(PI_TIMER_FAILED, "gpioSetTimerFunc failed"),
  MAKE_CODE_ENTRY(PI_BAD_WDOG_TIMEOUT, "timeout not 0-60000"),
  MAKE_CODE_ENTRY(PI_NO_ALERT_FUNC, "DEPRECATED"),
  MAKE_CODE_ENTRY(PI_BAD_CLK_PERIPH, "clock peripheral not 0-1"),
  MAKE_CODE_ENTRY(PI_BAD_CLK_SOURCE, "DEPRECATED"),
  MAKE_CODE_ENTRY(PI_BAD_CLK_MICROS, "clock micros not 1, 2, 4, 5, 8, or 10"),
  MAKE_CODE_ENTRY(PI_BAD_BUF_MILLIS, "buf millis not 100-10000"),
  MAKE_CODE_ENTRY(PI_BAD_DUTYRANGE, "dutycycle range not 25-40000"),
  MAKE_CODE_ENTRY(PI_BAD_DUTY_RANGE, "DEPRECATED (use PI_BAD_DUTYRANGE)"),
  MAKE_CODE_ENTRY(PI_BAD_SIGNUM, "signum not 0-63"),
  MAKE_CODE_ENTRY(PI_BAD_PATHNAME, "can't open pathname"),
  MAKE_CODE_ENTRY(PI_NO_HANDLE, "no handle available"),
  MAKE_CODE_ENTRY(PI_BAD_HANDLE, "unknown handle"),
  MAKE_CODE_ENTRY(PI_BAD_IF_FLAGS, "ifFlags > 4"),
  MAKE_CODE_ENTRY(PI_BAD_CHANNEL, "DMA channel not 0-15"),
  MAKE_CODE_ENTRY(PI_BAD_PRIM_CHANNEL, "DMA primary channel not 0-15"),
  MAKE_CODE_ENTRY(PI_BAD_SOCKET_PORT, "socket port not 1024-32000"),
  MAKE_CODE_ENTRY(PI_BAD_FIFO_COMMAND, "unrecognized fifo command"),
  MAKE_CODE_ENTRY(PI_BAD_SECO_CHANNEL, "DMA secondary channel not 0-15"),
  MAKE_CODE_ENTRY(PI_NOT_INITIALISED, "function called before gpioInitialise"),
  MAKE_CODE_ENTRY(PI_INITIALISED, "function called after gpioInitialise"),
  MAKE_CODE_ENTRY(PI_BAD_WAVE_MODE, "waveform mode not 0-3"),
  MAKE_CODE_ENTRY(PI_BAD_CFG_INTERNAL, "bad parameter in gpioCfgInternals call"),
  MAKE_CODE_ENTRY(PI_BAD_WAVE_BAUD, "baud rate not 50-250K(RX)/50-1M(TX)"),
  MAKE_CODE_ENTRY(PI_TOO_MANY_PULSES, "waveform has too many pulses"),
  MAKE_CODE_ENTRY(PI_TOO_MANY_CHARS, "waveform has too many chars"),
  MAKE_CODE_ENTRY(PI_NOT_SERIAL_GPIO, "no bit bang serial read on GPIO"),
  MAKE_CODE_ENTRY(PI_BAD_SERIAL_STRUC, "bad (null) serial structure parameter"),
  MAKE_CODE_ENTRY(PI_BAD_SERIAL_BUF, "bad (null) serial buf parameter"),
  MAKE_CODE_ENTRY(PI_NOT_PERMITTED, "GPIO operation not permitted"),
  MAKE_CODE_ENTRY(PI_SOME_PERMITTED, "one or more GPIO not permitted"),
  MAKE_CODE_ENTRY(PI_BAD_WVSC_COMMND, "bad WVSC subcommand"),
  MAKE_CODE_ENTRY(PI_BAD_WVSM_COMMND, "bad WVSM subcommand"),
  MAKE_CODE_ENTRY(PI_BAD_WVSP_COMMND, "bad WVSP subcommand"),
  MAKE_CODE_ENTRY(PI_BAD_PULSELEN, "trigger pulse length not 1-100"),
  MAKE_CODE_ENTRY(PI_BAD_SCRIPT, "invalid script"),
  MAKE_CODE_ENTRY(PI_BAD_SCRIPT_ID, "unknown script id"),
  MAKE_CODE_ENTRY(PI_BAD_SER_OFFSET, "add serial data offset > 30 minutes"),
  MAKE_CODE_ENTRY(PI_GPIO_IN_USE, "GPIO already in use"),
  MAKE_CODE_ENTRY(PI_BAD_SERIAL_COUNT, "must read at least a byte at a time"),
  MAKE_CODE_ENTRY(PI_BAD_PARAM_NUM, "script parameter id not 0-9"),
  MAKE_CODE_ENTRY(PI_DUP_TAG, "script has duplicate tag"),
  MAKE_CODE_ENTRY(PI_TOO_MANY_TAGS, "script has too many tags"),
  MAKE_CODE_ENTRY(PI_BAD_SCRIPT_CMD, "illegal script command"),
  MAKE_CODE_ENTRY(PI_BAD_VAR_NUM, "script variable id not 0-149"),
  MAKE_CODE_ENTRY(PI_NO_SCRIPT_ROOM, "no more room for scripts"),
  MAKE_CODE_ENTRY(PI_NO_MEMORY, "can't allocate temporary memory"),
  MAKE_CODE_ENTRY(PI_SOCK_READ_FAILED, "socket read failed"),
  MAKE_CODE_ENTRY(PI_SOCK_WRIT_FAILED, "socket write failed"),
  MAKE_CODE_ENTRY(PI_TOO_MANY_PARAM, "too many script parameters (> 10)"),
  MAKE_CODE_ENTRY(PI_NOT_HALTED, "DEPRECATED"),
  MAKE_CODE_ENTRY(PI_SCRIPT_NOT_READY, "script initialising"),
  MAKE_CODE_ENTRY(PI_BAD_TAG, "script has unresolved tag"),
  MAKE_CODE_ENTRY(PI_BAD_MICS_DELAY, "bad MICS delay (too large)"),
  MAKE_CODE_ENTRY(PI_BAD_MILS_DELAY, "bad MILS delay (too large)"),
  MAKE_CODE_ENTRY(PI_BAD_WAVE_ID, "non existent wave id"),
  MAKE_CODE_ENTRY(PI_TOO_MANY_CBS, "No more CBs for waveform"),
  MAKE_CODE_ENTRY(PI_TOO_MANY_OOL, "No more OOL for waveform"),
  MAKE_CODE_ENTRY(PI_EMPTY_WAVEFORM, "attempt to create an empty waveform"),
  MAKE_CODE_ENTRY(PI_NO_WAVEFORM_ID, "no more waveforms"),
  MAKE_CODE_ENTRY(PI_I2C_OPEN_FAILED, "can't open I2C device"),
  MAKE_CODE_ENTRY(PI_SER_OPEN_FAILED, "can't open serial device"),
  MAKE_CODE_ENTRY(PI_SPI_OPEN_FAILED, "can't open SPI device"),
  MAKE_CODE_ENTRY(PI_BAD_I2C_BUS, "bad I2C bus"),
  MAKE_CODE_ENTRY(PI_BAD_I2C_ADDR, "bad I2C address"),
  MAKE_CODE_ENTRY(PI_BAD_SPI_CHANNEL, "bad SPI channel"),
  MAKE_CODE_ENTRY(PI_BAD_FLAGS, "bad i2c/spi/ser open flags"),
  MAKE_CODE_ENTRY(PI_BAD_SPI_SPEED, "bad SPI speed"),
  MAKE_CODE_ENTRY(PI_BAD_SER_DEVICE, "bad serial device name"),
  MAKE_CODE_ENTRY(PI_BAD_SER_SPEED, "bad serial baud rate"),
  MAKE_CODE_ENTRY(PI_BAD_PARAM, "bad i2c/spi/ser parameter"),
  MAKE_CODE_ENTRY(PI_I2C_WRITE_FAILED, "i2c write failed"),
  MAKE_CODE_ENTRY(PI_I2C_READ_FAILED, "i2c read failed"),
  MAKE_CODE_ENTRY(PI_BAD_SPI_COUNT, "bad SPI count"),
  MAKE_CODE_ENTRY(PI_SER_WRITE_FAILED, "ser write failed"),
  MAKE_CODE_ENTRY(PI_SER_READ_FAILED, "ser read failed"),
  MAKE_CODE_ENTRY(PI_SER_READ_NO_DATA, "ser read no data available"),
  MAKE_CODE_ENTRY(PI_UNKNOWN_COMMAND, "unknown command"),
  MAKE_CODE_ENTRY(PI_SPI_XFER_FAILED, "spi xfer/read/write failed"),
  MAKE_CODE_ENTRY(PI_BAD_POINTER, "bad (NULL) pointer"),
  MAKE_CODE_ENTRY(PI_NO_AUX_SPI, "no auxiliary SPI on Pi A or B"),
  MAKE_CODE_ENTRY(PI_NOT_PWM_GPIO, "GPIO is not in use for PWM"),
  MAKE_CODE_ENTRY(PI_NOT_SERVO_GPIO, "GPIO is not in use for servo pulses"),
  MAKE_CODE_ENTRY(PI_NOT_HCLK_GPIO, "GPIO has no hardware clock"),
  MAKE_CODE_ENTRY(PI_NOT_HPWM_GPIO, "GPIO has no hardware PWM"),
  MAKE_CODE_ENTRY(PI_BAD_HPWM_FREQ, "invalid hardware PWM frequency"),
  MAKE_CODE_ENTRY(PI_BAD_HPWM_DUTY, "hardware PWM dutycycle not 0-1M"),
  MAKE_CODE_ENTRY(PI_BAD_HCLK_FREQ, "invalid hardware clock frequency"),
  MAKE_CODE_ENTRY(PI_BAD_HCLK_PASS, "need password to use hardware clock 1"),
  MAKE_CODE_ENTRY(PI_HPWM_ILLEGAL, "illegal, PWM in use for main clock"),
  MAKE_CODE_ENTRY(PI_BAD_DATABITS, "serial data bits not 1-32"),
  MAKE_CODE_ENTRY(PI_BAD_STOPBITS, "serial (half) stop bits not 2-8"),
  MAKE_CODE_ENTRY(PI_MSG_TOOBIG, "socket/pipe message too big"),
  MAKE_CODE_ENTRY(PI_BAD_MALLOC_MODE, "bad memory allocation mode"),
  MAKE_CODE_ENTRY(PI_TOO_MANY_SEGS, "too many I2C transaction segments"),
  MAKE_CODE_ENTRY(PI_BAD_I2C_SEG, "an I2C transaction segment failed"),
  MAKE_CODE_ENTRY(PI_BAD_SMBUS_CMD, "SMBus command not supported by driver"),
  MAKE_CODE_ENTRY(PI_NOT_I2C_GPIO, "no bit bang I2C in progress on GPIO"),
  MAKE_CODE_ENTRY(PI_BAD_I2C_WLEN, "bad I2C write length"),
  MAKE_CODE_ENTRY(PI_BAD_I2C_RLEN, "bad I2C read length"),
  MAKE_CODE_ENTRY(PI_BAD_I2C_CMD, "bad I2C command"),
  MAKE_CODE_ENTRY(PI_BAD_I2C_BAUD, "bad I2C baud rate, not 50-500k"),
  MAKE_CODE_ENTRY(PI_CHAIN_LOOP_CNT, "bad chain loop count"),
  MAKE_CODE_ENTRY(PI_BAD_CHAIN_LOOP, "empty chain loop"),
  MAKE_CODE_ENTRY(PI_CHAIN_COUNTER, "too many chain counters"),
  MAKE_CODE_ENTRY(PI_BAD_CHAIN_CMD, "bad chain command"),
  MAKE_CODE_ENTRY(PI_BAD_CHAIN_DELAY, "bad chain delay micros"),
  MAKE_CODE_ENTRY(PI_CHAIN_NESTING, "chain counters nested too deeply"),
  MAKE_CODE_ENTRY(PI_CHAIN_TOO_BIG, "chain is too long"),
  MAKE_CODE_ENTRY(PI_DEPRECATED, "deprecated function removed"),
  MAKE_CODE_ENTRY(PI_BAD_SER_INVERT, "bit bang serial invert not 0 or 1"),
  MAKE_CODE_ENTRY(PI_BAD_EDGE, "bad ISR edge value, not 0-2"),
  MAKE_CODE_ENTRY(PI_BAD_ISR_INIT, "bad ISR initialisation"),
  MAKE_CODE_ENTRY(PI_BAD_FOREVER, "loop forever must be last command"),
  MAKE_CODE_ENTRY(PI_BAD_FILTER, "bad filter parameter"),
  MAKE_CODE_ENTRY(PI_BAD_PAD, "bad pad number"),
  MAKE_CODE_ENTRY(PI_BAD_STRENGTH, "bad pad drive strength"),
  MAKE_CODE_ENTRY(PI_FIL_OPEN_FAILED, "file open failed"),
  MAKE_CODE_ENTRY(PI_BAD_FILE_MODE, "bad file mode"),
  MAKE_CODE_ENTRY(PI_BAD_FILE_FLAG, "bad file flag"),
  MAKE_CODE_ENTRY(PI_BAD_FILE_READ, "bad file read"),
  MAKE_CODE_ENTRY(PI_BAD_FILE_WRITE, "bad file write"),
  MAKE_CODE_ENTRY(PI_FILE_NOT_ROPEN, "file not open for read"),
  MAKE_CODE_ENTRY(PI_FILE_NOT_WOPEN, "file not open for write"),
  MAKE_CODE_ENTRY(PI_BAD_FILE_SEEK, "bad file seek"),
  MAKE_CODE_ENTRY(PI_NO_FILE_MATCH, "no files match pattern"),
  MAKE_CODE_ENTRY(PI_NO_FILE_ACCESS, "no permission to access file"),
  MAKE_CODE_ENTRY(PI_FILE_IS_A_DIR, "file is a directory"),
  MAKE_CODE_ENTRY(PI_BAD_SHELL_STATUS, "bad shell return status"),
  MAKE_CODE_ENTRY(PI_BAD_SCRIPT_NAME, "bad script name"),
  MAKE_CODE_ENTRY(PI_BAD_SPI_BAUD, "bad SPI baud rate, not 50-500k"),
  MAKE_CODE_ENTRY(PI_NOT_SPI_GPIO, "no bit bang SPI in progress on GPIO"),
  MAKE_CODE_ENTRY(PI_BAD_EVENT_ID, "bad event id"),
  MAKE_CODE_ENTRY(PI_CMD_INTERRUPTED, "Used by Python"),
  MAKE_CODE_ENTRY(PI_NOT_ON_BCM2711, "not available on BCM2711"),
  MAKE_CODE_ENTRY(PI_ONLY_ON_BCM2711, "only available on BCM2711"),
  MAKE_CODE_ENTRY(PI_PIGIF_ERR_0, "no description given in pigpio.h"),
  MAKE_CODE_ENTRY(PI_PIGIF_ERR_99, "no description given in pigpio.h"),
  MAKE_CODE_ENTRY(PI_CUSTOM_ERR_0, "no description given in pigpio.h"),
  MAKE_CODE_ENTRY(PI_CUSTOM_ERR_999, "no description given in pigpio.h"),
};

} // end of namespace pigpio
