#ifndef __QZSSDCX_H__
#define __QZSSDCX_H__

#include <Arduino.h>
#include <stdint.h>

#define DCX_MSG_UNKOWN 0
#define DCX_MSG_L_ALERT 1
#define DCX_MSG_J_ALERT 2
#define DCX_MSG_MT_INFO 3
#define DCX_MSG_OUTSIDE_JAPAN 4
#define DCX_MSG_NULL 5

struct DCXMessage {
  uint32_t dcx_msg_type : 3;
  uint32_t use_a12_to_a16 : 1;
  uint32_t use_ex1 : 1;
  uint32_t use_ex2_to_ex7 : 1;

  // Satellite Designation (SD)
  uint32_t sdmt : 1;
  uint32_t sdm : 9;

  // Common Alert Message Format (CAMF)
  uint32_t a1_message_type : 2;
  uint32_t a2_country_region_name : 9;
  uint32_t a3_provider_identifier : 5;
  uint32_t a4_hazard_category_type : 7;
  uint32_t a5_severity : 2;
  uint32_t a6_hazard_onset_week_number : 1;
  uint32_t a7_hazard_onset_ToW : 14;
  uint32_t a8_hazard_duration : 2;
  uint32_t a9_selection_of_Library : 1;
  uint32_t a10_version_of_library : 3;
  uint32_t a11_guidance_to_react_library : 10;
  uint32_t a12_ellipse_centre_latitude : 16;
  uint32_t a13_ellipse_centre_longitude : 17;
  uint32_t a14_ellipse_semi_major_axis : 5;
  uint32_t a15_ellipse_semi_minor_axis : 5;
  uint32_t a16_ellipse_azimuth_angle : 6;
  uint32_t a17_main_subject_for_specific_settings : 2;
  uint32_t a18_specific_settings : 15;

  // L-Alert or Municipality-transmitted information
  uint32_t ex1_target_area_code : 16;
  uint32_t ex2_evacuate_direction_type : 1;
  uint32_t ex3_additional_ellipse_centre_latitude : 17;
  uint32_t ex4_additional_ellipse_centre_longitude : 17;
  uint32_t ex5_additional_ellipse_semi_major_axis : 5;
  uint32_t ex6_additional_ellipse_semi_minor_axis : 5;
  uint32_t ex7_additional_ellipse_azimuth : 7;

  // J-Alert
  uint32_t ex8_target_area_code_list_type : 1;
  uint32_t ex9_target_area_code_list[2];
  uint32_t ex10_reserved : 3;

  uint32_t vn_version_number : 6;
};

class DCXDecoder {
public:
  DCXMessage r;

  DCXDecoder(){};
  ~DCXDecoder(){};

  bool decode(const uint8_t* l1smsg);

  static void printAll(Stream& stream, DCXMessage& r);
  static void printSummary(Stream& stream, DCXMessage& r);
  static void binary_print(Stream& stream, uint32_t dat, int length);

  static uint32_t get_bits(const uint8_t* l1smsg, int start_bits, int length);
  static float calc_latitude_deg(uint32_t latitude);
  static float calc_longitude_deg(uint32_t longitude);
  static float calc_radius(uint32_t a);
  static float calc_azimuth_angle(uint32_t a);
  static float calc_additional_latitude_deg(uint32_t latitude);
  static float calc_additional_longitude_deg(uint32_t longitude);
  static float calc_additional_azimuth_angle(uint32_t a);

  static const char* get_message_type_str(int message_type);
  static const char* get_message_type_str_ja(int message_type);
  static const char* get_country_region_name_str(int country_region_name);
  static const char* get_country_region_name_str_ja(int country_region_name);
  static const char* get_provider_name_japan(int provider_id);
  static const char* get_hazard_category_and_type(int hazard_code);
  static const char* get_hazard_category_and_type_ja(int hazard_code);
  static const char* get_severity_str(int severity);
  static const char* get_severity_str_ja(int severity);
  static const char* get_hazard_onset_week_number_str_ja(int week_number);
  static const char* get_hazard_onset_tow_str(int tow);
  static const char* get_hazard_onset_tow_str_ja(int tow);
  static const char* get_hazard_duration_str_ja(int duration);
  static const char* get_municipality_code_japan(int code);
  static const char* get_prefecture_name_ja(uint64_t bit_position);
  static const char* get_guidance_instruction_library_ja(int code);
  static const char* get_evacuate_direction_type_ja(int dir);
};

#endif
