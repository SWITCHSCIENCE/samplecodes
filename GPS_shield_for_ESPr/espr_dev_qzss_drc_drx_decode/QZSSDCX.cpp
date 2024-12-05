#include "QZSSDCX.h"

bool DCXDecoder::decode(const uint8_t* l1smsg) {
  int ofs = 8 + 6;  // PAB + MT

#define GETBIT(PARAM, len) \
  r.PARAM = get_bits(l1smsg, ofs, len); \
  ofs += len;

  // Satellite Designation (SD)
  GETBIT(sdmt, 1);
  GETBIT(sdm, 9);

  // Common Alert Message Format (CAMF)
  GETBIT(a1_message_type, 2);
  GETBIT(a2_country_region_name, 9);
  GETBIT(a3_provider_identifier, 5);
  GETBIT(a4_hazard_category_type, 7);
  GETBIT(a5_severity, 2);
  GETBIT(a6_hazard_onset_week_number, 1);
  GETBIT(a7_hazard_onset_ToW, 14);
  GETBIT(a8_hazard_duration, 2);
  GETBIT(a9_selection_of_Library, 1);
  GETBIT(a10_version_of_library, 3);
  GETBIT(a11_guidance_to_react_library, 10);
  GETBIT(a12_ellipse_centre_latitude, 16);
  GETBIT(a13_ellipse_centre_longitude, 17);
  GETBIT(a14_ellipse_semi_major_axis, 5);
  GETBIT(a15_ellipse_semi_minor_axis, 5);
  GETBIT(a16_ellipse_azimuth_angle, 6);
  GETBIT(a17_main_subject_for_specific_settings, 2);
  GETBIT(a18_specific_settings, 15);

  // DCXメッセージタイプを判別
  r.dcx_msg_type = DCX_MSG_UNKOWN;
  if (r.a1_message_type == 0
      && r.a3_provider_identifier == 0
      && r.a4_hazard_category_type == 0
      && r.a5_severity == 0
      && r.a11_guidance_to_react_library == 0) {
    // ほとんど場合NULLメッセージになるはず
    r.dcx_msg_type = DCX_MSG_NULL;
  } else {
    // 日本
    if (r.a2_country_region_name == 111) {
      if (r.a3_provider_identifier == 1) {
        r.dcx_msg_type = DCX_MSG_L_ALERT;
      } else if (r.a3_provider_identifier == 2 || r.a3_provider_identifier == 3) {
        r.dcx_msg_type = DCX_MSG_J_ALERT;
      } else if (r.a3_provider_identifier == 4) {
        r.dcx_msg_type = DCX_MSG_MT_INFO;
      }
    } else {
      r.dcx_msg_type = DCX_MSG_OUTSIDE_JAPAN;
    }
  }

  if (r.dcx_msg_type == DCX_MSG_J_ALERT) {
    r.use_a12_to_a16 = false;
  } else {
    r.use_a12_to_a16 = r.a12_ellipse_centre_latitude != 0
                       || r.a13_ellipse_centre_longitude != 0
                       || r.a14_ellipse_semi_major_axis != 0
                       || r.a15_ellipse_semi_minor_axis != 0
                       || r.a16_ellipse_azimuth_angle != 0;
  }

  // Extended Message
  ofs = 8 + 6 + 10 + 122;
  if (r.dcx_msg_type == DCX_MSG_L_ALERT || r.dcx_msg_type == DCX_MSG_MT_INFO) {
    GETBIT(ex1_target_area_code, 16);
    GETBIT(ex2_evacuate_direction_type, 1);
    GETBIT(ex3_additional_ellipse_centre_latitude, 17);
    GETBIT(ex4_additional_ellipse_centre_longitude, 17);
    GETBIT(ex5_additional_ellipse_semi_major_axis, 5);
    GETBIT(ex6_additional_ellipse_semi_minor_axis, 5);
    GETBIT(ex7_additional_ellipse_azimuth, 7);

    // EX1の対象エリアコードを使用するか判定
    // r.use_ex1 = r.use_a12_to_a16 == 0;
    r.use_ex1 = r.ex1_target_area_code != 0;

    // EX2～EX7の楕円情報を使用するか判定
    r.use_ex2_to_ex7 = false;
    if (r.dcx_msg_type == DCX_MSG_MT_INFO) {
      if (r.a11_guidance_to_react_library == 0) {
        r.use_ex2_to_ex7 = r.ex3_additional_ellipse_centre_latitude != 0
                           || r.ex4_additional_ellipse_centre_longitude != 0
                           || r.ex4_additional_ellipse_centre_longitude != 0
                           || r.ex5_additional_ellipse_semi_major_axis != 0
                           || r.ex6_additional_ellipse_semi_minor_axis != 0
                           || r.ex7_additional_ellipse_azimuth != 0;
      }
    }
  } else if (r.dcx_msg_type == DCX_MSG_J_ALERT) {
    GETBIT(ex8_target_area_code_list_type, 1);
    GETBIT(ex9_target_area_code_list[0], 32);
    GETBIT(ex9_target_area_code_list[1], 32);
    GETBIT(ex10_reserved, 3);
  } else if (r.dcx_msg_type == DCX_MSG_OUTSIDE_JAPAN) {
    // undefined
  }
  ofs = 8 + 6 + 10 + 122 + 74 - 6;
  GETBIT(vn_version_number, 6);

  return true;
}

// バイト配列からビット値を取得する関数
uint32_t DCXDecoder::get_bits(const uint8_t* buf, int start_bits, int length) {
  if (length <= 0 || length > 32) {
    return 0;  // 範囲外のビット長を拒否
  }

  int index = start_bits >> 3;  // 開始バイトの位置
  int shift = start_bits % 8;   // 開始ビットの位置

  uint32_t result = 0;
  int bits_collected = 0;

  while (bits_collected < length) {
    // 現在のバイトから取得できるビット数
    int bits_in_this_byte = 8 - shift;
    if (bits_in_this_byte > (length - bits_collected)) {
      bits_in_this_byte = length - bits_collected;
    }

    // ビットを抽出して結果に追加
    result <<= bits_in_this_byte;
    result |= (*(buf + index) >> (8 - shift - bits_in_this_byte)) & ((1 << bits_in_this_byte) - 1);

    // 次のバイトへ
    bits_collected += bits_in_this_byte;
    shift = 0;
    index++;
  }

  return result;
}

// A12
float DCXDecoder::calc_latitude_deg(uint32_t latitude) {
  return -90.0f + (180.0f * (float)latitude) / 65535.0f;
}

// A13
float DCXDecoder::calc_longitude_deg(uint32_t longitude) {
  return -180.0f + (360.0f * (float)longitude) / 131071.0f;
}

// A14/A15コードから半径を計算する関数
float DCXDecoder::calc_radius(uint32_t a) {
  float radius = powf(10, 2.33485568962 + ((float)a * (6.39794000867 - 2.33485568962)) / 31.0f);
  return radius / 1000.0f;  // km
}

// A16コードから方位角を計算する関数
float DCXDecoder::calc_azimuth_angle(uint32_t a) {
  // 方位角を計算
  float azimuth_angle = -90 + (180.0f * (float)a) / 64.0f;
  return azimuth_angle;
}

// EX3
float DCXDecoder::calc_additional_latitude_deg(uint32_t latitude) {
  return -90.0f + (180.0f * (float)latitude) / 131071.0f;
}

// EX4
float DCXDecoder::calc_additional_longitude_deg(uint32_t longitude) {
  return 45.0f + (180.0f * (float)longitude) / 131071.0f;
}

// EX7コードから方位角を計算する関数
float DCXDecoder::calc_additional_azimuth_angle(uint32_t a) {
  // 方位角を計算
  float azimuth_angle = -90 + (180.0f * (float)a) / 128.0f;
  return azimuth_angle;
}

void DCXDecoder::printAll(Stream& stream, DCXMessage& r) {
  if (r.dcx_msg_type == DCX_MSG_NULL) {
    stream.println("### DCX Message - Null Message ###");
  } else {
    if (r.dcx_msg_type == DCX_MSG_L_ALERT) {
      stream.println("### DCX Message - L Alert ###");
    } else if (r.dcx_msg_type == DCX_MSG_J_ALERT) {
      stream.println("### DCX Message - J Alert ###");
    } else if (r.dcx_msg_type == DCX_MSG_MT_INFO) {
      stream.println("### DCX Message - Municipality-transmitted Information ###");
    } else if (r.dcx_msg_type == DCX_MSG_OUTSIDE_JAPAN) {
      stream.println("### DCX Message - Information from organizations outside Japan ###");
    }
    stream.print(" A1 - Message type: ");
    stream.print(r.a1_message_type);
    stream.print(" --> ");
    stream.println(get_message_type_str_ja(r.a1_message_type));
    stream.print(" A2 - Country/region name: ");
    stream.print(r.a2_country_region_name);
    stream.print(" --> ");
    stream.println(get_country_region_name_str(r.a2_country_region_name));
    stream.print(" A3 - Provider identifier: ");
    stream.print(r.a3_provider_identifier);
    if (r.a2_country_region_name == 111 /*japan*/) {
      stream.print(" --> ");
      stream.print(get_provider_name_japan(r.a3_provider_identifier));
    }
    stream.println();
    stream.print(" A4 - Hazard category type: ");
    stream.print(r.a4_hazard_category_type);
    stream.print(" --> ");
    stream.println(get_hazard_category_and_type_ja(r.a4_hazard_category_type));
    stream.print(" A5 - Severity: ");
    stream.print(r.a5_severity);
    stream.print(" --> ");
    stream.println(get_severity_str_ja(r.a5_severity));
    stream.print(" A6 - Hazard onset week number: ");
    stream.print(r.a6_hazard_onset_week_number);
    stream.print(" --> ");
    stream.println(get_hazard_onset_week_number_str_ja(r.a6_hazard_onset_week_number));
    stream.print(" A7 - Hazard onset ToW: ");
    stream.print(r.a7_hazard_onset_ToW);
    stream.print(" --> ");
    stream.println(get_hazard_onset_tow_str_ja(r.a7_hazard_onset_ToW));
    stream.print(" A8 - Hazard duration: ");
    stream.print(r.a8_hazard_duration);
    stream.print(" --> ");
    stream.println(get_hazard_duration_str_ja(r.a8_hazard_duration));
    stream.print(" A9 - Selection of library: ");
    stream.println(r.a9_selection_of_Library);
    stream.print("A10 - Version of library: ");
    stream.println(r.a10_version_of_library);
    stream.print("A11 - Guidance to react library: ");
    stream.print(r.a11_guidance_to_react_library);
    if (r.dcx_msg_type == DCX_MSG_L_ALERT
        || r.dcx_msg_type == DCX_MSG_MT_INFO
        || r.dcx_msg_type == DCX_MSG_J_ALERT) {
      stream.print(" --> ");
      stream.print(get_guidance_instruction_library_ja(r.a11_guidance_to_react_library));
    }
    stream.println();
    stream.print("A12 - Ellipse centre latitude: ");
    stream.print(r.a12_ellipse_centre_latitude);
    stream.print(" --> ");
    stream.println(calc_latitude_deg(r.a12_ellipse_centre_latitude));
    stream.print("A13 - Ellipse centre longitude: ");
    stream.print(r.a13_ellipse_centre_longitude);
    stream.print(" --> ");
    stream.println(calc_longitude_deg(r.a13_ellipse_centre_longitude));
    stream.print("A14 - Ellipse semi major axis: ");
    stream.print(r.a14_ellipse_semi_major_axis);
    stream.print(" --> ");
    stream.println(calc_radius(r.a14_ellipse_semi_major_axis));
    stream.print("A15 - Ellipse semi minor axis: ");
    stream.print(r.a15_ellipse_semi_minor_axis);
    stream.print(" --> ");
    stream.println(calc_radius(r.a15_ellipse_semi_minor_axis));
    stream.print("A16 - Ellipse azimuth angle: ");
    stream.print(r.a16_ellipse_azimuth_angle);
    stream.print(" --> ");
    stream.println(calc_azimuth_angle(r.a16_ellipse_azimuth_angle));
    stream.print("A17 - Main subject for specific settings: ");
    stream.println(r.a17_main_subject_for_specific_settings);
    stream.print("A18 - Specific settings: ");
    stream.println(r.a18_specific_settings);
    if (r.dcx_msg_type == DCX_MSG_L_ALERT || r.dcx_msg_type == DCX_MSG_MT_INFO) {
      stream.print("Ex1 - Target area code: ");
      stream.print(r.ex1_target_area_code);
      stream.print(" --> ");
      stream.println(get_municipality_code_japan(r.ex1_target_area_code));
      stream.print("EX2 - Evacuate Direction Type: ");
      stream.print(r.ex2_evacuate_direction_type);
      stream.print(" --> ");
      stream.println(get_evacuate_direction_type_ja(r.ex2_evacuate_direction_type));
      stream.print("EX3 - Additional Ellipse Centre Latitude: ");
      stream.print(r.ex3_additional_ellipse_centre_latitude);
      stream.print(" --> ");
      stream.println(calc_additional_latitude_deg(r.ex3_additional_ellipse_centre_latitude));
      stream.print("EX4 - Additional Ellipse Centre Longitude: ");
      stream.print(r.ex4_additional_ellipse_centre_longitude);
      stream.print(" --> ");
      stream.println(calc_additional_longitude_deg(r.ex4_additional_ellipse_centre_longitude));
      stream.print("EX5 - Additional Ellipse Semi-Major Axis: ");
      stream.print(r.ex5_additional_ellipse_semi_major_axis);
      stream.print(" --> ");
      stream.println(calc_radius(r.ex5_additional_ellipse_semi_major_axis));
      stream.print("EX6 - Additional Ellipse Semi-Minor Axis: ");
      stream.print(r.ex6_additional_ellipse_semi_minor_axis);
      stream.print(" --> ");
      stream.println(calc_radius(r.ex6_additional_ellipse_semi_minor_axis));
      stream.print("EX7 - Additional Ellipse Azimuth: ");
      stream.print(r.ex7_additional_ellipse_azimuth);
      stream.print(" --> ");
      stream.println(calc_additional_azimuth_angle(r.ex7_additional_ellipse_azimuth));
    } else if (r.dcx_msg_type == DCX_MSG_J_ALERT) {
      stream.print("Ex8 - Target area code list type: ");
      stream.println(r.ex8_target_area_code_list_type);
      stream.print("Ex9 - Target area code list: ");
      if (r.ex8_target_area_code_list_type == 0) {  // Prefecture code
        uint32_t code1 = r.ex9_target_area_code_list[0];
        uint32_t code2 = r.ex9_target_area_code_list[1];
        bool sepa = false;
        binary_print(stream, code1, 32);
        binary_print(stream, code2, 32);
        stream.print(" --> ");
        for (int i = 0; i < 32; i++) {
          if (code1 & 0x80000000) {
            if (sepa) {
              stream.print(" ");
            }
            sepa = true;
            stream.print(get_prefecture_name_ja(i));
          }
          code1 <<= 1;
        }
        for (int i = 0; i < 32; i++) {
          if (code2 & 0x80000000) {
            if (sepa) {
              stream.print(" ");
            }
            sepa = true;
            stream.print(get_prefecture_name_ja(i + 32));
          }
          code2 <<= 1;
        }
      } else {  // Municipality code
        uint32_t mcodes[4] = {
          r.ex9_target_area_code_list[0] >> 16,
          r.ex9_target_area_code_list[0] & 0xffff,
          r.ex9_target_area_code_list[1] >> 16,
          r.ex9_target_area_code_list[1] & 0xffff,
        };
        bool sepa = false;
        for (int i = 0; i < 4; i++) {
          if (mcodes[i]) {
            if (sepa) {
              stream.print(" ");
            }
            sepa = true;
            stream.print(get_municipality_code_japan(mcodes[i]));
          }
        }
      }
      stream.println();
    }
    stream.print(" Vn - Version number: ");
    stream.println(r.vn_version_number);
  }
}

void DCXDecoder::printSummary(Stream& stream, DCXMessage& r) {
  if (r.dcx_msg_type == DCX_MSG_NULL) {
    stream.println("### 空メッセージ ###");
  } else {
    if (r.dcx_msg_type == DCX_MSG_L_ALERT) {
      stream.print("### Lアラート(");
    } else if (r.dcx_msg_type == DCX_MSG_J_ALERT) {
      stream.print("### Jアラート(");
    } else if (r.dcx_msg_type == DCX_MSG_MT_INFO) {
      stream.print("### 自治体送信情報(");
    } else if (r.dcx_msg_type == DCX_MSG_OUTSIDE_JAPAN) {
      stream.print("### 海外組織からの情報(");
    }
    stream.print(get_message_type_str_ja(r.a1_message_type));
    stream.println(") ###");
    stream.print("国/地域名: ");
    stream.println(get_country_region_name_str_ja(r.a2_country_region_name));
    stream.print("情報提供: ");
    if (r.a2_country_region_name == 111 /*japan*/) {
      stream.println(get_provider_name_japan(r.a3_provider_identifier));
    } else {
      stream.println("海外の防災組織");
    }
    stream.print("災害カテゴリ種別: ");
    stream.println(get_hazard_category_and_type_ja(r.a4_hazard_category_type));
    stream.print("重大度: ");
    stream.println(get_severity_str_ja(r.a5_severity));
    stream.print("災害発生週: ");
    stream.println(get_hazard_onset_week_number_str_ja(r.a6_hazard_onset_week_number));
    stream.print("災害発生日時: ");
    stream.println(get_hazard_onset_tow_str_ja(r.a7_hazard_onset_ToW));
    stream.print("災害の継続時間: ");
    stream.println(get_hazard_duration_str_ja(r.a8_hazard_duration));
    stream.print("避難行動: ");
    if (r.dcx_msg_type == DCX_MSG_L_ALERT
        || r.dcx_msg_type == DCX_MSG_MT_INFO
        || r.dcx_msg_type == DCX_MSG_J_ALERT) {
      stream.println(get_guidance_instruction_library_ja(r.a11_guidance_to_react_library));
    } else {
      stream.println(r.a11_guidance_to_react_library);
    }
    if (r.use_a12_to_a16) {
      stream.println("楕円情報");
      stream.print("  中心緯度: ");
      stream.println(calc_latitude_deg(r.a12_ellipse_centre_latitude), 3);
      stream.print("  中心経度: ");
      stream.println(calc_longitude_deg(r.a13_ellipse_centre_longitude), 3);
      stream.print("  長軸半径: ");
      stream.print(calc_radius(r.a14_ellipse_semi_major_axis), 3);
      stream.println(" km");
      stream.print("  短軸半径: ");
      stream.print(calc_radius(r.a15_ellipse_semi_minor_axis), 3);
      stream.println(" km");
      stream.print("  方位角: ");
      stream.println(calc_azimuth_angle(r.a16_ellipse_azimuth_angle), 3);
    }
    if (r.use_ex1) {
      stream.print("対象地域: ");
      stream.println(get_municipality_code_japan(r.ex1_target_area_code));
    }
    if (r.use_ex2_to_ex7) {
      stream.print("避難行動: ");
      stream.println(get_evacuate_direction_type_ja(r.ex2_evacuate_direction_type));
      stream.println("追加対象地域");
      stream.print("  中心緯度: ");
      stream.println(calc_additional_latitude_deg(r.ex3_additional_ellipse_centre_latitude));
      stream.print("  中心経度: ");
      stream.println(calc_additional_longitude_deg(r.ex4_additional_ellipse_centre_longitude));
      stream.print("  長軸半径: ");
      stream.print(calc_radius(r.ex5_additional_ellipse_semi_major_axis));
      stream.println(" km");
      stream.print("  短軸半径: ");
      stream.print(calc_radius(r.ex6_additional_ellipse_semi_minor_axis));
      stream.println(" km");
      stream.print("  方位角: ");
      stream.println(calc_additional_azimuth_angle(r.ex7_additional_ellipse_azimuth));
    }
    if (r.dcx_msg_type == DCX_MSG_J_ALERT) {
      stream.print("対象地域: ");
      if (r.ex8_target_area_code_list_type == 0) {  // Prefecture code
        uint32_t code1 = r.ex9_target_area_code_list[0];
        uint32_t code2 = r.ex9_target_area_code_list[1];
        bool sepa = false;
        for (int i = 0; i < 32; i++) {
          if (code1 & 0x80000000) {
            if (sepa) {
              stream.print(" ");
            }
            sepa = true;
            stream.print(get_prefecture_name_ja(i));
          }
          code1 <<= 1;
        }
        for (int i = 0; i < 32; i++) {
          if (code2 & 0x80000000) {
            if (sepa) {
              stream.print(" ");
            }
            sepa = true;
            stream.print(get_prefecture_name_ja(i + 32));
          }
          code2 <<= 1;
        }
      } else {  // Municipality code
        uint32_t mcodes[4] = {
          r.ex9_target_area_code_list[0] >> 16,
          r.ex9_target_area_code_list[0] & 0xffff,
          r.ex9_target_area_code_list[1] >> 16,
          r.ex9_target_area_code_list[1] & 0xffff,
        };
        bool sepa = false;
        for (int i = 0; i < 4; i++) {
          if (mcodes[i]) {
            if (sepa) {
              stream.print(" ");
            }
            sepa = true;
            stream.print(get_municipality_code_japan(mcodes[i]));
          }
        }
      }
      stream.println();
    }
  }
}

void DCXDecoder::binary_print(Stream& stream, uint32_t dat, int length) {
  dat <<= 32 - length;
  for (int i = 32 - length; i < 32; i++) {
    if (dat & 0x80000000) {
      stream.print("1");
    } else {
      stream.print("0");
    }
    dat <<= 1;
  }
}
