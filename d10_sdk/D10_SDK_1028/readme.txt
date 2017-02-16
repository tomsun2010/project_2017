s2l_2.6.0_20161028.tar.bz2
====================================================
Patch to SDK2.6.0 (s2l_linux_sdk_20150928_Release.tar.xz)

Usage:
    1. Extract Linux kernel in s2l_linux_sdk.
        $ cd s2l_linux_sdk/ambarella
        $ source build/env/Linaro-multilib-gcc4.9.env
        $ cd boards/hawthorn
        $ make sync_build_mkcfg
        $ make s2l_ipcam_config
        
    2. Extract ss2l_2.6.0_20161028.tar.bz2 under s2l_linux_sdk/ (in the same folder as ambarella/)
        $ cd s2l_linux_sdk/
        $ tar xjf s2l_2.6.0_20161028.tar.bz2
        
    3. Apply the patch for source files.
        $ cd s2l_2.6.0_20161028
        $ chmod +x apply.sh
        $ ./apply.sh
        
    4. Make
        $ cd s2l_linux_sdk/ambarella/boards/hawthorn
        $ source ../../build/env/armv7ahf-linaro-gcc.env
        $ make sync_build_mkcfg
        $ make s2l_ipcam_config
        $ make defconfig_public_linux
        $ make -j8

 [Bug fixes]
  1. (20151015) Fix issue of source buffer height can't change (Oryx).
  2. (20151020) Fix issue of Chinese font display problem (test_textinsert).
  3. (20151020) Fix issue of MCTF PM height check fail (IAV).
  4. (20151020) Fix issue of incorrect encode aspect ratio when both LDC and HDR are enabled (IAV).
  5. (20151020) Fix issue of ptb error for tftp upgrade (BSP).
  6. (20151020) Fix issue of incorrect AE line belt for ae line get operation (MW).
  7. (20151020) Fix issue of DSP hang when setting main buffer to 720p for mode 2 (Ucode).
  8. (20151022) Fix issue of DSP hang when raw capture is enabled for mode 2 (Ucode).
  9. (20151023) Fix issue of image artifacts caused by BPC pm for HDR 3x in mode 5 (Ucode).
 10. (20151023) Fix issue of DSP hang by QP ROI frame sync ccomand flooding (IAV / Ucode).
 11. (20151023) Fix issue of DSP hang when rotate small resolution stream (Ucode).
 12. (20151027) Fix issue of customer AE metering table load error (Oryx).
 13. (20151105) Fix issue of insufficient warp padding (Ucode).
 14. (20151111) Fix issue of DSP hang when preview B buffer is off in mode 2 (Ucode).
 15. (20151112) Fix issue of incorrect VOUT display during IDLE state when mixer is off (Ucode).
 16. (20151120) Fix issue of DSP hang when setting second buffer to be 0x0 (IAV).
 17. (20151120) Fix issue of getting AF statistic value invalid (MW).
 18. (20151121) Fix issue of DVE reset logic for VOUT1 (Ucode).
 19. (20151123) Fix issue of no existing memory in fastosd (amboot/boards).
 20. (20151125) Fix issue of the offset logic bug with -q option (upgrade_partition).
 21. (20151126) Fix issue of ME0 capture in mode 2&5 (Ucode).
 22. (20151130) Fix issue in RTP muxer, remove additional packet.release() operation, which causes data over-written (Oryx).
 23. (20151201) Fix issue of error message in image mw during mode switch if slow shutter is enabled (MW).
 24. (20151204) Fix issue of DSP hang when ME0 is enabled and all source buffers are
                configured for encode (Ucode).
 25. (20151210) Fix issue of write error in ubifs when ubifs remount with lowest and 
                highest conversion of clock (Linux).
 26. (20151210) Fix issue of color setting error of PM (Ucode). 
 27. (20151216) Fix issue of dsp hang for too small main buffer size (VGA & 480p) in mode 2 (Ucode).
 28. (20151217) Fix issue of reboot fail for emmc (Linux).
 29. (20151218) Fix issue of white lines on top of 720p streams, when stream fps is 21~29 (Ucode).
 30. (20151218) Fix issue of DSP hang when configuring preview C buffer (Ucode).
 31. (20151225) Fix issue of overlay over-written by MV statistics data (IAV/Ucode).
 32. (20151225) Fix issue of first file A/V sync and restart video causing recreate A/V queue (Oryx).
 33. (20160105) Fix issue of event recording failed to start if event recording is stopped during file writing (Oryx).
 34. (20160111) Fix issue of overlay abnormal when both MV dump and rotate are enabled (Ucode).
 35. (20160112) Fix issue of "insert-always" abnormal for osd when rotate is enabled (Ucode).
 36. (20160115) Fix issue of dsp hang when using upsample in mode 2 (Ucode).
 37. (20160218) Fix issue of hardware timer overflow (IAV).
 38. (20160229) Fix issue of VIN statistics problems in Mode 5 and 3x HDR as wrong
                left offset of second pass (IAV / Ucode).
 39. (20160311) Fix issue of too many frames skip with big motion when LBR change (Ucode).
 40. (20160329) Fix issue of memcpy error without '\0' in adj file name (MW).
 41. (20160331) Fix issue of bitrate limitation checking (IAV).
 42. (20160412) Fix issue of ROI info parse error in am_event_service_msg_action.cpp (Oryx).
 43. (20160412) Fix issue of deadlock of method_call & notify concurrent.
 44. (20160414) Fix issue of aliso performance downgrade when chroma radius is 32 and wide chroma radius is 128 (Ucode).
 45. (20160418) Fix issue of MV dump fail in multi-streaming as MV size confused (IAV).

 [System]
  1. (20151020) Add VIN overflow protection (IAV / Ucode).
  2. (20151020) Add VIN performance check (IAV).
  3. (20151020) Add decode only option for dram layout (IAV).
  4. (20151020) Refine vsync loss mechanism to shorten recover time after long unplug (IAV).
  5. (20151104) Add Input window check for LDC (IAV).
  6. (20151104) Change the Flash IO control RS/CO bit set (Linux).
  7. (20151106) Update ubi-tools to version 1.5.2 (3rd party).
  8. (20151109) Add default config for high clock (Board).
  9. (20151109) Remove bapi support (Amboot).
 10. (20151109) Add support for ME0 capture in mode 0 && 5 (IAV).
 11. (20151110) Use active window instead of max active window for BPC type PM in iav (IAV).
 12. (20151119) Add support for fast seek P frame, B frame and two reference for new GOP structure (IAV).
 13. (20151123) Add always insert flag to insert OSD including skipped frame (IAV/Ucode).
 14. (20151126) Add support for per-MB ZMV feature (IAV/Ucode).
 15. (20151204) Add support for MV statistics data dump (IAV/Ucode). 
 16. (20151204) Use snapshot for all streams by default (IAV).
 17. (20151208) Add support for EMMC boot on S2Lm (Amboot/Linux).
 18. (20151214) Add new "enc_done_ts" field in bits info to present the time of frame encode done (IAV).
 19. (20151214) Add UVC camera driver (IAV/Linux).
 20. (20151215) Add reserved fields for customized sensor resolution, sensor id and IAV IOCTL commands (IAV).
 21. (20151216) Change real time update command order, send it with encode start command in same block
                to make it take effect from the first frame (IAV).
 22. (20151217) Refine IDSP upsampling frame rate to support 8fps and 9fps (IAV/Ucode).
 23. (20151225) Refine privacy mask to support user-defined call (IAV).
 24. (20160111) Add frame crop for h264 encode (IAV).
 25. (20160112) Add the definition of the 32-bit word of md sw cat (Ucode).
 26. (20160115) Refine spi_register_master to be called after ambarella spi driver initialization (Linux).
 27. (20160117) Refine VIN reset for mode switch cases (IAV).
 28. (20160122) Refine GPIO reset sequence to make sensor fully reset (IAV).
 29. (20160125) Add new IOCTL for DSP clock disable/enable (IAV).
 30. (20160126) Refine independent IPB ROI setting for stream rotation mode (IAV).
 31. (20160126) Disable IDSP clock automatically when Cortex clock lower than 96 MHz (Linux).
 32. (20160126) Refine ME1 buffer logic to support two references and frame sync encode tools together (Ucode).
 33. (20160204) Refine DSP memory logic to save more DRAM (Ucode).
 34. (20160204) Refine fastboot to support OV4689 in mode 4 (Board).
 35. (20160218) Add Support for hw timer suspend/resume (IAV).
 36. (20160219) Use single exposure height in sensor config command (IAV).
 37. (20160226) Refine Pulseaudio configuration for lineout mode (Board).
 38. (20160229) Add support for VCA buffer dump both in normal boot and fast boot (IAV / Ucode).
 39. (20160229) Add support for inserting long reference P frame on the fly (IAV / Ucode).
 40. (20160301) Refine log API to avoid the conflict with syslog definition (Packages / Utils).
 41. (20160302) Add support for deblocking filter in encode configuration (IAV / Ucode).
 42. (20160303) Remove the logic of disabling deblocking when stream is 720p or 1080p
                and bit rate is greater than 15Mbps (Ucode).
 43. (20160304) Upgrade toolchain to Version 2015.11-2 to fix glibc bug "CVE-2015-7547" (Build).
 44. (20160308) Remove Chip Selection command from preview command sequence (IAV).
 45. (20160310) Add support for custom bitstream restriction config (IAV / Ucode).
 46. (20160310) Add support for ucode architecture identification (IAV / Ucode).
 47. (20160315) Refine DSP_INIT_DATA address to pre-allocate address which is saved in 
                orccode.bin from fixed address both in normal and fast boot (IAV / Ucode / Amboot).
 48. (20160315) Refine S2L33m VIN PPS limitation from 4MP30 to 6MP30 (IAV).
 49. (20160316) Refine DRAM refresh to support fast resume and OV9750 (IAV).
 50. (20160323) Add support for SPI-NAND on Kiwi board (Amboot / Linux).
 51. (20160323) Add support for SPI_CS_HIGH to set CS active high (Linux).
 52. (20160329) Add support for absolute bitrate mode (IAV / Ucode).
 53. (20160329) Add support for force_blend_tile config to have same stitching tile width in all blending passes (IAV / Ucode).
 54. (20160331) Refine analog driver to improve CVBS 576i(PAL) resolution (Vout).
 55. (20160405) Refine hw_timer to support real-time hw timer write operation (hw_timer).
 56. (20160408) Refine instant mv get for continuous mv dump (IAV).
 57. (20160412) Add support for flash h27u2g8f2c (Amboot).
 58. (20160418) Refine analog driver to improve CVBS 480i (NTSC) resolution (Vout).
 59. (20160421) Refine encode start / stop operations as using enc_mutex to mutex them (IAV).
  
 [Oryx]
  1. (20151008) Add multi-stream event recording support.
  2. (20151008) Update audio related make.inc due to pulse-audio upgrade.
  3. (20151009) Add MJPEG muxer.
  4. (20151013) Update RTSP server, when RTP muxer is disconnected actively with RTSP server,
                destroy all the client. Optimize RTP sessions, avoid race condition when
                clients request session SDP strings too fast.
  5. (20151016) Add MP4 combiner.
  6. (20151016) Update MP4 and TS muxer to support file recording without audio.
  7. (20151022) Rename encode improve to flat area improve in Oryx.
  8. (20151026) Refine makefile to avoid building error when run make in single thread.
  9. (20151027) Update source codes license declaration.
 10. (20151028) Add secure boot in FW upgrade module.
 11. (20151109) Video module: export capture time stamp for user.
 12. (20151110) Video module: enable change source buffer without relaunch apps_launcher.
 13. (20151118) Video module: change name long_term_intvl to fast_seek_intvl in config file and am_video_config.cpp; 
                Image quality module: refine image_quality module to prevent error msg when img_svc quit without DSP entering preview.
 14. (20151118) Video module: enable extra-buf to prevent OSD missing when do YUV capture.
                Utility module: use GDMA copy instead of memcpy to shorten YUV data copy time when do software encoding with YUV data.
 15. (20151126) Video module: add a stop vin air api for video service.
 16. (20151126) Stream module: update mp4 muxer for invalid pts and optimize auto parse file location.
 17. (20151203) Stream module: update mp4 muxer to support gps info.
 18. (20151207) Stream module: add muxer id for muxers to support starting and stopping specified muxers.
 19. (20151208) Stream module: when file system IO error, mp4 muxer do not exit. When file system is ready, file recording can continue.
 20. (20151214) Video module: add overlay update api for dynamic update string or picture.
 21. (20151216) Stream module: add mp4 fixer lib which can be used to fix damaged mp4 file without header.
 22. (20151226) Start video service after media service to make sure audio be sent to muxer before video.
 23. (20151228) Stream module: Make sure MP4 file creation time is the local time.
 24. (20160104) Stream module: 1. split mp4 muxer to muxer-mp4.so and muxer-mp4-event.so 2. Change filter start order of record engine.
 25. (20160114) Image Quality module: check IAV state and update sys_info before set IQ config.
 26. (20160308) Video module: reconstruct video module to be compatible with H264/H265.
 27. (20160418) Add playback service.
  
 [Sensor]
  1. (20151020) Refine HDR window offset for OV4689.
  2. (20151020) Refine sensor PLL to correct mono PTS for OV9718 / MN34220PL / OV4689.
  3. (20151112) Update B-ISO parameters for MN34220PL sensor.
  4. (20160118) Refine ov4689 default flip/mirror mode.
  5. (20160204) Raise IMX123 PLL clock to enlarge shutter ratio between exposures to support 3MP 2X HDR at 30fps.
  6. (20160316) Add support for OV9750 3A parameters.
  7. (20160323) Add hdr 4x ratio case and increase long shutter limitation to 1/60s for AR0230.
  8. (20160323) Refine some registers and APIs as official notification for AR0237.
  9. (20160426) Add support for OV2718 in linear and mode 4.
   
 [Library]
  1. (20151020) Add rotate correction into EIS library (libeis.so).
  2. (20151105) Add separate zoom factors for horizontal and vertical direction (libdewarp.so).
  3. (20151105) Add "rotate + pitch" correction into EIS library (libeis.so).
  4. (20151107) Update AE speed strategy (libimg_algo_s2l.a).
  5. (20151107) Add auto WDR parameter in aeb file (libimg_algo_s2l.a).
  6. (20151109) Use more accurate calculation for VIN/Main domain transform in main preproc lib (libmainpp.so).
  7. (20151109) Add day / night mode restore in IMG MW (libamp.so).
  8. (20151112) Re-order sharpen API calling in run-time update (libimg_algo_s2l.a).
  9. (20151231) Fix bug, mo_SA set wrong parameters.(libimg_algo_s2l.a).
 10. (20160106) Fix issue of shortest exposure may get incorrect histogram in 3X HDR case (libimg_algo_s2l.a).
 11. (20160111) Add AR0237 3A support in image middle-ware library (libamp.so).
 12. (20160115) Fix issue of dsp hang when mode switch between linear and HDR (libimg_algo_s2l.a).
 13. (20160229) Add support for moving average method for "pitch + rotate / rotate" correction (libeis.so).
 14. (20160308) Refine codes for tool-chain armv7ahf-linaro-gcc.env compilation (libeis.so).
 15. (20160309) Fix the AWB performance issue when manual WB mode switch to auto WB mode (libimg_algo_s2l.a).
 16. (20160315) Optimize 3A support in image middle-ware library (libamp.so):
                  a. Add mw_hdr_blend_info structure and get/set;
                  b. Refine HDR_2X_MAX_SHUTTER/HDR_3X_MAX_SHUTTER to calculate hdr_shutter_max with expo_ratio;
                  c. Add wait 1 irq between prepare_aaa and start_aaa when enable 3A after preview;
                  d. Add ae_lines refresh in generate_normal_ae_lines;
 17. (20160329) Add support for P-iris function in HDR mode (libimg_algo_s2l.a).
 18. (20160330) Refine memory management for moving average current value (libeis.so).
 19. (20160406) Refine img_mw as piris support and ae lines for hdr mode (libamp.so).
 20. (20160406) Refine value return in mw_start_aaa and mw_stop_aaa (libamp.so).
 21. (20160418) 1.Support multi process to get stats, not necessary to init lib in secondary process (libimg_dsp_s2l_hf.a).
                2.Enable SBPC for mode 4 (libimg_dsp_s2l_hf.a).
 22. (20160420) 1. Add AE/AWB pre-config in HDR case (libimg_algo_s2l.a);
                2. Add AE statistics data reversion in HDR case (libimg_algo_s2l.a).
 23. (20160421) Add support for smartrc, only support one stream. (libsmartrc.so).
 24. (20160425) Add 3A api for awb manual roi setting (libimg_algo_s2l.a).
  
 [Unit Test]
  1. (20151020) Decrease bias max value from 128 to 9 (test_frame_sync).
  2. (20151020) Remove set AE lines option in test_image (test_image).
  3. (20151020) Add item of rotate into EIS feature (test_eis_warp).
  4. (20151103) Remove workaround of BPC PM offset for HDR case in test_privacymask (test_privacymask).
  5. (20151105) Add options to support separate zoom factors for horizontal and vertical directions (test_ldc).
  6. (20151105) Add item of "rotate + pitch" into EIS feature (test_eis_warp).
  7. (20151126) Add support for per-MB ZMV feature (test_encode/test_frame_sync/test_qproi).
  8. (20151204) Add support for MV statistics data dump (test_statistics).
  9. (20151225) Refine MV statistics data dump (test_statistics).
 10. (20160111) Add frame crop setting (test_encode).
 11. (20160112) Refine LCD initial settings (lcd_digital.c).
 12. (20160125) Add DSP clock enable / disable option (test_encode).
 13. (20160126) Refine ROI setting (test_frame_sync / test_qproi).
 14. (20160229) Add support for VCA buffer dump in normal boot and fast boot (test_encode).
 15. (20160229) Add support for inserting long reference P frame on the fly (test_frame_sync / test_encode).
 16. (20160229) Refine interaction of test_eis_warp (test_eis_warp).
 17. (20160302) Add support for deblocking filter setting (test_encode).
 18. (20160308) Refine codes of test_eis_warp for toolchain armv7ahf-linaro-gcc.env compilation (test_eis_warp).
 19. (20160311) Fix issue of EFM failed with 480p resolution (test_efm).
 20. (20160315) Add "mw_hdr_blend_info" structure and get/set API examples (test_image).
 21. (20160329) Add support for absolute bitrate enable (test_encode).
 22. (20160329) Add support for ucode arch, version and modified time get from dsp (test_encode).
 23. (20160408) Add support for continuous MVdump (test_statistics).
 24. (20160418) Add support for split-fast-seek and refine split-svct option (test_stream).
 25. (20160422) Add support for smartrc (test_smartrc).
  
 [Document]
  1. (20160229) Add fast boot and VCA dump with OV4689&&mode 4 in fast boot (fast boot).
  2. (20160229) Add VCA dump and long reference P insert in Unit Test (Unit Test).
  3. (20160310) Update toolchain section in document.
