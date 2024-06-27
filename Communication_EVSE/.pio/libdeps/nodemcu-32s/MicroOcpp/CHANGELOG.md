# Changelog

## [1.1.0] - 2024-05-21

### Changed

- Replace `PollResult<bool>` with enum `UnlockConnectorResult` ([#271](https://github.com/matth-x/MicroOcpp/pull/271))
- Rename master branch into main
- Tx logic directly checks if WebSocket is offline ([#282](https://github.com/matth-x/MicroOcpp/pull/282))
- `ocppPermitsCharge` ignores Faulted state ([#279](https://github.com/matth-x/MicroOcpp/pull/279))
- `setEnergyMeterInput` expects `int` input ([#301](https://github.com/matth-x/MicroOcpp/pull/301))

### Added

- File index ([#270](https://github.com/matth-x/MicroOcpp/pull/270))
- Config `Cst_TxStartOnPowerPathClosed` to put back TxStartPoint ([#271](https://github.com/matth-x/MicroOcpp/pull/271))
- Build flag `MO_ENABLE_RESERVATION=0` disables Reservation module ([#302](https://github.com/matth-x/MicroOcpp/pull/302))
- Build flag `MO_ENABLE_LOCAL_AUTH=0` disables LocalAuthList module ([#303](https://github.com/matth-x/MicroOcpp/pull/303))
- Function `bool isConnected()` in `Connection` interface ([#282](https://github.com/matth-x/MicroOcpp/pull/282))
- Build flags for customizing memory limits of SmartCharging ([#260](https://github.com/matth-x/MicroOcpp/pull/260))
- SConscript ([#287](https://github.com/matth-x/MicroOcpp/pull/287))
- C-API for custom Configs store ([297](https://github.com/matth-x/MicroOcpp/pull/297))
- Certificate Management, UCs M03 - M05 ([#262](https://github.com/matth-x/MicroOcpp/pull/262), [#274](https://github.com/matth-x/MicroOcpp/pull/274), [#292](https://github.com/matth-x/MicroOcpp/pull/292))
- FTP Client ([#291](https://github.com/matth-x/MicroOcpp/pull/291))
- `ProtocolVersion` selects v1.6 or v2.0.1 ([#247](https://github.com/matth-x/MicroOcpp/pull/247))
- Build flag `MO_ENABLE_V201=1` enables OCPP 2.0.1 features ([#247](https://github.com/matth-x/MicroOcpp/pull/247))
    - Variables (non-persistent), UCs B05 - B07 ([#247](https://github.com/matth-x/MicroOcpp/pull/247), [#284](https://github.com/matth-x/MicroOcpp/pull/284))
    - Transactions (preview only), UCs E01 - E12 ([#247](https://github.com/matth-x/MicroOcpp/pull/247))
    - StatusNotification compatibility ([#247](https://github.com/matth-x/MicroOcpp/pull/247))
    - ChangeAvailability compatibility ([#285](https://github.com/matth-x/MicroOcpp/pull/285))
    - Reset compatibility, UCs B11 - B12 ([#286](https://github.com/matth-x/MicroOcpp/pull/286))
    - RequestStart-/StopTransaction, UCs F01 - F02 ([#289](https://github.com/matth-x/MicroOcpp/pull/289))

### Fixed

- Fix defect idTag check in `endTransaction` ([#275](https://github.com/matth-x/MicroOcpp/pull/275))
- Make field localAuthorizationList in SendLocalList optional
- Update charging profiles when flash disabled (relates to [#260](https://github.com/matth-x/MicroOcpp/pull/260))
- Ignore UnlockConnector when handler not set ([#271](https://github.com/matth-x/MicroOcpp/pull/271))
- Reject ChargingProfile if unit not supported ([#271](https://github.com/matth-x/MicroOcpp/pull/271))
- Fix building with debug level warn and error
- Reduce debug output FW size overhead ([#304](https://github.com/matth-x/MicroOcpp/pull/304))
- Fix transaction freeze in offline mode ([#279](https://github.com/matth-x/MicroOcpp/pull/279), [#287](https://github.com/matth-x/MicroOcpp/pull/287))
- Fix compilation error caused by `PRId32` ([#279](https://github.com/matth-x/MicroOcpp/pull/279))
- Don't load FW-mngt. module when no handlers set ([#271](https://github.com/matth-x/MicroOcpp/pull/271))
- Change arduinoWebSockets URL param to path ([#278](https://github.com/matth-x/MicroOcpp/issues/278))
- Avoid creating conf when operation fails ([#290](https://github.com/matth-x/MicroOcpp/pull/290))
- Fix whitespaces in MeterValues ([#301](https://github.com/matth-x/MicroOcpp/pull/301))

## [1.0.3] - 2024-04-06

### Fixed

- Fix nullptr access in endTransaction ([#275](https://github.com/matth-x/MicroOcpp/pull/275))
- Backport: Fix building with debug level warn and error

## [1.0.2] - 2024-03-24

### Fixed

- Correct MO version numbers in code (they were still `1.0.0`)

## [1.0.1] - 2024-02-27

### Fixed

- Allow `nullptr` as parameter for `mocpp_set_console_out` ([#224](https://github.com/matth-x/MicroOcpp/issues/224))
- Fix `mocpp_tick_ms()` on esp-idf roll-over after 12 hours
- Pin ArduinoJson to v6.21 ([#245](https://github.com/matth-x/MicroOcpp/issues/245))
- Fix bounds checking in SmartCharging module ([#260](https://github.com/matth-x/MicroOcpp/pull/260))

## [1.0.0] - 2023-10-22

_First release_

### Changed

- `mocpp_initialize` takes OCPP URL without explicit host, port ([#220](https://github.com/matth-x/MicroOcpp/pull/220))
- `endTransaction` checks authorization of `idTag`
- Update configurations API ([#195](https://github.com/matth-x/MicroOcpp/pull/195))
- Update Firmware- and DiagnosticsService API ([#207](https://github.com/matth-x/MicroOcpp/pull/207))
- Update Connection interface
- Update Authorization module functions ([#213](https://github.com/matth-x/MicroOcpp/pull/213))
- Reflect changes in C-API
- Change build flag prefix from `MOCPP_` to `MO_`
- Change `mo_set_console_out` to `mocpp_set_console_out`
- Revise README.md
- Revise misleading debug messages
- Update Arduino IDE manifest ([#206](https://github.com/matth-x/MicroOcpp/issues/206))

### Added

- Auto-recovery switch in `mocpp_initialize` params
- WebAssembly port
- Configurable `MO_PARTITION_LABEL` for the esp-idf SPIFFS integration ([#218](https://github.com/matth-x/MicroOcpp/pull/218))
- `MO_TX_CLEAN_ABORTED=0` keeps aborted txs in journal
- `MO_VERSION` specifier
- `MO_PLATFORM_NONE` for compilation on custom platforms
- `endTransaction_authorized` enforces the tx end
- Add valgrind, ASan, UBSan CI/CD steps ([#189](https://github.com/matth-x/MicroOcpp/pull/189))

### Fixed

- Reservation ([#196](https://github.com/matth-x/MicroOcpp/pull/196))
- Fix immediate FW-update Download phase abort ([#216](https://github.com/matth-x/MicroOcpp/pull/216))
- `stat` usage on arduino-esp32 LittleFS
- SetChargingProfile JSON capacity calculation
- Set correct idTag when Reset triggers StopTx
- Execute operations only once despite multiple .conf send attempts ([#207](https://github.com/matth-x/MicroOcpp/pull/207))
- ConnectionTimeOut only applies when connector is still unplugged
- Fix valgrind warnings

## [1eff6e5] - 23-08-23

_Previous point with breaking changes on master_

Renaming to MicroOcpp is completed since this commit. See the [migration guide](https://matth-x.github.io/MicroOcpp/migration/) for more details on what's changed. Changelogs and semantic versioning are adopted starting with v1.0.0

## [0.3.0] - 23-08-19

_Last version under the project name ArduinoOcpp_
