# Driver\_-\_Li710
SDI-12 interface driver for the LiCor Li-710 Evapotranspiration sensor

### Error Response

Device has two potential error responses (in addition to general SDI-12 and sensor errors)

**`LI710_ERROR`** -> `0xA0010000`

**`LI710_WARNING`** -> `0xF01A0000`

The sensor errors (described in detail in [manual](https://licor.app.box.com/s/3uehdcs2tkwhb120zm3l961pauvjhl9d)) are split between warnings are errors and the bit position of the given diagnostic code is reported as the error code sub-type

Example:

Error Code -> `0xA0010712`

Indicates an error code of 'Poor sonic signal' on sensor connected to Talon 1, Port 2

Below is a listing of fault descriptions, type, and corresponding codes

| Bit Position | Description | Type | Error Code |
| --------- | ----------- | ---- | ---------- |
| 0 | Flow out of range | Error | `0xA00100tp` |
| 3 | Low RH sensor voltage | Error | `0xA00103tp` |
| 4 | Cell temperature out of range | Error | `0xA00104tp` |
| 6 | No sonic | Error | `0xA00106tp` |
| 7 | Poor sonic signal | Error | `0xA00107tp` |
| 8 | Rain detected | Warning | `0xF01A08tp` |
| 9 | High humidity shutdown | Warning | `0xF01A09tp` |
| 10 | Cold temperature shutdown | Warning | `0xF01A0Atp` |
| 11 | High cell pressure relative to ambient | Error | `0xA0010Btp` |
| 12 | Low cell pressure relative to ambient | Error | `0xA0010Ctp` |
| 14 | Low pump voltage | Error | `0xA0010Etp` |
| 15 | High pump voltage | Error | `0xA0010Ftp` |

**Note 1:** `t` and `p` indicate insertion of Talon value and Port value respectively 

**Note 2:** Bit positions not listed are considered 'reserved' by the system and no description is given - they will however be passed on as Error type if they are reported by sensor

In general a 'Warning' is a state which will be self corrected, or at least has no ability for intervention (too cold, too humid, etc). Conversely 'Error' type codes generally require intervention -- See [Li-710 Manual (Section 9, 10)](https://licor.app.box.com/s/3uehdcs2tkwhb120zm3l961pauvjhl9d) for more details.


### Interface Notes

### Interface Errata

#### Sensor Invalid Response

Depending on the specific sensor reporting, an out of range value can be reported at `-9999` or `9999999` - this does not appear in the documentation but is observed in testing 

#### Sensor Not Ready Response

When the sensor first starts up and a trigger (`aXT!`) command has not been issued, the sensor responds with ill defined values in the Diagnostic (`DIAG`) field.

Example response:

_Power to Sensor_

`aR0!`->`a-9999-9999-9999-9999-9999-9999-9999-9999-99` 

The system uses a `-9999` value to indicate a null value, while this is not specified in the sensor manual or in the SDI-12 specification, it is generally accepted in the community. 

However, the sensor also **responds with a `-99` for the Diagnostic Value**, which is not specified in the list of potential diagnostic codes - **take note of this for parsing and decoding responses!**

#### Send Data Idiosyncrasy 

Generally one would get sensor data using Start Measurement (`aM!` or `aM0!` thru `aM9!`) followed by Send Data (`aD0!` thru `aD9!`), however for the Li710 (as of sensor firmware version `1.0`) this does not work as expected. 

Instead of sending the entire group results with each Send Data call, the sensor instead only sends 3 at a time in the following way.

After sending an `aXT!` command to update data, the sensor responds in the following way to Start Measurement and Send Data commands 

##### Start Measurement Commands - `aMx!`

`aXT!`->`0READY`

`aM0!`->`00119`

delay 11 seconds

`aD0!`->`a+ET+LE+H`

`aD1!`->`a+VPD+PA+TA`

`aD2!`->`a+RH+SEQ+DIAG`

While this does not violate SDI-12 specifications[^1], it is inconsistent when compared with using the Continuous Measurement commands, despite indication from the SDI-12 specification[^2] that they would be formatted in the same manor, and as a result could lead to confusion.

##### Continuous Measurement Commands - `aRx!`

`aXT!`->`0READY`

`aR0!`->`a+ET+LE+H+VPD+PA+TA+RH+SEQ+DIAG`

Note: At this point the Send Data command can be used to retrieve this information as well

`aD0!`->`a+ET+LE+H+VPD+PA+TA+RH+SEQ+DIAG`

**This idiosyncrasy seems to arise from a maximum length discrepancy in the SDI-12 specification[^3] regarding length of data returns from various sources. The total length of a single group exceeds this 35 character limit** Given this, it seems that either using Continuous Measurement (`aR0!` thru `aR9!`) commands or using Concurrent Measurement (`aC0!` thru `aC9!`) commands.

##### Concurrent Measurement Commands - `aCx!`

`aXT!`->`0READY`

`aC0!`->`001109`

delay 11 seconds

`aD0!`->`a+ET+LE+H+VPD+PA+TA+RH+SEQ+DIAG`

[^1]: _"If the expected number of measurements is not returned in response to a D0! command, the data recorder issues D1!, D2!, etc. until all measurement values are received (The expected number of measurements is given in the response to an M, C or V command.)"_ - [SDI-12 Interface Standard v1.4](https://sdi-12.org/archives_folder/SDI-12_version-1_4-Jan-30-2021.pdf), Page 15

[^2]: _"Continuous Measurements ... (formatted like the D commands)"_ - [SDI-12 Interface Standard v1.4](https://sdi-12.org/archives_folder/SDI-12_version-1_4-Jan-30-2021.pdf), Page 8

[^3]: _"The maximum number of characters that can be returned in the \<values\> part of the response to a D command is either 35 or 75. If the D command is issues to retrieve data in response to a concurrent command, or a high volume ASCII command, the maximum is 75. Otherwise, the maximum is 35."_ - [SDI-12 Interface Standard v1.4](https://sdi-12.org/archives_folder/SDI-12_version-1_4-Jan-30-2021.pdf), Page 16

### Additional Documentation

#### Sensor Responses to Metadata Calls (not in manual)

`aIM_001`->`a,ET,mm,Actual Evapotranspiration (ET);`

`aIM_002`->`a,LE,W+1m-2,Latent Energy flux (LE);`

`aIM_003`->`a,H,W+1m-2,Heat Flux (H);`

`aIM_004`->`a,VPD,kPa,Vapor Pressure Deficit (VPD);`

`aIM_005`->`a,PA,kPa,Atmospheric Pressure (P);`

`aIM_006`->`a,TA,C,Air Temperature (T);`

`aIM_007`->`a,RH,percent,Relative Humidity ambient (RH);`

`aIM_008`->`a,SEQ,#,Sequence Number;`

`aIM_009`->`a,DIAG,#,Diagnostic Value;`

...

`aIM1_001`->`a,ET,mm,Actual Evapotranspiration (ET);`

`aIM1_002`->`a,LE,W+1m-2,Latent Energy flux (LE);`

`aIM1_003`->`a,H,W+1m-2,Heat Flux (H);`

`aIM1_004`->`a,PA,kPa,Atmospheric Pressure (P);`

`aIM1_005`->`a,TA,C,Air Temperature (T);`

`aIM1_006`->`a,RH,percent,Relative Humidity ambient (RH);`

`aIM1_007`->`a,SEQ,#,Sequence Number;`

`aIM1_008`->`a,SAMP_CNT,#,Sample Count;`

`aIM1_009`->`a,DIAG,#,Diagnostic Value;`

...

`aIM2_001`->`a,AH,g+1m-3,Absolute Humidity ambient;`

`aIM2_002`->`a,RH,percent,Relative Humidity ambient (RH);`

`aIM2_003`->`a,SVP,kPa,Saturated Vapor Pressure ambient;`

`aIM2_004`->`a,VPD,kPa,Vapor Pressure Deficit (VPD);`

`aIM2_005`->`a,PA,kPa,Atmospheric Pressure (P);`

`aIM2_006`->`a,TA,C,Air Temperature (T);`

`aIM2_007`->`a,TD,C,Dewpoint Temperature;`

`aIM2_008`->`a,TILT,degrees,Inclinometer Tilt;`

...

`aIM3_001`->`a,PUMP_V,V,Pump Voltage;`

`aIM3_002`->`a,PA_CELL,kPa,Cell Pressure;`

`aIM3_003`->`a,RH_CELL,percent,Cell Relative Humidity;`

`aIM3_004`->`a,TA_CELL,C,Cell Temperature;`

`aIM3_005`->`a,RH_ENCL,percent,Enclosure RH;`

`aIM3_006`->`a,FLOW,sccm,Flow;`

`aIM3_007`->`a,INPUT_V,V,Input Voltage;`

`aIM3_008`->`a,DATA_QC,#,Data QC;`